#pragma once

#include <vulkan/vulkan.h>

struct FrameContext 
{
    VkCommandBuffer command_buffer = {};
    VkSemaphore image_available = {};
    VkSemaphore render_finished = {};
    VkFence in_flight = {};
};