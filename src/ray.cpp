#include <stdio.h>
#include "bitmap.h"

#include "Types.h"
#include "ray.h"
#include <float.h>
#include <time.h>
#include "Threads.h"
#define RAYS_PER_PIXEL 512
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
	  
	  u32 bitmapColour = packRGBA({1.0, colour.x, colour.y, colour.z});
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
      lane_f32 pixelXOffset = randomBilateral32(&entropy)*camera->halfPixelWidth;
      lane_f32 pixelYOffset = randomBilateral32(&entropy)*camera->halfPixelHeight;
	      
      lane_v3 filmPos = camera->filmCenter +
	cameraY * filmY * camera->halfFilmHeight +
	cameraY * filmY * pixelYOffset +
	cameraX * filmX * camera->halfFilmWidth +
	cameraX * filmX * pixelXOffset;
      
      lane_v3 rayOrigin;
      rayOrigin = camera->pos;
      lane_v3 rayDirection;
      rayDirection = normalize(filmPos - camera->pos);

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

	  lane_v3 emitColour = maskLaneV3(gatherV3(world->materials, matIndex, emitColour), laneMask);
	  //if matIndex is set, then we hit something
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
	  
	      //update attenuation based on reflection colour
	      attenuation = hadamard(reflectColour, attenuation*cosAttenuation);

	      //setup for next bounce
	      rayOrigin = rayOrigin + rayDirection * minDist;
	  
	      lane_v3 pureBounce = rayDirection - bounceNormal*2.0f*dot(rayDirection, bounceNormal);
	      //TODO: different noise for random
	      lane_f32 x = randomBilateral32(&entropy);
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
  Image image = allocateImage(1280,720);
  u32* out = image.pixels;
  
  //World setup
  World world = {};
  Material materials[6];
  materials[0] = {};
  materials[0].reflectColour = {};
  materials[0].emitColour = { 1.0f, 1.0f, 1.0f };
  
  materials[1] = {};
  materials[1].reflectColour = { 0.8f, 0.7f, 0.2f };
  materials[1].scatterScale = 0.2;
  
  materials[2].emitColour = {};
  materials[2].reflectColour = { 0.0f, 0.2f, 1.0f };
  materials[2].scatterScale = 0.7;
  
  materials[3].emitColour = {99.0f,0.0f,0.0f};
  materials[3].reflectColour = { 0.0f, 0.0f, 0.0f };
  materials[3].scatterScale = 1;
  
  
  materials[4].emitColour = {};
  materials[4].reflectColour = { 0.1f, 0.4f, 0.8f };
  materials[4].scatterScale = 1.0;
  
  materials[5].emitColour = {0.0f,0.0f,0.0f};
  materials[5].reflectColour = {0.2f,0.9f,0.2f};
  materials[5].scatterScale = 0.9;

  
  Plane planes[1] = {};
  planes[0].normal = { 0.0f, 0.0f, 1.0f };
  planes[0].dist = 0.0f;
  planes[0].matIndex = 1;

  Sphere spheres[4] = {};
  spheres[0].position = { 0.0f, 0.0f, 0.0f };
  spheres[0].radius = 1.0f;
  spheres[0].matIndex = 2;

  spheres[1].position = { -2.0f, 1.0f, 2.0f };
  spheres[1].radius = 1.2f;
  spheres[1].matIndex = 4;
  
  spheres[2].position = { 2.0f, 0.0f, 1.0f };
  spheres[2].radius = 1.0f;
  spheres[2].matIndex = 3;
  
  spheres[3].position = { 2.0f, 10.0f, 6.0f };
  spheres[3].radius = 2.0f;
  spheres[3].matIndex = 5;
    
  world.planes = planes;
  world.planeCount = sizeof(planes) / sizeof(Plane);
  world.spheres = spheres;
  world.sphereCount = sizeof(spheres) / sizeof(Sphere);
  world.materials = materials;
  world.materialCount = sizeof(materials) / sizeof(Material);

  Camera camera;
  camera.pos = { 0.0, -10.0, 1.0 };
  //aim at origin
  //aim in -z direction, local to camera
  //Just getting 3 orthoganal vectors for the camera
  camera.target = { 0.0, 0.0, 0.0 };
  camera.Z = normalize(camera.pos - camera.target);
  camera.X = normalize(cross({ 0.0, 0.0, 1.0 }, camera.Z));
  camera.Y = normalize(cross(camera.Z, camera.X));
  
  camera.filmCenter = camera.pos - camera.Z;
  camera.filmWidth = 1.0f;
  camera.filmHeight = 1.0f;
  if (image.width > image.height)
    {
      camera.filmWidth = (f32)image.width / (f32)image.height;
    }
  else if (image.height > image.width)
    {
      camera.filmHeight = (f32)image.height / (f32)image.width;
    }
  camera.halfFilmWidth = 0.5f * camera.filmWidth;
  camera.halfFilmHeight = 0.5f * camera.filmHeight;
  camera.halfPixelWidth = 0.5f / (f32)image.width;
  camera.halfPixelHeight = 0.5f / (f32)image.height;


  
  //split into tiles
  u32 tileWidth = 64;//image.width / coreCount;
  u32 tileHeight = tileWidth;
  //biased so that it goes over, so you dont have an empty region
  u32 tileCountX = (image.width + tileWidth - 1) / tileWidth;
  u32 tileCountY = (image.height + tileHeight - 1) / tileHeight;
  world.totalTileCount = tileCountY * tileCountX;
  u32 tilesCompleted = 0;

  WorkQueue queue = {};
  queue.totalOrders = world.totalTileCount;
  queue.queue = (WorkOrder*)malloc(sizeof(WorkOrder) * queue.totalOrders);
  queue.raysPerPixel = RAYS_PER_PIXEL;
  queue.camera = &camera;
  
  printf("Config: %d cores, %d rays per pixel, %dx%d tiles at %dk/tile\nLane Width: %d\n",
	 coreCount, queue.raysPerPixel,
	 tileWidth, tileHeight, tileWidth*tileHeight*4/1024,
	 LANE_WIDTH);
 
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

	  
	  order->world = &world;
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
  while (world.tilesCompleted < world.totalTileCount)
    {
      //make this core work too
      u32 workIndex = LockedAdd(&queue.workIndex, 1);
      if (workIndex < queue.totalOrders)
	{
	  WorkOrder order = queue.queue[workIndex];
	  renderTile(order.world, order.image,
		     order.minX, order.onePastMaxX,
		     order.minY, order.onePastMaxY, queue.raysPerPixel, &camera);
	}
      
      printf("\rRaytracing...%llu%%", 100 * world.tilesCompleted / world.totalTileCount);
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
  printf("Total bounces: %llu\n", world.bounceCount);
  printf("Took %lfms per bounce\n",  (double)elapsed/ world.bounceCount);
  
  
  writeImage(&image, "out.bmp");
  
  free(image.pixels);

  
  return 0;
}
