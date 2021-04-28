#ifndef __WORLD_HEADER
#define __WORLD_HEADER

#define WORLD_SPHERE_COUNT 8
#define WORLD_PLANE_COUNT 8
#define WORLD_MATERIAL_COUNT 8

#include "Math.h"
#ifdef __USE_OPENCL
//OpenCL structs
typedef struct __attribute__((packed))
{
  vec3 position;
  float pad; //TODO: figure out better way for padding for opencl's coversion of float3 to float4
  f32 radius;
  s32 matIndex;
}Sphere;

typedef struct __attribute__((packed))
{ 
  vec3 normal;
  float pad;
  f32 dist; //distance along normal
  s32 matIndex;
}Plane;

typedef struct  __attribute__((packed))
{
  vec3 emitColour;
  float pad1;  
  vec3 reflectColour;
  float pad2;
  f32 scatterScale;
}Material;


typedef struct  __attribute__((packed))
{
  Plane planes[WORLD_PLANE_COUNT];
  Sphere spheres[WORLD_SPHERE_COUNT];
  Material materials[WORLD_MATERIAL_COUNT];
  volatile u32 bounceCount;
  s32 planeCount;
  s32 sphereCount;
  s32 materialCount;
  u32 totalTileCount;
}World;


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
  Plane planes[WORLD_PLANE_COUNT];
  Sphere spheres[WORLD_SPHERE_COUNT];
  Material materials[WORLD_MATERIAL_COUNT];
  int planeCount;  
  int sphereCount;
  int materialCount;
  u32 totalTileCount;
  volatile u64 bounceCount;
  volatile u64 tilesCompleted;
}World;


#endif //__USE_OPENCL

World* initWorld();
void addSphereToWorld(World* world, Sphere* sphere);
void addPlaneToWorld(World* world, Plane* plane);
void addMaterialToWorld(World* world, Material* mat);


#endif //__WORLD_HEADER
