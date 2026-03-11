#ifndef RENDER_INTERNAL_H
#define RENDER_INTERNAL_H 1

#include "render/render.h"

#include <cglm/cglm.h>
#include <vulkan/vulkan.h>

#include "core/types.h"
#include "backend/vulkan/vulkan_backend.h"

struct Vertex
{
    vec3 position;
    vec2 uv;
};

#define CUBE_RADIUS 0.5f

static const Vertex cube_vertex_array[] =
{
    // +X
    {{ +CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +1.0f}},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +0.0f}},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS }, { +1.0f, +0.0f}},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +1.0f}},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS }, { +0.0f, +1.0f}},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +0.0f}},

    // -X
    {{ -CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +0.0f }},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +1.0f }},
    {{ -CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS }, { +0.0f, +1.0f }},
    {{ -CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +0.0f }},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS }, { +1.0f, +0.0f }},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +1.0f }},

    // +Y
    {{ -CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS }, { +0.0f, +1.0f }},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS }, { +1.0f, +0.0f }},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +1.0f }},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS }, { +0.0f, +1.0f }},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +0.0f }},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS }, { +1.0f, +0.0f }},

    // -Y
    {{ -CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +1.0f }},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +0.0f }},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS }, { +1.0f, +1.0f }},
    {{ -CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +1.0f }},
    {{ -CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS }, { +0.0f, +0.0f }},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +0.0f }},

    // +Z
    {{ -CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS}, { +0.0f, +1.0f }},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS}, { +1.0f, +0.0f }},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS}, { +0.0f, +0.0f }},
    {{ -CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS}, { +0.0f, +1.0f }},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS}, { +1.0f, +1.0f }},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS}, { +1.0f, +0.0f }},

    // -Z
    {{ +CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS}, { +0.0f, +1.0f}},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS}, { +1.0f, +0.0f}},
    {{ -CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS}, { +0.0f, +0.0f}},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS}, { +0.0f, +1.0f}},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS}, { +1.0f, +1.0f}},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS}, { +1.0f, +0.0f}},
};

struct Image
{
    u32 width;
    u32 height;
    u32 channels;
    u8* pixels;
};

struct FrameView
{
    vec3 position;

    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 view_projection_matrix;
};

struct Render
{
    VulkanBackend* vulkan_backend;

    FrameView main_frame_view;
};

Image render_image_load(const char* path);
void render_image_destroy(Image* image);

#endif