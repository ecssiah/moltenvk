#ifndef RENDERER_INTERNAL_H
#define RENDERER_INTERNAL_H 1

#include <vulkan/vulkan.h>

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

struct Renderer
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

static void r_init(struct Renderer* renderer, struct Platform* platform);
static void r_render(struct Renderer* renderer);

#endif