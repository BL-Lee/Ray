typedef float lane_f32;
typedef uint32_t lane_u32;
typedef vec3 lane_v3;

lane_f32 laneF32FromLaneU32(lane_u32 A)
{
  return A;
}
lane_u32 laneU32FromU32(u32 A)
{
  return A;
}
lane_f32 laneF32FromF32(f32 A)
{
  return A;
}
lane_v3 laneV3FromV3(vec3 A)
{
  return A;
}


void ConditionalAssign(lane_u32* target, lane_u32 condition, lane_u32 value)
{
  condition = condition ? 0xFFFFFFFF : 0;
    *target = (condition & value) | (~condition & *target);
}

void ConditionalAssign(lane_f32* target, lane_u32 condition, lane_f32 value)
{
  ConditionalAssign((lane_u32*)target, condition, *(lane_u32*)&value);
}

void ConditionalAssign(lane_v3* target, lane_u32 condition, lane_v3 value)
{
  ConditionalAssign((lane_f32*)&target->x, condition, value.x);
  ConditionalAssign((lane_f32*)&target->y, condition, value.y);
  ConditionalAssign((lane_f32*)&target->z, condition, value.z);
}

lane_f32 Max(lane_f32 first, lane_f32 second)
{
  return first > second ? first : second;
}

lane_u32 MaskAllZeros(lane_u32 mask)
{
  return !mask;
}

lane_f32 HorizontalAdd(lane_f32 lane)
{
  return lane;
}

vec3 HorizontalAdd(lane_v3 A)
{
  vec3 result;
  result.x = HorizontalAdd(A.x);
  result.y = HorizontalAdd(A.y);
  result.z = HorizontalAdd(A.z);
  return result;  
}
lane_u32 andNot(lane_u32 A, lane_u32 B)
{
  return (~A) & (B);
}

lane_f32 gatherF32_(void* basePointer, u32 stride, lane_u32 indices)
{
  lane_f32 result = *(f32*)((u8*)basePointer + indices*stride);
  return result;
}
lane_v3 gatherV3_(void* basePointer, u32 stride, lane_u32 indices)
{
  lane_v3 result = *(vec3*)((u8*)basePointer + indices*stride);
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
  result.x = maskLaneF32(target.x, mask);
  result.y = maskLaneF32(target.y, mask);
  result.z = maskLaneF32(target.z, mask);
  return result;  
  
}

lane_u32 randomLaneU32(u32* seed)
{
  return xorshift32(seed);
}

