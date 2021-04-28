#include "world.h"
#include "stdlib.h"

World* initWorld()
{
  //World setup
  World* world = (World*)malloc(sizeof(World));
  world->materials[0] = {};
  world->materials[0].reflectColour = {};
  world->materials[0].emitColour = { 1.0f, 1.0f, 1.0f };
  
  world->materials[1] = {};
  world->materials[1].reflectColour = { 0.8f, 0.7f, 0.2f };
  world->materials[1].scatterScale = 0.2;
  
  world->materials[2].emitColour = {};
  world->materials[2].reflectColour = { 0.0f, 0.2f, 1.0f };
  world->materials[2].scatterScale = 0.7;

  world->materials[3].emitColour = {99.0f,0.0f,0.0f};
  world->materials[3].reflectColour = { 0.0f, 0.0f, 0.0f };
  world->materials[3].scatterScale = 1;
    
  world->materials[4].emitColour = {};
  world->materials[4].reflectColour = { 0.1f, 0.4f, 0.8f };
  world->materials[4].scatterScale = 1.0;
  
  world->materials[5].emitColour = {0.0f,0.0f,0.0f};
  world->materials[5].reflectColour = {0.2f,0.9f,0.2f};
  world->materials[5].scatterScale = 0.9;
  world->materialCount = 6;

  //planes
  world->planes[0].normal = { 0.0f, 0.0f, 1.0f };
  world->planes[0].dist = 0.0f;
  world->planes[0].matIndex = 1;
  world->planeCount = 1;

  //spheres
  world->spheres[0].position = { 0.0f, 0.0f, 0.0f };
  world->spheres[0].radius = 1.0f;
  world->spheres[0].matIndex = 2;

  world->spheres[1].position = { -2.0f, 1.0f, 2.0f };
  world->spheres[1].radius = 1.2f;
  world->spheres[1].matIndex = 4;
  
  world->spheres[2].position = { 2.0f, 0.0f, 1.0f };
  world->spheres[2].radius = 1.0f;
  world->spheres[2].matIndex = 3;
  
  world->spheres[3].position = { 2.0f, 10.0f, 6.0f };
  world->spheres[3].radius = 2.0f;
  world->spheres[3].matIndex = 5;
  world->sphereCount = 4;

  return world;
}

void addSphereToWorld(World* world, Sphere* sphere);
void addPlaneToWorld(World* world, Plane* plane);
void addMaterialToWorld(World* world, Material* mat);
