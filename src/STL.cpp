#include "STL.h"
#include <stdio.h>

void loadSTLShape(World* world, const char* fileName)
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
  for (u32 i = 0; i < triangleCount; i++)
    {
      Triangle triangle;
      fread((void*)&triangle.normal,    sizeof(vec3), 1, fileHandle);//rearrange struct for cache?
      fread((void*)&triangle.v0,        sizeof(vec3), 1, fileHandle);
      fread((void*)&triangle.v1,        sizeof(vec3), 1, fileHandle);
      fread((void*)&triangle.v2,        sizeof(vec3), 1, fileHandle);      
      fread((void*)&attributeByteCount, sizeof(u16),  1, fileHandle);
      triangle.matIndex = 1;
      triangle.v0 *= 0.1;
      triangle.v1 *= 0.1;
      triangle.v2 *= 0.1;
      //triangle.v0.x *= -1.0f;
      //triangle.v1.x *= -1.0f;
      //triangle.v2.x *= -1.0f;

      addTriangleToWorld(world, &triangle);
    }
  fclose(fileHandle);
}
