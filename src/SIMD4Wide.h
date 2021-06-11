
#include <immintrin.h>
typedef struct lane_f32
{
  __m128  V;
  lane_f32 &operator=(f32);
}lane_f32;

typedef struct lane_u32
{
  __m128i V;
  lane_u32 &operator=(u32);
}lane_u32;

typedef struct lane_v3
{
  lane_f32  x;
  lane_f32  y;
  lane_f32  z;
  lane_v3 &operator=(vec3);
}lane_v3;

//Loads
lane_f32 laneF32FromF32(f32 A)
{
  lane_f32 result;
  result.V = _mm_set1_ps(A);
  return result;
}
lane_u32 laneU32FromU32(u32 A)
{
  lane_u32 result;
  result.V = _mm_set1_epi32(A);
  return result;
}
lane_v3 laneV3FromV3(vec3 A)
{
  lane_v3 result;
  result.x = A.x;
  result.y = A.y;
  result.z = A.z;
  return result;
}
lane_f32 laneF32FromLaneU32(lane_u32 A)
{
  lane_f32 result;
  result.V = _mm_cvtepi32_ps(A.V);
  return result;
}


//u32 operators
lane_u32 operator+(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm_add_epi32(A.V, B.V);
  return result;
}
lane_u32 operator-(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm_sub_epi32(A.V, B.V);
  return result;
}
lane_u32 operator*(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm_mul_epi32(A.V, B.V);
  return result;
}
lane_u32 operator<<(lane_u32 A, u32 amount)
{
  lane_u32 result;
  result.V = _mm_slli_epi32(A.V, amount);
  return result;
}
lane_u32 operator>>(lane_u32 A, u32 amount)
{
  lane_u32 result;
  result.V = _mm_srli_epi32(A.V, amount);
  return result;
}
lane_u32 operator|(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm_or_si128(A.V, B.V);
  return result;
}

lane_u32 operator&(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm_and_si128(A.V, B.V);
  return result;
}
lane_u32 operator^(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm_xor_si128(A.V, B.V);
  return result;
}
lane_u32 operator<(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm_cmplt_epi32(A.V, B.V);
  return result;
}
lane_u32 operator>(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm_cmpgt_epi32(A.V, B.V);
  return result;
}
lane_u32 operator==(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm_cmpeq_epi32(A.V, B.V);
  return result;
}


//f32 operators
lane_f32 operator+(lane_f32 A, lane_f32 B)
{
  lane_f32 result;
  result.V = _mm_add_ps(A.V, B.V);
  return result;
}
lane_f32 operator-(lane_f32 A, lane_f32 B)
{
  lane_f32 result;
  result.V = _mm_sub_ps(A.V, B.V);
  return result;
}
lane_f32 operator*(lane_f32 A, lane_f32 B)
{
  lane_f32 result;
  result.V = _mm_mul_ps(A.V, B.V);
  return result;
}
lane_f32 operator/(lane_f32 A, lane_f32 B)
{
  lane_f32 result;
  result.V = _mm_div_ps(A.V, B.V);
  return result;
}
lane_u32 operator<(lane_f32 A, lane_f32 B)
{
  lane_u32 result;
  result.V = _mm_castps_si128(_mm_cmplt_ps(A.V, B.V));
  return result;
}
lane_u32 operator>(lane_f32 A, lane_f32 B)
{
  lane_u32 result;
  result.V = _mm_castps_si128(_mm_cmpgt_ps(A.V, B.V));
  return result;
}
lane_f32 operator^(lane_f32 A, lane_f32 B)
{
  lane_f32 result;
  result.V = _mm_xor_ps(A.V, B.V);
  return result;
}
lane_f32 xorLaneF32(lane_f32 A, lane_f32 B)
{
  return A ^ B;
}

lane_f32 operator&(lane_u32 A, lane_f32 B)
{
  lane_f32 result;
  result.V = _mm_and_si128(_mm_castsi128_ps(A.V), B.V);
  return result;
}


lane_u32 andNot(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm_andnot_si128(A.V, B.V);
  return result;
}
inline lane_f32 sqrt(lane_f32 A)
{
  lane_f32 result;
  result.V = _mm_sqrt_ps(A.V);
  return result;
}
lane_f32 loadF32Values(f32 A, f32 B, f32 C, f32 D)
{
  lane_f32 result;
  result.V = _mm_set_ps(D,C,B,A);
  return result;
}
lane_f32 loadF32Values(f32* values)
{
  lane_f32 result;
  result.V = _mm_set_ps(values[3], values[2], values[1], values[0]);
  return result;
}
lane_v3 loadV3Values(vec3* values)
{
  lane_v3 result;
  result.x = loadF32Values(values[0].x, values[1].x, values[2].x, values[3].x);
  result.y = loadF32Values(values[0].y, values[1].y, values[2].y, values[3].y);
  result.z = loadF32Values(values[0].z, values[1].z, values[2].z, values[3].z);
  return result;
}
lane_f32 gatherF32_(void* basePointer, u32 stride, lane_u32 indices)
{
  //indices pointer, each one individually
  u32* V = (u32*)&indices.V;

  lane_f32 result;
  result.V = _mm_setr_ps(*(f32*)((u8*)basePointer + V[0]*stride),
			 *(f32*)((u8*)basePointer + V[1]*stride),
			 *(f32*)((u8*)basePointer + V[2]*stride),
			 *(f32*)((u8*)basePointer + V[3]*stride));

  return result;
  
}



void ConditionalAssign(lane_u32* target, lane_u32 condition, lane_u32 value)
{
  *target = (condition & value) | andNot(condition, *target);
}

void ConditionalAssign(lane_f32* target, lane_u32 condition, lane_f32 value)
{
  ConditionalAssign((lane_u32*)target, condition, *(lane_u32*)&value);
}

void ConditionalAssign(lane_v3* target, lane_u32 condition, lane_v3 value)
{
  ConditionalAssign(&target->x, condition, value.x);
  ConditionalAssign(&target->y, condition, value.y);
  ConditionalAssign(&target->z, condition, value.z);
}

int MaskAllZeros(lane_u32 mask)
{
  return _mm_movemask_epi8(mask.V) == 0;
}

u64 HorizontalAdd(lane_u32 lane)
{
  u32* V = (u32*)&lane.V;
  u64 result = (u64)V[0] + (u64)V[1] + (u64)V[2] + (u64)V[3];
  return result;
  
}
f32 HorizontalAdd(lane_f32 lane)
{
  f32* V = (f32*)&lane.V;
  f32 result = V[0] + V[1] + V[2] + V[3];
  return result;
}

lane_f32 max(lane_f32 first, lane_f32 second)
{
  lane_f32 result;
  result.V = _mm_max_ps(first.V, second.V);
  return result;
}
lane_f32 min(lane_f32 first, lane_f32 second)
{
  lane_f32 result;
  result.V = _mm_min_ps(first.V, second.V);
  return result;
}

lane_u32 randomLaneU32(u32* seed)
{
  u32 s1 = xorshift32(seed);
  u32 s2 = xorshift32(seed);
  u32 s3 = xorshift32(seed);
  u32 s4 = xorshift32(seed);
  lane_u32 result;
  result.V = _mm_setr_epi32(s1,s2,s3,s4);
  return result;
}
