#include <stdio.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
typedef struct _cl_image_desc {
          cl_mem_object_type image_type;
          size_t image_width;
          size_t image_height;
          size_t image_depth;
          size_t image_array_size;
          size_t image_row_pitch;
          size_t image_slice_pitch;
          cl_uint num_mip_levels;
          cl_uint num_samples;
          cl_mem buffer;
} cl_image_desc;
#endif
#include "Types.h"


u32 clInitPlatform(cl_platform_id* id);
u32 clInitDevice(cl_device_id* id, cl_platform_id platID);
cl_context clInitContext(cl_device_id* id, cl_platform_id* platId,cl_command_queue* queue);
cl_program clInitProgram(cl_context context, cl_device_id devID, const char* source);
cl_kernel clInitKernel(cl_program program, const char* func);

