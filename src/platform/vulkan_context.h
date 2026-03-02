#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H

#include <vulkan/vulkan.h>

#include "core/types.h"

typedef struct
{
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice device;
    uint32 graphics_queue_family_index;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkCommandPool command_pool;
}
VulkanContext;

#endif