#ifndef STL_LOADER_HEADER
#define STL_LOADER_HEADER

#include "world.h"
#include "Math.h"

void loadSTLShape(World* world, SpatialHeirarchy* SH, const char* fileName);
void loadSTLShape(World* world, SpatialHeirarchy* SH, const char* fileName, vec3 offset);

#endif


