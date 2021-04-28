#include <stdio.h>
#include <time.h>

#include "bitmap.cpp"
#include "world.cpp"
#include "camera.cpp"
#include "ray.h"
#include "OpenCL.cpp"

#define RAYS_PER_PIXEL 512
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
  //Image setup
  Image image = allocateImage(IMAGE_WIDTH, IMAGE_HEIGHT);
  u32* out = image.pixels;

  World* world = initWorld();
  Camera* camera = initCamera(image);
  
  size_t globalWorkSize[2] = {image.width, image.height};
  size_t localWorkSize[2] = {8,8};
  
  printf("Config: Use GPU, %d rays per pixel, %dx%d image, work group size: %lux%lu\n\tLens radius: %.4f\n",
	 RAYS_PER_PIXEL,
	 IMAGE_WIDTH, IMAGE_HEIGHT,
	 localWorkSize[0], localWorkSize[1],
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

  size_t maxWorkGroupSize;
  clGetDeviceInfo(deviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t),  &maxWorkGroupSize, NULL);
  
  int error;
  cl_mem clWorld = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(World), world, &error);
  cl_mem clCamera = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Camera), camera, &error);
  
  cl_image_format clImageFormat = {CL_RGBA, CL_UNORM_INT8};
  cl_image_desc clImageDesc = {};
  clImageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
  clImageDesc.image_width = IMAGE_WIDTH;
  clImageDesc.image_height = IMAGE_HEIGHT;
    
  cl_mem clImage = clCreateImage(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, &clImageFormat, &clImageDesc, image.pixels, &error);
  if (error)
    {
      printf("Error creating image\n\tError code:%d\n", error);
      return 1;
    }
   
  clSetKernelArg(kernel, 0, sizeof(cl_mem), &clWorld);
  clSetKernelArg(kernel, 1, sizeof(cl_mem), &clCamera);
  u32 sampleCount = RAYS_PER_PIXEL;
  clSetKernelArg(kernel, 2, sizeof(u32), &sampleCount);
  clSetKernelArg(kernel, 3, sizeof(cl_mem), &clImage);

  printf(" Done!\n");
  printf("Ray tracing...");
  fflush(stdout);
  
  //render
  struct timespec start, finish;
  double elapsed;
  clock_gettime(CLOCK_MONOTONIC, &start);
  u64 startTime = clock();

  //do stuff
  clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
  clFinish(commandQueue);  
  
  clock_gettime(CLOCK_MONOTONIC, &finish);

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
  
  elapsed = (finish.tv_sec - start.tv_sec) * 1000.0;
  elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000.0;
  //finish up and print stats
  printf("Took %lfms\n", elapsed);
  printf("Total bounces: %d\n", world->bounceCount);
  printf("Took %lfms per bounce\n",  (double)elapsed/ world->bounceCount);
  
  writeImage(&image, "outGPU.bmp");
  
  clReleaseMemObject(clWorld);
  clReleaseMemObject(clCamera);
  clReleaseMemObject(clImage);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(commandQueue);
  clReleaseContext(context);

  free(image.pixels);
  free(world);
  free(camera);
  
  return 0;
}
