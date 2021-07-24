/*
Much of the vector math, style taken from Handmade Hero by Casey Muratori
Handmade Hero day 362: https://www.youtube.com/watch?v=5tKiQd73rPk
 */

#ifndef __BL_MATH_HEADER
#define __BL_MATH_HEADER

#include <xmmintrin.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int16_t  s16; 
typedef int32_t  s32;
typedef int64_t  s64;
typedef float    f32;
typedef double   f64; 

/*

  Vector 2

 */
# include <math.h> //NOTE: this is for sqrt and pow, should figure out how to implement them yourself
#ifdef _MSC_VER
# if defined(_M_AMD64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
#  define BL_MATH_USE_SSE 1
# endif

#endif
#ifdef __SSE__
# define BL_MATH_USE_SSE 1
#endif

typedef struct vec2
{
	float x;
	float y;
  
  inline vec2 &operator+=(vec2 b);
  inline vec2 &operator-=(vec2 b);
  inline vec2 &operator*=(float b);
  inline vec2 &operator/=(float b);
} vec2;

inline vec2 operator+(vec2& a, vec2& b)
{
  vec2 result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  return result;
}

inline vec2 operator-(vec2& a, vec2& b)
{
	vec2 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

inline vec2 operator*(vec2& a, float b)
{
  vec2 result;
  result.x = a.x * b;
  result.y = a.y * b;
  return result;
}

inline vec2 operator/(vec2& a, float b)
{
  vec2 result;
  result.x = a.x / b;
  result.y = a.y / b;
  return result;
}

inline vec2 &vec2::operator+=(vec2 b)
{
  *this = *this + b;
  return *this;
}

inline vec2 &vec2::operator-=(vec2 b)
{
  *this = *this - b;
  return *this;
}

inline vec2 &vec2::operator*=(float b)
{
  *this = *this * b;
  return *this;
}

inline vec2 &vec2::operator/=(float b)
{
  *this = *this / b;
  return *this;
}


/*

  Vector 3

 */
#define PACKED_VEC3_SIZE 12
typedef struct vec3
{
  float x;
  float y;
  float z;
  #ifdef __USE_OPENCL
  float pad = 0.0f;
  #endif
  inline vec3 &operator+=(vec3 b);
  inline vec3 &operator-=(vec3 b);
  inline vec3 &operator*=(float b);
  inline vec3 &operator/=(float b);



#ifdef __USE_OPENCL
vec3(float a, float b, float c):x{a},y{b},z{c},pad{0.0f}{}
vec3():x{0},y{0},z{0},pad{0}{}
  #else
vec3(float a, float b, float c):x{a},y{b},z{c}{}
vec3():x{0},y{0},z{0}{}
#endif
} vec3;

inline vec3 operator+(vec3 a, vec3 b)
{
	vec3 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

inline vec3 operator-(vec3 a, vec3 b)
{
	vec3 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}

inline vec3 operator*(vec3 a, float b)
{
  vec3 result;
  result.x = a.x * b;
  result.y = a.y * b;
  result.z = a.z * b;
  return result;
}
inline vec3 operator*(float b, vec3 a)
{
  vec3 result;
  result.x = a.x * b;
  result.y = a.y * b;
  result.z = a.z * b;
  return result;
}


inline vec3 operator/(vec3 a, float b)
{
  vec3 result;
  result.x = a.x / b;
  result.y = a.y / b;
  result.z = a.z / b;
  return result;
}
inline vec3 operator/(vec3 a, vec3 b)
{
  vec3 result;
  result.x = a.x / b.x;
  result.y = a.y / b.y;
  result.z = a.z / b.z;
  return result;
}
inline vec3 operator/(vec3 a, int b)
{
  vec3 result;
  result.x = a.x / (f32)b;
  result.y = a.y / (f32)b;
  result.z = a.z / (f32)b;
  return result;
}

inline vec3 &vec3::operator+=(vec3 b)
{
  *this = *this + b;
  return *this;
}

inline vec3 &vec3::operator-=(vec3 b)
{
  *this = *this - b;
  return *this;
}

inline vec3 &vec3::operator*=(float b)
{
  *this = *this * b;
  return *this;
}

inline vec3 &vec3::operator/=(float b)
{
  *this = *this / b;
  return *this;
}

typedef struct vec3ui
{
  u32 x;
  u32 y;
  u32 z;
  inline vec3ui &operator+=(vec3ui b);
  inline vec3ui &operator-=(vec3ui b);
  inline vec3ui &operator*=(u32 b);
  inline vec3ui &operator/=(u32 b);

}vec3ui;

inline vec3ui operator+(vec3ui a, vec3ui b)
{
	vec3ui result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

inline vec3ui operator-(vec3ui a, vec3ui b)
{
	vec3ui result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}

inline vec3ui operator*(vec3ui a, u32 b)
{
  vec3ui result;
  result.x = a.x * b;
  result.y = a.y * b;
  result.z = a.z * b;
  return result;
}

inline vec3ui operator/(vec3ui a, u32 b)
{
  vec3ui result;
  result.x = a.x / b;
  result.y = a.y / b;
  result.z = a.z / b;
  return result;
}

inline vec3ui &vec3ui::operator+=(vec3ui b)
{
  *this = *this + b;
  return *this;
}

inline vec3ui &vec3ui::operator-=(vec3ui b)
{
  *this = *this - b;
  return *this;
}

inline vec3ui &vec3ui::operator*=(u32 b)
{
  *this = *this * b;
  return *this;
}

inline vec3ui &vec3ui::operator/=(u32 b)
{
  *this = *this / b;
  return *this;
}
/*

  Vector 4

 */
 
typedef struct vec4
{
  float x;
  float y;
  float z;
  float w;
  
  inline vec4 &operator+=(vec4 b);
  inline vec4 &operator-=(vec4 b);
  inline vec4 &operator*=(float b);
  inline vec4 &operator/=(float b);
} vec4;

inline vec4 operator+(vec4 a, vec4 b)
{
	vec4 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	result.w = a.w + b.w;
	return result;
}

inline vec4 operator-(vec4 a, vec4 b)
{
	vec4 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	result.w = a.w - b.w;
	return result;
}

inline vec4 operator*(vec4 a, float b)
{
  vec4 result;
  result.x = a.x * b;
  result.y = a.y * b;
  result.z = a.z * b;
  result.w = a.w * b;
  return result;
}

inline vec4 operator/(vec4 a, float b)
{
  vec4 result;
  result.x = a.x / b;
  result.y = a.y / b;
  result.z = a.z / b;
  result.w = a.w / b;
  return result;
}

inline vec4 &vec4::operator+=(vec4 b)
{
  *this = *this + b;
  return *this;
}

inline vec4 &vec4::operator-=(vec4 b)
{
  *this = *this - b;
  return *this;
}

inline vec4 &vec4::operator*=(float b)
{
  *this = *this * b;
  return *this;
}

inline vec4 &vec4::operator/=(float b)
{
  *this = *this / b;
  return *this;
}

/*

  2x2 Matrix

 */
typedef union _mat2
{
  float A[2][2]; //A[col][row]
  #if BL_MATH_USE_SSE
  __m128 V;
  #endif
} mat2;

inline mat2 operator*(mat2& f, mat2& s)
{
  mat2 result;
  result.A[0][0] = f.A[0][0] * s.A[0][0] + f.A[1][0] * s.A[0][1];
  result.A[1][0] = f.A[0][0] * s.A[1][0] + f.A[1][0] * s.A[1][1];
  result.A[0][1] = f.A[0][1] * s.A[0][0] + f.A[1][1] * s.A[0][1];
  result.A[1][1] = f.A[0][1] * s.A[1][0] + f.A[1][1] * s.A[1][1];
  return result;
}
#if BL_MATH_USE_SSE
inline mat2 operator+(mat2& f, mat2& s)
{
  mat2 result;
  result.V = _mm_add_ps(f.V, s.V);
  return result;
}
inline mat2 operator-(mat2& f, mat2& s)
{
  mat2 result;
  result.V = _mm_sub_ps(f.V, s.V);
  return result;
}

#else
inline mat2 operator+(mat2& f, mat2& s)
{
  mat2 result;
  result.A[0][0] = f.A[0][0] + s.A[0][0];
  result.A[0][1] = f.A[0][1] + s.A[0][1];
  result.A[1][0] = f.A[1][0] + s.A[1][0];
  result.A[1][1] = f.A[1][1] + s.A[1][1];  
  return result;
}
inline mat2 operator-(mat2& f, mat2& s)
{
  mat2 result;
  result.A[0][0] = f.A[0][0] - s.A[0][0];
  result.A[0][1] = f.A[0][1] - s.A[0][1];
  result.A[1][0] = f.A[1][0] - s.A[1][0];
  result.A[1][1] = f.A[1][1] - s.A[1][1];
  
  return result;
}
#endif

/*

  3x3 Matrix

 */
typedef struct mat3
{
  float A[3][3];
} mat3;
inline mat3 operator*(mat3& f, mat3& s)
{
  mat3 result;
  result.A[0][0] = f.A[0][0] * s.A[0][0] + f.A[1][0] * s.A[0][1] + f.A[2][0] * s.A[2][0];
  result.A[1][0] = f.A[0][0] * s.A[1][0] + f.A[1][0] * s.A[1][1] + f.A[2][0] * s.A[2][1];
  result.A[2][0] = f.A[0][0] * s.A[2][0] + f.A[1][0] * s.A[2][1] + f.A[2][0] * s.A[2][2];
  
  result.A[0][1] = f.A[0][1] * s.A[0][0] + f.A[1][1] * s.A[0][1] + f.A[2][1] * s.A[2][0];
  result.A[1][1] = f.A[0][1] * s.A[1][0] + f.A[1][1] * s.A[1][1] + f.A[2][1] * s.A[2][1];
  result.A[2][1] = f.A[0][1] * s.A[2][0] + f.A[1][1] * s.A[2][1] + f.A[2][1] * s.A[2][2];

  result.A[0][2] = f.A[0][2] * s.A[0][0] + f.A[2][1] * s.A[0][1] + f.A[2][2] * s.A[2][0];
  result.A[1][2] = f.A[0][2] * s.A[1][0] + f.A[2][1] * s.A[1][1] + f.A[2][2] * s.A[2][1];
  result.A[2][2] = f.A[0][2] * s.A[2][0] + f.A[2][1] * s.A[2][1] + f.A[2][2] * s.A[2][2];

  return result;
}

/*

  4x4 Matrix

 */
typedef union
{
  float A[4][4];
  #if BL_MATH_USE_SSE
  __m128 Columns[4];
  #endif
} mat4;
#if BL_MATH_USE_SSE
inline __m128 LinearCombineSSE(__m128 Left, mat4 Right)
{
    __m128 Result;
    Result = _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0x00), Right.Columns[0]);
    Result = _mm_add_ps(Result, _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0x55), Right.Columns[1]));
    Result = _mm_add_ps(Result, _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0xaa), Right.Columns[2]));
    Result = _mm_add_ps(Result, _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0xff), Right.Columns[3]));    
    return (Result);
}

inline mat4 operator*(mat4& f, mat4& s)
{
  mat4 Result;
  Result.Columns[0] = LinearCombineSSE(s.Columns[0], f);
  Result.Columns[1] = LinearCombineSSE(s.Columns[1], f);
  Result.Columns[2] = LinearCombineSSE(s.Columns[2], f);
  Result.Columns[3] = LinearCombineSSE(s.Columns[3], f);     
  return Result;
}

#else
inline mat4 operator*(mat4& f, mat4& s)
{
  mat4 result;

  result.A[0][0] = f.A[0][0] * s.A[0][0] + f.A[1][0] * s.A[0][1] + f.A[2][0] * s.A[0][2] + f.A[3][0] * s.A[0][3];
  result.A[1][0] = f.A[0][0] * s.A[1][0] + f.A[1][0] * s.A[1][1] + f.A[2][0] * s.A[1][2] + f.A[3][0] * s.A[1][3];
  result.A[2][0] = f.A[0][0] * s.A[2][0] + f.A[1][0] * s.A[2][1] + f.A[2][0] * s.A[2][2] + f.A[3][0] * s.A[2][3];
  result.A[3][0] = f.A[0][0] * s.A[3][0] + f.A[1][0] * s.A[3][1] + f.A[2][0] * s.A[3][2] + f.A[3][0] * s.A[3][3];
  
  result.A[0][1] = f.A[0][1] * s.A[0][0] + f.A[1][1] * s.A[0][1] + f.A[2][1] * s.A[0][2] + f.A[3][1] * s.A[0][3];
  result.A[1][1] = f.A[0][1] * s.A[1][0] + f.A[1][1] * s.A[1][1] + f.A[2][1] * s.A[1][2] + f.A[3][1] * s.A[1][3];
  result.A[2][1] = f.A[0][1] * s.A[2][0] + f.A[1][1] * s.A[2][1] + f.A[2][1] * s.A[2][2] + f.A[3][1] * s.A[2][3];
  result.A[3][1] = f.A[0][1] * s.A[3][0] + f.A[1][1] * s.A[3][1] + f.A[2][1] * s.A[3][2] + f.A[3][1] * s.A[3][3];

  result.A[0][2] = f.A[0][2] * s.A[0][0] + f.A[1][2] * s.A[0][1] + f.A[2][2] * s.A[0][2] + f.A[3][2] * s.A[0][3];
  result.A[1][2] = f.A[0][2] * s.A[1][0] + f.A[1][2] * s.A[1][1] + f.A[2][2] * s.A[1][2] + f.A[3][2] * s.A[1][3];
  result.A[2][2] = f.A[0][2] * s.A[2][0] + f.A[1][2] * s.A[2][1] + f.A[2][2] * s.A[2][2] + f.A[3][2] * s.A[2][3];
  result.A[3][2] = f.A[0][2] * s.A[3][0] + f.A[1][2] * s.A[3][1] + f.A[2][2] * s.A[3][2] + f.A[3][2] * s.A[3][3];

  result.A[0][3] = f.A[0][3] * s.A[0][0] + f.A[1][3] * s.A[0][1] + f.A[2][3] * s.A[0][2] + f.A[3][3] * s.A[0][3];
  result.A[1][3] = f.A[0][3] * s.A[1][0] + f.A[1][3] * s.A[1][1] + f.A[2][3] * s.A[1][2] + f.A[3][3] * s.A[1][3];
  result.A[2][3] = f.A[0][3] * s.A[2][0] + f.A[1][3] * s.A[2][1] + f.A[2][3] * s.A[2][2] + f.A[3][3] * s.A[2][3];
  result.A[3][3] = f.A[0][3] * s.A[3][0] + f.A[1][3] * s.A[3][1] + f.A[2][3] * s.A[3][2] + f.A[3][3] * s.A[3][3];

  return result;
}
#endif

/* Vector products */
inline float dot(vec2 a, vec2 b)
{
  return a.x * b.x + a.y * b.y;
}

inline float dot(vec3 a, vec3 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float dot(vec4 a, vec4 b)
{
  return a.x * b.x + a.y * b.y +
         a.z * b.z + a.w * b.w;
}
inline vec2 hadamard(vec2 a, vec2 b)
{
  vec2 result;
  result.x = a.x * b.x;
  result.y = a.y * b.y;
  return result;
}
inline vec3 hadamard(vec3 a, vec3 b)
{
  vec3 result;

  result.x = a.x * b.x;
  result.y = a.y * b.y;
  result.z = a.z * b.z;
  return result;
}
inline vec4 hadamard(vec4 a, vec4 b)
{
  vec4 result;
  result.x = a.x * b.x;
  result.y = a.y * b.y;
  result.z = a.z * b.z;
  result.w = a.w * b.w;
  return result;
}
inline vec3 cross(vec3 a, vec3 b)
{
  vec3 result;
  result.x = a.y*b.z - a.z*b.y;
  result.y = a.z*b.x - a.x*b.z;
  result.z = a.x*b.y - a.y*b.x;
  return result;
}

inline float length(vec2 a)
{
  return sqrt(dot(a,a));
}
inline float length(vec3 a)
{
  return sqrt(dot(a,a));
}
inline float length(vec4 a)
{
  return sqrt(dot(a,a));
}

inline vec3 normalize(vec3 a)
{
  vec3 result;
  result = a / length(a);
  return result;
}
inline vec2 normalize(vec2 a)
{
  vec2 result;
  result = a / length(a);
  return result;
}

/* Misc */

inline float max(float a, float b)
{
  return a < b ? b : a;
}
inline float min(float a, float b)
{
  return a < b ? a : b;
}

inline int max(int a, int b)
{
  return a < b ? b : a;
}
inline int min(int a, int b)
{
  return a < b ? a : b;
}

/*
  Fast inverse square root from the quake III engine
 */
inline float Q_rsqrt( float number )
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;                       // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck? 
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

/*
pure math functions
 */
/*
UNCOMMENT when you learn how to override the built in abs and sqrt

 */
/*
inline float abs(float a) 
{
  int c = *(int*)&a;
  c &= 0x7FFFFFFF;
  return *(float*)&c;
}
inline int abs(int a)
{
  int y = a >> 31;
  return (a + y) ^ a;
}
*/
inline int isNegative(float a)
{
  unsigned int c = *(unsigned int*)&a; //uint for logical right shift
  c |= 0x80000000;
  return (int)(c >> 31);
}
inline int isNegative(int a)
{
  unsigned int c = a | 0x80000000;
  return (int)(c >> 31);
}

/* sqrt calculation using Newton's method */
/*
inline float sqrt(float a)
{
  float x = abs(a);
  while(1)
    {
      float newX = 0.5*(x + a/x);
      if (abs(x - newX) < 0.00001)
	return newX;
      x = newX;
    }
}*/

/*Colour*/
inline unsigned int packRGBAtoARGB(vec4 a)
{
  unsigned char R = (unsigned char)(a.x * 255.0f);
  unsigned char G = (unsigned char)(a.y * 255.0f);
  unsigned char B = (unsigned char)(a.z * 255.0f);
  unsigned char A = (unsigned char)(a.w * 255.0f);
  return (A<<24) | (R<<16) | (G<<8) | (B<<0);
}

inline float linearToSRGB(float linear)
{
  float result;
  if (linear < 0) linear = 0;
  else if (linear > 1) linear = 1;
  if (linear <= 0.0031308f)      
    result = linear * 12.92f;	
  else
    result = 1.055f * pow(linear,(f32)1/2.4f) - 0.055f;
  return result;
  
}

inline vec3 linearToSRGB(vec3 linear)
{
  vec3 result;  
  result.x = linearToSRGB(linear.x);
  result.y = linearToSRGB(linear.y);
  result.z = linearToSRGB(linear.z);
  return result;
}


/*Random*/

inline uint64_t xorshift64(uint64_t *state)
{
  uint64_t x = *state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  return *state = x;
}
inline float randomUnilateral64(uint64_t *state)
{
  return (float)xorshift64(state) / (float)((uint64_t)-1);//4294967295.0f);
}
inline float randomBilateral64(uint64_t *state)
{
  return 1.0f - 2.0f*randomUnilateral64(state);
}
  
inline u32 xorshift32(u32 *state)
{
  /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
  u32 x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return *state = x;
}
inline f32 randomUnilateral32(u32 *state)
{
  return (f32)(xorshift32(state)) / ((uint32_t)-1);
}
inline f32 randomBilateral32(u32 *state)
{
  return 1.0f - 2.0f*randomUnilateral32(state);
}


//scales
float lerp(float min, float max, float value)
{
  return min + value*(max-min);
}
vec3 lerp(vec3 min, vec3 max, float value)
{
  return min + (max-min)*value;
}


#endif
