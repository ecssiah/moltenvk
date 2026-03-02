#ifndef FRAME_CONTEXT_H
#define FRAME_CONTEXT_H

#include <vulkan/vulkan.h>

typedef struct 
{
    VkCommandBuffer command_buffer;
    VkSemaphore image_available;
    VkSemaphore render_finished;
    VkFence in_flight;
}
FrameContext;

#endif