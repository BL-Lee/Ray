#ifndef __WORLD_HEADER
#define __WORLD_HEADER

#define WORLD_SPHERE_COUNT 32
#define WORLD_PLANE_COUNT 8
#define WORLD_MATERIAL_COUNT 8
#define WORLD_TRIANGLE_COUNT 1024
#define WORLD_DIRECTIONAL_LIGHT_COUNT 4
#define WORLD_LINE_COUNT 128
#define SPATIAL_BOX_COUNT 32

#include "Math.h"
#include "float.h"
#ifdef __USE_OPENCL
//OpenCL structs
#pragma pack(push,1)
#else
#include "SIMD.h"
#endif
//
//CPU structs
//

typedef struct
{
  u32 planes[1];//indices into world
  u32 spheres[2];
  u32 triangles[40];
  s32 planeCount;
  s32 sphereCount;
  s32 triangleCount;
}Object;

typedef struct _SpatialBox
{
  //these are in a different area of memory, that way plane Distances are cache friendly
  //might not even need pointer if theyre the same index as the object in SH?
  f32 planeDistances[7][2];
}SpatialBox;


typedef struct
{
  SpatialBox boxes[SPATIAL_BOX_COUNT];
  Object objects[32];
  vec3 planeNormals[7];
  u32 objectCount = 0;
}SpatialHeirarchy;


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

typedef struct _Line
{
  vec3 origin;
  vec3 direction;
  f32 length;
}Line;


typedef struct _Material
{
  float scatterScale;
  vec3 emitColour;
  vec3 reflectColour;
}Material;

typedef struct _DirectionalLight
{
  vec3 direction;
  vec3 colour;
}DirectionalLight;

typedef struct _World
{
  Plane planes[WORLD_PLANE_COUNT];
  Sphere spheres[WORLD_SPHERE_COUNT];
  Material materials[WORLD_MATERIAL_COUNT];
  Triangle triangles[WORLD_TRIANGLE_COUNT];
  DirectionalLight dLights[WORLD_DIRECTIONAL_LIGHT_COUNT];
  Line lines[WORLD_LINE_COUNT];
  int dLightCount;
  int planeCount;  
  int sphereCount;
  int materialCount;
  int triangleCount;
  int lineCount;
  u32 totalTileCount;
  volatile u64 bounceCount;
  volatile u64 tilesCompleted;
  //SpatialHeirarchy SH;
}World;

#ifdef __USE_OPENCL
#pragma pack(pop)
#endif



World* initWorld(SpatialHeirarchy* SH);

Triangle* getTriangleFromWorld(World* world, u32* index);
Sphere* getSphereFromWorld(World* world, u32* index);
Plane* getPlaneFromWorld(World* world, u32* index);


void addMaterialToWorld(World* world, Material* mat);
void addObjectToSpatialHeirarchy(SpatialHeirarchy* SH, Object* object);

void addSphereToObject(Object* object, u32 index);
void addPlaneToObject(Object* object, u32 index);
void addTriangleToObject(Object* object, u32 index);

void generateSpatialHeirarchy(World* world, SpatialHeirarchy* SH);
void computeExtentBounds(Object* object, SpatialBox* box, World* world, SpatialHeirarchy* SH);
void splitBox(SpatialHeirarchy* SH, SpatialBox* box, World* world, u32 axis);
#endif //__WORLD_HEADER
