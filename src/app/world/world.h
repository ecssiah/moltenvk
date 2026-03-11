#ifndef WORLD_H
#define WORLD_H 1

typedef struct World World;
typedef struct Platform Platform;

World* world_create(void);
void world_destroy(World* world);

void world_init(World* world);
void world_update(World* world, Platform* platform);

#endif