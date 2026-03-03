#include "renderer.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "backend/vulkan/vulkan_backend.h"
#include "platform/platform.h"

Renderer* renderer_create(PlatformWindow* platform_window)
{
    Renderer* renderer = malloc(sizeof (Renderer));

    return renderer;
}

void renderer_init(Renderer* renderer, PlatformWindow* window)
{
    VulkanBackend* vulkan_backend = vulkan_backend_create(window);

    renderer->vulkan_backend = vulkan_backend;
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
