#include "world.h"
#include "BLMath.h"
#include "SIMD.h"

static lane_f32 rayTolerance = laneF32FromF32( 0.00001f );
static lane_f32 minHitDistance = laneF32FromF32( 0.00001f );


inline static lane_u32 rayTriangleTest(Triangle triangle,
				  lane_v3* rayD,
				  lane_v3* rayO,
				   lane_f32* mDist,
				   lane_f32* dist)				       
{
  lane_v3 rayDirection = *rayD;
  lane_v3 rayOrigin = *rayO;
  lane_f32 minDist = *mDist;
  lane_v3 v0;
  lane_v3 v1;
  lane_v3 v2;
  lane_v3 normal;
  v0 = triangle.v0;
  v1 = triangle.v1;
  v2 = triangle.v2;
  normal = triangle.normal; //normalize(cross(v1-v0, v2-v0));
	      
  lane_f32 denom = dot(normal, rayDirection);
  lane_u32 toleranceMask = (denom > rayTolerance) | (denom < -rayTolerance);

  //currently slightly faster without this condition
  //if (!MaskAllZeros(toleranceMask))
  {
    lane_f32 triangleOffset; //like the planeDist but for the triangle
    triangleOffset = -dot(normal, v0);
    lane_f32 triangleDist;
    triangleDist = -(dot(normal, rayOrigin) + triangleOffset) / denom; 
		
    lane_u32 planeHitMask;
    planeHitMask = (triangleDist > minHitDistance) & (triangleDist < minDist);
    if (!MaskAllZeros(planeHitMask))
      {
	lane_u32 triangleHitMask;
	triangleHitMask = 0xFFFFFFFF;
		  
	lane_v3 planePoint;
	planePoint = (rayDirection * triangleDist) + rayOrigin;

	lane_v3 edgePerp;
		  
	lane_v3 edge0 = v1 - v0;
	edgePerp = cross(edge0, planePoint - v0);
	triangleHitMask &= dot(normal, edgePerp) > laneF32FromF32(0.0f);

	lane_v3 edge1 = v2 - v1;
	edgePerp = cross(edge1, planePoint - v1);
	triangleHitMask &= dot(normal, edgePerp) > laneF32FromF32(0.0f);

	lane_v3 edge2 = v0 - v2;
	edgePerp = cross(edge2, planePoint - v2);
	triangleHitMask &= dot(normal, edgePerp) > laneF32FromF32(0.0f);

	*dist = triangleDist;
	return triangleHitMask & planeHitMask;

	//*dist = triangleDist;
	//return hitMask;
      }
    return laneU32FromU32(0);
  }
}

inline static lane_u32 raySphereTest(Sphere sphere,
				  lane_v3* rayD,
				  lane_v3* rayO,
				   lane_f32* mDist,
				   lane_f32* dist)
{
  lane_v3 rayDirection = *rayD;
  lane_v3 rayOrigin = *rayO;
  lane_f32 minDist = *mDist;
  lane_v3 spherePos;
  spherePos = sphere.position;
  lane_f32 sphereRadius;
  sphereRadius = sphere.radius;
	  
  lane_v3 relativeSpherePos = rayOrigin - spherePos;
  lane_f32 a = dot(rayDirection, rayDirection);
  lane_f32 b = 2*dot(rayDirection, relativeSpherePos);
  lane_f32 c = dot(relativeSpherePos, relativeSpherePos) - sphereRadius * sphereRadius;

  lane_f32 root = b*b - 4*a*c;
  lane_u32 rootMask = root > rayTolerance;	  
  if (!MaskAllZeros(rootMask))
    {
      //lane_f32 farDist = (-b + sqrt(root)) / 2*a;	      
      lane_f32 nearDist = (-b - sqrt(root)) / 2*a;
      lane_u32 distMask = (nearDist > minHitDistance) & (nearDist < minDist);
      *dist = nearDist;
      return rootMask & distMask;
    }
  return rootMask;
}
inline static lane_u32 rayPlaneTest(Plane plane,
				  lane_v3* rayD,
				  lane_v3* rayO,
				   lane_f32* mDist,
				   lane_f32* dist)
{
  lane_v3 rayDirection = *rayD;
  lane_v3 rayOrigin = *rayO;
  lane_f32 minDist = *mDist;

  lane_v3 planeNormal;
  planeNormal = plane.normal;
	  
  lane_f32 denom = dot(planeNormal, rayDirection);
  lane_u32 toleranceMask = (denom > rayTolerance) | (denom < -rayTolerance);

  //currently slightly faster without this condition
  //if (!MaskAllZeros(toleranceMask))
    lane_f32 planeDist;
    planeDist = plane.dist;

    lane_f32 pDist = (-planeDist - dot(planeNormal, rayOrigin)) / denom;
    lane_u32 distMask = (pDist > minHitDistance) & (pDist < minDist);
    *dist = pDist;
    return toleranceMask & distMask; 
}
