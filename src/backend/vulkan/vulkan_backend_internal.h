#ifndef VULKAN_BACKEND_INTERNAL_H
#define VULKAN_BACKEND_INTERNAL_H 1

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "core/types.h"
#include "platform/platform.h"
#include "vulkan_backend.h"

struct VulkanSwapchainContext
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
};

struct VulkanPipelineContext
{
    VkRenderPass render_pass;
    VkPipelineLayout layout;
    VkPipeline pipeline;
};

struct VulkanFrame
{
    VkCommandBuffer command_buffer;
    VkSemaphore image_available;
    VkSemaphore render_finished;
    VkFence in_flight;
};

struct VulkanFrameContext
{
    u32 frame_index;
    VulkanFrame frame_array[MAX_FRAMES_IN_FLIGHT];
};

struct VulkanDeviceContext
{
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice device;
    u32 graphics_queue_family_index;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkCommandPool command_pool;
};

struct VulkanBackend
{
    VulkanSwapchainContext vulkan_swapchain_context;
    VulkanFrameContext vulkan_frame_context;
    VulkanPipelineContext voxel_pipeline_context;
    VulkanDeviceContext vulkan_device_context;
};

void vb_create_instance(VulkanBackend* vulkan_backend);
void vb_create_surface(VulkanBackend* vulkan_backend, PlatformWindow* window);
void vb_pick_physical_device(VulkanBackend* vulkan_backend);
void vb_create_logical_device(VulkanBackend* vulkan_backend);

void vb_create_swapchain_context(VulkanBackend* vulkan_backend);
void vb_destroy_swapchain_context(VulkanBackend* vulkan_backend);
void vb_create_swapchain(VulkanBackend* vulkan_backend);
void vb_create_image_views(VulkanBackend* vulkan_backend);
void vb_create_render_pass(VulkanBackend* vulkan_backend);

void vb_create_frame_context(VulkanBackend* vulkan_backend);
void vb_render_frame(VulkanBackend* vulkan_backend);

void vb_create_voxel_pipeline(VulkanBackend* vulkan_backend);
void vb_create_graphics_pipeline(VulkanBackend* vulkan_backend);
void vb_create_frame_buffers(VulkanBackend* vulkan_backend);

void vb_create_command_pool(VulkanBackend* vulkan_backend);
void vb_record_command_buffer(VulkanBackend* vulkan_backend, VkCommandBuffer command_buffer, u32 image_index);

VkShaderModule vb_create_shader_module(VkDevice device, const char* filename);

#endif