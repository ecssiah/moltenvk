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
    VkDevice device = render->vulkan_device_context.device;

    vkDeviceWaitIdle(device);

    render_vulkan_destroy_voxel_pipeline(render);
    render_vulkan_destroy_nuklear_pipeline(render);

    render_vulkan_destroy_frame_context(render);

    render_vulkan_destroy_swapchain_context(render);

    render_vulkan_destroy_device_context(render);

    free(render);
}

void render_init(Render* render, Platform* platform)
{
    glm_vec3_zero(render->position);

    glm_mat4_identity(render->view_matrix);
    glm_mat4_identity(render->projection_matrix);
    glm_mat4_identity(render->projection_view_matrix);

    const float fov = glm_rad(60.0f);
    const float aspect_ratio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    const float near_plane = 0.1f;
    const float far_plane = 10.0f;

    perspective_lh(fov, aspect_ratio, near_plane, far_plane, render->projection_matrix);

    glm_mat4_mul(
        render->projection_matrix, 
        render->view_matrix, 
        render->projection_view_matrix
    );

    render_vulkan_create_and_init_device_context(render, platform);
    render_vulkan_create_and_init_swapchain_context(render);
    render_vulkan_create_and_init_voxel_pipeline(render);
    render_vulkan_create_and_init_frame_context(render);

    render_vulkan_create_voxel_mesh(render);
}

void render_update(Render* render, World* world, f64 delta_time)
{
    vec3 forward;
    camera_get_forward(&world->camera, forward);

    vec3 center;
    glm_vec3_add(world->camera.position, forward, center);

    glm_mat4_identity(render->view_matrix);

    look_at_lh(world->camera.position, center, GLM_ZUP, render->view_matrix);

    glm_vec3_copy(world->camera.position, render->position);
    glm_mat4_mul(render->projection_matrix, render->view_matrix, render->projection_view_matrix);
}

void render_draw(Render* render)
{
    render_begin_frame(render);
    render_record_frame(render);
    render_submit_frame(render);
    render_present_frame(render);

    VulkanFrame* frame = &render->vulkan_frame_context.frame_array[render->vulkan_frame_context.frame_index];

    vkWaitForFences(
        render->vulkan_device_context.device, 
        1, 
        &frame->in_flight, 
        VK_TRUE, 
        UINT64_MAX
    );

    u32 image_index;

    vkAcquireNextImageKHR(
        render->vulkan_device_context.device, 
        render->vulkan_swapchain_context.swapchain, 
        UINT64_MAX, 
        frame->image_available, 
        VK_NULL_HANDLE, 
        &image_index
    );

    vkResetFences(render->vulkan_device_context.device, 1, &frame->in_flight);
    vkResetCommandBuffer(frame->command_buffer, 0);

    render_vulkan_record_command_buffer(render, frame->command_buffer, image_index);

    VkPipelineStageFlags wait_stage_array[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit_info = 
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame->image_available,
        .pWaitDstStageMask = wait_stage_array,
        .commandBufferCount = 1,
        .pCommandBuffers = &frame->command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &render->vulkan_swapchain_context.render_finished_array[image_index],
    };

    vkQueueSubmit(
        render->vulkan_device_context.graphics_queue,
        1,
        &submit_info,
        frame->in_flight
    );

    VkPresentInfoKHR present_info = 
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &render->vulkan_swapchain_context.render_finished_array[image_index],
        .swapchainCount = 1,
        .pSwapchains = &render->vulkan_swapchain_context.swapchain,
        .pImageIndices = &image_index,
    };

    vkQueuePresentKHR(render->vulkan_device_context.present_queue, &present_info);

    render->vulkan_frame_context.frame_index = (render->vulkan_frame_context.frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}