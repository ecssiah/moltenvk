#pragma  once

#include <vulkan/vulkan.h>
#include <vector>

struct SwapchainContext
{
    // Swapchain
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent2D extent = {};

    // Swapchain images
    std::vector<VkImage> image_vec = {};
    std::vector<VkImageView> image_view_vec = {};

    // Depth attachment (swapchain-dependent)
    VkFormat depth_format = VK_FORMAT_UNDEFINED;
    VkImage depth_image = VK_NULL_HANDLE;
    VkDeviceMemory depth_memory = VK_NULL_HANDLE;
    VkImageView depth_view = VK_NULL_HANDLE;

    // Framebuffers (one per swapchain image)
    std::vector<VkFramebuffer> framebuffer_vec = {};
};