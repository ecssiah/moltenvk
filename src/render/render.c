#include "render/render_internal.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform/platform.h"
#include "backend/vulkan/vulkan_backend.h"

Render* render_create()
{
    Render* render = malloc(sizeof(*render));

    return render;
}

void render_init(Render* render, Platform* platform)
{
    VulkanBackend* vulkan_backend = vulkan_backend_create(platform);
    render->vulkan_backend = vulkan_backend;
}

void render_destroy(Render* render)
{
    vulkan_backend_destroy(render->vulkan_backend);

    render->vulkan_backend = NULL;
}

void render_draw(Render* render)
{
    vulkan_backend_render(render->vulkan_backend);
}

void render_update(Render* render)
{
    render_draw(render);
}