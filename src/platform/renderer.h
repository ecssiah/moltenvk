#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

struct Renderer
{

    void init();
    void draw_frame();
    void wait_idle();
    void cleanup();

    GLFWwindow* window = nullptr;

    VkInstance instance = {};
    VkSurfaceKHR surface = {};
    VkPhysicalDevice physical_device = {VK_NULL_HANDLE};
    VkDevice device = {};

    VkQueue graphics_queue = {};
    VkQueue present_queue = {};
    
    uint32_t graphics_queue_familiy_index = {UINT32_MAX};

    VkSwapchainKHR swapchain = {};
    VkFormat swapchain_format = {};
    VkExtent2D swapchain_extent = {};
    std::vector<VkImage> swapchain_image_vec;
    std::vector<VkImageView> image_view_vec;

    VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;

    std::vector<VkFramebuffer> frame_buffer_vec;

    VkCommandPool command_pool = {};

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    std::vector<VkCommandBuffer> command_buffer_vec;

    std::vector<VkSemaphore> image_available_semaphore_vec;
    std::vector<VkSemaphore> render_finished_semaphore_vec;
    std::vector<VkFence> in_flight_fence_vec;

    uint32_t current_frame = 0;

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
    void create_sync_objects();
    void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);
    void create_graphics_pipeline();

};