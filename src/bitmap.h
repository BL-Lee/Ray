#ifndef __PIXELS_TO_BITMAP_HEADER
#define __PIXELS_TO_BITMAP_HEADER

#include "Types.h"

typedef struct _BMP_Header
{
  u16 name;
  u32 size;
  u16 res1;
  u16 res2;
  u32 offset;
}BMP_Header;

//40
typedef struct _BITMAPINFOHEADER
{
  u32 size;
  s32 width;
  s32 height;
  u16 planes;
  u16 bitsPerPixel;
  u32 compressionMethod;
  u32 sizeOfBitmap;
  s32 horizPPM;
  s32 vertPPM;
  u32 colourPaletteSize;
  u32 importantColours;
}BITMAPINFOHEADER;

typedef struct _Image
{
  u32 width;
  u32 height;
  u32* pixels;
}Image;

u8* BMPHeaderToByteBuffer(BMP_Header*);
u8* BITMAPINFOHEADERToByteBuffer(BITMAPINFOHEADER*);
void writeImage(Image*, const char*);
Image allocateImage(u32, u32);

#endif
