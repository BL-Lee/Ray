#ifndef __RAY_HEADER
#define __RAY_HEADER

#include "Math.h"
#include "world.h"
#include "camera.h"

#ifndef __USE_OPENCL
typedef struct _WorkOrder
{
  
  World* world;
  Image image;
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


vec3 rayTrace(World* world, Camera* camera, lane_f32 filmY, lane_f32 filmX,  u32 sampleCount, u32 screenX, u32 screenY);//, u16* maskPtr);
void renderTile(World* world, Image image,
		u32 minX, u32 onePastMaxX,
		u32 minY, u32 onePastMaxY,
		u32 raysPerPixel);

#endif //__USE_OPENCL
#endif
