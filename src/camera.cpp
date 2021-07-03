#include "camera.h"
#include "bitmap.h"
#include "stdlib.h"

Camera* initCamera(Image image)
{
  Camera* camera = (Camera*)malloc(sizeof(Camera));
  camera->pos = { 3.0, -10.0, 1.0 };
  camera->lensRadius = 0.10f;
  //aim at origin
  //aim in -z direction, local to camera
  //Just getting 3 orthoganal vectors for the camera
  camera->target = { 0.0, 0.0, 0.0 };
  camera->Z = normalize(camera->pos - camera->target);
  camera->X = normalize(cross({ 0.0, 0.0, 1.0 }, camera->Z));
  camera->Y = normalize(cross(camera->Z, camera->X));

  camera->focusDist = length(camera->pos - camera->target);
  camera->filmCenter = camera->pos - (camera->Z * camera->focusDist);
  camera->filmWidth = 1.0f;
  camera->filmHeight = 1.0f;
  if (image.width > image.height)
    {
      camera->filmWidth = (f32)image.width / (f32)image.height;
    }
  else if (image.height > image.width)
    {
      camera->filmHeight = (f32)image.height / (f32)image.width;
    }
  camera->filmWidth *= camera->focusDist;
  camera->filmHeight *= camera->focusDist;
  camera->halfFilmWidth = 0.5f * camera->filmWidth;
  camera->halfFilmHeight = 0.5f * camera->filmHeight;
  camera->halfPixelWidth = camera->halfFilmWidth * (1.0f / (f32)image.width);
  camera->halfPixelHeight = camera->halfFilmHeight * (1.0f / (f32)image.height);

  return camera;
}
