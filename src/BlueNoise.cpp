#include "Math.h"
#include <stdlib.h>
#include "bitmap.h"

//Blue noise sampler from https://perso.liris.cnrs.fr/david.coeurjolly/publications/heitz19.pdf
//#include "samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_1spp.cpp"
/*
f32 sampleXorThing(u32 x, u32 y, u32 sampleIndex, u32 sampleDimension)
{
  //u32 point = y * 128 + x;
  x = x % 128;
  y = y % 128;
  sampleIndex = sampleIndex % 256;
  sampleDimension = sampleDimension % 8;

  u32 rankedSampleIndex = sampleIndex ^ scramblingTile[x + (y*128) + sampleDimension];
  u32 value = (sobol_256spp_256d[rankedSampleIndex * 256 + sampleDimension]);

  value = value ^ (rankingTile[((y * 128) + (x)) + sampleDimension]);

  f32 result = (((f32)value / (f32)255) - 0.5) * 2.0f;
  
  return result;

}
*/
vec3 sample3dBlueNoise(Image* blueNoise, u32 x, u32 y, u32 offset)
{
  offset = offset * 6;
  u32 t = offset;
  f32 divisorx = randomUnilateral32(&t);
  f32 divisory = randomUnilateral32(&t);
  u32 px = (u32)((f32)(x+offset)/divisorx) % blueNoise->width;
  u32 py = (u32)((f32)(y+offset)/divisory) % blueNoise->height;
  u32 value = blueNoise->pixels[py * blueNoise->width + px];
  vec3 result;
  result.x = ((((value & 0x000000FF)>>0 ) / (f32)255) - 0.5) * 2.0f;
  result.y = ((((value & 0x0000FF00)>>8 ) / (f32)255) - 0.5) * 2.0f;
  result.z = ((((value & 0x00FF0000)>>16) / (f32)255) - 0.5) * 2.0f;
  return result;
}


vec2* generateBlueNoiseGrid(u32 width, f32 r)
{
  u32 entropy = 0xF2819abf;
  u32 k = 30;
  f32 cellSize = r*r/2;
  
  u32 grid[width /(u32) cellSize][width / (u32)cellSize];
  s32 totalSize = width * width / (cellSize * cellSize);
  u32* p = &grid[0][0];
  for (u32 i = 0; i < totalSize; i++)
  {
    *p = -1;
    p++;
  }
  vec2* points = (vec2*)malloc(sizeof(vec2) * 512);
  vec2 activeList[512];
  
  u32 activeListIndex = 1;
  activeList[0].x = randomUnilateral32(&entropy) * width;
  activeList[0].y = randomUnilateral32(&entropy) * width;
  u32 x = activeList[0].x / (u32)cellSize;
  u32 y = activeList[0].y / (u32)cellSize;
  
  grid[x][y] = 0;
  points[0] = activeList[0];
  u32 pointsIndex = 0;
  
  while (activeListIndex != 0)
  {
    u32 added = 0;
    for (int j = 0; j < k; j++)
    {
      vec2 offset = {randomBilateral32(&entropy), randomBilateral32(&entropy)};
      offset = normalize(offset);
      offset *= (randomUnilateral32(&entropy) + 1) * r;      
      vec2 newPoint = activeList[activeListIndex - 1] + offset;
      if (newPoint.x < 0 || newPoint.x > width || newPoint.y < 0 || newPoint.y > width) continue;
      x = newPoint.x / (u32)cellSize;
      y = newPoint.y / (u32)cellSize;
      printf("new: %f %f\n", newPoint.x, newPoint.y);
      int occupied = 0;
      for (int k = -1; k < 2; k++)
      {
	for (int l = -1; l < 2; l++)
	{
	  
	  if (x+k > 0 && x+k < width/cellSize &&
	      y+l > 0 && y+l < width/cellSize &&
	      grid[x+k][y+l] != -1)
	  {
	    occupied = 1;
	  }
	}
      }
      if (!occupied)
      {
	grid[x][y] = activeListIndex;
	activeList[activeListIndex++] = newPoint;
	points[pointsIndex++] = newPoint;
	if (pointsIndex > 511) { return points; }
	added = 1;
      }
    }
    if (!added)
    {      
      activeListIndex--;
      printf("Bruh\n");
    }
  }
  
  return points;
}
