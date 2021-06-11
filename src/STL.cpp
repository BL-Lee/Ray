#include "STL.h"
#include <stdio.h>

void loadSTLShape(World* world, const char* fileName)
{
  loadSTLShape(world, fileName, {});
}
void loadSTLShape(World* world, const char* fileName, vec3 offset)
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
  for (u32 i = 0; i < triangleCount; i++)
    {
      Triangle triangle;
      fread((void*)&triangle.normal,    PACKED_VEC3_SIZE, 1, fileHandle);//rearrange struct for cache?
      fread((void*)&triangle.v0,        PACKED_VEC3_SIZE, 1, fileHandle);
      fread((void*)&triangle.v1,        PACKED_VEC3_SIZE, 1, fileHandle);
      fread((void*)&triangle.v2,        PACKED_VEC3_SIZE, 1, fileHandle);      
      fread((void*)&attributeByteCount, sizeof(u16),  1, fileHandle);
      
      triangle.matIndex = 1;

      //TODO: transformation matrix
      triangle.v0 *= 0.1;
      triangle.v1 *= 0.1;
      triangle.v2 *= 0.1;

      triangle.v0 += offset;
      triangle.v1 += offset;
      triangle.v2 += offset;

      object.triangles[object.triangleCount] = world->triangleCount;
      object.triangleCount++;
      addTriangleToWorld(world, &triangle);
      
    }
  fclose(fileHandle);
  addObjectToWorld(world, &object);
}
