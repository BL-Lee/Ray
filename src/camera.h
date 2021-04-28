#ifndef __CAMERA_HEADER
#define __CAMERA_HEADER

#ifdef __USE_OPENCL
typedef struct __attribute__((packed))
{
  vec3 pos;
  float pad1;//wowwwwwww
  vec3 target;
  float pad2;
  vec3 Z;
  float pad3;
  vec3 X;
  float pad4;
  vec3 Y;
  float pad5;

  vec3 filmCenter;
  float pad6;
  f32 focusDist;
  f32 filmWidth;
  f32 filmHeight;

  f32 halfFilmWidth;
  f32 halfFilmHeight;
  f32 halfPixelWidth;
  f32 halfPixelHeight;

  f32 lensRadius;
}Camera;

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
