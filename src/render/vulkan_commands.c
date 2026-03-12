#include "render/render.h"

VkCommandBuffer render_vulkan_begin_single_time_commands(Render* render)
{
    VkCommandBufferAllocateInfo alloc_info = 
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = render->vulkan_device_context.command_pool,
        .commandBufferCount = 1
    };

    VkCommandBuffer command_buffer;

    vkAllocateCommandBuffers(
        render->vulkan_device_context.device,
        &alloc_info,
        &command_buffer
    );

    VkCommandBufferBeginInfo begin_info = 
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

void render_vulkan_end_single_time_commands(Render* render, VkCommandBuffer command_buffer)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = 
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer
    };

    vkQueueSubmit(
        render->vulkan_device_context.graphics_queue,
        1,
        &submit_info,
        VK_NULL_HANDLE
    );

    vkQueueWaitIdle(render->vulkan_device_context.graphics_queue);

    vkFreeCommandBuffers(
        render->vulkan_device_context.device,
        render->vulkan_device_context.command_pool,
        1,
        &command_buffer
    );
}
