#ifndef SWAPCHAIN_CONTEXT_H
#define SWAPCHAIN_CONTEXT_H

#include <vulkan/vulkan.h>

struct SwapchainContext
{
    // Swapchain
    VkSwapchainKHR swapchain;
    VkFormat format;
    VkExtent2D extent;

    uint32_t image_count;
    VkImage* image_array;
    VkImageView* image_view_array;

    // Depth attachment (swapchain-dependent)
    VkFormat depth_format;
    VkImage depth_image;
    VkDeviceMemory depth_memory;
    VkImageView depth_view;

    VkFramebuffer* framebuffer_array;
};

#endif