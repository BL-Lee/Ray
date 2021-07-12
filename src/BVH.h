#ifndef __BVH_HEADER
#define __BVH_HEADER

#include "world.h"
#include "Math.h"

typedef struct _BVHNode
{
  u32 triangleIndices[128];
}BVHNode;

//add all indices
//sort by codes
//Build heirarchy from codes

#define BVH_DIGIT_COUNT 9
typedef struct _BVH
{
  u32 digits = BVH_DIGIT_COUNT;
  vec3 center;
  vec3 dimensions; //radius in each axis Each split divides in 2
  BVHNode nodes[pow(2,digits)];
}BVH;



//mark off world into [0-1] cube
//assign morton code to primitives
//place everything with most significant bit as 0 in one child, 1 as the other child
//do it again for other bits

BVH* constructBVH(World* world);
BVHNode* getLeafFromCode(BVH* bvh, u32 code);

#endif
