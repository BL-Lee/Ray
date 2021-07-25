#ifndef __RAY_HEADER
#define __RAY_HEADER

#include "BLMath.h"
#include "world.h"
#include "camera.h"


#ifndef __USE_OPENCL
typedef struct _WorkOrder
{
  
  World* world;
  Image image;
  SpatialHeirarchy* SH;
  u32 minX;
  u32 onePastMaxX;
  u32 minY;
  u32 onePastMaxY;
  
}WorkOrder;

typedef struct _WorkQueue
{
  
  WorkOrder* queue;
  u32 totalOrders;
  u32 raysPerPixel;
  volatile u64 workIndex;
  Camera* camera;
  
}WorkQueue;

/*
//TODO: inverted rayDirection
inline lane_u32 rayAABBIntersection(lane_v3 rayOrigin, lane_v3 rayDirection, SpatialBox* box)
{
    lane_v3 epsilon;
    epsilon = laneV3FromV3({0.000001,0.000001,0.000001});

  lane_v3 inverseRayDirection = laneV3FromV3({1.0f,1.0f,1.0f}) / (rayDirection + epsilon);
  lane_u32 result;
  lane_f32 tx1, tx2, ty1, ty2, tz1, tz2, tmin, tmax;
  tx1 = ((box->position.x - box->dimensions.x) - rayOrigin.x) * (inverseRayDirection.x);
  tx2 = ((box->position.x + box->dimensions.x) - rayOrigin.x) * (inverseRayDirection.x);

  tmin = min(tx1, tx2);
  tmax = max(tx1, tx2);  
  
  ty1 = ((box->position.y - box->dimensions.y) - rayOrigin.y) * (inverseRayDirection.y);
  ty2 = ((box->position.y + box->dimensions.y) - rayOrigin.y) * (inverseRayDirection.y);

  tmin = max(tmin, min(ty1, ty2));
  tmax = min(tmax, max(ty1, ty2));

  tz1 = ((box->position.z - box->dimensions.z) - rayOrigin.z) * (inverseRayDirection.z);
  tz2 = ((box->position.z + box->dimensions.z) - rayOrigin.z) * (inverseRayDirection.z);

  tmin = max(tmin, min(tz1, tz2));
  tmax = min(tmax, max(tz1, tz2));


  result = tmin < tmax;
  result |= tmin < laneF32FromF32(0.0f);
  //result = tmin > tmax;
  return result;// | (tmin < laneF32FromF32(0.0f));//(tmin > tmax || tmin < laneF32FromF32(0.0f));

      //minDist =  FLT_MAX;
      //finalColour = {1.0f,0.0f,0.0f};//hit from outside
      //minDist =  tmax;
      //finalColour = {0.0f,0.0f,1.0}; // hit from inside
}
*/

static vec3 rayTrace(World* world, Camera* camera, SpatialHeirarchy* SH, lane_f32* filmY, lane_f32* filmX,  u32 sampleCount, u32 screenX, u32 screenY);//, u16* maskPtr);
static void renderTile(World* world, Image image,
		u32 minX, u32 onePastMaxX,
		u32 minY, u32 onePastMaxY,
		u32 raysPerPixel);

#endif //__USE_OPENCL
#endif
