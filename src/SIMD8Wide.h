#include <immintrin.h>
typedef struct lane_f32
{
  __m256  V;
  lane_f32 &operator=(f32);
}lane_f32;

typedef struct lane_u32
{
  __m256i V;
  lane_u32 &operator=(u32);
}lane_u32;

typedef struct lane_v3
{
  lane_f32  X;
  lane_f32  Y;
  lane_f32  Z;
  lane_v3 &operator=(vec3);
}lane_v3;

//Loads
lane_f32 laneF32FromF32(f32 A)
{
  lane_f32 result;
  result.V = _mm256_set1_ps(A);
  return result;
}
lane_u32 laneU32FromU32(u32 A)
{
  lane_u32 result;
  result.V = _mm256_set1_epi32(A);
  return result;
}
lane_v3 laneV3FromV3(vec3 A)
{
  lane_v3 result;
  result.X = A.x;
  result.Y = A.y;
  result.Z = A.z;
  return result;
}
lane_f32 laneF32FromLaneU32(lane_u32 A)
{
  lane_f32 result;
  result.V = _mm256_cvtepi32_ps(A.V);
  return result;
}
lane_u32 laneU32FromLaneF32(lane_f32 A)
{
  lane_u32 result;
  result.V = _mm256_cvtps_epi32(A.V);
  return result;
}


//u32 operators
lane_u32 operator+(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm256_add_epi32(A.V, B.V);
  return result;
}
lane_u32 operator-(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm256_sub_epi32(A.V, B.V);
  return result;
}
lane_u32 operator*(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm256_mul_epi32(A.V, B.V);
  return result;
}
lane_u32 operator<<(lane_u32 A, u32 amount)
{
  lane_u32 result;
  result.V = _mm256_slli_epi32(A.V, amount);
  return result;
}
lane_u32 operator>>(lane_u32 A, u32 amount)
{
  lane_u32 result;
  result.V = _mm256_srli_epi32(A.V, amount);
  return result;
}
lane_u32 operator|(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm256_or_si256(A.V, B.V);
  return result;
}
lane_u32 operator==(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm256_cmpeq_epi32(A.V, B.V);
  return result;
}
lane_u32 operator&(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm256_and_si256(A.V, B.V);
  return result;
}
lane_u32 operator^(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm256_xor_si256(A.V, B.V);
  return result;
}
lane_u32 operator>(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm256_cmpgt_epi32(A.V, B.V);
  return result;
}
lane_u32 operator<(lane_u32 A, lane_u32 B)
{
  lane_u32 result = (A > B & A == B) ^ laneU32FromU32(-1);
  // a < b = ~(a > b & a == b) = (a > b & a == b) ^ 0xFFFFF...
  //since theres not NOT or less than operator intrinsic
  return result;
}
//f32 operators
lane_f32 operator+(lane_f32 A, lane_f32 B)
{
  lane_f32 result;
  result.V = _mm256_add_ps(A.V, B.V);
  return result;
}
lane_f32 operator-(lane_f32 A, lane_f32 B)
{
  lane_f32 result;
  result.V = _mm256_sub_ps(A.V, B.V);
  return result;
}
lane_f32 operator*(lane_f32 A, lane_f32 B)
{
  lane_f32 result;
  result.V = _mm256_mul_ps(A.V, B.V);
  return result;
}
lane_f32 operator/(lane_f32 A, lane_f32 B)
{
  lane_f32 result;
  result.V = _mm256_div_ps(A.V, B.V);
  return result;
}
lane_u32 operator<(lane_f32 A, lane_f32 B)
{
  lane_u32 result;
  result.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_LT_OQ));
  return result;
}
lane_u32 operator>(lane_f32 A, lane_f32 B)
{
  lane_u32 result;
  result.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_GT_OQ));
  return result;
}

lane_f32 operator&(lane_u32 A, lane_f32 B)
{
  lane_f32 result;
  result.V = _mm256_and_ps(_mm256_castsi256_ps(A.V), B.V);
  return result;
}


lane_u32 andNot(lane_u32 A, lane_u32 B)
{
  lane_u32 result;
  result.V = _mm256_andnot_si256(A.V, B.V);
  return result;
}
inline lane_f32 sqrt(lane_f32 A)
{
  lane_f32 result;
  result.V = _mm256_sqrt_ps(A.V);
  return result;
}
lane_f32 loadF32Values(f32 A, f32 B, f32 C, f32 D, f32 E, f32 F, f32 G, f32 H)
{
  lane_f32 result;
  result.V = _mm256_setr_ps(A,B,C,D,E,F,G,H);
  return result;
}
lane_f32 loadF32Values(f32* values)
{
  lane_f32 result;
  result.V = _mm256_set_ps(values[0], values[1], values[2], values[3],
			   values[4], values[5], values[6], values[7]);
  return result;
}
lane_v3 loadV3Values(vec3* values)
{
  lane_v3 result;
  result.X = loadF32Values(values[0].x, values[1].x, values[2].x, values[3].x,
			   values[4].x, values[5].x, values[6].x, values[7].x);
  result.Y = loadF32Values(values[0].y, values[1].y, values[2].y, values[3].y,
			   values[4].y, values[5].y, values[6].y, values[7].y);
  result.Z = loadF32Values(values[0].z, values[1].z, values[2].z, values[3].z,
			   values[4].z, values[5].z, values[6].z, values[7].z);
  return result;
}
lane_f32 gatherF32_(void* basePointer, u32 stride, lane_u32 indices)
{
  //indices pointer, each one individually
  u32* V = (u32*)&indices.V;
  const int scale = stride;
  lane_f32 result;
  result.V = _mm256_setr_ps(*(f32*)((u8*)basePointer + V[0]*stride),
			    *(f32*)((u8*)basePointer + V[1]*stride),
			    *(f32*)((u8*)basePointer + V[2]*stride),
			    *(f32*)((u8*)basePointer + V[3]*stride),
			    *(f32*)((u8*)basePointer + V[4]*stride),
			    *(f32*)((u8*)basePointer + V[5]*stride),
			    *(f32*)((u8*)basePointer + V[6]*stride),
			    *(f32*)((u8*)basePointer + V[7]*stride));
			    
  //result.V = _mm256_i32gather_ps(basePointer, indices.V, );
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
  ConditionalAssign(&target->X, condition, value.X);
  ConditionalAssign(&target->Y, condition, value.Y);
  ConditionalAssign(&target->Z, condition, value.Z);
}

int MaskAllZeros(lane_u32 mask)
{
  return _mm256_movemask_epi8(mask.V) == 0;
}


u64 HorizontalAdd(lane_u32 lane)
{
  
  u32* V = (u32*)&lane.V;
  u64 result = (u64)V[0] + (u64)V[1] + (u64)V[2] + (u64)V[3] +
    (u64)V[4] + (u64)V[5] + (u64)V[6] + (u64)V[7];
  
  return result;
    
}
f32 HorizontalAdd(lane_f32 lane)
{
    // hiQuad = ( x7, x6, x5, x4 )
    const __m128 hiQuad = _mm256_extractf128_ps(lane.V, 1);
    // loQuad = ( x3, x2, x1, x0 )
    const __m128 loQuad = _mm256_castps256_ps128(lane.V);
    // sumQuad = ( x3 + x7, x2 + x6, x1 + x5, x0 + x4 )
    const __m128 sumQuad = _mm_add_ps(loQuad, hiQuad);
    // loDual = ( -, -, x1 + x5, x0 + x4 )
    const __m128 loDual = sumQuad;
    // hiDual = ( -, -, x3 + x7, x2 + x6 )
    const __m128 hiDual = _mm_movehl_ps(sumQuad, sumQuad);
    // sumDual = ( -, -, x1 + x3 + x5 + x7, x0 + x2 + x4 + x6 )
    const __m128 sumDual = _mm_add_ps(loDual, hiDual);
    // lo = ( -, -, -, x0 + x2 + x4 + x6 )
    const __m128 lo = sumDual;
    // hi = ( -, -, -, x1 + x3 + x5 + x7 )
    const __m128 hi = _mm_shuffle_ps(sumDual, sumDual, 0x1);
    // sum = ( -, -, -, x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 )
    const __m128 sum = _mm_add_ss(lo, hi);
    return _mm_cvtss_f32(sum);
}

lane_f32 Max(lane_f32 first, lane_f32 second)
{
  lane_f32 result;
  result.V = _mm256_max_ps(first.V, second.V);
  return result;
}

lane_u32 randomLaneU32(u32* seed)
{
  u32 s1 = xorshift32(seed);
  u32 s2 = xorshift32(seed);
  u32 s3 = xorshift32(seed);
  u32 s4 = xorshift32(seed);
  u32 s5 = xorshift32(seed);
  u32 s6 = xorshift32(seed);
  u32 s7 = xorshift32(seed);
  u32 s8 = xorshift32(seed);
  lane_u32 result;
  result.V = _mm256_setr_epi32(s1,s2,s3,s4,s5,s6,s7,s8);
  return result;
}
