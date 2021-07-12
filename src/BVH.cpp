#include "BVH.h"

BVH* constructBVH(World* world)
{
  BVH* bvh = (BVH*)malloc(sizeof(BVH));
  bvh->center = {0.0f,0.0f,0.0f};
  bvh->dimensions = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
  for (u32 i = 0; i < world->triangleCount; i++)
    {
      Triangle t = world->triangles[i];
      bvh->dimensions.x = max(abs(t.v0.x - bvh->center), bvh->dimensions.x);
      bvh->dimensions.y = max(abs(t.v0.y - bvh->center), bvh->dimensions.y);
      bvh->dimensions.z = max(abs(t.v0.z - bvh->center), bvh->dimensions.z);

      bvh->dimensions.x = max(abs(t.v1.x - bvh->center), bvh->dimensions.x);
      bvh->dimensions.y = max(abs(t.v1.y - bvh->center), bvh->dimensions.y);
      bvh->dimensions.z = max(abs(t.v1.z - bvh->center), bvh->dimensions.z); 

      bvh->dimensions.x = max(abs(t.v2.x - bvh->center), bvh->dimensions.x);
      bvh->dimensions.y = max(abs(t.v2.y - bvh->center), bvh->dimensions.y);
      bvh->dimensions.z = max(abs(t.v2.z - bvh->center), bvh->dimensions.z); 
    }

  
}
BVHNode* getLeafFromCode(BVH* bvh, u32 code)
{
}


