#include <stdio.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include "Types.h"

u32 clInitPlatform(cl_platform_id* id);
u32 clInitDevice(cl_device_id* id, cl_platform_id platID);
cl_context clInitContext(cl_device_id* id, cl_platform_id* platId,cl_command_queue* queue);
cl_program clInitProgram(cl_context context, cl_device_id devID, const char* source);
cl_kernel clInitKernel(cl_program program, const char* func);

