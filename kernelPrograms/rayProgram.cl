
#define WORLD_SPHERE_COUNT 32
#define WORLD_PLANE_COUNT 8
#define WORLD_MATERIAL_COUNT 8
#define WORLD_TRIANGLE_COUNT 4086
#define WORLD_DIRECTIONAL_LIGHT_COUNT 4
#define WORLD_LINE_COUNT 2048
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

typedef struct __attribute__((packed))_cl_Line
{
  float3 origin;
  float3 direction;
  float length;
}clLine;

typedef struct __attribute__((packed))_clMaterial
{
  float scatterScale;
  float3 emitColour;
  float3 reflectColour;
}clMaterial;

typedef struct __attribute__((packed))_clDirectionalLight
{
  float3 direction;
  float3 colour;
}clDirectionalLight;


typedef struct __attribute__((packed))_clObject
{
  uint planes[1];//indices into world
  uint spheres[2];
  uint triangles[64];
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


typedef struct __attribute__((packed))_clSpatialHeirarchy
{
  clSpatialBox boxes[SPATIAL_BOX_COUNT];
  clObject objects[SPATIAL_BOX_COUNT];
  float3 planeNormals[7];
  uint objectCount;
}clSpatialHeirarchy;

#define BVH_ITEM_COUNT 2048
#define BVH_DIGIT_COUNT 3

typedef struct __attribute__((packed))_clBVHNode
{
  float3 center;
  float3 dimensions;
  float dist;
  uint depth;
}clBVHNode;

typedef struct __attribute__((packed))_clBVHItem
{
  uint triangleIndex;
  uint mortonCode;
}clBVHItem;
//add all indices
//sort by codes
//Build heirarchy from codes
typedef struct __attribute__((packed))_clBVH
{
  float3 center;
  float3 dimensions; //radius in each axis Each split divides in 2
  clBVHItem items[BVH_ITEM_COUNT];
  uint indices[(1 << (BVH_DIGIT_COUNT + 1)) - 1]; //each code, and where it points to in items
  uint itemCount;
}clBVH;


typedef struct __attribute__((packed))_clWorld
{
  clPlane planes[WORLD_PLANE_COUNT];
  clSphere spheres[WORLD_SPHERE_COUNT];
  clMaterial materials[WORLD_MATERIAL_COUNT];
  clTriangle triangles[WORLD_TRIANGLE_COUNT];
  clDirectionalLight dLights[WORLD_DIRECTIONAL_LIGHT_COUNT];
  clLine lines[WORLD_LINE_COUNT];
  int dLightCount;
  int planeCount;
  int sphereCount;
  int materialCount;
  int triangleCount;
  int lineCount;
  uint totalTileCount;
  volatile uint bounceCount;
  volatile uint pad;
  volatile ulong tilesCompleted;
  //clSpatialHeirarchy SH;
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

inline static uint rayTriangleTest(clTriangle triangle,\
                                   float3 rayDirection, float3 rayOrigin,\
                                   float minDist,\
                                   float* dist)
{
    float tolerance = 0.00001;
  float minHitDistance = 0.00001;

  float3 v0 = triangle.v0;
  float3 v1 = triangle.v1;
  float3 v2 = triangle.v2;
  float3 normal = triangle.normal;
	      
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
    if (planeHitMask & toleranceMask)
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
            *dist = triangleDist;
            return hitMask;
          }
      }
  }
  return 0;

}


//get the matIndex of the first thing we hit, not necessarily the closest one
uint rayCast( __global clWorld* world, __constant clSpatialHeirarchy* SH, float3 rayOrigin, float3 rayDirection)
{
  float minDist = FLT_MAX;
  uint matIndex = 0;
  float tolerance = 0.00001;
  float minHitDistance = 0.00001;
  //check which spatial box to iterate through
  for (uint boxIndex = 0; boxIndex < SH->objectCount; boxIndex++)
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
          planeNormal = SH->planeNormals[boundingPlane];
		  	  
          float denom = dot(planeNormal, rayDirection);
                  
          float planeNearOffset;
          planeNearOffset = SH->boxes[boxIndex].planeDistances[boundingPlane][0];

          float planeFarOffset;
          planeFarOffset  = SH->boxes[boxIndex].planeDistances[boundingPlane][1];

          float numerator;
          numerator = dot(planeNormal, rayOrigin);
		    
          float farDist = (-planeFarOffset - numerator) / denom;
          float nearDist = (-planeNearOffset - numerator) / denom;

          //conditionally swap the distances if denom is > 0
          if ( denom >= 0.0 )
            {
              float temp = farDist;
              farDist = nearDist;
              nearDist = temp;
            }

          boxHitDistNear = max(nearDist, boxHitDistNear);
          boxHitDistFar = min(farDist, boxHitDistFar);
		    
          uint distMask = boxHitDistNear < boxHitDistFar;
          hitMask &= distMask;
          if (!hitMask)
            {
              break;
            }
        }

      //at least one ray hit this box
      if (hitMask) {

        clObject object = SH->objects[boxIndex];

        //iterate over all planes to see if they intersect
        for (int i = 0; i < object.planeCount; i++)
          {
            clPlane plane = world->planes[object.planes[i]];
            float denom = dot(plane.normal, rayDirection);
            //if( (denom > tolerance) | (denom < -tolerance))
              {
                float dist = (-plane.dist - dot(plane.normal, rayOrigin)) / denom;
                if ((dist > minHitDistance) & (dist < minDist))
                  {
                    matIndex = plane.matIndex;
                    return matIndex;
                  }
              }
          }
        //iterate over all spheres
        for (int i = 0; i < object.sphereCount; i++)
          {
            clSphere sphere = world->spheres[object.spheres[i]];
	  
            float3 relativeSpherePos = rayOrigin - sphere.position;
            float a = dot(rayDirection, rayDirection);
            float b = 2*dot(rayDirection, relativeSpherePos);
            float c = dot(relativeSpherePos, relativeSpherePos) - sphere.radius * sphere.radius;

            float root = b*b - 4*a*c;
            uint rootMask = root > tolerance;
            if (rootMask)
              {
                float dist = (-b - sqrt(root)) / 2*a;
                uint distMask = dist > minHitDistance & dist < minDist;
                uint hitMask = distMask && rootMask;
                if (hitMask)
                  {
                    matIndex = sphere.matIndex;
                    return matIndex;
                  }
              }
              }
        for (int i = 0; i < object.triangleCount; i++)
          {
            clTriangle triangle = world->triangles[object.triangles[i]];

            float triangleDist;
            uint hitMask = rayTriangleTest(triangle, rayDirection, rayOrigin, minDist, &triangleDist);
		  					 
            if (hitMask)
              {
                matIndex = triangle.matIndex;
                return matIndex;
              }
            

          }
      }
    }

  return matIndex;
}

/*

__kernel void rayTrace(__global clWorld* world, __global clCamera* camera, __constant clSpatialHeirarchy* SHParam, uint sampleCount, __write_only image2d_t image)
{  
  int2 pixel = {get_global_id(0), get_global_id(1)};

  
  float2 film = {-1.0 + (2.0 * (float)pixel.x / (float)get_image_width(image)),
    -1.0 + (2.0 * (float)pixel.y / (float)get_image_height(image))};

  uint entropy = (pixel.x + 9123891 * 912) * (pixel.y * 19275 - 1923);
  float tolerance = 0.0001;
  float minHitDistance = 0.0001;
  float3 finalColour = {0,0,0};
  uint bouncesComputed = 0;
  __local clSpatialHeirarchy SH;
  SH = *SHParam;
  
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
      for (uint bounceCount = 0; bounceCount < 3; bounceCount++)
	{	                   
	  float minDist = FLT_MAX;
	  uint matIndex = 0;
	  bouncesComputed++;

          //check which spatial box to iterate through
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
                  
                  float planeNearOffset;
                  planeNearOffset = SH.boxes[boxIndex].planeDistances[boundingPlane][0];

                  float planeFarOffset;
                  planeFarOffset  = SH.boxes[boxIndex].planeDistances[boundingPlane][1];

                  float numerator;
                  numerator = dot(planeNormal, rayOrigin);
		    
                  float farDist = (-planeFarOffset - numerator) / denom;
                  float nearDist = (-planeNearOffset - numerator) / denom;

                  //conditionally swap the distances if denom is > 0
                  if ( denom >= 0.0 )
                    {
                      float temp = farDist;
                      farDist = nearDist;
                      nearDist = temp;
                    }

                  boxHitDistNear = max(nearDist, boxHitDistNear);
                  boxHitDistFar = min(farDist, boxHitDistFar);
		    
                  uint distMask = boxHitDistNear < boxHitDistFar;
                  hitMask &= distMask;
                  if (!hitMask)
                    {
                      //TODO: try taking out break
                      break;
                    }
                }

	      //at least one ray hit this box
	      if (hitMask)
              {

		clObject object = SH.objects[boxIndex];

                //iterate over all planes to see if they intersect
                for (int i = 0; i < object.planeCount; i++)
                  {
                    clPlane plane = world->planes[object.planes[i]];
                    float denom = dot(plane.normal, rayDirection);
                    //if( (denom > tolerance) | (denom < -tolerance))
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
                for (int i = 0; i < object.sphereCount; i++)
                  {
                    clSphere sphere = world->spheres[object.spheres[i]];
	  
                    float3 relativeSpherePos = rayOrigin - sphere.position;
                    float a = dot(rayDirection, rayDirection);
                    float b = 2*dot(rayDirection, relativeSpherePos);
                    float c = dot(relativeSpherePos, relativeSpherePos) - sphere.radius * sphere.radius;

                    float root = b*b - 4*a*c;
                    uint rootMask = root > tolerance;
                    //if (rootMask)
                      {
                        float dist = (-b - sqrt(root)) / 2*a;
                        uint distMask = dist > minHitDistance & dist < minDist;
                        uint hitMask = distMask && rootMask;
                        if (hitMask)
                          {
                            minDist = dist;
                            bounceNormal = normalize((rayOrigin + rayDirection * dist) - sphere.position);
                            matIndex = sphere.matIndex;
                          }
                      }
                  }

                for (int i = 0; i < object.triangleCount; i++)
                  {
                    clTriangle triangle = world->triangles[object.triangles[i]];

                    float3 v0 = triangle.v0;
                    float3 v1 = triangle.v1;
                    float3 v2 = triangle.v2;
                    //v2 = triangle.normal;
                    float3 normal = triangle.normal;
	      
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
                      if (planeHitMask & toleranceMask)
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
              }
          }

          if (bounceCount == 0)
            {
              for (uint i = 0; i < world->lineCount; i++)
                {
                  clLine line = world->lines[i];
                  float3 lineDir;
                  lineDir = line.direction;
                  float3 lineOrigin;
                  lineOrigin = line.origin;

                  float3 perpDir = cross(rayDirection, lineDir);

                  float3 n2 = cross(lineDir, perpDir);
                  float3 c1 = rayOrigin + rayDirection * (dot(lineOrigin - rayOrigin, n2) / dot(rayDirection, n2));

                  float3 n1 = cross(rayDirection, perpDir);
                  float3 c2 = lineOrigin + lineDir * (dot(rayOrigin - lineOrigin, n1) / dot(lineDir, n1));

                  float dist = length(c2 - c1);
                  float3 rayOriginToPoint = c1 - rayOrigin;
                  float3 lineOriginToPoint = c2 - lineOrigin;

                  uint distMask = dist < 0.01;
		
                  uint minDistMask = (length(rayOriginToPoint) < minDist) & (dot(rayDirection, rayOriginToPoint) > 0.0);
                  uint lengthMask = (length(lineOriginToPoint) < line.length) & (dot(lineDir, lineOriginToPoint) > 0.0f);
		
                  uint hitMask = distMask & minDistMask & lengthMask;

                  if (hitMask) {
                    matIndex = 3;
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
	      float cosAttenuation = Max(dot(rayDirection*(-1.0f), bounceNormal), 0.3);

	     
	      //update attenuation based on reflection colour
	      attenuation = mat.reflectColour * (attenuation*cosAttenuation);

	      //setup for next bounce
	      rayOrigin = rayOrigin + rayDirection * minDist;

              for (uint dlightIndex = 0; dlightIndex < world->dLightCount; dlightIndex++)
		{
		  clDirectionalLight dlight = world->dLights[dlightIndex];

		  uint inDirection = dot(bounceNormal, dlight.direction) < -tolerance;
		  if (inDirection) {
		  
		    float3 oppositeLightDir = -dlight.direction;
                    float3 shadowRayOrigin = rayOrigin + bounceNormal * 0.01f;                    
		    uint lightMatIndex = rayCast(world, SHParam, shadowRayOrigin, oppositeLightDir);

		    uint hitSomething = lightMatIndex != 0;		  
		    float3 emitColour =  dlight.colour;// * !hitSomething;
                    if (hitSomething)
                      emitColour = world->materials[0].emitColour;
		    resultColour += emitColour * attenuation;
                    bouncesComputed++;
		  }
		}

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
*/
uint positionToMortonCode(__constant clBVH* bvh, float3 position)
{
  //get coordinates scaling from 0 to b as integer on each axis
  uint b = 1 << ((BVH_DIGIT_COUNT / 3) - 1);
  float3 offsetCenter = position + bvh->center + bvh->dimensions;
  float3 scaledCenter = offsetCenter * b / bvh->dimensions;
  int3 mortonCenter = { (uint)scaledCenter.x,
    (uint)scaledCenter.y,
    (uint)scaledCenter.z };
  uint code = 0;
      
  //interleave the bits
  for (uint digit = 0; digit < BVH_DIGIT_COUNT / 3; digit ++)
    {
      uint bit = (mortonCenter.x & (1 << (digit))) != 0;
      code |= bit << (digit*3 + 2);
      bit = (mortonCenter.y & (1 << (digit))) != 0;
      code |= bit << (digit*3 + 1);
      bit = (mortonCenter.z & (1 << (digit))) != 0;
      code |= bit << (digit*3 + 0);
    }
  return code;
}

inline static uint rayAABBTest(float3 center, float3 dimensions,
                               float3 rayDirection,
                               float3 rayOrigin,
                               float* dist)
{
  //float3 rayDirection = *rayD;
  float3 rayInvDir = float3(1.0f,1.0f,1.0f) / rayDirection;
  //float3 rayOrigin = *rayO;
  

  float3 minimums = center - dimensions;
  float3 maximums = center + dimensions;
  
  //aabb test using slab method from
  //https://tavianator.com/2011/ray_box.html

  float3 tMax = (maximums - rayOrigin) *rayInvDir;
  float3 tMin = (minimums - rayOrigin) * rayInvDir;
  
  float minDist = min(tMin.x, tMax.x);
  float maxDist = max(tMin.x, tMax.x);

  minDist = max(minDist, min(tMin.y, tMax.y));
  maxDist = min(maxDist, max(tMin.y, tMax.y));

  minDist = max(minDist, min(tMin.z, tMax.z));
  maxDist = min(maxDist, max(tMin.z, tMax.z));

  uint hitBox = (minDist < maxDist);
  if (hitBox)
    {
      if (minDist < 0)
        {
          *dist = maxDist;
        }
      else
        {
          *dist = minDist;
        }
    }

  return hitBox;
}


__kernel void rayTrace(__global clWorld* world, __global clCamera* camera, __constant clBVH* bvh, uint sampleCount, __write_only image2d_t image)
{  
  int2 pixel = {get_global_id(0), get_global_id(1)};

  
  float2 film = {-1.0 + (2.0 * (float)pixel.x / (float)get_image_width(image)),
    -1.0 + (2.0 * (float)pixel.y / (float)get_image_height(image))};

  uint entropy = (pixel.x + 9123891 * 912) * (pixel.y * 19275 - 1923);
  float tolerance = 0.0001;
  float minHitDistance = 0.0001;
  float3 finalColour = {0,0,0};
  uint bouncesComputed = 0;
  
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
      for (uint bounceCount = 0; bounceCount < 3; bounceCount++)
	{	                   
	  float minDist = FLT_MAX;
	  uint matIndex = 0;
	  bouncesComputed++;
          
	  clBVHNode boxStack[(1 << (BVH_DIGIT_COUNT))];
	  clBVHNode* initial = boxStack;
	  initial->center = bvh->center;
	  initial->dimensions = bvh->dimensions;
	  initial->depth = 0;
          

          uint hitNode = rayAABBTest(initial->center, initial->dimensions,
                                     rayDirection, rayOrigin,
                                     &initial->dist);
          
	  uint stackCount = hitNode;
          uint hitATriangle = 0;
	  while (stackCount > 0 && !hitATriangle)
	    {
              stackCount--;
              float minNodeDist = FLT_MAX;
              uint boxIndex = 0;
              for (int i = stackCount; i >= 0; i--)
                {
                  if (boxStack[i].dist < minNodeDist)
                    {
                      boxIndex = i;
                    }
                }
                            
	      clBVHNode node = boxStack[boxIndex];
              boxStack[boxIndex] = boxStack[stackCount];
              //test intersection
              //if leaf trace it
              if (node.depth == BVH_DIGIT_COUNT / 3)
                {
                  //Get code from node's position
                  uint mortonCode = positionToMortonCode(bvh, node.center);
                  uint index = bvh->indices[mortonCode];
                  //matIndex=3;
                  //Start at index and continue until different leaf
                  while (bvh->items[index].mortonCode == mortonCode)
                    {
                      clTriangle triangle = world->triangles[bvh->items[index].triangleIndex];
                      float triangleDist;
                      uint hitMask = rayTriangleTest(triangle, rayDirection, rayOrigin, minDist, &triangleDist);
                      if (hitMask)
                        {		
                          minDist = triangleDist;
                          bounceNormal = triangle.normal;
                          matIndex = triangle.matIndex;
                          hitATriangle = 1;
                        }
                      index++;
                    }
                }
              //otherwise split to stack
              else
                {
                  for (int x = -1; x < 2; x+= 2)
                    {
                      for (int y = -1; y < 2; y+= 2)
                        {
                          for (int z = -1; z < 2; z+= 2)				
                            {
                              float3 nextDims = node.dimensions / 2.0f;
                              float3 nextCenter =
                                {
                                  node.center.x + (nextDims.x * x),
                                  node.center.y + (nextDims.y * y),
                                  node.center.z + (nextDims.z * z)
                                };

                              float nodeDist;
                              uint hitNode = rayAABBTest(node.center, node.dimensions,
                                                         rayDirection, rayOrigin,
                                                         &nodeDist);
                              if (hitNode)
                                {
                                  clBVHNode split = {};
                                  split.depth = node.depth + 1;
                                  split.center = nextCenter;
                                  split.dimensions = nextDims;
                                  split.dist = nodeDist;
                                  boxStack[stackCount] = split;
                                  stackCount++;
                                }
                            }
                        }
                    }
                }

            }


          /*
          if (bounceCount == 0)
            {
              for (uint i = 0; i < world->lineCount; i++)
                {
                  clLine line = world->lines[i];
                  float3 lineDir;
                  lineDir = line.direction;
                  float3 lineOrigin;
                  lineOrigin = line.origin;

                  float3 perpDir = cross(rayDirection, lineDir);

                  float3 n2 = cross(lineDir, perpDir);
                  float3 c1 = rayOrigin + rayDirection * (dot(lineOrigin - rayOrigin, n2) / dot(rayDirection, n2));

                  float3 n1 = cross(rayDirection, perpDir);
                  float3 c2 = lineOrigin + lineDir * (dot(rayOrigin - lineOrigin, n1) / dot(lineDir, n1));

                  float dist = length(c2 - c1);
                  float3 rayOriginToPoint = c1 - rayOrigin;
                  float3 lineOriginToPoint = c2 - lineOrigin;

                  uint distMask = dist < 0.01;
		
                  uint minDistMask = (length(rayOriginToPoint) < minDist) & (dot(rayDirection, rayOriginToPoint) > 0.0);
                  uint lengthMask = (length(lineOriginToPoint) < line.length) & (dot(lineDir, lineOriginToPoint) > 0.0f);
		
                  uint hitMask = distMask & minDistMask & lengthMask;

                  if (hitMask) {
                    matIndex = 3;
                  }
                }
            }
          */
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
	      float cosAttenuation = Max(dot(rayDirection*(-1.0f), bounceNormal), 0.3);

	     
	      //update attenuation based on reflection colour
	      attenuation = mat.reflectColour * (attenuation*cosAttenuation);

	      //setup for next bounce
	      rayOrigin = rayOrigin + rayDirection * minDist;
              /*
              for (uint dlightIndex = 0; dlightIndex < world->dLightCount; dlightIndex++)
		{
		  clDirectionalLight dlight = world->dLights[dlightIndex];

		  uint inDirection = dot(bounceNormal, dlight.direction) < -tolerance;
		  if (inDirection) {
		  
		    float3 oppositeLightDir = -dlight.direction;
                    float3 shadowRayOrigin = rayOrigin + bounceNormal * 0.01f;                    
		    uint lightMatIndex = rayCast(world, SHParam, shadowRayOrigin, oppositeLightDir);

		    uint hitSomething = lightMatIndex != 0;		  
		    float3 emitColour =  dlight.colour;// * !hitSomething;
                    if (hitSomething)
                      emitColour = world->materials[0].emitColour;
		    resultColour += emitColour * attenuation;
                    bouncesComputed++;
		  }
		}
              */
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

