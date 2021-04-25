#ifndef __SIMD_LANE_HEADER
#define __SIMD_LANE_HEADER


#include <stdint.h>
#include "Math.h"



#define LANE_WIDTH 8

#if (LANE_WIDTH == 8)

    #include "SIMD8Wide.h"

#elif (LANE_WIDTH == 4)

    #include "SIMD4Wide.h"

#elif (LANE_WIDTH == 1)

    #include "SIMD1Wide.h"

#else

#error Only LANE_WIDTH of 1 or 4 supported!

#endif //LANE_WIDTH

#if (LANE_WIDTH != 1)

//uints
lane_u32 operator+(lane_u32 A, u32 B)
{
  lane_u32 result = A + laneU32FromU32(B);
  return result;
}
lane_u32 operator+(u32 A, lane_u32 B)
{
  lane_u32 result = B + laneU32FromU32(A);
  return result;
}
lane_u32 operator+=(lane_u32& A, lane_u32 B)
{
  A = A + B;
  return A;
}
lane_u32 operator-(lane_u32 A, u32 B)
{
  lane_u32 result = A - laneU32FromU32(B);
  return result;
}
lane_u32 operator-(u32 A, lane_u32 B)
{
  lane_u32 result = laneU32FromU32(A) - B;
  return result;
}
lane_u32 operator-=(lane_u32& A, lane_u32 B)
{
  A = A - B;
  return A;
}
lane_u32 operator*(lane_u32 A, u32 B)
{
  lane_u32 result = A * laneU32FromU32(B);
  return result;
}
lane_u32 operator*(u32 A, lane_u32 B)
{
  lane_u32 result = laneU32FromU32(A) * B;
  return result;
}
lane_u32 operator*=(lane_u32& A, lane_u32 B)
{
  A = A + B;
  return A;
}
lane_u32 operator|=(lane_u32& A, lane_u32 B)
{
  A = A | B;
  return A;
}
lane_u32 operator&=(lane_u32& A, lane_u32 B)
{
  A = A & B;
  return A;
}
lane_u32 operator^=(lane_u32& A, lane_u32 B)
{
  A = A ^ B;
  return A;
}
lane_u32& lane_u32::operator=(u32 A)
{
  *this = laneU32FromU32(A);
  return *this;
}





//floats
lane_f32 operator+(lane_f32 A, f32 B)
{
  lane_f32 result = A + laneF32FromF32(B);
  return result;
}
lane_f32 operator+(f32 A, lane_f32 B)
{
  lane_f32 result = B + laneF32FromF32(A);
  return result;
}
lane_f32 operator+=(lane_f32& A, lane_f32 B)
{
  A = A + B;
  return A;
}
lane_f32 operator-(lane_f32 A, f32 B)
{
  lane_f32 result = A - laneF32FromF32(B);
  return result;
}
lane_f32 operator-(f32 A, lane_f32 B)
{
  lane_f32 result = laneF32FromF32(A) - B;
  return result;
}
lane_f32 operator-=(lane_f32& A, lane_f32 B)
{
  A = A - B;
  return A;
}
lane_f32 operator*(lane_f32 A, f32 B)
{
  lane_f32 result = A * laneF32FromF32(B);
  return result;
}
lane_f32 operator*(f32 A, lane_f32 B)
{
  lane_f32 result = laneF32FromF32(A) * B;
  return result;
}
lane_f32 operator*=(lane_f32& A, lane_f32 B)
{
  A = A * B;
  return A;
}
lane_f32 operator/(lane_f32 A, f32 B)
{
  lane_f32 result = A / laneF32FromF32(B);
  return result;
}
lane_f32 operator/(f32 A, lane_f32 B)
{
  lane_f32 result = laneF32FromF32(A) / B;
  return result;
}
lane_f32 operator/=(lane_f32& A, lane_f32 B)
{
  A = A / B;
  return A;
}
lane_f32& lane_f32::operator=(f32 A)
{
  *this = laneF32FromF32(A);
  return *this;
}
lane_f32 operator-(lane_f32 A)
{
  lane_f32 result = laneF32FromF32(0.0f);
  result = result - A;
  return result;
}

//vec3s
lane_v3 operator+(lane_v3 A, lane_v3 B)
{
  lane_v3 result;
  result.X = A.X + B.X;
  result.Y = A.Y + B.Y;
  result.Z = A.Z + B.Z;
  return result;
}
lane_v3 operator-(lane_v3 A, lane_v3 B)
{
  lane_v3 result;
  result.X = A.X - B.X;
  result.Y = A.Y - B.Y;
  result.Z = A.Z - B.Z;
  return result;
}
lane_v3 operator*(lane_v3 A, lane_v3 B)
{
  lane_v3 result;
  result.X = A.X - B.X;
  result.Y = A.Y - B.Y;
  result.Z = A.Z - B.Z;
  return result;
}

lane_v3 operator/(lane_v3 A, lane_v3 B)
{
  lane_v3 result;
  result.X = A.X - B.X;
  result.Y = A.Y - B.Y;
  result.Z = A.Z - B.Z;
  return result;
}

lane_v3 operator+(lane_v3 A, vec3 B)
{
  lane_v3 result = A + laneV3FromV3(B);
  return result;
}
lane_v3 operator+(vec3 A, lane_v3 B)
{
  lane_v3 result = B + laneV3FromV3(A);
  return result;
}
lane_v3 operator+=(lane_v3& A, lane_v3 B)
{
  A = A + B;
  return A;
}
lane_v3 operator-(lane_v3 A, vec3 B)
{
  lane_v3 result = A - laneV3FromV3(B);
  return result;
}
lane_v3 operator-(vec3 A, lane_v3 B)
{
  lane_v3 result = laneV3FromV3(A) - B;
  return result;
}
lane_v3 operator-=(lane_v3& A, lane_v3 B)
{
  A = A - B;
  return A;
}
lane_v3 operator*(lane_v3 A, vec3 B)
{
  lane_v3 result = A * laneV3FromV3(B);
  return result;
}
lane_v3 operator*(vec3 A, lane_v3 B)
{
  lane_v3 result = laneV3FromV3(A) * B;
  return result;
}
lane_v3 operator*(lane_v3 A, lane_f32 B)
{
  lane_v3 result;
  result.X = A.X * B;
  result.Y = A.Y * B;
  result.Z = A.Z * B;
  return result;
}
lane_v3 operator*(lane_f32 A, lane_v3 B)
{
  lane_v3 result;
  result.X = A * B.X;
  result.Y = A * B.Y;
  result.Z = A * B.Z;
  return result;
}
lane_v3 operator*(lane_v3 A, f32 B)
{
  lane_v3 result;
  lane_f32 f = laneF32FromF32(B);
  result.X = A.X * f;
  result.Y = A.Y * f;
  result.Z = A.Z * f;
  return result;
}
lane_v3 operator*(f32 A, lane_v3 B)
{
  lane_v3 result;
  lane_f32 f = laneF32FromF32(A);
  result.X = f * B.X;
  result.Y = f * B.Y;
  result.Z = f * B.Z;
  return result;
}
lane_v3 operator*=(lane_v3& A, lane_v3 B)
{
  A = A * B;
  return A;
}
lane_v3 operator/(lane_v3 A, vec3 B)
{
  lane_v3 result = A / laneV3FromV3(B);
  return result;
}
lane_v3 operator/(vec3 A, lane_v3 B)
{
  lane_v3 result = laneV3FromV3(A) / B;
  return result;
}
lane_v3 operator/(lane_v3 A, lane_f32 B)
{
  lane_v3 result;
  result.X = A.X / B;
  result.Y = A.Y / B;
  result.Z = A.Z / B;
  return result;
}
lane_v3 operator/(lane_f32 A, lane_v3 B)
{
  lane_v3 result = A / B;
  return result;
}
lane_v3 operator/=(lane_v3& A, lane_v3 B)
{
  A = A / B;
  return A;
}
lane_v3& lane_v3::operator=(vec3 A)
{
  *this = laneV3FromV3(A);
  return *this;
}
///MATH
inline lane_f32 dot(lane_v3 a, lane_v3 b)
{
  return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
}

inline lane_v3 hadamard(lane_v3 a, lane_v3 b)
{
  lane_v3 result;
  result.X = a.X * b.X;
  result.Y = a.Y * b.Y;
  result.Z = a.Z * b.Z;
  return result;
}
inline lane_v3 cross(lane_v3 a, lane_v3 b)
{
  lane_v3 result;
  result.X = a.Y*b.Z - a.Z*b.Y;
  result.Y = a.Z*b.X - a.X*b.Z;
  result.Z = a.X*b.Y - a.Y*b.X;
  return result;
}

inline lane_f32 length(lane_v3 a)
{
  return sqrt(dot(a,a));
}

inline lane_v3 normalize(lane_v3 a)
{
  lane_v3 result;
  result = a / length(a);
  return result;
}

inline lane_v3 lerp(lane_v3 min, lane_v3 max, lane_f32 value)
{
  return min + (max-min)*value;
}

lane_v3 gatherV3_(void* basePointer, u32 stride, lane_u32 indices)
{
  lane_v3 result;
  result.X = gatherF32_((f32*)basePointer + 0, stride, indices);
  result.Y = gatherF32_((f32*)basePointer + 1, stride, indices);
  result.Z = gatherF32_((f32*)basePointer + 2, stride, indices);
  return result;
}

vec3 HorizontalAdd(lane_v3 A)
{
  vec3 result;
  result.x = HorizontalAdd(A.X);
  result.y = HorizontalAdd(A.Y);
  result.z = HorizontalAdd(A.Z);
  return result;
  
}
lane_v3 operator&(lane_u32 A, lane_v3 B)
{
  lane_v3 result;
  result.X = A & B.X;
  result.Y = A & B.Y;
  result.Z = A & B.Z;
  return result;
}

lane_f32 maskLaneF32(lane_f32 target, lane_u32 mask)
{
  lane_u32 uresult = mask & *(lane_u32*)&target;
  lane_f32 result = *(lane_f32*)&uresult;
  return result;
}


lane_v3 maskLaneV3(lane_v3 target, lane_u32 mask)
{
  lane_v3 result;
  result.X = maskLaneF32(target.X, mask);
  result.Y = maskLaneF32(target.Y, mask);
  result.Z = maskLaneF32(target.Z, mask);
  return result;  
}

lane_u32 xorshift32(lane_u32 *state)
{
  /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
  lane_u32 x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return *state = x;
}

inline lane_f32 randomBilateral32(lane_u32 *state)
{
  return 2.0f * laneF32FromLaneU32(xorshift32(state)) / ((u32)-1);
}

inline lane_f32 randomUnilateral32(lane_u32 *state)
{
  return (randomBilateral32(state) + 1.0f) / 2.0f;
}

#endif //LANEWIDTH != 1

#define gatherF32(basePointer, index, member) gatherF32_(&(basePointer)->member, sizeof(*basePointer), index)
#define gatherV3(basePointer, index, member) gatherV3_(&(basePointer)->member, sizeof(*basePointer), index)
  


#endif //__SIMD_LANE_HEADER
