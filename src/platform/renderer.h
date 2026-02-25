#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

struct Renderer
{

    void init();
    void cleanup();
    void draw();

private:

    void create_instance();
    void create_surface(GLFWwindow* window);
    void pick_physical_device();
    void create_logical_device();
    void create_swapchain();
    void create_image_views();
    void create_render_pass();
    void create_frame_buffers();
    void create_command_pool();
    void allocate_command_buffer();

    GLFWwindow* window = nullptr;

    VkInstance instance = {};
    VkSurfaceKHR surface = {};
    VkPhysicalDevice physical_device = {VK_NULL_HANDLE};
    VkDevice device = {};
    VkQueue graphics_queue = {};
    uint32_t graphics_queue_familiy_index = {UINT32_MAX};

    VkSwapchainKHR swapchain = {};
    VkFormat swapchain_format = {};
    VkExtent2D swapchain_extent = {};
    std::vector<VkImage> swapchain_image_vec;
    std::vector<VkImageView> image_view_vec;

    VkRenderPass render_pass = {};
    std::vector<VkFramebuffer> frame_buffer_vec;

    VkCommandPool command_pool = {};
    VkCommandBuffer command_buffer = {};

};