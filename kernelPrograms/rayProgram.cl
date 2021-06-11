
#define WORLD_SPHERE_COUNT 32
#define WORLD_PLANE_COUNT 8
#define WORLD_MATERIAL_COUNT 8
#define WORLD_TRIANGLE_COUNT 1024
#define SPATIAL_BOX_COUNT 64


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

typedef struct __attribute__((packed))_clTriangle
{
  float3 v0;
  float3 v1;
  float3 v2;
  float3 normal;
  int matIndex;
}clTriangle;


typedef struct __attribute__((packed))_clMaterial
{
  float3 emitColour;
  float3 reflectColour;
  float scatterScale;
}clMaterial;

typedef struct __attribute__((packed))_clObject
{
  uint planes[1];//indices into world
  uint spheres[2];
  uint triangles[40];
  int planeCount;
  int sphereCount;
  int triangleCount;
}clObject;

typedef struct __attribute__((packed))_clSpatialBox
{
  //these are in a different area of memory, that way plane Distances are cache friendly
  //might not even need pointer if theyre the same index as the object in SH?
  float planeDistances[7][2];
}clSpatialBox;


typedef struct __attribute__((packed))
{
  clSpatialBox boxes[32];
  clObject objects[32];
  float3 planeNormals[7];
  uint objectCount;
}clSpatialHeirarchy;

typedef struct __attribute__((packed))_clWorld
{
  clPlane planes[WORLD_PLANE_COUNT];
  clSphere spheres[WORLD_SPHERE_COUNT];
  clMaterial materials[WORLD_MATERIAL_COUNT];
  clTriangle triangles[WORLD_TRIANGLE_COUNT];
  volatile uint bounceCount;
  int planeCount;
  int sphereCount;
  int materialCount;
  int triangleCount;
  uint totalTileCount;
  clSpatialHeirarchy SH;
}clWorld;

typedef struct __attribute__((packed))_clCamera
{
  float3 pos;
  float3 target;
  float3 Z;
  float3 X;
  float3 Y;

  float3 filmCenter;
  float focusDist;
  float filmWidth;
  float filmHeight;

  float halfFilmWidth;
  float halfFilmHeight;
  float halfPixelWidth;
  float halfPixelHeight;
  float lensRadius;
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
    float tolerance = 0.0001;
  float minHitDistance = 0.0001;
  float3 finalColour = {0,0,0};
  uint bouncesComputed = 0;
  clSpatialHeirarchy SH = world->SH;
  
  for (uint i = 0; i < sampleCount; i++)
    {
      float2 pixelOffset = {randomUnilateral32(&entropy) * camera->halfPixelWidth * 2.0,
	                     randomUnilateral32(&entropy) * camera->halfPixelHeight * 2.0};
 	      
      float3 filmPos = camera->filmCenter +
	camera->Y * film.y * (camera->halfFilmHeight + pixelOffset.y) +
	camera->X * film.x * camera->halfFilmWidth + pixelOffset.x;

      float3 lensXOffset = {randomBilateral32(&entropy) * camera->lensRadius * camera->X};
      float3 lensYOffset = {randomBilateral32(&entropy) * camera->lensRadius * camera->Y};

      
      float3 rayOrigin = camera->pos + lensXOffset + lensYOffset;
      float3 rayDirection = normalize(filmPos - rayOrigin);      
      float3 bounceNormal = {};
      float3 resultColour = {0.0,0.0,0.0};
      float3 attenuation = {1.0f,1.0f,1.0f};//how much the colour changes from the bounced material
      //Each time, a ray can bounce this many times
      for (uint bounceCount = 0; bounceCount < 8; bounceCount++)
	{	                   
	  float minDist = FLT_MAX;
	  uint matIndex = 0;
	  bouncesComputed++;
          
          //check which spatial box to iterate through
          //cl_SpatialBox box = SH.boxes[0];
          /*          
                      for (uint boxIndex = 0; boxIndex < SH.objectCount; boxIndex++)
                      {
                      float boxHitDistNear;
                      boxHitDistNear = -FLT_MAX;
                      float boxHitDistFar;
                      boxHitDistFar = FLT_MAX;

                      uint hitMask;
                      hitMask = 0xFFFFFFFF;
                      //check if this ray collides with this bounding "box"
                      for (uint boundingPlane = 0; boundingPlane < 7; boundingPlane++)
                      {
                      float3 planeNormal;
                      planeNormal = SH.planeNormals[boundingPlane];
		  	  
                      float denom = dot(planeNormal, rayDirection);
                      uint toleranceMask = (denom > tolerance) || (denom < -tolerance);


                      //currently slightly faster without this condition
                      //if (!MaskAllZeros(toleranceMask))
                      {
                      float planeNearOffset;
                      planeNearOffset = SH.boxes[boxIndex].planeDistances[boundingPlane][0];

                      float planeFarOffset;
                      planeFarOffset  = SH.boxes[boxIndex].planeDistances[boundingPlane][1];


                      float numerator;
                      numerator = dot(planeNormal, rayOrigin);
		    
                      float farDist = (-planeFarOffset - numerator) / denom;
                      float nearDist = (-planeNearOffset - numerator) / denom;

                      //conditionally swap the distances if denom is > 0
                      if (denom > 0.0f) {
                      float temp = farDist;
                      farDist = nearDist;
                      nearDist = temp;
                      }

                      boxHitDistNear = max(nearDist, boxHitDistNear);
                      boxHitDistFar = min(farDist, boxHitDistFar);
		    
                      uint distMask = boxHitDistNear < boxHitDistFar;
                      hitMask = toleranceMask && distMask && hitMask;
                      if (!hitMask)
		      {
                      break;
		      }
                      }
                      }
                      //at least one ray hit this box
                      if (hitMask) {

                      clObject object = SH.objects[boxIndex];
          */
          
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
          for (int i = 0; i < world->triangleCount; i++)	    
            {
              clTriangle triangle = world->triangles[i];

              float3 v0 = triangle.v0;
              float3 v1 = triangle.v1;
              float3 v2 = triangle.v2;
              //v2 = triangle.normal;
              float3 normal = normalize(cross(v1-v0, v2-v0));
	      
              float denom = dot(normal, rayDirection);
              uint toleranceMask = (denom > tolerance) | (denom < -tolerance);

              //if (toleranceMask)
              {
                float triangleOffset; //like the planeDist but for the triangle
                triangleOffset = -dot(normal, v0);
                float triangleDist;
                triangleDist = -(dot(normal, rayOrigin) + triangleOffset) / denom; 
		
                uint planeHitMask;
                planeHitMask = (triangleDist > minHitDistance) & (triangleDist < minDist);
                if (planeHitMask)
                  {
                    uint triangleHitMask;
                    triangleHitMask = 0x1;
		  
                    float3 planePoint;
                    planePoint = (rayDirection * triangleDist) + rayOrigin;

                    float3 edgePerp;
		  
                    float3 edge0 = v1 - v0;
                    edgePerp = cross(edge0, planePoint - v0);
                    triangleHitMask &= dot(normal, edgePerp) > 0.0;

                    float3 edge1 = v2 - v1;
                    edgePerp = cross(edge1, planePoint - v1);
                    triangleHitMask &= dot(normal, edgePerp) > 0.0;

                    float3 edge2 = v0 - v2;
                    edgePerp = cross(edge2, planePoint - v2);
                    triangleHitMask &= dot(normal, edgePerp) > 0.0;

                    uint hitMask = triangleHitMask && planeHitMask;
		  					 
                    if (hitMask)
                      {
                        minDist = triangleDist;
                        bounceNormal = normal;
                        matIndex = triangle.matIndex;
                      }
                  }
              }
            }
        
          //  }
          //}
        
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
	      float cosAttenuation = Max(dot(rayDirection*(-1.0f), bounceNormal), 0.4);

	     
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

