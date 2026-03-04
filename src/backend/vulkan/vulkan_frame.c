#include "vulkan_backend.h"
#include "vulkan_backend_internal.h"

void vf_create_frame_context(VulkanBackend* vulkan_backend)
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vulkan_backend->vulkan_device_context.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    VkCommandBuffer command_buffer_array[MAX_FRAMES_IN_FLIGHT];

    vkAllocateCommandBuffers(
        vulkan_backend->vulkan_device_context.device, 
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
        VulkanFrame* frame = &vulkan_backend->vulkan_frame_context.frame_array[frame_index];
        
        frame->command_buffer = command_buffer_array[frame_index];

        vkCreateSemaphore(
            vulkan_backend->vulkan_device_context.device, 
            &semaphore_create_info, 
            NULL, 
            &frame->image_available
        );

        vkCreateSemaphore(
            vulkan_backend->vulkan_device_context.device, 
            &semaphore_create_info, 
            NULL, 
            &frame->render_finished
        );

        vkCreateFence(
            vulkan_backend->vulkan_device_context.device, 
            &fence_create_info, 
            NULL, 
            &frame->in_flight
        );
    }
}

void vf_record_command_buffer(VulkanBackend* vulkan_backend, VkCommandBuffer command_buffer, u32 image_index)
{
    VkCommandBufferBeginInfo command_buffer_info = 
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    vkBeginCommandBuffer(command_buffer, &command_buffer_info);

    VkClearValue clear_color =
    {
        .color = { 0.1f, 0.1f, 0.2f, 1.0f },
    };

    VkRect2D render_area = 
    {
        .offset = {0, 0},
        .extent = vulkan_backend->vulkan_swapchain_context.extent,
    };

    VkRenderPassBeginInfo render_pass_begin_info =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = vulkan_backend->voxel_pipeline_context.render_pass,
        .framebuffer = vulkan_backend->vulkan_swapchain_context.framebuffer_array[image_index],
        .renderArea = render_area,
        .clearValueCount = 1,
        .pClearValues = &clear_color,
    };

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_begin_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        command_buffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        vulkan_backend->voxel_pipeline_context.pipeline
    );

    VkDeviceSize offset_array[] = {0};

    vkCmdBindVertexBuffers(
        command_buffer,
        0,
        1,
        &vulkan_backend->voxel_pipeline_context.vertex_buffer,
        offset_array
    );

    vkCmdBindDescriptorSets(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        vulkan_backend->voxel_pipeline_context.layout,
        0,
        1,
        &vulkan_backend->voxel_pipeline_context.descriptor_set,
        0,
        NULL
    );
    
    vkCmdDraw(
        command_buffer,
        6,
        1,
        0,
        0
    );

    vkCmdEndRenderPass(command_buffer);
    vkEndCommandBuffer(command_buffer);
}

void vf_render_frame(VulkanBackend* vulkan_backend)
{
    VulkanFrame* frame = &vulkan_backend->vulkan_frame_context.frame_array[vulkan_backend->vulkan_frame_context.frame_index];

    vkWaitForFences(
        vulkan_backend->vulkan_device_context.device, 
        1, 
        &frame->in_flight, 
        VK_TRUE, 
        UINT64_MAX
    );

    u32 image_index;

    vkAcquireNextImageKHR(
        vulkan_backend->vulkan_device_context.device, 
        vulkan_backend->vulkan_swapchain_context.swapchain, 
        UINT64_MAX, 
        frame->image_available, 
        VK_NULL_HANDLE, 
        &image_index
    );

    vkResetFences(vulkan_backend->vulkan_device_context.device, 1, &frame->in_flight);
    vkResetCommandBuffer(frame->command_buffer, 0);

    vf_record_command_buffer(vulkan_backend, frame->command_buffer, image_index);

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
        .pSignalSemaphores = &frame->render_finished,
    };

    vkQueueSubmit(
        vulkan_backend->vulkan_device_context.graphics_queue,
        1,
        &submit_info,
        frame->in_flight
    );

    VkPresentInfoKHR present_info = 
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame->render_finished,
        .swapchainCount = 1,
        .pSwapchains = &vulkan_backend->vulkan_swapchain_context.swapchain,
        .pImageIndices = &image_index,
    };

    vkQueuePresentKHR(vulkan_backend->vulkan_device_context.present_queue, &present_info);

    vulkan_backend->vulkan_frame_context.frame_index = (vulkan_backend->vulkan_frame_context.frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}