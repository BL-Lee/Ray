
typedef struct __attribute__((packed))_clSphere
{
  float3 position;
  float radius;
  int matIndex;
}clSphere;

typedef struct __attribute__((packed))_clPlane
{ 
  float3 normal;
  float dist; //distance along normal
  int matIndex;
}clPlane;

typedef struct __attribute__((packed))_clMaterial
{
  float3 emitColour;
  float3 reflectColour;
  float scatterScale;
}clMaterial;

typedef struct __attribute__((packed))_clWorld
{
  clPlane planes[8];
  clSphere spheres[8];
  clMaterial materials[8];
  volatile uint bounceCount;
  int planeCount;
  int sphereCount;
  int materialCount;
  uint totalTileCount;
}clWorld;

typedef struct __attribute__((packed))_clCamera
{
  float3 pos;
  float3 target;
  float3 Z;
  float3 X;
  float3 Y;

  float3 filmCenter;
  float filmWidth;
  float filmHeight;

  float halfFilmWidth;
  float halfFilmHeight;
  float halfPixelWidth;
  float halfPixelHeight;
}clCamera;

float Max(float a, float b);
uint xorshift32(uint *state);
float randomUnilateral32(uint *state);
float randomBilateral32(uint *state);
float lerp(float min, float max, float value);
float3 lerp3(float3 min, float3 max, float value);

float Max(float a, float b)
{
  return a > b ? a : b;
}
uint xorshift32(uint *state)
{
  /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
  uint x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return *state = x;
}

float randomUnilateral32(uint *state)
{
  return (float)(xorshift32(state)) / ((uint)-1);
}

float randomBilateral32(uint *state)
{
  return 1.0f - 2.0f*randomUnilateral32(state);
}

float lerp(float min, float max, float value)
{
  return min + value*(max-min);
}
float3 lerp3(float3 min, float3 max, float value)
{
  return min + (max-min)*value;
}
float4 BGRAtoRGBA(float4 in)
{
  float4 result;
  result.x = in.z;
  result.y = in.y;
  result.z = in.x;
  result.w = in.w;
  return result;
}

float linearToSRGB(float linear)
{
  float result;
  if (linear < 0) linear = 0;
  else if (linear > 1) linear = 1;
  if (linear <= 0.0031308f)      
    result = linear * 12.92f;	
  else
    result = 1.055f * pow(linear,(float)1/2.4f) - 0.055f;
  return result;  
}

__kernel void rayTrace(__global clWorld* world, __global clCamera* camera,
		       uint sampleCount, __write_only image2d_t image)
{  
  int2 pixel = {get_global_id(0), get_global_id(1)};

  
  float2 film = {-1.0 + (2.0 * (float)pixel.x / (float)get_image_width(image)),
    -1.0 + (2.0 * (float)pixel.y / (float)get_image_height(image))};

  uint entropy = (pixel.x + 9123891 * 912) * (pixel.y * 19275 - 1923);
  //printf("world planeCount:  %d\n", world->planeCount);
  float tolerance = 0.0001;
  float minHitDistance = 0.0001;
  float3 finalColour = {0,0,0};
  uint bouncesComputed = 0;
  
  for (uint i = 0; i < sampleCount; i++)
    {
      float2 pixelOffset = {randomBilateral32(&entropy)*camera->halfPixelWidth,
	                     randomBilateral32(&entropy)*camera->halfPixelHeight};
 	      
      float3 filmPos = camera->filmCenter +
	camera->Y * film.y * camera->halfFilmHeight +
	camera->Y * film.y * pixelOffset.y +
	camera->X * film.x * camera->halfFilmWidth +
	camera->X * film.x * pixelOffset.x;

      
      float3 rayOrigin = camera->pos;
      float3 rayDirection = normalize(filmPos - camera->pos);
      float3 bounceNormal = {};
      float3 resultColour = {0.0,0.0,0.0};
      float3 attenuation = {1.0f,1.0f,1.0f};//how much the colour changes from the bounced material
      //Each time, a ray can bounce this many times
      for (uint bounceCount = 0; bounceCount < 8; bounceCount++)
	{	  
	  
	  float minDist = FLT_MAX;
	  uint matIndex = 0;
	  bouncesComputed++;
	  //iterate over all planes to see if they intersect
	  for (int i = 0; i < world->planeCount; i++)
	    {
	      clPlane plane = world->planes[i];
	      float denom = dot(plane.normal, rayDirection);
	      if( (denom > tolerance) | (denom < -tolerance))
		{
		  float dist = (-plane.dist - dot(plane.normal, rayOrigin)) / denom;
		  if ((dist > minHitDistance) && (dist < minDist))
		    {
		      minDist = dist;
		      bounceNormal = plane.normal;
		      matIndex = plane.matIndex;
		    }
		}
	    }
	  //iterate over all spheres
	  for (int i = 0; i < world->sphereCount; i++)
	    {
	      clSphere sphere = world->spheres[i];
	  
	      float3 relativeSpherePos = rayOrigin - sphere.position;
	      float a = dot(rayDirection, rayDirection);
	      float b = 2*dot(rayDirection, relativeSpherePos);
	      float c = dot(relativeSpherePos, relativeSpherePos) - sphere.radius * sphere.radius;

	      float root = b*b - 4*a*c;
	      if (root > tolerance)
		{
		//float farDist = (-b + sqrt(root)) / 2*a;	      
		  float dist = (-b - sqrt(root)) / 2*a;
		  if (dist > minHitDistance && dist < minDist)
		    {
		      minDist = dist;
		      bounceNormal = normalize((rayOrigin + rayDirection * dist) - sphere.position);
		      matIndex = sphere.matIndex;
		    }
		}
	    }
	  clMaterial mat = world->materials[matIndex];
	  //if matIndex is set, then we hit something
	  resultColour += mat.emitColour * attenuation; // does hadamard product
	  if (!matIndex)
	    {
	      break;
	    }
	  else
	    {
	      //add any colour this object emits, times the attenuation
	      //clamp to 0-inf
	      float cosAttenuation = Max(dot(rayDirection*(-1.0f), bounceNormal), 0);

	     
	      //update attenuation based on reflection colour
	      attenuation = mat.reflectColour * (attenuation*cosAttenuation);

	      //setup for next bounce
	      rayOrigin = rayOrigin + rayDirection * minDist;
	  
	      float3 pureBounce = rayDirection - bounceNormal*2.0f*dot(rayDirection, bounceNormal);
	      //TODO: different noise for random
	      float3 randomDir = {randomBilateral32(&entropy), randomBilateral32(&entropy), randomBilateral32(&entropy)};
	      float3 randomBounce = normalize(bounceNormal + randomDir);
	      rayDirection = lerp3(randomBounce, pureBounce, mat.scatterScale);	  
	    }
	}
      finalColour += resultColour;
    }
  atomic_add(&world->bounceCount, bouncesComputed);
  float4 outColour = {finalColour / sampleCount, 0.0}; //NOTE: BGRA
  outColour.x = linearToSRGB(outColour.x);
  outColour.y = linearToSRGB(outColour.y);
  outColour.z = linearToSRGB(outColour.z);
  write_imagef(image, pixel, BGRAtoRGBA(outColour));
}

