#ifndef RENDER_INTERNAL_H
#define RENDER_INTERNAL_H 1

#include "render/render.h"

#include <vulkan/vulkan.h>

#include "core/types.h"
#include "backend/vulkan/vulkan_backend.h"

struct Vertex
{
    f32 position[3];
    f32 uv[2];
};

struct Image
{
    u32 width;
    u32 height;
    u32 channels;
    u8* pixels;
};

struct Render
{
    VulkanBackend* vulkan_backend;
};

static const Vertex quad_vertex_array[6] =
{
    {
        {-0.5f, -0.5f, +0.0f}, 
        {+0.0f, +1.0f}
    },
    {
        {+0.5f, -0.5f, +0.0f}, 
        {+1.0f, +1.0f}
    },
    {
        {+0.5f, +0.5f, +0.0f}, 
        {+1.0f, +0.0f}
    },
    {
        {-0.5f, -0.5f, +0.0f}, 
        {+0.0f, +1.0f}
    },
    {
        {+0.5f, +0.5f, +0.0f}, 
        {+1.0f, +0.0f}
    },
    {
        {-0.5f, +0.5f, +0.0f}, 
        {+0.0f, +0.0f}
    },
};

Image render_image_load(const char* path);
void render_image_destroy(Image* image);

#endif