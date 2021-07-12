#include <stdio.h>
#include "OpenCL.h"
u32 clInitPlatform(cl_platform_id* id)
{
  //get list of available platforms
  u32 platformCount;
  if (clGetPlatformIDs(1, id, &platformCount) != CL_SUCCESS)
    {
      printf("\nUnable to get platform id\n");
      return 0;
    }
  else
    {
      return platformCount;
    }
}

u32 clInitDevice(cl_device_id* id, cl_platform_id platID)
{
  //try to get a supported gpu device
    u32 deviceCount;
    if (clGetDeviceIDs(platID, CL_DEVICE_TYPE_GPU, 1, id, &deviceCount) != CL_SUCCESS)
    {
      printf("\nUnable to get device id\n");
      return 0;
    }
  else
    {
      return deviceCount;
    }  
}

cl_context clInitContext(cl_device_id* id, cl_platform_id* platID,cl_command_queue* queue)
{
   //setup context properties list, must be terminated with 0
  cl_context_properties properties[3];
  properties[0] = CL_CONTEXT_PLATFORM;
  properties[1] = (cl_context_properties) *platID;
  properties[2] = 0;
  
  //create a context with the gpu
  int error;
  cl_context context = clCreateContext(properties, 1, id, NULL, NULL, &error);
  if (error)
    {
      printf("\nError creating context\n\tError ID: %d\n", error);
      return 0;
    }

  //create command queue using the context and device
  *queue = clCreateCommandQueue(context, *id, 0, &error);
  if (error)
    {
      printf("\nError creating command queue\n\tError ID: %d\n", error);
      return 0;
    }
  return context;
}

cl_program clInitProgram(cl_context context, cl_device_id devID, const char* source)
{
  int error;
  cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, &error);
  if (error)
    {
      printf("\nError creating program\n\tError Code: %d\n", error);
      return 0;
    }
  char options[] = "-cl-unsafe-math-optimizations -cl-mad-enable -cl-fast-relaxed-math";
  //char options[] = "-cl-opt-disable";  
  //compile the program
  if ((error = clBuildProgram(program, 1, &devID, options, NULL, NULL)) != CL_SUCCESS)
    {
      
      printf("\nError bugilding program\n\tError Code: %d\n", error);
      char buffer[16384];
      size_t length = 0;
      clGetProgramBuildInfo(
			    program,              // valid program object
			    devID,                // valid device_id that executable was built
			    CL_PROGRAM_BUILD_LOG, // indicate to retrieve build log
			    sizeof(buffer),       // size of the buffer to write log to
			    buffer,               // the actual buffer to write log to
			    &length               // the actual size in bytes of data copied to buffe
			    );
      printf("len: %zu\n", length);
      printf("%s\n",buffer);
      exit(1);
    }
  else
    {
      return program;
    }  
  
}

cl_kernel clInitKernel(cl_program program, const char* func)
{
  int error;
  //specify which kernel to execute
  cl_kernel kernel = clCreateKernel(program, func, &error);
  return kernel;  
}

//for reference
#if 0
int main(int argc, char** argv)
{
  
  
 
  //setup data to operate on
  float inputData[arrayLength] = {1,2,3,4,5,6,7,8,9,10};
  int dataSize = sizeof(float)*arrayLength;
  

  //create buffers for input and output
  cl_mem input = clCreateBuffer(context, CL_MEM_READ_ONLY,
				dataSize, NULL, &error);
  cl_mem output = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
				 dataSize, NULL, &error);


  //load data into the input buffer
  // NOTE: the 3rd argument defines whether the write is blocking or not
  // NOTE: 2nd last arg is a list of events that need to happen before the write can be executed
  // NOTE: last arg is an event that can be used for later
  clEnqueueWriteBuffer(commandQueue, input, CL_TRUE, 0, dataSize, inputData, 0, NULL, NULL);

  //set the arg list for the kernel command
  clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
  clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
  size_t global = arrayLength;

  
  //enqueue kernel command for execution
  clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &global, NULL, 0, NULL, NULL);
  clFinish(commandQueue);

  //copy results out of the output buffer
  float results[arrayLength];  
  clEnqueueReadBuffer(commandQueue, output, CL_TRUE, 0, dataSize, results, 0, NULL, NULL);

  //do what you want with the results
  printf("Initial values vs Computed values:\n");
  for (int i = 0; i < arrayLength; i++)
    {
      printf("\t%f : %f\n", inputData[i], results[i]);
    }
  

  //cleanup
  clReleaseMemObject(input);
  clReleaseMemObject(output);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(commandQueue);
  clReleaseContext(context);

  return 0;
  
}

#endif
