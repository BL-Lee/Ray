#ifndef __BVH_HEADER
#define __BVH_HEADER

#include "world.h"
#include "BLMath.h"



//must be divisble by 3 for 3 dimensions
#define BVH_DIGIT_COUNT 6
#define BVH_ITEM_COUNT 2048
#ifdef __USE_OPENCL
#pragma pack(push,1)
#endif

typedef struct _BVHNode
{
  vec3 center;
  vec3 dimensions;
  u32 depth;
}BVHNode;

typedef struct _BVHItem
{
  u32 triangleIndex;
  u32 mortonCode;
}BVHItem;
//add all indices
//sort by codes
//Build heirarchy from codes

typedef struct _BVH
{
  u32 digits = BVH_DIGIT_COUNT;
  vec3 center;
  vec3 dimensions; //radius in each axis Each split divides in 2
  BVHItem items[BVH_ITEM_COUNT];
  u32 indices[(1 << (BVH_DIGIT_COUNT)) - 1]; //each code, and where it points to in items
  u32 itemCount;
}BVH;

#ifdef __USE_OPENCL
#pragma pack(pop)
#endif

//mark off world into [0-1] cube
//assign morton code to primitives
//place everything with most significant bit as 0 in one child, 1 as the other child
//do it again for other bits

BVH* constructBVH(World* world);
vec3 mortonCodeToPosition(BVH* bvh, u32 code);
u32 positionToMortonCode(BVH* bvh, vec3 position);
//void radixSortBVH(BVHItem* bucket, u32 start, u32 end, u32 digit);
void radixSortBVH(BVHItem* bucket, BVHItem* bucketZero, BVHItem* bucketOne,  u32 start, u32 end, u32 digit);
#endif
