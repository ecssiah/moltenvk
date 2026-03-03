#ifndef RENDERER_INTERNAL_H
#define RENDERER_INTERNAL_H 1

#include <vulkan/vulkan.h>

#include "backend/vulkan/vulkan_backend.h"

struct Renderer
{
    VulkanBackend* vulkan_backend;
};

static void r_init(struct Renderer* renderer, struct Platform* platform);
static void r_draw(struct Renderer* renderer);

#endif