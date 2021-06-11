#ifndef __CAMERA_HEADER
#define __CAMERA_HEADER

#ifdef __USE_OPENCL
  #pragma pack(push, 1)
typedef struct
{
  vec3 pos;
  vec3 target;
  vec3 Z;
  vec3 X;
  vec3 Y;

  vec3 filmCenter;
  f32 focusDist;
  f32 filmWidth;
  f32 filmHeight;

  f32 halfFilmWidth;
  f32 halfFilmHeight;
  f32 halfPixelWidth;
  f32 halfPixelHeight;

  f32 lensRadius;
}Camera;
  #pragma pack(pop)
#else
typedef struct _Camera
{
  vec3 pos;
  vec3 target;
  vec3 Z;
  vec3 X;
  vec3 Y;

  vec3 filmCenter;
  f32 focusDist;
  f32 filmWidth;
  f32 filmHeight;

  f32 halfFilmWidth;
  f32 halfFilmHeight;
  f32 halfPixelWidth;
  f32 halfPixelHeight;

  f32 lensRadius;
}Camera;
#endif //__USE_OPENCL

Camera* initCamera();

#endif //__CAMERA_HEADER
