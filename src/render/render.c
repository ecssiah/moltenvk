#include "render/render_internal.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform/platform.h"
#include "backend/vulkan/vulkan_backend.h"

static void render_draw(Render* render);

Render* render_create()
{
    Render* render = malloc(sizeof(*render));

    return render;
}

void render_destroy(Render* render)
{
    vulkan_backend_destroy(render->vulkan_backend);

    render->vulkan_backend = NULL;

    free(render);
}

void render_init(Render* render, Platform* platform)
{
    VulkanBackend* vulkan_backend = vulkan_backend_create(platform);
    
    render->vulkan_backend = vulkan_backend;

    glm_vec3_zero(render->main_frame_view.position);

    glm_mat4_identity(render->main_frame_view.view_matrix);
    glm_mat4_identity(render->main_frame_view.projection_matrix);
    glm_mat4_identity(render->main_frame_view.view_projection_matrix);

    vulkan_backend_init(vulkan_backend);
}

void render_update(Render* render)
{
    render_draw(render);
}

static void render_draw(Render* render)
{
    vulkan_backend_draw(render->vulkan_backend);
}