#include "render/render.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define GLFW_INCLUDE_VULKAN

#include "core/math/math.h"
#include "app/camera.h"
#include "app/world/world.h"
#include "platform/platform.h"

Render* render_create(Platform* platform)
{
    Render* render = malloc(sizeof(*render));

    return render;
}

void render_destroy(Render* render)
{
    vkDeviceWaitIdle(render->vulkan_device_context.device);

    vulkan_backend_destroy_frame_context(render);
    vulkan_backend_destroy_voxel_pipeline(render);
    vulkan_backend_destroy_swapchain_context(render);
    vulkan_backend_destroy_device_context(render);

    free(render);
}

void render_init(Render* render, Platform* platform)
{
    vulkan_backend_create_and_init_device_context(render, platform);
    vulkan_backend_create_and_init_swapchain_context(render);
    vulkan_backend_create_and_init_voxel_pipeline(render);
    vulkan_backend_create_and_init_frame_context(render);

    vulkan_backend_create_voxel_mesh(render);

    glm_vec3_zero(render->scene_context.position);

    glm_mat4_identity(render->scene_context.view_matrix);
    glm_mat4_identity(render->scene_context.projection_matrix);
    glm_mat4_identity(render->scene_context.projection_view_matrix);

    float aspect_ratio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    perspective_lh(glm_rad(60.0f), aspect_ratio, 0.1f, 10.0f, render->scene_context.projection_matrix);

    glm_mat4_mul(
        render->scene_context.projection_matrix, 
        render->scene_context.view_matrix, 
        render->scene_context.projection_view_matrix
    );
}

// PushConstants push_constants;

// glm_mat4_copy(
//     vulkan_backend->vulkan_frame_context.projection_view_matrix, 
//     push_constants.projection_view_matrix
// );

// vkCmdPushConstants(
//     vulkan_backend->command_buffer,
//     vulkan_backend->voxel_pipeline_context.layout,
//     VK_SHADER_STAGE_VERTEX_BIT,
//     0,
//     sizeof(PushConstants),
//     &push_constants
// );

void render_update(Render* render, World* world)
{
    vec3 forward;
    camera_get_forward(&world->camera, forward);

    vec3 up = {0.0f, 0.0f, 1.0f};

    vec3 center;
    glm_vec3_add(world->camera.position, forward, center);

    mat4 view_matrix;
    glm_mat4_identity(view_matrix);

    look_at_lh(world->camera.position, center, up, view_matrix);

    glm_mat4_print(view_matrix, stdout);

    //
}

void render_draw(Render* render)
{
    vulkan_backend_draw_frame(render);
}