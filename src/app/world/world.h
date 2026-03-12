#ifndef WORLD_H
#define WORLD_H 1

#include "platform/platform.h"
#include "app/camera.h"

typedef struct Platform Platform;

typedef struct World
{
    Camera camera;
}
World;

World* world_create(void);
void world_destroy(World* world);

void world_init(World* world);
void world_update(World* world, Platform* platform);

#endif