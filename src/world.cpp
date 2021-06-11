
#include "world.h"
#include "stdlib.h"

World* initWorld()
{
  //World setup
  World* world = (World*)malloc(sizeof(World));
  world->sphereCount = 0;

  world->materials[0] = {};
  world->materials[0].reflectColour = {};
  world->materials[0].emitColour = { 0.8f, 0.50f, 0.3f };
  
  world->materials[1] = {};
  world->materials[1].reflectColour = { 0.8f, 0.7f, 0.2f };
  world->materials[1].scatterScale = 0.2;
  
  world->materials[2].emitColour = {};
  world->materials[2].reflectColour = { 0.0f, 0.2f, 1.0f };
  world->materials[2].scatterScale = 0.7;

  world->materials[3].emitColour = {99.0f,99.0f,99.0f};
  world->materials[3].reflectColour = { 0.0f, 0.0f, 0.0f };
  world->materials[3].scatterScale = 1;
    
  world->materials[4].emitColour = {};
  world->materials[4].reflectColour = { 0.1f, 0.4f, 0.8f };
  world->materials[4].scatterScale = 1.0;
  
  world->materials[5].emitColour = {0.0f,0.0f,0.0f};
  world->materials[5].reflectColour = {0.2f,0.9f,0.2f};
  world->materials[5].scatterScale = 0.9;

  world->materials[6].emitColour = {0.2f,0.2f,0.2f};
  world->materials[6].reflectColour = {0.0f,0.0f,0.0f};
  world->materials[6].scatterScale = 0.0;

  world->materialCount = 7;


  //planes
  world->planes[0].normal = { 0.0f, 0.0f, 1.0f };
  world->planes[0].dist = 0.0f;
  world->planes[0].matIndex = 1;
  world->planeCount = 0;

  //spheres
  Object object = {};
  Sphere sphere;
  
  sphere.position = { 8.0f, 8.0f, 0.0f };
  sphere.radius = 1.0f;
  sphere.matIndex = 2;
  
  addSphereToObject(world, &object);
  addSphereToWorld(world, &sphere);
  addObjectToWorld(world, &object);

  object = {};
  sphere.position = { -2.0f, 1.0f, 2.0f };
  sphere.radius = 1.2f;
  sphere.matIndex = 4;
  
  addSphereToObject(world, &object);
  addSphereToWorld(world, &sphere);
  addObjectToWorld(world, &object);

  object = {};
  sphere.position = { 2.0f, 0.0f, 1.0f };
  sphere.radius = 1.0f;
  sphere.matIndex = 3;

  addSphereToObject(world, &object);
  addSphereToWorld(world, &sphere);
  addObjectToWorld(world, &object);

  object = {};
  sphere.position = { 2.0f, 10.0f, 6.0f };
  sphere.radius = 1.0f;
  sphere.matIndex = 5;

  addSphereToObject(world, &object);
  addSphereToWorld(world, &sphere);
  addObjectToWorld(world, &object);

  
  //triangles
  world->triangleCount = 0;
  Triangle triangle = {};
  object = {};
  triangle.normal = { 0.0f, 0.0f, 1.0f };
  triangle.v0 = { 0.0, 0.0, 0.0 };
  triangle.v1 = { -2.0, 1.0, 2.0 };
  triangle.v2 = { 2.0, 10.0, 6.0 };
  triangle.matIndex = 2;
  
  addTriangleToObject(world, &object);
  addTriangleToWorld(world, &triangle);
  addObjectToWorld(world, &object);

  u32 entropy = 0xfd1a1cb;
  for (int i = 0; i < 5; i++)
    {
      Object object = {};
      vec3 loc =
	{
	  randomBilateral32(&entropy) * 6.0f,
	  randomBilateral32(&entropy) * 6.0f,
	  randomBilateral32(&entropy) * 6.0f
	};
      Sphere sphere = {};
      sphere.position = loc;
      sphere.radius = randomUnilateral32(&entropy) + 1.0f;
      sphere.matIndex = (randomUnilateral32(&entropy) * world->materialCount);
      if (sphere.matIndex == 0) { sphere.matIndex = 1; }
      if (sphere.matIndex == world->materialCount) { sphere.matIndex = world->materialCount - 1; }
      

      addSphereToObject(world, &object);
      addSphereToWorld(world, &sphere);
      addObjectToWorld(world, &object);

    }  


  world->bounceCount = 0;

  return world;
}

void addSphereToWorld(World* world, Sphere* sphere)
{
  Sphere* place = &(world->spheres[world->sphereCount]);
  memcpy(place, sphere, sizeof(Sphere));
  world->sphereCount++;
}

void addPlaneToWorld(World* world, Plane* plane)
{
  Plane* place = &(world->planes[world->planeCount]);
  memcpy(place, plane, sizeof(Plane));
  world->planeCount++;
}

void addMaterialToWorld(World* world, Material* material)
{
  Material* place = &(world->materials[world->materialCount]);
  memcpy(place, material, sizeof(Material));
  world->materialCount++;
}
void addTriangleToWorld(World* world, Triangle* triangle)
{
  Triangle* place = &(world->triangles[world->triangleCount]);
  memcpy(place, triangle, sizeof(Triangle));
  world->triangleCount++;
}
void addObjectToWorld(World* world, Object* object)
{
  Object* place = world->SH.objects + world->SH.objectCount;
  memcpy(place, object, sizeof(Object));
  world->SH.objectCount++;
}

void addSphereToObject(World* world, Object* object)
{
  object->spheres[object->sphereCount] = world->sphereCount;
  object->sphereCount++;
}
void addPlaneToObject(World* world, Object* object)
{
  object->planes[object->planeCount] = world->planeCount;  
  object->planeCount++;
}
void addTriangleToObject(World* world, Object* object)
{
  object->triangles[object->triangleCount] = world->triangleCount;
  object->triangleCount++;
}

/*

u32 shapeCountInBox(SpatialBox* box, World* world)
{
  u32 shapeCount = 0;
  for (u32 shape = 0; shape < world->sphereCount; shape++)
    {
      Sphere sphere = world->spheres[shape];
      vec3 max =
 	{
	  sphere.position.x + sphere.radius,
	  sphere.position.y + sphere.radius,
	  sphere.position.z + sphere.radius
	};
      vec3 min =
	{
	  sphere.position.x - sphere.radius,
	  sphere.position.y - sphere.radius,
	  sphere.position.z - sphere.radius
	};

      if (
	  min.x < box->position.x + box->dimensions.x &&
	  max.x > box->position.x - box->dimensions.x &&
	  min.y < box->position.y + box->dimensions.y &&
	  max.y > box->position.y - box->dimensions.y &&
	  min.z < box->position.z + box->dimensions.z &&
	  max.z > box->position.z - box->dimensions.z 
	   )
	{
	  shapeCount++;
	  box->spheres[box->sphereCount] = shape;
	  box->sphereCount++;
	}
    }
  for (u32 shape = 0; shape < world->triangleCount; shape++)
    {
      Triangle triangle = world->triangles[shape];
      f32 maxX = max(max(triangle.v0.x, triangle.v1.x), triangle.v2.x);
      f32 minX = min(min(triangle.v0.x, triangle.v1.x), triangle.v2.x);
      f32 maxY = max(max(triangle.v0.y, triangle.v1.y), triangle.v2.y);
      f32 minY = min(min(triangle.v0.y, triangle.v1.y), triangle.v2.y);
      f32 maxZ = max(max(triangle.v0.z, triangle.v1.z), triangle.v2.z);
      f32 minZ = min(min(triangle.v0.z, triangle.v1.z), triangle.v2.z);
   
      if (
	  minX < box->position.x + box->dimensions.x &&
	  maxX > box->position.x - box->dimensions.x &&
	  minY < box->position.y + box->dimensions.y &&
	  maxY > box->position.y - box->dimensions.y &&
	  minZ < box->position.z + box->dimensions.z &&
	  maxZ > box->position.z - box->dimensions.z 
	  )
	{
	  shapeCount++;
	  box->triangles[box->triangleCount] = shape;
	  box->triangleCount++;
	}      
    }
  return shapeCount;
  }*/
/*
void generateSpatialHeirarchy(World* world, SpatialHeirarchy* SH)
{
  
  for (u32 i = 0; i < SPATIAL_BOX_COUNT; i++)
    {
      SH->boxes[i].first = -1;
      SH->boxes[i].second = -1;
    }
    
  SH->boxCount = 1;
  SpatialBox* box = SH->boxes;
  box->dimensions = {12,12,12};

  splitBox(SH, box, world, 0);
}
*/
//Assumes all the objects have been added already
void generateSpatialHeirarchy(World* world, SpatialHeirarchy* SH)
{
  
  for (u32 i = 0; i < 5; i++)
    {
      for (u32 p = 0; p < 7; p++)
	{
	  SH->boxes[i].planeDistances[p][0] =  FLT_MAX;
	  SH->boxes[i].planeDistances[p][1] = -FLT_MAX;
	}
    }
    
  SH->planeNormals[0] = { 1.0f, 0.0f, 0.0f };
    SH->planeNormals[1] = { 0.0f, 1.0f, 0.0f };
    SH->planeNormals[2] = { 0.0f, 0.0f, 1.0f };
    f32 sqrt3Over3 = sqrt(3.0f) / 3.0f;
    SH->planeNormals[3] = {  sqrt3Over3 ,  sqrt3Over3 , sqrt3Over3 };
    SH->planeNormals[4] = { -sqrt3Over3 ,  sqrt3Over3 , sqrt3Over3 };
    SH->planeNormals[5] = { -sqrt3Over3 , -sqrt3Over3 , sqrt3Over3 };
    SH->planeNormals[6] = {  sqrt3Over3 , -sqrt3Over3 , sqrt3Over3 };

  for (u32 o = 0; o < SH->objectCount; o++)
    {
      computeExtentBounds(SH->objects + o, SH->boxes + o, world);
    }
}

void computeExtentBounds(Object* object, SpatialBox* box, World* world)
{
  SpatialHeirarchy* SH = &world->SH;
  float distance = 0.0f;
  for (u32 p = 0; p < 7; p++)
    {

      for (u32 i = 0; i < object->triangleCount; i++)
	{
	  Triangle triangle = world->triangles[object->triangles[i]];
	  distance = -dot(SH->planeNormals[p], triangle.v0);
	  if (distance < box->planeDistances[p][0])
	    box->planeDistances[p][0] = distance;
	  if (distance > box->planeDistances[p][1])
	    box->planeDistances[p][1] = distance;

	  distance = -dot(SH->planeNormals[p], triangle.v1);
	  if (distance < box->planeDistances[p][0])
	    box->planeDistances[p][0] = distance;
	  if (distance > box->planeDistances[p][1])
	    box->planeDistances[p][1] = distance;	  

	  distance = -dot(SH->planeNormals[p], triangle.v2);
	  if (distance < box->planeDistances[p][0])
	    box->planeDistances[p][0] = distance;
	  if (distance > box->planeDistances[p][1])
	    box->planeDistances[p][1] = distance;	  

	}
      for (u32 i = 0; i < object->sphereCount; i++)
	{
	  Sphere sphere = world->spheres[object->spheres[i]];
	  distance = -dot(SH->planeNormals[p], sphere.position);
	  box->planeDistances[p][0] = distance - sphere.radius;
	  box->planeDistances[p][1] = distance + sphere.radius;
	}
    }
}
/*
void splitBox(SpatialHeirarchy* SH, SpatialBox* box, World* world, u32 axis)
{
  if (box->dimensions.x < 0.1 ||
      box->dimensions.y < 0.1 ||
      box->dimensions.z < 0.1){
    return;
  }
      
  u32 shapeCount = shapeCountInBox(box, world);
  if (shapeCount > 900)
    {
      vec3 halfPositionOffset;
      vec3 newDims = box->dimensions;
      vec3 halfAxisVec;
      vec3 halfAxisZeroed;
      switch (axis)
	{
	case 0:
	  halfAxisVec    = {0.5f, 1.0f, 1.0f};
	  halfAxisZeroed = {0.5f, 0.0f, 0.0f};
	  newDims.x /= 2.0f;
	  break;
	case 1:
	  halfAxisVec    = {1.0f, 0.5f, 1.0f};
	  halfAxisZeroed = {0.0f, 0.5f, 0.0f};
	  newDims.y /= 2.0f;
	  break;
	case 2:
	  halfAxisVec    = {1.0f, 1.0f, 0.5f};
	  halfAxisZeroed = {0.0f, 0.0f, 0.5f};
	  newDims.z /= 2.0f;
	  break;
	}
      
      SpatialBox* boxA = SH->boxes + SH->boxCount;
      box->first = SH->boxCount;
      SH->boxCount++;
      halfPositionOffset = hadamard(box->dimensions, halfAxisZeroed);      
      boxA->position = box->position + halfPositionOffset;     
      boxA->dimensions = newDims;
      //printf("Box added: %d : pos: %f %f %f dims: %f %f %f\n", SH->boxCount - 1, boxA->position.x, boxA->position.y, boxA->position.z, boxA->dimensions.x, boxA->dimensions.y, boxA->dimensions.z);
      splitBox(SH, boxA, world, (axis + 1) % 3);
            
      SpatialBox* boxB = SH->boxes + SH->boxCount;      
      box->second = SH->boxCount;
      SH->boxCount++;
      boxB->position = box->position - halfPositionOffset;
      boxB->dimensions = newDims;
      //printf("Box added: %d : pos: %f %f %f dims: %f %f %f\n", SH->boxCount - 1, boxB->position.x, boxB->position.y, boxB->position.z, boxB->dimensions.x, boxB->dimensions.y, boxB->dimensions.z);
      splitBox(SH, boxB, world, (axis + 1) % 3);
    }
}
*/
