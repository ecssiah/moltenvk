#ifndef RENDER_H
#define RENDER_H 1

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "core/types.h"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct
{
    // Swapchain
    VkSwapchainKHR swapchain;
    VkFormat format;
    VkExtent2D extent;

    u32 image_count;
    VkImage* image_array;
    VkImageView* image_view_array;

    // Depth attachment (swapchain-dependent)
    VkFormat depth_format;
    VkImage depth_image;
    VkDeviceMemory depth_memory;
    VkImageView depth_view;

    VkFramebuffer* framebuffer_array;
}
SwapchainContext;

typedef struct
{
    VkRenderPass render_pass;
    VkPipelineLayout layout;
    VkPipeline pipeline;
}
Pipeline;

typedef struct 
{
    VkCommandBuffer command_buffer;
    VkSemaphore image_available;
    VkSemaphore render_finished;
    VkFence in_flight;
}
FrameContext;

typedef struct
{
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice device;
    u32 graphics_queue_family_index;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkCommandPool command_pool;
}
VulkanContext;

typedef struct
{
    VulkanContext vulkan_context;
    SwapchainContext swapchain_context;
    FrameContext frame_context_array[MAX_FRAMES_IN_FLIGHT];
    Pipeline voxel_pipeline;
    
    u32 frame_index;
}
Render;

void r_init(Render* render, GLFWwindow* window);
void r_render(Render* render);
void r_resize(Render* render);
void r_quit(Render* render);

void r_create_swapchain_context(Render* render);
void r_destroy_swapchain_context(Render* render);

void r_create_voxel_pipeline(Render* render);

void r_create_instance(Render* render);
void r_create_surface(Render* render, GLFWwindow* window);
void r_pick_physical_device(Render* render);
void r_create_logical_device(Render* render);
void r_create_frame_contexts(Render* render);

void r_create_swapchain(Render* render);
void r_create_image_views(Render* render);
void r_create_render_pass(Render* render);
void r_create_graphics_pipeline(Render* render);
void r_create_frame_buffers(Render* render);

void r_create_command_pool(Render* render);
void r_record_command_buffer(Render* render, VkCommandBuffer command_buffer, u32 image_index);

size_t r_read_file_binary(const char* filename, char** out_buffer);
VkShaderModule r_create_shader_module(VkDevice device, const char* filename);

#endif