#include <stdio.h>
#include <time.h>

#include "bitmap.cpp"
#include "world.cpp"
#include "camera.cpp"
#include "ray.h"
#include "OpenCL.cpp"
#include "STL.cpp"
#include "timer.h"
#include "BVH.cpp"

#ifndef RAYS_PER_PIXEL
 #define RAYS_PER_PIXEL 16
#endif
#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 720

char* loadKernelProgram(const char* filePath)
{
  FILE* fileHandle = fopen(filePath,"rb");
  if (!fileHandle)
    {
      printf("Error opening file\n");
      return NULL;
    }
  fseek(fileHandle, 0, SEEK_END);
  long fsize = ftell(fileHandle);
  fseek(fileHandle, 0, SEEK_SET);  /* same as rewind(f); */
  
  char *string = (char*)malloc(fsize + 1);
  fread(string, 1, fsize, fileHandle);
  fclose(fileHandle);

  string[fsize] = 0;
  //  printf("loaded file\n%s\n", string);
  return string;
}


int main(int argc, char** argv)
{
  const char* fileName;
  if (argc > 1)
    {
      fileName = argv[1];
    }
  else
    {
      fileName = "out.bmp";
    }

  //Image setup
  Image image = allocateImage(IMAGE_WIDTH, IMAGE_HEIGHT);
  u32* out = image.pixels;

  SpatialHeirarchy SH = {};
  World* world = initWorld(&SH);
  //loadSTLShape(world, &SH, "assets/models/Dodecahedron.stl", vec3(0.5f,0.0f,0.0f));
  Camera* camera = initCamera(image);
  u32 entropy = 0xf81422;
  for (int i = 0; i < 10; i++)
    {
      vec3 loc =
	{
	  randomBilateral32(&entropy) * 5.0f,
	  randomBilateral32(&entropy) * 5.0f,
	  randomBilateral32(&entropy) * 5.0f
	};

      loadSTLShape(world, &SH, "assets/models/Dodecahedron.stl", loc);
    }
  
  BVH* bvh = constructBVH(world);
  printf("Building Spatial Heirarchy...");
  
  Timer SHTimer;
  startTimer(&SHTimer);
  generateSpatialHeirarchy(world, &SH);
  endTimer(&SHTimer);
  
  printf("Done. Took %fms\n", getTimeElapsedMS(&SHTimer));
  printf("\t%d Objects\n\t%d Triangles\n\t%d Spheres\n\t%d Planes\n\t%d Directional Lights\n\t%d Debug Lines\n", SH.objectCount, world->triangleCount, world->sphereCount, world->planeCount, world->dLightCount, world->lineCount);
        
  u32 tileWidth = 128;//image.width / coreCount;
  u32 tileHeight = tileWidth;
  
  printf("Config: Use GPU, %d rays per pixel\n\t%dx%d image, %dx%d tiles at %dk/tile\n\tLens radius: %.4f\n",
	 RAYS_PER_PIXEL,
	 IMAGE_WIDTH, IMAGE_HEIGHT,
	 tileWidth, tileHeight, tileWidth*tileHeight*4/1024,
	 camera->lensRadius);

 
  //gpu boilerplate
  printf("Initializing OpenCL...");
  cl_platform_id platID;
  clInitPlatform(&platID);
  cl_device_id deviceID;
  clInitDevice(&deviceID, platID);
  cl_command_queue commandQueue;
  cl_context context = clInitContext(&deviceID, &platID, &commandQueue);
  char* kernelSource = loadKernelProgram("kernelPrograms/rayProgram.cl");
  cl_program program = clInitProgram(context, deviceID, kernelSource);
  cl_kernel kernel = clInitKernel(program, "rayTrace");


  cl_ulong localSize;
  clGetDeviceInfo(deviceID, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &localSize, 0);

  size_t maxWorkGroupSize;
  clGetDeviceInfo(deviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t),  &maxWorkGroupSize, NULL);
  printf("Max Local size: %llu, Max work group size: %zu\nSize of spatial Heirarchy: %lu", localSize, maxWorkGroupSize, sizeof(SpatialHeirarchy));

  
  int error;
  cl_mem clWorld = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(World), world, &error);
  cl_mem clCamera = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Camera), camera, &error);
  //cl_mem clSH = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(SpatialHeirarchy), &SH, &error);
  cl_mem clBVH = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(BVH), bvh, &error);
  cl_image_format clImageFormat = {CL_RGBA, CL_UNORM_INT8};
  cl_image_desc clImageDesc = {};
  clImageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
  clImageDesc.image_width = IMAGE_WIDTH;
  clImageDesc.image_height = IMAGE_HEIGHT;
    
  //cl_mem clImage = clCreateImage(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, &clImageFormat, &clImageDesc, image.pixels, &error);
  cl_mem clImage = clCreateImage2D (context,
				    CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
				    &clImageFormat,
				    clImageDesc.image_width,
				    clImageDesc.image_height,
				    0,
				    image.pixels,
				    &error);
  if (error)
    {
      printf("Error creating image\n\tError code:%d\n", error);
      return 1;
    }
   
  clSetKernelArg(kernel, 0, sizeof(cl_mem), &clWorld);
  clSetKernelArg(kernel, 1, sizeof(cl_mem), &clCamera);
  clSetKernelArg(kernel, 2, sizeof(cl_mem), &clBVH);
  u32 sampleCount = RAYS_PER_PIXEL;
  clSetKernelArg(kernel, 3, sizeof(u32), &sampleCount);
  clSetKernelArg(kernel, 4, sizeof(cl_mem), &clImage);

  printf(" Done!\n");
  printf("Ray tracing...");
  fflush(stdout);
  
  //render
  
  /*
    clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);*/
  
  //If we don't split it into different workers for big jobs then the GPU will take too long and the watchdog will kill the rendering
  //split into tiles
  //biased so that it goes over, so you dont have an empty region
  u32 tileCountX = (image.width + tileWidth - 1) / tileWidth;
  u32 tileCountY = (image.height + tileHeight - 1) / tileHeight;
  size_t globalOffset[2];
  size_t globalSize[2];

  Timer rayTimer;
  startTimer(&rayTimer);

  for (u32 tileY = 0; tileY < tileCountY; tileY++)
    {
      u32 minY = tileY * tileHeight;
      u32 onePastMaxY = minY + tileHeight;
      if (onePastMaxY > image.height)
	{
	  onePastMaxY = image.height;
	}
      for (u32 tileX = 0; tileX < tileCountX; tileX++)
	{
	  u32 minX = tileX * tileWidth;
	  u32 onePastMaxX = minX + tileWidth;
	  if (onePastMaxX > image.width)
	    {
	      onePastMaxX = image.width;
	    }
	  globalOffset[0]  =  minX;
	  globalOffset[1]  =  minY;
	  globalSize[0] = onePastMaxX - minX;
	  globalSize[1] = onePastMaxY - minY;
	  clEnqueueNDRangeKernel(commandQueue, kernel, 2, globalOffset, globalSize, NULL, 0, NULL, NULL);
	}	  
    }
  clFinish(commandQueue);  
  
  endTimer(&rayTimer);

  printf(" Done!\n");
  
  //Load results
  size_t origin[3] = {0,0,0};
  size_t region[3] = {image.width, image.height, 1};
  error = clEnqueueReadImage(commandQueue, clImage, CL_TRUE, origin, region, 0, 0, image.pixels, 0, NULL, NULL);
  if (error != CL_SUCCESS)
    {
      printf("Error writing image\n\tError code: %d\n",error);
      return 1;
   }
  clEnqueueReadBuffer(commandQueue, clWorld, CL_TRUE, 0, sizeof(World), world,0, NULL, NULL);

  
  
  //finish up and print stats
  double elapsed = getTimeElapsedMS(&rayTimer);
  printf("Took %lfms\n", elapsed);
  printf("Total bounces: %llu\n", world->bounceCount);
  printf("Took %lfms per bounce\n",  (double)elapsed / world->bounceCount);
  
  writeImage(&image, fileName);
  
  clReleaseMemObject(clWorld);
  clReleaseMemObject(clCamera);
  clReleaseMemObject(clBVH);
  clReleaseMemObject(clImage);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(commandQueue);
  clReleaseContext(context);

  free(image.pixels);
  free(world);
  free(camera);
  free(bvh);
  
  return 0;
}
