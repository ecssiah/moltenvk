#pragma  once

#include <vulkan/vulkan.h>

struct SwapchainContext
{
    // Swapchain
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent2D extent = {};

    uint32_t image_count;
    VkImage* image_array;
    VkImageView* image_view_array;

    // Depth attachment (swapchain-dependent)
    VkFormat depth_format = VK_FORMAT_UNDEFINED;
    VkImage depth_image = VK_NULL_HANDLE;
    VkDeviceMemory depth_memory = VK_NULL_HANDLE;
    VkImageView depth_view = VK_NULL_HANDLE;

    VkFramebuffer* framebuffer_array;
};