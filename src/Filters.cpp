#include "Filters.h"
#include <stdlib.h>
#include "bitmap.h"

vec4 uintPixelToFloat(u32 pixel)
{
  vec4 out;
  out.w += (f32)((u32)(pixel & 0xFF000000)>>24);
  out.z += (f32)((u32)(pixel & 0x00FF0000)>>16);
  out.y += (f32)((u32)(pixel & 0x0000FF00)>>8 );
  out.x += (f32)((u32)(pixel & 0x000000FF)>>0 );
  return out;
}

void edgeStoppingGaussianBlur(Image* image, u16* shapeMask)
{
  u32* outImage = (u32*)malloc(sizeof(u32)*image->width*image->height);
  u32 kernel[3][3] =
    {
      {1,2,1},
      {2,4,2},
      {1,2,1}
    };
  for (u32 imageY = 1; imageY < image->height - 1; imageY++)
    {
      for (u32 imageX = 1; imageX < image->width - 1; imageX++)
	{
	  // 3x3 kernel
	  f32 valueA = 0;
	  f32 valueR = 0;
	  f32 valueG = 0;
	  f32 valueB = 0;
	  u16 shapeMaskVal = shapeMask[imageY*image->width + imageX];
	  u32 totalK = 0;
	  for (s32 y = imageY - 1; y < imageY+2; y++)
	    {
	      for (s32 x = imageX - 1; x < imageX + 2; x++)
		{
		  // spot = *(inputImage + x + y*width);
		  
		  u32 kernelValue = kernel[y - imageY + 1][x - imageX + 1];
		  u32 imageValue = image->pixels[y*image->width + x];
		  if (shapeMask[y*image->width + x] == shapeMaskVal)
		    {
		      valueA += kernelValue * (f32)((u32)(imageValue & 0xFF000000)>>24);
		      valueR += kernelValue * (f32)((u32)(imageValue & 0x00FF0000)>>16);
		      valueG += kernelValue * (f32)((u32)(imageValue & 0x0000FF00)>>8 );
		      valueB += kernelValue * (f32)((u32)(imageValue & 0x000000FF)>>0 );
		      totalK += kernelValue;
		    }
		}
	    }
	  valueA /= totalK;
	  valueR /= totalK;
	  valueG /= totalK;
	  valueB /= totalK;
	  outImage[imageY * image->width + imageX] =
	    ((u32)(valueA)<<24) |
	    ((u32)(valueR)<<16) |
	    ((u32)(valueG)<<8 ) |
	    ((u32)(valueB)<<0 );
	}
    }
  free(image->pixels);
  image->pixels = outImage;
}
/*
void HDRToLDR(Image* image, f32 exposure)
{
  for (u32 imageY = 1; imageY < image->height - 1; imageY++)
    {
      for (u32 imageX = 1; imageX < image->width - 1; imageX++)
	{
	  vec4 value = 1;
	}
    }
}
*/
