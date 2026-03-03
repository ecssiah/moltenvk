#ifndef RENDERER_H
#define RENDERER_H 1

#include "platform/platform.h"

typedef struct Mesh Mesh;
typedef struct Texture Texture;
typedef struct Material Material;
typedef struct Renderer Renderer;

Renderer* renderer_create(PlatformWindow* platform_window);
void renderer_draw(Renderer* Renderer);
void renderer_destroy(Renderer* Renderer);

#endif