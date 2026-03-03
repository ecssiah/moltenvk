#include "renderer/renderer.h"
#include "renderer/renderer_internal.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform/platform.h"
#include "backend/vulkan/vulkan_backend.h"

static void r_init(Renderer* renderer, PlatformWindow* platform_window)
{
    VulkanBackend* vulkan_backend = vulkan_backend_create(platform_window);
    renderer->vulkan_backend = vulkan_backend;
}

Renderer* renderer_create(PlatformWindow* platform_window)
{
    Renderer* renderer = malloc(sizeof (Renderer));

    r_init(renderer, platform_window);

    return renderer;
}

void renderer_draw(Renderer* renderer)
{
    vulkan_backend_draw(renderer->vulkan_backend);
}

void renderer_destroy(Renderer* renderer)
{
    vulkan_backend_destroy(renderer->vulkan_backend);

    renderer->vulkan_backend = NULL;
}