#pragma  once

#include <vulkan/vulkan.h>

struct VulkanContext
{
    VkInstance instance = {};
    VkSurfaceKHR surface = {};
    VkPhysicalDevice physical_device = {};
    VkDevice device = {};
    uint32_t graphics_queue_family_index = UINT32_MAX;
    VkQueue graphics_queue = {};
    VkQueue present_queue = {};
    VkCommandPool command_pool = {};
};