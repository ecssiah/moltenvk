#include "renderer/renderer.h"
#include "renderer/renderer_internal.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform/platform.h"
#include "backend/vulkan/vulkan_backend.h"

static void r_init(Renderer* renderer, Platform* platform)
{
    VulkanBackend* vulkan_backend = vulkan_backend_create(platform);
    renderer->vulkan_backend = vulkan_backend;
}

static void r_render(Renderer* renderer)
{
    vulkan_backend_render(renderer->vulkan_backend);
}

Renderer* renderer_create(Platform* platform)
{
    Renderer* renderer = malloc(sizeof (Renderer));

    r_init(renderer, platform);

    return renderer;
}

void renderer_update(Renderer* renderer)
{
    r_render(renderer);
}

void renderer_destroy(Renderer* renderer)
{
    vulkan_backend_destroy(renderer->vulkan_backend);

    renderer->vulkan_backend = NULL;
}