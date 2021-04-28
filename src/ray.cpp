#include <stdio.h>
#include <float.h>
#include <time.h>

#include "bitmap.cpp"
#include "world.cpp"
#include "camera.cpp"
#include "ray.h"
#include "Threads.h"

#define RAYS_PER_PIXEL 512
#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 720

#if 0
#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 24

char screenBuffer[CONSOLE_HEIGHT * CONSOLE_WIDTH + 1];
char greyScale[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";


int main(int argc, char** argv) {
  //terminal ascii code for clearing the screen
    puts("\x1b[2J");
    while (1)
      {
      for (int i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++)
	{
	  //for gradient
	  double leftPercentage = (double)(i % (CONSOLE_WIDTH - 1)) / CONSOLE_WIDTH;
	  double downPercentage = (double)(i / (CONSOLE_WIDTH - 1)) / CONSOLE_HEIGHT;
	  int darkValue = ((leftPercentage + downPercentage) / 2) * 62;
	 
	  //Put line breaks on the edges
	  if (i % CONSOLE_WIDTH == CONSOLE_WIDTH - 1)
	    screenBuffer[i] = '\n';
	  else
	    screenBuffer[i] = greyScale[darkValue];
	}
      //Puts the cursor back at the top left
      puts("\x1b[1;1H");
      puts(screenBuffer);


      //Later sleep for diff amount, currently: 1 second
      sleep(1);
      }
}
#endif

void renderTile(World* world, Image image,
		u32 minX, u32 onePastMaxX,
		u32 minY, u32 onePastMaxY,
		u32 sampleCount, Camera* camera)
{  
  for (u32 y = minY; y < onePastMaxY; y++)
    {
      u32* out = image.pixels + y*image.width + minX;
      lane_f32 filmY = laneF32FromF32(-1.0f + (2.0f * (f32)y / (f32)image.height));
      for (u32 x = minX; x < onePastMaxX; x++)
	{
	  lane_f32 filmX = laneF32FromF32(-1.0f + (2.0f * (f32)x / (f32)image.width));
	
	  vec3 colour = rayTrace(world, camera, filmY, filmX, sampleCount);	  
	  colour = linearToSRGB(colour);
	  
	  u32 bitmapColour = packRGBAtoARGB({ colour.x, colour.y, colour.z, 1.0f });
	  //ARGB
	  *out = bitmapColour;
	  out++;
	}      
    }
  LockedAdd(&world->tilesCompleted, 1);
}

//for multithreading, maybe make a different function later
void* threadProc(void* args)
{
  WorkQueue* queue = (WorkQueue*)args;
  u32 workIndex = LockedAdd(&queue->workIndex, 1);
  while (workIndex < queue->totalOrders)
    {
      WorkOrder order = queue->queue[workIndex];
      renderTile(order.world, order.image,
	     order.minX, order.onePastMaxX,
		 order.minY, order.onePastMaxY, queue->raysPerPixel, queue->camera);
      
      workIndex = LockedAdd(&queue->workIndex, 1);
      printf("\rRaytracing...%llu%%", 100 * order.world->tilesCompleted / order.world->totalTileCount);
      fflush(stdout);
    }
  return NULL;
}


vec3 rayTrace(World* world, Camera* camera, lane_f32 filmY, lane_f32 filmX,  u32 sampleCount)
{
  //initialize tolerances and colours
  vec3 finalColour = {};
  lane_f32 tolerance;
  tolerance = 0.00001f;
  lane_f32 minHitDistance;
  minHitDistance = 0.00001f;
  

  //Randomness for the random bounce
  u32 seed = (world->bounceCount + 9129238) * (world->tilesCompleted+123109981);
  lane_u32 entropy;
  entropy = randomLaneU32(&seed);

  lane_u32 laneIncrement = laneU32FromU32(1);  


  u32 laneWidth = LANE_WIDTH;
  u32 rayCount = sampleCount / laneWidth;

  lane_v3 cameraX = laneV3FromV3(camera->X);
  lane_v3 cameraY = laneV3FromV3(camera->Y);

  
  lane_u32 bouncesComputed = laneU32FromU32(0);
  for (u32 rayIndex = 0; rayIndex < rayCount; rayIndex++)
    {

      //setup for this ray
      lane_f32 pixelXOffset = randomUnilateral32(&entropy)*camera->halfPixelWidth * 2.0f;
      lane_f32 pixelYOffset = randomUnilateral32(&entropy)*camera->halfPixelHeight * 2.0f;

      lane_v3 filmPos = camera->filmCenter +
	cameraY * filmY * (camera->halfFilmHeight + pixelYOffset) + 
	cameraX * filmX * (camera->halfFilmWidth + pixelXOffset);
      
      lane_v3 lensXOffset = randomBilateral32(&entropy) * camera->lensRadius * cameraX;
      lane_v3 lensYOffset = randomBilateral32(&entropy) * camera->lensRadius * cameraY;
            
      lane_v3 rayOrigin;
      rayOrigin = camera->pos + lensYOffset + lensXOffset;
      lane_v3 rayDirection;
      rayDirection = normalize(filmPos - rayOrigin);

      lane_u32 laneMask;
      laneMask = 0xFFFFFFFF;
      lane_v3 bounceNormal = {};
      lane_v3 resultColour = {};
      lane_v3 attenuation;
      attenuation = vec3{1.0f,1.0f,1.0f};//how much the colour changes from the bounced material
      //Each time, a ray can bounce this many times
      for (u32 bounceCount = 0; bounceCount < 8; bounceCount++)
	{	  	  
	  lane_f32 minDist;
	  minDist = FLT_MAX;
	  lane_u32 matIndex;
	  matIndex = 0;
      
	  bouncesComputed += laneIncrement & laneMask;

	  //iterate over all planes to see if they intersect
	  for (u32 i = 0; i < world->planeCount; i++)
	    {
	      Plane plane = world->planes[i];
	      lane_v3 planeNormal;
	      planeNormal = plane.normal;
	  
	      lane_f32 denom = dot(planeNormal, rayDirection);
	      lane_u32 toleranceMask = (denom > tolerance) | (denom < -tolerance);

	      //currently slightly faster without this condition
	      //if (!MaskAllZeros(toleranceMask))
		{
		  lane_f32 planeDist;
		  planeDist = plane.dist;

		  lane_f32 dist = (-planeDist - dot(planeNormal, rayOrigin)) / denom;
		  lane_u32 distMask = (dist > minHitDistance) & (dist < minDist);
		  lane_u32 hitMask = toleranceMask & distMask;
		  if (!MaskAllZeros(hitMask))
		    {
		      lane_u32 planeMatIndex;
		      planeMatIndex = plane.matIndex;
		      ConditionalAssign(&minDist,      hitMask, dist);
		      ConditionalAssign(&bounceNormal, hitMask, planeNormal);
		      ConditionalAssign(&matIndex,     hitMask, planeMatIndex);
		    }
		}
	    }
	  //iterate over all spheres
	  for (u32 i = 0; i < world->sphereCount; i++)
	    {
	      Sphere sphere = world->spheres[i];

	      lane_v3 spherePos;
	      spherePos = sphere.position;
	      lane_f32 sphereRadius;
	      sphereRadius = sphere.radius;
	  
	      lane_v3 relativeSpherePos = rayOrigin - spherePos;
	      lane_f32 a = dot(rayDirection, rayDirection);
	      lane_f32 b = 2*dot(rayDirection, relativeSpherePos);
	      lane_f32 c = dot(relativeSpherePos, relativeSpherePos) - sphereRadius * sphereRadius;

	      lane_f32 root = b*b - 4*a*c;
	      lane_u32 rootMask = root > tolerance;	  
	      if (!MaskAllZeros(rootMask))
		{
		//lane_f32 farDist = (-b + sqrt(root)) / 2*a;	      
		  lane_f32 dist = (-b - sqrt(root)) / 2*a;
		  lane_u32 distMask = (dist > minHitDistance) & (dist < minDist);

		  lane_u32 hitMask = rootMask & distMask;
		  if (!MaskAllZeros(hitMask))
		    {
		      lane_u32 sphereMatIndex;
		      sphereMatIndex = sphere.matIndex;	  
		      ConditionalAssign(&minDist,      hitMask, dist);
		      ConditionalAssign(&bounceNormal, hitMask, normalize((rayOrigin + rayDirection * dist) - spherePos));	      
		      ConditionalAssign(&matIndex,     hitMask, sphereMatIndex);
		    }
		}
	    }

	  //Set the colour based on what we hit
	  lane_v3 emitColour = maskLaneV3(gatherV3(world->materials, matIndex, emitColour), laneMask);
	  laneMask = andNot((matIndex == laneU32FromU32(0)), laneMask);      
	  resultColour += hadamard(emitColour, attenuation);
	  
	  if (MaskAllZeros(laneMask))
	    {
	      break;
	    }
	  else
	    {
	      lane_v3 reflectColour = gatherV3(world->materials, matIndex, reflectColour);
	      lane_f32 scatterScale = gatherF32(world->materials, matIndex, scatterScale);

	      //add any colour this object emits, times the attenuation
	      //clamp to 0-inf
	      lane_f32 cosAttenuation = Max(dot(rayDirection*(-1.0f), bounceNormal), laneF32FromF32(0));
	      //cosAttenuation = 1.0f;
	  
	      //update attenuation based on reflection colour
	      attenuation = hadamard(reflectColour, attenuation*cosAttenuation);

	      //setup for next bounce
	      rayOrigin = rayOrigin + rayDirection * minDist;
	  
	      lane_v3 pureBounce = rayDirection - bounceNormal*2.0f*dot(rayDirection, bounceNormal);	      	    lane_f32 x = randomBilateral32(&entropy);
	      lane_f32 y = randomBilateral32(&entropy);
	      lane_f32 z = randomBilateral32(&entropy);	  
	      lane_v3 randomBounce = normalize(bounceNormal + lane_v3{x,y,z});
	      rayDirection = lerp(randomBounce, pureBounce,scatterScale);	  
	    }
	}      
      finalColour += HorizontalAdd(resultColour);
    }
  LockedAdd(&world->bounceCount, HorizontalAdd(bouncesComputed));
  return finalColour / (f32)sampleCount;
}

int main(int argc, char** argv)
{
  u32 coreCount = GetNumProcessors();
  //assert(coreCount > 0);

  //Image setup
  Image image = allocateImage(IMAGE_WIDTH, IMAGE_HEIGHT);
  u32* out = image.pixels;
  
  World* world = initWorld();
  Camera* camera = initCamera(image);
  
  //split into tiles
  u32 tileWidth = 64;//image.width / coreCount;
  u32 tileHeight = tileWidth;
  //biased so that it goes over, so you dont have an empty region
  u32 tileCountX = (image.width + tileWidth - 1) / tileWidth;
  u32 tileCountY = (image.height + tileHeight - 1) / tileHeight;
  world->totalTileCount = tileCountY * tileCountX;
  u32 tilesCompleted = 0;

  WorkQueue queue = {};
  queue.totalOrders = world->totalTileCount;
  queue.queue = (WorkOrder*)malloc(sizeof(WorkOrder) * queue.totalOrders);
  queue.raysPerPixel = RAYS_PER_PIXEL;
  queue.camera = camera;
  
  printf("Config: Use CPU, %d cores, %d rays per pixel, %dx%d image,\n\t%dx%d tiles at %dk/tile, Lane Width: %d\n\tLens radius: %.4f\n",
	 coreCount, queue.raysPerPixel,
	 IMAGE_WIDTH, IMAGE_HEIGHT,
	 tileWidth, tileHeight, tileWidth*tileHeight*4/1024,
	 LANE_WIDTH, camera->lensRadius);
 
  //render
  WorkOrder* order = queue.queue;
  for (u32 tileY = 0; tileY < tileCountY; tileY++)
    {
      u32 minY = tileY * tileHeight;
      u32 onePastMaxY = minY + tileHeight;
      if (onePastMaxY > image.height)
	{
	  onePastMaxY = image.height;
	}
      for (u32 tileX = 0; tileX < tileCountX; tileX++)
	{
	  u32 minX = tileX * tileWidth;
	  u32 onePastMaxX = minX + tileWidth;
	  if (onePastMaxX > image.width)
	    {
	      onePastMaxX = image.width;
	    }
	  
	  order->world = world;
	  order->image = image;
	  order->minX = minX;
	  order->onePastMaxX = onePastMaxX;
	  order->minY = minY;
	  order->onePastMaxY = onePastMaxY;

	  order++;
	}
    }

  struct timespec start, finish;
  double elapsed;

  clock_gettime(CLOCK_MONOTONIC, &start);

  u64 startTime = clock();
  pthread_t threadIDs[coreCount-1];
  //make the threads, dont use this core, its running the main program
  for (u32 coreThread = 1; coreThread < coreCount; coreThread++)
    {
      CreateThread(&threadIDs[coreThread-1], threadProc, &queue);
    }
  while (world->tilesCompleted < world->totalTileCount)
    {
      //make this core work too
      u32 workIndex = LockedAdd(&queue.workIndex, 1);
      if (workIndex < queue.totalOrders)
	{
	  WorkOrder order = queue.queue[workIndex];
	  renderTile(order.world, order.image,
		     order.minX, order.onePastMaxX,
		     order.minY, order.onePastMaxY, queue.raysPerPixel, camera);
	}
      
      printf("\rRaytracing...%llu%%", 100 * world->tilesCompleted / world->totalTileCount);
      fflush(stdout);
    }
  for (u32 coreThread = 1; coreThread < coreCount; coreThread++)
    {
      JoinThread(threadIDs[coreThread-1], NULL);
    }
  printf("\n");

  clock_gettime(CLOCK_MONOTONIC, &finish);
  
  elapsed = (finish.tv_sec - start.tv_sec) * 1000.0;
  elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000.0;
  //finish up and print stats
  printf("Took %lfms\n", elapsed);
  printf("Total bounces: %llu\n", world->bounceCount);
  printf("Took %lfms per bounce\n",  (double)elapsed/ world->bounceCount);
  
  
  writeImage(&image, "out.bmp");
  
  free(image.pixels);
  free(world);
  free(camera);

  
  return 0;
}
