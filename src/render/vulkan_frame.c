#include "render/render.h"

#include <cglm/cglm.h>

#include "core/log/log.h"

void vulkan_backend_create_and_init_frame_context(Render* render)
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = render->vulkan_device_context.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    VkCommandBuffer command_buffer_array[MAX_FRAMES_IN_FLIGHT];

    vkAllocateCommandBuffers(
        render->vulkan_device_context.device, 
        &command_buffer_allocate_info, 
        command_buffer_array
    );

    VkSemaphoreCreateInfo semaphore_create_info = 
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fence_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (u32 frame_index = 0; frame_index < MAX_FRAMES_IN_FLIGHT; ++frame_index)
    {
        VulkanFrame* frame = &render->vulkan_frame_context.frame_array[frame_index];
        
        frame->command_buffer = command_buffer_array[frame_index];

        vkCreateSemaphore(
            render->vulkan_device_context.device, 
            &semaphore_create_info, 
            NULL, 
            &frame->image_available
        );

        vkCreateFence(
            render->vulkan_device_context.device, 
            &fence_create_info, 
            NULL, 
            &frame->in_flight
        );
    }

    render->vulkan_frame_context.frame_index = 0;

    LOG_INFO("Vulkan Frame Initialized");
}

void vulkan_backend_destroy_frame_context(Render* render)
{
    for (u32 frame_index = 0; frame_index < MAX_FRAMES_IN_FLIGHT; ++frame_index)
    {
        VulkanFrame* frame = &render->vulkan_frame_context.frame_array[frame_index];

        vkDestroyFence(render->vulkan_device_context.device, frame->in_flight, NULL);
        vkDestroySemaphore(render->vulkan_device_context.device, frame->image_available, NULL);
    }
}

void vulkan_backend_record_command_buffer(Render* render, VkCommandBuffer command_buffer, u32 image_index) 
{
    VkCommandBufferBeginInfo command_buffer_info = 
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    vkBeginCommandBuffer(command_buffer, &command_buffer_info);

    VkRect2D render_area = 
    {
        .offset = {0, 0},
        .extent = render->vulkan_swapchain_context.extent,
    };

    VkClearValue clear_values[2];

    clear_values[0].color = (VkClearColorValue){{0.0f, 0.0f, 0.0f, 1.0f}};
    clear_values[1].depthStencil = (VkClearDepthStencilValue){1.0f, 0};

    VkRenderPassBeginInfo render_pass_begin_info =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render->voxel_pipeline_context.render_pass,
        .framebuffer = render->vulkan_swapchain_context.framebuffer_array[image_index],
        .renderArea = render_area,
        .clearValueCount = 2,
        .pClearValues = clear_values
    };

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_begin_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        command_buffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        render->voxel_pipeline_context.pipeline
    );

    VkViewport viewport =
    {
        .x = 0,
        .y = 0,
        .width  = (float)render_area.extent.width,
        .height = (float)render_area.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor =
    {
        .offset = {0, 0},
        .extent = render_area.extent,
    };

    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    VkDeviceSize offset_array[] = {0};

    vkCmdBindVertexBuffers(
        command_buffer,
        0,
        1,
        &render->voxel_pipeline_context.vertex_buffer,
        offset_array
    );

    vkCmdBindDescriptorSets(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        render->voxel_pipeline_context.layout,
        0,
        1,
        &render->voxel_pipeline_context.descriptor_set,
        0,
        NULL
    );

    PushConstants push_constants;

    glm_mat4_mul(render->projection_matrix, render->view_matrix, render->projection_view_matrix);
    glm_mat4_copy(render->projection_view_matrix, push_constants.projection_view_matrix);

    vkCmdPushConstants(
        render->vulkan_frame_context.frame_array[render->vulkan_frame_context.frame_index].command_buffer,
        render->voxel_pipeline_context.layout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(PushConstants),
        &push_constants
    );

    vkCmdDraw(
        command_buffer,
        36,
        1,
        0,
        0
    );

    vkCmdEndRenderPass(command_buffer);
    vkEndCommandBuffer(command_buffer);
}

void vulkan_backend_draw_frame(Render* render) 
{

}