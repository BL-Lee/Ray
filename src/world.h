#ifndef __WORLD_HEADER
#define __WORLD_HEADER

#define WORLD_SPHERE_COUNT 8
#define WORLD_PLANE_COUNT 8
#define WORLD_MATERIAL_COUNT 8
#define WORLD_TRIANGLE_COUNT 64

#include "Math.h"
#ifdef __USE_OPENCL

#if defined(_MSC_VER) 
#pragma pack(push,1)
//OpenCL structs
typedef struct 
{
  vec3 position;
  float pad; //TODO: figure out better way for padding for opencl's coversion of float3 to float4
  f32 radius;
  s32 matIndex;
}Sphere;

typedef struct 
{ 
  vec3 normal;
  float pad;
  f32 dist; //distance along normal
  s32 matIndex;
}Plane;

typedef struct _Triangle
{
  vec3 v0;
  float pad1;  
  vec3 v1;
  float pad2;
  vec3 v2;
  float pad3;
  vec3 normal;
  float pad4;
  int matIndex;
}Triangle;

typedef struct 
{
  vec3 emitColour;
  float pad1;  
  vec3 reflectColour;
  float pad2;
  f32 scatterScale;
}Material;


typedef struct 
{
  Plane planes[WORLD_PLANE_COUNT];
  Sphere spheres[WORLD_SPHERE_COUNT];
  Material materials[WORLD_MATERIAL_COUNT];
  Triangle triangles[WORLD_TRIANGLE_COUNT];
  volatile u32 bounceCount;
  s32 planeCount;
  s32 sphereCount;
  s32 materialCount;
  S32 triangleCount;
  u32 totalTileCount;
}World;
#pragma pack(pop)


#elif defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)//should really be gnu compiler
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

typedef struct __attribute__((packed))
{
  vec3 v0;
  float pad0;
  vec3 v1;
  float pad1;
  vec3 v2;
  float pad2;
  vec3 normal;
  float pad3;
  int matIndex;
}Triangle;


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
  Triangle triangles[WORLD_TRIANGLE_COUNT];
  volatile u32 bounceCount;
  s32 planeCount;
  s32 sphereCount;
  s32 materialCount;
  s32 triangleCount;
  u32 totalTileCount;
}World;

#endif

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

typedef struct _Triangle
{
  vec3 v0;
  vec3 v1;
  vec3 v2;
  vec3 normal;
  int matIndex;
}Triangle;

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
  Triangle triangles[WORLD_TRIANGLE_COUNT];
  int planeCount;  
  int sphereCount;
  int materialCount;
  int triangleCount;
  u32 totalTileCount;
  volatile u64 bounceCount;
  volatile u64 tilesCompleted;
}World;


#endif //__USE_OPENCL

World* initWorld();
void addSphereToWorld(World* world, Sphere* sphere);
void addPlaneToWorld(World* world, Plane* plane);
void addMaterialToWorld(World* world, Material* mat);
void addTriangleToWorld(World* world, Triangle* triangle);

#endif //__WORLD_HEADER
