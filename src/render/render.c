#include "render/render.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define GLFW_INCLUDE_VULKAN

#include "core/log/log.h"
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
    render->window_width = WINDOW_WIDTH;
    render->window_height = WINDOW_HEIGHT;

    render->framebuffer_resized = false;

    glm_vec3_zero(render->position);

    glm_mat4_identity(render->view_matrix);
    glm_mat4_identity(render->projection_matrix);
    glm_mat4_identity(render->projection_view_matrix);

    const float fov = glm_rad(60.0f);
    const float aspect_ratio = (float)render->window_width / (float)render->window_height;
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

    render_nuklear_init(render);

    platform->framebuffer_resize_callback = render_framebuffer_resize_callback;
    platform->render = render;
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

void render_begin_frame(Render* render, VulkanFrame* vulkan_frame)
{
    vkWaitForFences(
        render->vulkan_device_context.device,
        1,
        &vulkan_frame->in_flight_fence,
        VK_TRUE,
        UINT64_MAX
    );
}

bool render_record_frame(Render* render, VulkanFrame* vulkan_frame)
{
    u32 image_index;

    VkResult acquire_result = vkAcquireNextImageKHR(
        render->vulkan_device_context.device,
        render->vulkan_swapchain_context.swapchain,
        UINT64_MAX,
        vulkan_frame->image_available_semaphore,
        VK_NULL_HANDLE,
        &image_index
    );

    if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        render_vulkan_recreate_swapchain(render);

        return false;
    }

    if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR)
    {
        LOG_FATAL("Failed to acquire swapchain image.");
    }

    vkResetFences(render->vulkan_device_context.device, 1, &vulkan_frame->in_flight_fence);
    vkResetCommandBuffer(vulkan_frame->command_buffer, 0);

    render_vulkan_record_command_buffer(render, vulkan_frame->command_buffer, image_index);

    // Record Nuklear UI draw commands into the same command buffer
    render_nuklear_record(render, vulkan_frame->command_buffer);

    vulkan_frame->image_index = image_index;

    return true;
}

void render_submit_frame(Render* render, VulkanFrame* vulkan_frame)
{
    u32 image_index = vulkan_frame->image_index;

    VkPipelineStageFlags wait_stage_array[] = 
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSubmitInfo submit_info =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vulkan_frame->image_available_semaphore,
        .pWaitDstStageMask = wait_stage_array,
        .commandBufferCount = 1,
        .pCommandBuffers = &vulkan_frame->command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &vulkan_frame->render_finished_semaphore,
    };

    vkQueueSubmit(
        render->vulkan_device_context.graphics_queue,
        1,
        &submit_info,
        vulkan_frame->in_flight_fence
    );
}

void render_present_frame(Render* render, VulkanFrame* vulkan_frame)
{
    u32 image_index = vulkan_frame->image_index;

    VkPresentInfoKHR present_info =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vulkan_frame->render_finished_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &render->vulkan_swapchain_context.swapchain,
        .pImageIndices = &image_index,
    };

    VkResult present_result = vkQueuePresentKHR(
        render->vulkan_device_context.present_queue,
        &present_info
    );

    if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR)
    {
        render_vulkan_recreate_swapchain(render);

        return;
    }

    if (present_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to present swapchain image");
    }
}

void render_draw(Render* render)
{
    if (render->framebuffer_resized)
    {
        render_vulkan_recreate_swapchain(render);

        render->framebuffer_resized = false;
    }

    u32 current_frame_index = render->vulkan_frame_context.frame_index;

    VulkanFrame* vulkan_frame = &render->vulkan_frame_context.frame_array[current_frame_index];

    render_begin_frame(render, vulkan_frame);

    render_nuklear_draw(render);
    render_nuklear_convert(render);
    render_nuklear_upload(render);

    if (!render_record_frame(render, vulkan_frame))
    {
        return;
    }

    render_submit_frame(render, vulkan_frame);
    render_present_frame(render, vulkan_frame);

    u32 next_frame_index = (render->vulkan_frame_context.frame_index + 1) % MAX_FRAMES_IN_FLIGHT;

    render->vulkan_frame_context.frame_index = next_frame_index;
}

void render_framebuffer_resize_callback(Platform* platform, int width, int height)
{
    Render* render = platform->render;

    render->window_width = width;
    render->window_height = height;

    render->framebuffer_resized = true;
}