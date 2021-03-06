
#include "world.h"
#include "stdlib.h"


World* initWorld(SpatialHeirarchy* SH)
{
  //World setup
  World* world = (World*)malloc(sizeof(World));
  world->sphereCount = 0;

  world->materials[0] = {};
  world->materials[0].reflectColour = {};
  world->materials[0].emitColour = { 0.11f, 0.11f, 0.11f };
  
  world->materials[1] = {};
  world->materials[1].reflectColour = { 0.2f, 0.7f, 0.2f };
  world->materials[1].scatterScale = 0.2;
  
  world->materials[2].emitColour = {};
  world->materials[2].reflectColour = { 0.0f, 0.2f, 1.0f };
  world->materials[2].scatterScale = 0.7;

  world->materials[3].emitColour = {10.0f,10.0f,10.0f};
  world->materials[3].reflectColour = { 0.0f, 0.0f, 0.0f };
  world->materials[3].scatterScale = 1;
    
  world->materials[4].emitColour = {};
  world->materials[4].reflectColour = { 0.5f, 0.7f, 0.8f };
  world->materials[4].scatterScale = 1.0;
  
  world->materials[5].emitColour = {0.0f,0.0f,0.0f};
  world->materials[5].reflectColour = {0.6f, 0.4f, 0.7f};
  world->materials[5].scatterScale = 0.9;

  world->materials[6].emitColour = {0.2f,0.2f,0.2f};
  world->materials[6].reflectColour = {0.0f,0.0f,0.0f};
  world->materials[6].scatterScale = 0.0;

  world->materialCount = 7;

  Object object = {};

  u32 planeIndex;
  Plane* plane = getPlaneFromWorld(world, &planeIndex);
  //planes
  plane->normal = normalize(vec3( 0.0f, 0.001f, 1.0f ));
  plane->dist = 0.0f;
  plane->matIndex = 1;
  addPlaneToObject(&object, planeIndex);
  addObjectToSpatialHeirarchy(SH, &object);

  //spheres

  u32 sphereIndex;
  Sphere* sphere = getSphereFromWorld(world, &sphereIndex);
  object = {};
  sphere->position = { 8.0f, 8.0f, 0.0f };
  sphere->radius = 1.0f;
  sphere->matIndex = 2;
  
  addSphereToObject(&object, sphereIndex);
  addObjectToSpatialHeirarchy(SH, &object);

  sphere = getSphereFromWorld(world, &sphereIndex);
  object = {};
  sphere->position = { -2.0f, 1.0f, 2.0f };
  sphere->radius = 1.2f;
  sphere->matIndex = 4;
  
  addSphereToObject(&object, sphereIndex);
  addObjectToSpatialHeirarchy(SH, &object);

  sphere = getSphereFromWorld(world, &sphereIndex);
  object = {};
  sphere->position = { 3.0f, 0.0f, 1.0f };
  sphere->radius = 1.0f;
  sphere->matIndex = 3;

  addSphereToObject(&object, sphereIndex);
  addObjectToSpatialHeirarchy(SH, &object);

  sphere = getSphereFromWorld(world, &sphereIndex);
  object = {};
  sphere->position = { 0.0f, 0.0f, 2.0f };
  sphere->radius = 1.0f;
  sphere->matIndex = 5;

  addSphereToObject(&object, sphereIndex);
  addObjectToSpatialHeirarchy(SH, &object);

  //triangles
  world->triangleCount = 0;

  u32 triIndex;
  //Triangle* triangle = getTriangleFromWorld(world, &triIndex);
  /*
  object = {};
  triangle->v0 = { -1000.0, -1000.0, 1.0 };
  triangle->v1 = { 1000.0, -1000.0, 0.0 };
  triangle->v2 = { 500.0, 1000.0, 0.0 };
  triangle->normal = normalize(cross(triangle->v1-triangle->v0, triangle->v2-triangle->v0));
  triangle->matIndex = 1;
  
  addTriangleToObject(&object, triIndex);
  addObjectToSpatialHeirarchy(SH, &object);
  */

  u32 entropy = 0x710af89;
  for (int i = 0; i < 5; i++)
    {
      Object object = {};
      vec3 loc =
	{
	  randomBilateral32(&entropy) * 6.0f,
	  randomBilateral32(&entropy) * 6.0f,
	  randomUnilateral32(&entropy) * 6.0f
	};
      u32 sphereIndex;
      Sphere* sphere = getSphereFromWorld(world, &sphereIndex);
      sphere->position = loc;
      sphere->radius = randomUnilateral32(&entropy) + 1.0f;
      sphere->matIndex = (randomUnilateral32(&entropy) * world->materialCount);
      if (sphere->matIndex == 0) { sphere->matIndex = 1; }
      if (sphere->matIndex == world->materialCount) { sphere->matIndex = world->materialCount - 1; }

      //addSphereToObject(&object, sphereIndex);
      //addObjectToSpatialHeirarchy(SH, &object);
    }  

  //world->sphereCount = 0;
  world->bounceCount = 0;

  world->dLights[0] = {};
  world->dLights[0].direction = normalize(vec3(-0.00000f,0.001f,-1.0f));
  world->dLights[0].colour = {0.3f,0.3f,0.3f};

  /*
  world->dLights[1] = {};
  world->dLights[1].direction = normalize(vec3(1.0f,0.0f,-1.0f));
  world->dLights[1].colour = {0.8f,0.0f,0.8f};
  */

  world->dLightCount = 1;
  
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
void addObjectToSpatialHeirarchy(SpatialHeirarchy* SH, Object* object)
{
  Object* place = SH->objects + SH->objectCount;
  memcpy(place, object, sizeof(Object));
  SH->objectCount++;
}

void addSphereToObject(Object* object, u32 index)
{
  object->spheres[object->sphereCount] = index;
  object->sphereCount++;
}
void addPlaneToObject(Object* object, u32 index)
{
  object->planes[object->planeCount] = index;  
  object->planeCount++;
}
void addTriangleToObject(Object* object, u32 index)
{
  object->triangles[object->triangleCount] = index;
  object->triangleCount++;
}

Triangle* getTriangleFromWorld(World* world, u32* index)
{
  Triangle* triangle = world->triangles + world->triangleCount;
  *index = world->triangleCount;
  #ifdef _RAY_DEBUG
  if (world->triangleCount > WORLD_TRIANGLE_COUNT)
    {
      printf("ERROR: World Triangles Overflow\n");
    }
  #endif
  world->triangleCount++;
  return triangle;
}
Sphere* getSphereFromWorld(World* world, u32* index)
{
  Sphere* sphere = world->spheres + world->sphereCount;
  *index = world->sphereCount;
  #ifdef _RAY_DEBUG
  if (world->sphereCount > WORLD_SPHERE_COUNT)
    {
      printf("ERROR: World Spheres Overflow\n");
    }
  #endif
  world->sphereCount++;
  return sphere;
}
Plane* getPlaneFromWorld(World* world, u32* index)
{
  Plane* plane = world->planes + world->planeCount;
  *index = world->planeCount;
  #ifdef _RAY_DEBUG
  if (world->planeCount > WORLD_PLANE_COUNT)
    {
      printf("ERROR: World Planes Overflow\n");
    }
  #endif
  world->planeCount++;
  return plane;
}

void addLineToWorld(World* world, vec3 origin, vec3 direction, f32 length)
{
  if (world->lineCount == WORLD_LINE_COUNT)
    {
      printf("WORLD LINE OVERFLOW. EXITING...\n");
      exit(1);
    }
  Line* line = world->lines + world->lineCount;
  line->origin = origin;
  line->direction = direction;
  line->length = length;
  world->lineCount++;
}
void addDebugRectToWorld(World* world, vec3 center, vec3 dimensions)
{
  //bottom corner
  addLineToWorld(world,
		 center - dimensions,
		 {1.0f, 0.0f, 0.0f},
		 dimensions.x * 2.0f);

  addLineToWorld(world,
		 center - dimensions,
		 {0.0f, 1.0f, 0.0f},
		 dimensions.y * 2.0f);

  addLineToWorld(world,
		 center - dimensions,
		 {0.0f, 0.0f, 1.0f},
		 dimensions.z * 2.0f);

  //top corner
  addLineToWorld(world,
		 center + dimensions,
		 {0.0f, 0.0f, -1.0f},
		 dimensions.z * 2.0f);

  addLineToWorld(world,
		 center + dimensions,
		 {0.0f, -1.0f, .0f},
		 dimensions.y * 2.0f);
      
  addLineToWorld(world,
		 center + dimensions,
		 {-1.0f, 0.0f, 0.0f},
		 dimensions.x * 2.0f);

  //bottom right
  addLineToWorld(world,
		 center + hadamard({-1.0f,1.0f,1.0f},dimensions),
		 {0.0f, -1.0f, 0.0f},
		 dimensions.y * 2.0f);

  addLineToWorld(world,
		 center + hadamard({-1.0f,1.0f,1.0f},dimensions),
		 {0.0f, 0.0f, -1.0f},
		 dimensions.z * 2.0f);
  
  //top right
  addLineToWorld(world,
		 center + hadamard({1.0f,-1.0f,1.0f},dimensions),
		 {-1.0f, 0.0f, 0.0f},
		 dimensions.x * 2.0f);

  addLineToWorld(world,
		 center + hadamard({1.0f,-1.0f,1.0f},dimensions),
		 {0.0f, 0.0f, -1.0f},
		 dimensions.z * 2.0f);

  //another
  addLineToWorld(world,
		 center + hadamard({1.0f,1.0f,-1.0f},dimensions),
		 {-1.0f, 0.0f, 0.0f},
		 dimensions.x * 2.0f);

  addLineToWorld(world,
		 center + hadamard({1.0f,1.0f,-1.0f},dimensions),
		 {0.0f, -1.0f, 0.0f},
		 dimensions.y * 2.0f);
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
  
  for (u32 i = 0; i < SPATIAL_BOX_COUNT; i++)
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
      computeExtentBounds(SH->objects + o, SH->boxes + o, world, SH);
    }
}

void computeExtentBounds(Object* object, SpatialBox* box, World* world, SpatialHeirarchy* SH)
{
  //SpatialHeirarchy* SH = &world->SH;
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

      //TODO: if its axis aligned then itll be a thin plane itself
      for (u32 i = 0; i < object->planeCount; i++)
	{
	  Plane plane = world->planes[object->planes[i]];
	  box->planeDistances[p][0] = -FLT_MAX;
	  box->planeDistances[p][1] = FLT_MAX;
	}

    }
}
