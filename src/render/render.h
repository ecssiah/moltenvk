#ifndef RENDER_H
#define RENDER_H 1

typedef struct Vertex Vertex;
typedef struct Image Image;
typedef struct Mesh Mesh;
typedef struct Texture Texture;
typedef struct Material Material;
typedef struct Render Render;
typedef struct Platform Platform;

typedef struct FrameView FrameView;

Render* render_create();
void render_destroy(Render* render);

void render_init(Render* render, Platform* platform);
void render_update(Render* render);

#endif