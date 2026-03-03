#ifndef RENDERER_H
#define RENDERER_H 1

#include "platform/platform.h"

typedef struct Mesh Mesh;
typedef struct Texture Texture;
typedef struct Material Material;
typedef struct Renderer Renderer;

Renderer* renderer_create(Platform* platform);
void renderer_update(Renderer* renderer);
void renderer_destroy(Renderer* renderer);

#endif