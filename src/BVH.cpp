#include "BVH.h"

BVH* constructBVH(World* world)
{
  //Initialize
  BVH* bvh = (BVH*)malloc(sizeof(BVH));
  *bvh = {};
  bvh->center = {0.0f,0.0f,0.0f};
  bvh->dimensions = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

  //Calculate bounds
  for (u32 i = 0; i < world->triangleCount; i++)
    {
      Triangle t = world->triangles[i];
      bvh->dimensions.x = max(abs(t.v0.x - bvh->center.x), bvh->dimensions.x);
      bvh->dimensions.y = max(abs(t.v0.y - bvh->center.y), bvh->dimensions.y);
      bvh->dimensions.z = max(abs(t.v0.z - bvh->center.z), bvh->dimensions.z);

      bvh->dimensions.x = max(abs(t.v1.x - bvh->center.x), bvh->dimensions.x);
      bvh->dimensions.y = max(abs(t.v1.y - bvh->center.y), bvh->dimensions.y);
      bvh->dimensions.z = max(abs(t.v1.z - bvh->center.z), bvh->dimensions.z); 

      bvh->dimensions.x = max(abs(t.v2.x - bvh->center.x), bvh->dimensions.x);
      bvh->dimensions.y = max(abs(t.v2.y - bvh->center.y), bvh->dimensions.y);
      bvh->dimensions.z = max(abs(t.v2.z - bvh->center.z), bvh->dimensions.z); 
    }
  
  //Calculate codes for triangles
  for (u32 i = 0; i < world->triangleCount; i++)
    {
      Triangle t = world->triangles[i];

      //get the code for each axis
      vec3 center = (t.v0 + t.v1 + t.v2) / 3.0f;
      u32 code = positionToMortonCode(bvh, center);
      /*
      u32 b = 1 << ((BVH_DIGIT_COUNT / 3) - 1);

      vec3 offsetCenter = center + bvh->center + bvh->dimensions;
      //printf("%f %f %f\n", offsetCenter.x, offsetCenter.y, offsetCenter.z);
      vec3 scaledCenter = offsetCenter * b / bvh->dimensions;
      //printf("%f %f %f\n", scaledCenter.x, scaledCenter.y, scaledCenter.z);
      vec3ui mortonCenter = { (u32)scaledCenter.x,
	(u32)scaledCenter.y,
	(u32)scaledCenter.z };
      //printf("%d %d %d\n", mortonCenter.x, mortonCenter.y, mortonCenter.z);
      u32 code = 0;
      
      //interleave the bits
      for (u32 digit = 0; digit < BVH_DIGIT_COUNT / 3; digit ++)
	{
	  u32 bit = (mortonCenter.x & (1 << (digit))) != 0;
	  code |= bit << (digit*3 + 2);
	  bit = (mortonCenter.y & (1 << (digit))) != 0;
	  code |= bit << (digit*3 + 1);
	  bit = (mortonCenter.z & (1 << (digit))) != 0;
	  code |= bit << (digit*3 + 0);
	}
      */
      // printf("%d\n", code);
      //add to BVH
      BVHItem item = {};
      item.triangleIndex = i;
      item.mortonCode = code;
      bvh->items[bvh->itemCount] = item;
      bvh->itemCount++; //TODO: assert less than item size
      if (bvh->itemCount > BVH_ITEM_COUNT)
	{
	  printf("BVH ITEM OVERFLOW: ITEM %d ADDED\n", bvh->itemCount);
	}
    }


  //Sort by codes
  BVHItem bucketZero[bvh->itemCount];
  BVHItem bucketOne[bvh->itemCount];
  radixSortBVH(bvh->items, bucketZero, bucketOne, 0, bvh->itemCount, BVH_DIGIT_COUNT);

  //Set lookup indices
  u32 code = 0;
  for (int i = 0; i < bvh->itemCount; i++)
    {

      if (bvh->items[i].mortonCode > code)
	{
	  code = bvh->items[i].mortonCode;	  
	}
      if (bvh->items[i].mortonCode == code)
	{
	  bvh->indices[code] = i;
	  /*printf("ADDED DEBUG RECT:\n\tCenter:%f %f %f\n\tDimensions:%f %f %f\n\tCode: %d\n",
		 center.x, center.y, center.z,
		 dimensions.x, dimensions.y, dimensions.z,
		 code);*/
	  vec3 center = mortonCodeToPosition(bvh, code);
	  vec3 dimensions = bvh->dimensions / (f32)(1 << (BVH_DIGIT_COUNT / 3));	  
	  addDebugRectToWorld(world, center, dimensions);

	  code++;
	}
    }
  
  return bvh;
}
vec3 mortonCodeToPosition(BVH* bvh, u32 code)
{
  vec3 center = bvh->center;
  for (int i = 0; i < BVH_DIGIT_COUNT; i += 3)
    {
      
      vec3 offset = {0,0,0};
      f32 xOn = (1 << (i + 2)) & code ? 1.0f : -1.0f;
      offset.x += xOn * bvh->dimensions.x / (1 << ((BVH_DIGIT_COUNT - i) / 3));
      
      f32 yOn = (1 << (i + 1)) & code ? 1.0f : -1.0f;
      offset.y += yOn * bvh->dimensions.y / (1 << ((BVH_DIGIT_COUNT - i) / 3));
      
      f32 zOn = (1 << (i + 0)) & code ? 1.0f : -1.0f;
      offset.z += zOn * bvh->dimensions.z / (1 << ((BVH_DIGIT_COUNT - i) / 3));
      
      center += offset;
    }
  return center;
}
u32 positionToMortonCode(BVH* bvh, vec3 position)
{
  u32 b = 1 << ((BVH_DIGIT_COUNT / 3) - 1);

  vec3 offsetCenter = position + bvh->center + bvh->dimensions;
  //printf("%f %f %f\n", offsetCenter.x, offsetCenter.y, offsetCenter.z);
  vec3 scaledCenter = offsetCenter * b / bvh->dimensions;
  //printf("%f %f %f\n", scaledCenter.x, scaledCenter.y, scaledCenter.z);
  vec3ui mortonCenter = { (u32)scaledCenter.x,
    (u32)scaledCenter.y,
    (u32)scaledCenter.z };
  //printf("%d %d %d\n", mortonCenter.x, mortonCenter.y, mortonCenter.z);
  u32 code = 0;
      
  //interleave the bits
  for (u32 digit = 0; digit < BVH_DIGIT_COUNT / 3; digit ++)
    {
      u32 bit = (mortonCenter.x & (1 << (digit))) != 0;
      code |= bit << (digit*3 + 2);
      bit = (mortonCenter.y & (1 << (digit))) != 0;
      code |= bit << (digit*3 + 1);
      bit = (mortonCenter.z & (1 << (digit))) != 0;
      code |= bit << (digit*3 + 0);
    }
  return code;
}


void radixSortBVH(BVHItem* bucket, BVHItem* bucketZero, BVHItem* bucketOne,  u32 start, u32 end, u32 digit)
{
  if (end - start < 1 || digit == 0) return;

  u32 bucketZeroCount = 0;
  u32 bucketOneCount = 0;
  for (u32 i = start; i < end; i++)
    {
      if ((1 << (digit - 1)) & bucket[i].mortonCode)
	{
	  bucketOne[bucketOneCount] = bucket[i];
	  bucketOneCount++;
	}
      else
	{
	  bucketZero[bucketZeroCount] = bucket[i];
	  bucketZeroCount++;	      
	}
    }
  memcpy(bucket + start,                   bucketZero, sizeof(BVHItem) * bucketZeroCount);
  memcpy(bucket + start + bucketZeroCount, bucketOne,  sizeof(BVHItem) * bucketOneCount);
  radixSortBVH(bucket,
	       bucketZero, bucketOne,
	    start, start + bucketZeroCount,
	    digit - 1);
  radixSortBVH(bucket,
	       bucketZero, bucketOne,
	    start + bucketZeroCount, start + bucketZeroCount + bucketOneCount,
	    digit - 1);  
}




