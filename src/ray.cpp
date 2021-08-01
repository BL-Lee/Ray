#include <stdio.h>
#include <float.h>
#include <time.h>

#include "bitmap.cpp"
#include "world.cpp"
#include "camera.cpp"
#include "Filters.cpp"
#include "timer.h"
#include "ray.h"
#include "Threads.h"
#include "STL.cpp"
#include "BlueNoise.cpp"
#include "BVH.cpp"
#include "Intersections.cpp"

#ifndef RAYS_PER_PIXEL 
 #define RAYS_PER_PIXEL 128
#endif
#ifndef DEBUG_LINES
 #define DEBUG_LINES 0
#endif
#ifndef USE_SH
 #define USE_SH 0
#endif
#ifndef USE_LBVH
 #define USE_LBVH 1
#endif
#ifndef DIRECTIONAL_LIGHTS
 #define DIRECTIONAL_LIGHTS 1
#endif


#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 720

//u16* shapeMask = (u16*)malloc(sizeof(u16)*IMAGE_HEIGHT*IMAGE_WIDTH);
Image* blueNoise;

static void renderTile(World* world, Image image,
		       SpatialHeirarchy* SH, BVH* bvh, 
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
	
	  vec3 colour = rayTrace(world, camera, SH, bvh, &filmY, &filmX, sampleCount, x, y);//, &shapeMask[y*image.width+x]);
	  //colour = HDRToLDR(colour);
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
static void* threadProc(void* args)
{
  WorkQueue* queue = (WorkQueue*)args;
  u32 workIndex = LockedAdd(&queue->workIndex, 1);
  while (workIndex < queue->totalOrders)
    {
      WorkOrder order = queue->queue[workIndex];
      renderTile(order.world, order.image,
		 order.SH, order.bvh,
	     order.minX, order.onePastMaxX,
		 order.minY, order.onePastMaxY, queue->raysPerPixel, queue->camera);
      
      workIndex = LockedAdd(&queue->workIndex, 1);
      printf("\rRaytracing...%llu%%", 100 * order.world->tilesCompleted / order.world->totalTileCount);
      fflush(stdout);
    }
  return NULL;
}


static lane_u32 rayCast(World* world, SpatialHeirarchy* SH, lane_v3 *Origin, lane_v3 *Direction)
{

  lane_v3 rayOrigin = *Origin;
  lane_v3 rayDirection = *Direction;

  lane_f32 minDist;
  minDist = laneF32FromF32(FLT_MAX);

  lane_f32 tolerance;
  tolerance = 0.00001f;
  lane_f32 minHitDistance;
  minHitDistance = 0.00001f;

  lane_u32 matIndex;
  matIndex = 0;

  for (u32 boxIndex = 0; boxIndex < SH->objectCount; boxIndex++)
    {
      lane_f32 boxHitDistNear;
      boxHitDistNear = -FLT_MAX;
      lane_f32 boxHitDistFar;
      boxHitDistFar = FLT_MAX;

      lane_u32 hitMask;
      hitMask = 0xFFFFFFFF;


      //check if this ray collides with this bounding "box"
      for (u32 boundingPlane = 0; boundingPlane < 7; boundingPlane++)
	{
	  lane_v3 planeNormal;
	  planeNormal = SH->planeNormals[boundingPlane];
		  	  
	  lane_f32 denom = dot(planeNormal, rayDirection);

	  lane_f32 planeNearOffset;
	  planeNearOffset = SH->boxes[boxIndex].planeDistances[boundingPlane][0];

	  lane_f32 planeFarOffset;
	  planeFarOffset  = SH->boxes[boxIndex].planeDistances[boundingPlane][1];


	  lane_f32 numerator;
	  numerator = dot(planeNormal, rayOrigin);
		    
	  lane_f32 farDist = (-planeFarOffset - numerator) / denom;
	  lane_f32 nearDist = (-planeNearOffset - numerator) / denom;
	

	  //conditionally swap the distances if denom is >= 0 (we use == 0 so that the ordering stays correct when dividing by 0)
	  //TODO: >= operator for lane_f32
	  lane_u32 swapMask = andNot(denom < laneF32FromF32(0.0f), laneU32FromU32(0xFFFFFFFF));

	  //condiitonal swap using xor
	  xorSwap(swapMask, &farDist,  &nearDist);

	  boxHitDistNear = maxLaneF32(nearDist, boxHitDistNear);
	  boxHitDistFar = minLaneF32(farDist, boxHitDistFar);
		    
	  lane_u32 distMask = (boxHitDistNear < boxHitDistFar);

	  hitMask &= distMask;
	  if (MaskAllZeros(hitMask))
	    {
	      break;
	    }

	}
      //at least one ray hit this box
      if (!MaskAllZeros(hitMask)) {

	Object object = SH->objects[boxIndex];
	//iterate over all planes to see if they intersect
      	for (u32 i = 0; i < object.planeCount; i++)
	  {
	    Plane plane = world->planes[object.planes[i]];
	    lane_f32 planeDist;
	    lane_u32 hitMask = rayPlaneTest(plane,
					    &rayDirection, &rayOrigin,
					    &minDist, &planeDist);
	    if (!MaskAllZeros(hitMask))
	      {
		return laneU32FromU32(plane.matIndex);
	      }
	  }
	//iterate over all spheres
      	for (u32 i = 0; i < object.sphereCount; i++)
	  {
	    Sphere sphere = world->spheres[object.spheres[i]];
	    lane_f32 sphereDist;
	    lane_u32 hitMask = raySphereTest(sphere,
					     &rayDirection, &rayOrigin,
					     &minDist, &sphereDist);
	    if (!MaskAllZeros(hitMask))
	      {
		return laneU32FromU32(sphere.matIndex);
	      }
	  }
	  for (u32 i = 0; i < object.triangleCount; i++)
	    {
	      Triangle triangle = world->triangles[object.triangles[i]];

	      lane_f32 triangleDist;
	      lane_u32 hitMask = rayTriangleTest(triangle,
						 &rayDirection, &rayOrigin,
						 &minDist, &triangleDist);
	      if (!MaskAllZeros(hitMask))
		{		
		  return laneU32FromU32(triangle.matIndex);
		}		      
	    }
      }
    }
  return matIndex;
}

//TODO: When using 8 wide lanes, lane_f32's last 4 floats get clobbered to 0 when passed by value, why?
static vec3 rayTrace(World* world, Camera* camera, SpatialHeirarchy* SH, BVH* bvh, lane_f32* filmYP, lane_f32* filmXP,  u32 sampleCount, u32 screenX, u32 screenY)//, u16* maskPtr)
{
  lane_f32 filmY = *filmYP;
  lane_f32 filmX = *filmXP;
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

  u32 timesUsed = 0;
  lane_u32 bouncesComputed = laneU32FromU32(0);
  for (u32 rayIndex = 0; rayIndex < rayCount; rayIndex++)
    {

      //setup for this ray
      lane_f32 pixelXOffset = randomUnilateral32(&entropy)*camera->halfPixelWidth * 2.0f;
      lane_f32 pixelYOffset = randomUnilateral32(&entropy)*camera->halfPixelHeight * 2.0f;
      
      lane_v3 filmPos = camera->filmCenter +
	cameraY * filmY * (camera->halfFilmHeight + pixelYOffset) + 
	cameraX * filmX * (camera->halfFilmWidth + pixelXOffset);

      lane_v3 lensXOffset = cameraX * camera->lensRadius * randomBilateral32(&entropy);
      lane_v3 lensYOffset = cameraY * randomBilateral32(&entropy) * camera->lensRadius;
                  
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

      lane_f32 totalDist;
      totalDist = 0;

      //Each time, a ray can bounce this many times
      for (u32 bounceCount = 0; bounceCount < 3; bounceCount++)
	{	  	  
	  lane_f32 minDist;
	  minDist = FLT_MAX;
	  lane_u32 matIndex;
	  matIndex = 0;
      
	  bouncesComputed += laneIncrement & laneMask;
#if USE_LBVH
	  
	  BVHNode boxStack[(1 << (BVH_DIGIT_COUNT))];
	  BVHNode* initial = boxStack;
	  initial->center = bvh->center;
	  initial->dimensions = bvh->dimensions;
	  initial->depth = 0;
	  u32 stackCount = 1;
	  while (stackCount > 0)
	    {
	      stackCount--;
	      BVHNode node = boxStack[stackCount];
	      lane_f32 nodeDist;
	      lane_u32 hitBoxMask = rayAABBTest(node.center, node.dimensions,
					      &rayDirection, &rayOrigin,
					      &nodeDist);
	      //test intersection
	      if (!MaskAllZeros(hitBoxMask))
		{
		  //if leaf trace it
		  if (node.depth == BVH_DIGIT_COUNT / 3)
		    {
		      //Get code from node's position
		      u32 mortonCode = positionToMortonCode(bvh, node.center);
		      u32 index = bvh->indices[mortonCode];
		      //Start at index and continue until different leaf
		      while (bvh->items[index].mortonCode == mortonCode)
			{
			  Triangle triangle = world->triangles[bvh->items[index].triangleIndex];
			  //printf("b");
			  lane_f32 triangleDist;
			  lane_u32 hitMask = rayTriangleTest(triangle,
							     &rayDirection, &rayOrigin,
							     &minDist, &triangleDist);
			  if (!MaskAllZeros(hitMask))
			    {		
			      lane_v3 normal;
			      normal = triangle.normal;
			      lane_u32 triangleMatIndex;
			      triangleMatIndex = triangle.matIndex;
			      ConditionalAssign(&minDist,      hitMask, triangleDist);
			      ConditionalAssign(&bounceNormal, hitMask, normal);
			      ConditionalAssign(&matIndex,     hitMask, triangleMatIndex);
			    }
			  index++;
			}
		    }
		  //otherwise split to stack
		  else
		    {
		      for (s32 x = -1; x < 2; x+= 2)
			{
			  for (s32 y = -1; y < 2; y+= 2)
			    {
			      for (s32 z = -1; z < 2; z+= 2)				
				{
				  vec3 nextDims = node.dimensions / 2.0f;
				  vec3 nextCenter =
				    {
				      node.center.x + (nextDims.x * x),
				      node.center.y + (nextDims.y * y),
				      node.center.z + (nextDims.z * z)
				    };

				  BVHNode split = {};
				  split.depth = node.depth + 1;
				  split.center = nextCenter;
				  split.dimensions = nextDims;
				  boxStack[stackCount] = split;
				  stackCount++;
				}
			    }
			}
		    }

		}
	    }

#endif
#if USE_SH
	  for (u32 boxIndex = 0; boxIndex < SH->objectCount; boxIndex++)
	    {
	      lane_f32 boxHitDistNear;
	      boxHitDistNear = -FLT_MAX;
	      lane_f32 boxHitDistFar;
	      boxHitDistFar = FLT_MAX;

	      lane_u32 hitMask;
	      hitMask = 0xFFFFFFFF;
	      //check if this ray collides with this bounding "box"
	      for (u32 boundingPlane = 0; boundingPlane < 7; boundingPlane++)
		{
		  lane_v3 planeNormal;
		  planeNormal = SH->planeNormals[boundingPlane];
		  	  
		  lane_f32 denom = dot(planeNormal, rayDirection);
		  
		  lane_f32 planeNearOffset;
		  planeNearOffset = SH->boxes[boxIndex].planeDistances[boundingPlane][0];

		  lane_f32 planeFarOffset;
		  planeFarOffset  = SH->boxes[boxIndex].planeDistances[boundingPlane][1];

		  lane_f32 numerator;
		  numerator = dot(planeNormal, rayOrigin);
		    
		  lane_f32 farDist = (-planeFarOffset - numerator) / denom;
		  lane_f32 nearDist = (-planeNearOffset - numerator) / denom;

		  //conditionally swap the distances if denom is > 0
		  lane_u32 swapMask = andNot(denom < laneF32FromF32(0.0f), laneU32FromU32(0xFFFFFFFF));
		  //condiitonal swap using xor
		  xorSwap(swapMask, &farDist,  &nearDist);

		  boxHitDistNear = maxLaneF32(nearDist, boxHitDistNear);
		  boxHitDistFar = minLaneF32(farDist, boxHitDistFar);

		  lane_u32 distMask = (boxHitDistNear < boxHitDistFar) & hitMask;
		  hitMask = distMask;
		  if (MaskAllZeros(hitMask))
		    {
		      break;
		    }
		}
	      //at least one ray hit this box
	      if (!MaskAllZeros(hitMask)) {

		Object object = SH->objects[boxIndex];

		//iterate over all planes to see if they intersect
		for (u32 i = 0; i < object.planeCount; i++)
		  {
		    Plane plane = world->planes[object.planes[i]];
		    lane_f32 planeDist;
		    lane_u32 hitMask = rayPlaneTest(plane,
						     &rayDirection, &rayOrigin,
						     &minDist, &planeDist);
		    if (!MaskAllZeros(hitMask))
		      {
			lane_u32 planeMatIndex;
			planeMatIndex = plane.matIndex;
			lane_v3 planeNormal;
			planeNormal = plane.normal;
			ConditionalAssign(&minDist,      hitMask, planeDist);
			ConditionalAssign(&bounceNormal, hitMask, planeNormal);
			ConditionalAssign(&matIndex,     hitMask, planeMatIndex);
		      }
		  }
		//iterate over all spheres
		for (u32 i = 0; i < object.sphereCount; i++)
		  {
		    Sphere sphere = world->spheres[object.spheres[i]];
		    lane_f32 sphereDist;
		    lane_u32 hitMask = raySphereTest(sphere,
						     &rayDirection, &rayOrigin,
						     &minDist, &sphereDist);
		    if (!MaskAllZeros(hitMask))
		      {
			lane_v3 spherePos;
			spherePos = sphere.position;
			lane_u32 sphereMatIndex;			
			sphereMatIndex = sphere.matIndex;	  
			ConditionalAssign(&minDist,      hitMask, sphereDist);
			ConditionalAssign(&bounceNormal, hitMask, normalize((rayOrigin + rayDirection * sphereDist) - spherePos));	      
			ConditionalAssign(&matIndex,     hitMask, sphereMatIndex);
		
		      }
		  }

		for (u32 i = 0; i < object.triangleCount; i++)	    
		  {

		    Triangle triangle = world->triangles[object.triangles[i]];

		    lane_f32 triangleDist;
		    lane_u32 hitMask = rayTriangleTest(triangle,
						       &rayDirection, &rayOrigin,
						       &minDist, &triangleDist);
		    if (!MaskAllZeros(hitMask))
		      {		
			lane_v3 normal;
			normal = triangle.normal;
			lane_u32 triangleMatIndex;
			triangleMatIndex = triangle.matIndex;
			ConditionalAssign(&minDist,      hitMask, triangleDist);
			ConditionalAssign(&bounceNormal, hitMask, normal);
			ConditionalAssign(&matIndex,     hitMask, triangleMatIndex);
		      }		      
		  }
	      }
	    }
#endif //use SH
#if DEBUG_LINES
	  if (bounceCount == 0) {
	    //TODO: sort lines into spatial partition
	    for (u32 i = 0; i < world->lineCount; i++)
	      {
		Line line = world->lines[i];
		lane_v3 lineDir;
		lineDir = line.direction;
		lane_v3 lineOrigin;
		lineOrigin = line.origin;

		lane_v3 perpDir = cross(rayDirection, lineDir);

		lane_v3 n2 = cross(lineDir, perpDir);
		lane_v3 c1 = rayOrigin + rayDirection * (dot(lineOrigin - rayOrigin, n2) / dot(rayDirection, n2));

		lane_v3 n1 = cross(rayDirection, perpDir);
		lane_v3 c2 = lineOrigin + lineDir * (dot(rayOrigin - lineOrigin, n1) / dot(lineDir, n1));

		lane_f32 dist = length(c2 - c1);
		lane_v3 rayOriginToPoint = c1 - rayOrigin;
		lane_v3 lineOriginToPoint = c2 - lineOrigin;

		lane_u32 distMask = dist < laneF32FromF32(0.01f);
		
		lane_u32 minDistMask = (length(rayOriginToPoint) < minDist) & (dot(rayDirection, rayOriginToPoint) > laneF32FromF32(0.0f));
		lane_u32 lengthMask = (length(lineOriginToPoint) < laneF32FromF32(line.length)) & (dot(lineDir, lineOriginToPoint) > laneF32FromF32(0.0f));
		
		lane_u32 hitMask = distMask & minDistMask & lengthMask;

		if (!MaskAllZeros(hitMask)) {
		  ConditionalAssign(&matIndex,     hitMask, laneU32FromU32(3));
		}
	      }
	  }
#endif
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

	      //For a mask of what shape is on this pixel
	      /*
		if (!*maskPtr)
		{
		*maskPtr = matIndex.V[0];
		}
	      */
	      lane_v3 reflectColour = gatherV3(world->materials, matIndex, reflectColour);
	      lane_f32 scatterScale = gatherF32(world->materials, matIndex, scatterScale);

	      //add any colour this object emits, times the attenuation
	      //clamp to 0-inf
	      //totally arbirary 0.4 right now, i just didnt like it at 0
	      lane_f32 cosAttenuation = maxLaneF32(dot(rayDirection*(-1.0f), bounceNormal), laneF32FromF32(0.3));
	      //setup for next bounce
	      rayOrigin = rayOrigin + rayDirection * minDist;

	      //update attenuation based on reflection colour	      
	      attenuation = hadamard(reflectColour, attenuation*cosAttenuation);

#if DIRECTIONAL_LIGHTS
	      //TODO: different lane widths give different shadows
	      //TODO: interpolation, shadows are very noticeable around edges
	      if (bounceCount == 0)
		{
		  for (u32 dlightIndex = 0; dlightIndex < world->dLightCount; dlightIndex++)
		    {
		      DirectionalLight dlight = world->dLights[dlightIndex];

		      lane_u32 inDirection = dot(bounceNormal, laneV3FromV3(dlight.direction)) < -tolerance;
		      if (!MaskAllZeros(inDirection))
			{
		  
			  lane_v3 oppositeLightDir = laneV3FromV3(dlight.direction * -1.0f);
			  lane_v3 shadowRayOrigin = rayOrigin + bounceNormal * 0.01f;
			  lane_u32 lightMatIndex = rayCast(world, SH, &shadowRayOrigin, &oppositeLightDir);
			  lane_u32 hitSomething = (lightMatIndex != laneU32FromU32(0));
			  emitColour = laneV3FromV3(dlight.colour);
			  ConditionalAssign(&emitColour, hitSomething, laneV3FromV3(world->materials[0].emitColour));
		
			  resultColour += hadamard(emitColour, attenuation);

			  bouncesComputed += laneIncrement & laneMask;
			}
		      
		    }
		}
#endif
	      lane_v3 pureBounce = rayDirection - bounceNormal*2.0f*dot(rayDirection, bounceNormal);
	      /*
	      vec3 tempBounce;
	      tempBounce.x = sampleXorThing(screenX, screenY, rayIndex, 0);
	      tempBounce.y = sampleXorThing(screenX, screenY, rayIndex, 1);
	      tempBounce.z = sampleXorThing(screenX, screenY, rayIndex, 2);
	      */
	      lane_f32 x = randomBilateral32(&entropy);
	      lane_f32 y = randomBilateral32(&entropy);
	      lane_f32 z = randomBilateral32(&entropy);	  
	      lane_v3 randomBounce = normalize(bounceNormal + lane_v3{x,y,z});
	      rayDirection = lerp(randomBounce, pureBounce,scatterScale);

	      totalDist += minDist;
	    }
	}
      finalColour += HorizontalAdd(resultColour);
    }
  LockedAdd(&world->bounceCount, HorizontalAdd(bouncesComputed));
  return finalColour / (f32)sampleCount;
}

int main(int argc, char** argv)
{
  blueNoise = loadImage("textures/LDR_RGBA_1.bmp");
  
  const char* fileName;
  if (argc > 1)
    {
      fileName = argv[1];
    }
  else
    {
      fileName = "out.bmp";
    }
  #ifdef _RAY_DEBUG
  u32 coreCount = 1;
  #else  
  u32 coreCount = GetNumProcessors();
  #endif

  //Image setup
  Image image = allocateImage(IMAGE_WIDTH, IMAGE_HEIGHT);
  u32* out = image.pixels;

  SpatialHeirarchy SH = {};
  World* world = initWorld(&SH);
  //loadSTLShape(world, &SH,  "assets/models/Dodecahedron.stl", vec3(0.5f,0.0f,0.0f));
  Camera* camera = initCamera(image);

  u32 entropy = 0xf81422;
  for (int i = 0; i < 30; i++)
    {
      vec3 loc =
	{
	  randomBilateral32(&entropy) * 5.0f,
	  randomBilateral32(&entropy) * 5.0f,
	  randomBilateral32(&entropy) * 5.0f
	};

      loadSTLShape(world, &SH, "assets/models/Dodecahedron.stl", loc);
    }
  loadSTLShape(world, &SH, "assets/models/Dodecahedron.stl");

  BVH* bvh = constructBVH(world);
  printf("Building Spatial Heirarchy...");
  Timer SHTimer;
  startTimer(&SHTimer);
  generateSpatialHeirarchy(world, &SH);
  endTimer(&SHTimer);  
  printf("Done. Took %fms\n", getTimeElapsedMS(&SHTimer));
  printf("\t%d Objects\n\t%d Triangles\n\t%d Spheres\n\t%d Planes\n\t%d Directional Lights\n", SH.objectCount, world->triangleCount, world->sphereCount, world->planeCount, world->dLightCount);
  #if DEBUG_LINES
  printf("\t%d Debug Lines\n", world->lineCount);
  #endif



  
  //split into tiles
  u32 tileWidth = 64;//image.width / coreCount;
  u32 tileHeight = tileWidth;
  //biased so that it goes over, so you dont have an empty region
  u32 tileCountX = (image.width + tileWidth - 1) / tileWidth;
  u32 tileCountY = (image.height + tileHeight - 1) / tileHeight;
  world->totalTileCount = tileCountY * tileCountX;
  world->tilesCompleted = 0;

  WorkQueue queue = {};
  queue.totalOrders = world->totalTileCount;
  queue.queue = (WorkOrder*)malloc(sizeof(WorkOrder) * queue.totalOrders);
  queue.raysPerPixel = RAYS_PER_PIXEL;
  queue.camera = camera;
  
  printf("Config: Use CPU, %d cores, %d rays per pixel\n\t%dx%d image, %dx%d tiles at %dk/tile\n\tLane Width: %d\n\tLens radius: %.4f\n",
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
	  order->SH = &SH;
	  order->bvh = bvh;
	  order->minX = minX;
	  order->onePastMaxX = onePastMaxX;
	  order->minY = minY;
	  order->onePastMaxY = onePastMaxY;

	  order++;
	}
    }
  Timer rayTimer;
  startTimer(&rayTimer);
  
  u64* threadIDs = (u64*)malloc(sizeof(u64)* (coreCount - 1));
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
		     order.SH,
		     order.bvh,
		     order.minX, order.onePastMaxX,
		     order.minY, order.onePastMaxY, queue.raysPerPixel, camera);
	}
      
      printf("\rRaytracing...%llu%%", 100 * world->tilesCompleted / world->totalTileCount);
      fflush(stdout);
    }
  for (u32 coreThread = 1; coreThread < coreCount; coreThread++)
    {
      JoinThread((void*)&threadIDs[coreThread-1], NULL);
    }
  printf("\n");

  endTimer(&rayTimer);
  double elapsed = getTimeElapsedMS(&rayTimer);

  //finish up and print stats
  printf("Took %lfms\n", elapsed);
  printf("Total bounces: %llu\n", world->bounceCount);
  printf("Took %lfms per bounce\n",  (double)elapsed / world->bounceCount);


  //edgeStoppingGaussianBlur(&image, shapeMask);


  
  writeImage(&image, fileName);

  free(image.pixels);
  free(world);
  free(camera);
  free(threadIDs);
  free(blueNoise->pixels);
  free(blueNoise);
  //free(bvh);
  return 0;
}
