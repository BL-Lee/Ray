#include "STL.h"
#include <stdio.h>

void loadSTLShape(World* world, SpatialHeirarchy* SH,  const char* fileName)
{
  loadSTLShape(world, SH,  fileName, {});
}
void loadSTLShape(World* world, SpatialHeirarchy* SH, const char* fileName, vec3 offset)
{
  FILE* fileHandle = fopen(fileName, "rb");
  if (!fileHandle)
    {
      return;
    }
  u8 header[80];
  fread((void*)header, sizeof(u8), 80, fileHandle);
  u32 triangleCount;
  u16 attributeByteCount;
  fread((void*)&triangleCount, sizeof(u32), 1, fileHandle);
  Object object = {};
  u32 entropy = *(u32*)((f32*)&offset.x);
  for (u32 i = 0; i < triangleCount; i++)
    {
      u32 index;
      Triangle* triangle = getTriangleFromWorld(world, &index);
      fread((void*)&triangle->normal,    PACKED_VEC3_SIZE, 1, fileHandle);//rearrange struct for cache?
      fread((void*)&triangle->v0,        PACKED_VEC3_SIZE, 1, fileHandle);
      fread((void*)&triangle->v1,        PACKED_VEC3_SIZE, 1, fileHandle);
      fread((void*)&triangle->v2,        PACKED_VEC3_SIZE, 1, fileHandle);      
      fread((void*)&attributeByteCount, sizeof(u16),  1, fileHandle);


      
      triangle->matIndex = (entropy % 5) + 1;

      //TODO: transformation matrix
      triangle->v0 *= 0.1;
      triangle->v1 *= 0.1;
      triangle->v2 *= 0.1;

      triangle->v0 += offset;
      triangle->v1 += offset;
      triangle->v2 += offset;

      addTriangleToObject(&object, index);      
    }
  fclose(fileHandle);
  addObjectToSpatialHeirarchy(SH, &object);
}
