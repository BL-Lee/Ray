#ifndef __RAY_HEADER
#define __RAY_HEADER

/*
#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"
*/
#include "Math.h"
#include "SIMD.h"
//typedef hmm_vec2 vec2;
//typedef hmm_vec3 vec3;
//typedef hmm_vec4 vec4;

typedef struct _Sphere
{
  vec3 position;
  float radius;
  int matIndex;
}Sphere;

typedef struct _Plane
{ 
  vec3 normal;
  float dist; //distance along normal
  int matIndex;
}Plane;

typedef struct _Material
{
  float scatterScale;
  vec3 emitColour;
  vec3 reflectColour;
}Material;


typedef struct _World
{
  Plane* planes;
  int planeCount;
  Sphere* spheres;
  int sphereCount;
  Material* materials;
  int materialCount;
  u32 totalTileCount;
  volatile u64 bounceCount;
  volatile u64 tilesCompleted;
}World;

typedef struct _Camera
{
  vec3 pos;
  vec3 target;
  vec3 Z;
  vec3 X;
  vec3 Y;

  vec3 filmCenter;
  f32 filmWidth;
  f32 filmHeight;

  f32 halfFilmWidth;
  f32 halfFilmHeight;
  f32 halfPixelWidth;
  f32 halfPixelHeight;

}Camera;

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


vec3 rayTrace(World* world, Camera* camera, lane_f32 filmY, lane_f32 filmX,  u32 sampleCount);
void renderTile(World* world, Image image,
		u32 minX, u32 onePastMaxX,
		u32 minY, u32 onePastMaxY,
		u32 raysPerPixel);



#endif
