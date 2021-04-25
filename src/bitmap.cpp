#include "bitmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BMP_HEADER_PACKED_SIZE 14
//Way to put a header into a buffer without relying on packed structs
//Which in this case would be gcc dependent
u8* BMPHeaderToByteBuffer(BMP_Header* header)
{
  u8* buffer = (u8*)malloc(BMP_HEADER_PACKED_SIZE); //size of the packed header
  memcpy(buffer + 0 , &header->name, 2);
  memcpy(buffer + 2 , &header->size, 4);
  memcpy(buffer + 6 , &header->res1, 2);
  memcpy(buffer + 8 , &header->res2, 2);
  memcpy(buffer + 10, &header->offset, 4);
  return buffer;
}

#define BITMAPINFOHEADER_PACKED_SIZE 40
//Way to put a header into a buffer without relying on packed structs
//Which in this case would be gcc dependent
//This way sucks...
u8* BITMAPINFOHEADERToByteBuffer(BITMAPINFOHEADER* header)
{
  u8* buffer = (u8*)malloc(BITMAPINFOHEADER_PACKED_SIZE); //size of the packed header
  
  memcpy(buffer + 0  , &header->size,              4);
  memcpy(buffer + 4  , &header->width,             4);
  memcpy(buffer + 8  , &header->height,            4);
  memcpy(buffer + 12 , &header->planes,            2);
  memcpy(buffer + 14 , &header->bitsPerPixel,      2);
  memcpy(buffer + 16 , &header->compressionMethod, 4);
  memcpy(buffer + 20 , &header->sizeOfBitmap,      4);
  memcpy(buffer + 24 , &header->horizPPM,          4);
  memcpy(buffer + 28 , &header->vertPPM,           4);
  memcpy(buffer + 32 , &header->colourPaletteSize, 4);
  memcpy(buffer + 36 , &header->importantColours,  4);
  
  return buffer;
}

void writeImage(Image* image, const char* file)
{
  //Note (ben): currently set to RGBA
  u32 bitsPerPixel = 32;
  u16 bytesPerPixel = 4;
  
  u32 imageSize = image->width * image->height * bytesPerPixel;
  
  //generate info header
  BITMAPINFOHEADER infoHeader = {};
  infoHeader.size = BITMAPINFOHEADER_PACKED_SIZE;
  infoHeader.width = image->width;
  infoHeader.height = --image->height;
  infoHeader.planes = 1;
  infoHeader.bitsPerPixel = bitsPerPixel;
  infoHeader.compressionMethod = 0; //no compression
  infoHeader.sizeOfBitmap = 0; //can use 0 if no compression used
  infoHeader.horizPPM = 1;
  infoHeader.vertPPM = 1;
  infoHeader.colourPaletteSize = 0;
  infoHeader.importantColours = 0;
  
  u8* infoBuffer = BITMAPINFOHEADERToByteBuffer(&infoHeader);

  //generate header
  BMP_Header header = {};
  header.name = 0x4d42;
  header.size = BMP_HEADER_PACKED_SIZE + BITMAPINFOHEADER_PACKED_SIZE + imageSize;
  header.res1 = 0;//reserved values, default to 0
  header.res2 = 0;
  header.offset = BMP_HEADER_PACKED_SIZE + BITMAPINFOHEADER_PACKED_SIZE;
  
  u8* headerBuffer = BMPHeaderToByteBuffer(&header);

  //write file
  FILE *fileHandle = fopen(file,"wb");
  fwrite(headerBuffer, BMP_HEADER_PACKED_SIZE, 1, fileHandle);
  fwrite(infoBuffer, BITMAPINFOHEADER_PACKED_SIZE, 1, fileHandle);
  fwrite(image->pixels, bytesPerPixel, image->height * image->width, fileHandle);

  free(headerBuffer);
  free(infoBuffer);
  
  fclose(fileHandle);
}

Image allocateImage(u32 width, u32 height)
{
  Image image = {};
  image.pixels = (u32*)malloc(sizeof(u32) * width * height);
  image.height = height;
  image.width = width;
  return image;
}
