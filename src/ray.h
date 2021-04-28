#ifndef __RAY_HEADER
#define __RAY_HEADER

#include "Math.h"


#ifdef __USE_OPENCL
//OpenCL structs
typedef struct __attribute__((packed))
{
  vec3 position;
  float pad;
  f32 radius;
  s32 matIndex;
}cl_sphere;

typedef struct __attribute__((packed))
{ 
  vec3 normal;
  float pad;
  f32 dist; //distance along normal
  s32 matIndex;
}cl_plane;

typedef struct  __attribute__((packed))
{
  vec3 emitColour;
  float pad1;  
  vec3 reflectColour;
  float pad2;
  f32 scatterScale;
}cl_material;


typedef struct  __attribute__((packed))
{
  cl_plane planes[8];
  cl_sphere spheres[8];
  cl_material materials[8];
  volatile u32 bounceCount;
  s32 planeCount;
  s32 sphereCount;
  s32 materialCount;
  u32 totalTileCount;
}cl_world;

typedef struct __attribute__((packed))
{
  vec3 pos;
  float pad1;//wowwwwwww
  vec3 target;
  float pad2;
  vec3 Z;
  float pad3;
  vec3 X;
  float pad4;
  vec3 Y;
  float pad5;

  vec3 filmCenter;
  float pad6;
  f32 focusDist;
  f32 filmWidth;
  f32 filmHeight;

  f32 halfFilmWidth;
  f32 halfFilmHeight;
  f32 halfPixelWidth;
  f32 halfPixelHeight;

  f32 lensRadius;
}cl_camera;

#else
#include "SIMD.h"
//
//CPU structs
//
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
  Plane planes[8];
  Sphere spheres[8];
  Material materials[8];
  int planeCount;  
  int sphereCount;
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
  f32 focusDist;
  f32 filmWidth;
  f32 filmHeight;

  f32 halfFilmWidth;
  f32 halfFilmHeight;
  f32 halfPixelWidth;
  f32 halfPixelHeight;

  f32 lensRadius;
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

#endif //__USE_OPENCL
#endif
