#include "vulkan_backend.h"
#include "vulkan_backend_internal.h"

void vb_create_frame_context(VulkanBackend* vulkan_backend)
{
    VkCommandBufferAllocateInfo command_buffevb_allocate_info;
    command_buffevb_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffevb_allocate_info.pNext = NULL;
    command_buffevb_allocate_info.commandPool = vulkan_backend->vulkan_device_context.command_pool;
    command_buffevb_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffevb_allocate_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    VkCommandBuffer command_buffevb_array[MAX_FRAMES_IN_FLIGHT];

    vkAllocateCommandBuffers(
        vulkan_backend->vulkan_device_context.device, 
        &command_buffevb_allocate_info, 
        command_buffevb_array
    );

    VkSemaphoreCreateInfo semaphore_create_info;
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = NULL;
    semaphore_create_info.flags = 0;

    VkFenceCreateInfo fence_create_info;
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.pNext = NULL;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    u32 frame_index;
    for (frame_index = 0; frame_index < MAX_FRAMES_IN_FLIGHT; ++frame_index)
    {
        VulkanFrame* frame = &vulkan_backend->vulkan_frame_context.frame_array[frame_index];
        
        frame->command_buffer = command_buffevb_array[frame_index];

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

void vb_record_command_buffer(VulkanBackend* vulkan_backend, VkCommandBuffer command_buffer, u32 image_index)
{
    VkCommandBufferBeginInfo command_buffevb_info;
    command_buffevb_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffevb_info.pNext = NULL;
    command_buffevb_info.flags = 0;
    command_buffevb_info.pInheritanceInfo = NULL;

    vkBeginCommandBuffer(command_buffer, &command_buffevb_info);

    VkClearValue cleavb_color;
    cleavb_color.color.float32[0] = 0.1f;
    cleavb_color.color.float32[1] = 0.1f;
    cleavb_color.color.float32[2] = 0.2f;
    cleavb_color.color.float32[3] = 1.0f;

    VkRect2D rendevb_area;
    rendevb_area.offset.x = 0;
    rendevb_area.offset.y = 0;
    rendevb_area.extent = vulkan_backend->vulkan_swapchain_context.extent;

    VkRenderPassBeginInfo rendevb_pass_begin_info;
    rendevb_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rendevb_pass_begin_info.pNext = NULL;
    rendevb_pass_begin_info.renderPass = vulkan_backend->voxel_pipeline_context.render_pass;
    rendevb_pass_begin_info.framebuffer = vulkan_backend->vulkan_swapchain_context.framebuffer_array[image_index];
    rendevb_pass_begin_info.renderArea = rendevb_area;
    rendevb_pass_begin_info.clearValueCount = 1;
    rendevb_pass_begin_info.pClearValues = &cleavb_color;

    vkCmdBeginRenderPass(
        command_buffer,
        &rendevb_pass_begin_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        command_buffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        vulkan_backend->voxel_pipeline_context.pipeline
    );
    
    vkCmdDraw(command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);
    vkEndCommandBuffer(command_buffer);
}

void vb_render_frame(VulkanBackend* vulkan_backend)
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

    vb_record_command_buffer(vulkan_backend, frame->command_buffer, image_index);

    VkPipelineStageFlags wait_stage_array[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &frame->image_available;
    submit_info.pWaitDstStageMask = wait_stage_array;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &frame->command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &frame->render_finished;

    vkQueueSubmit(
        vulkan_backend->vulkan_device_context.graphics_queue,
        1,
        &submit_info,
        frame->in_flight
    );

    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = NULL;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &frame->render_finished;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vulkan_backend->vulkan_swapchain_context.swapchain;
    present_info.pImageIndices = &image_index;
    present_info.pResults = NULL;

    vkQueuePresentKHR(vulkan_backend->vulkan_device_context.present_queue, &present_info);

    vulkan_backend->vulkan_frame_context.frame_index = (vulkan_backend->vulkan_frame_context.frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}