#include <vulkan/vulkan_core.h>
#ifndef VULKAN_BACKEND_INTERNAL_H
#define VULKAN_BACKEND_INTERNAL_H 1

#include <vulkan/vulkan.h>
#include <cglm/cglm.h>

#include "core/types.h"
#include "platform/platform.h"
#include "vulkan_backend.h"

struct VulkanTexture
{
    VkImage image;
    VkImageView image_view;
    VkDeviceMemory image_memory;
    VkSampler sampler;

    u32 width;
    u32 height;
};

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
    VkImageView depth_image_view;

    VkSemaphore* render_finished_array;
    VkFramebuffer* framebuffer_array;
};

struct VulkanPipelineContext
{
    VkRenderPass render_pass;
    VkPipelineLayout layout;
    VkPipeline pipeline;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_set;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_memory;
    VulkanTexture vulkan_texture;
};

struct VulkanFrame
{
    VkCommandBuffer command_buffer;
    VkSemaphore image_available;
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

typedef struct PushConstants
{
    mat4 projection_view_matrix;
} 
PushConstants;

// VULKAN DEVICE

void vulkan_backend_create_device_context(VulkanBackend* vulkan_backend, Platform* platform);
void vulkan_backend_destroy_device_context(VulkanBackend* vulkan_backend);

void vulkan_backend_create_instance(VulkanBackend* vulkan_backend);
void vulkan_backend_create_surface(VulkanBackend* vulkan_backend, Platform* platform);
void vulkan_backend_choose_physical_device(VulkanBackend* vulkan_backend);
void vulkan_backend_create_logical_device(VulkanBackend* vulkan_backend);
void vulkan_backend_create_command_pool(VulkanBackend* vulkan_backend);

// VULKAN SWAPCHAIN

void vulkan_backend_create_swapchain_context(VulkanBackend* vulkan_backend);
void vulkan_backend_destroy_swapchain_context(VulkanBackend* vulkan_backend);

void vulkan_backend_create_swapchain(VulkanBackend* vulkan_backend);
void vulkan_backend_create_frame_buffers(VulkanBackend* vulkan_backend);
void vulkan_backend_create_image_views(VulkanBackend* vulkan_backend);
void vulkan_backend_create_render_pass(VulkanBackend* vulkan_backend);
void vulkan_backend_create_depth_resources(VulkanBackend* vulkan_backend);

// VULKAN PIPELINE

void vulkan_backend_create_voxel_pipeline(VulkanBackend* vulkan_backend);
void vulkan_backend_destroy_voxel_pipeline(VulkanBackend* vulkan_backend);

void vulkan_backend_create_graphics_pipeline(VulkanBackend* vulkan_backend);
VkShaderModule vulkan_backend_create_shader_module(VkDevice device, const char* filename);

void vulkan_backend_update_texture_descriptor(
    VulkanBackend* vulkan_backend,
    VkImageView image_view,
    VkSampler sampler
);

// VULKAN FRAME

void vulkan_backend_create_fame_context(VulkanBackend* vulkan_backend);
void vulkan_backend_destroy_frame_context(VulkanBackend* vulkan_backend);

void vulkan_backend_record_command_buffer(VulkanBackend* vulkan_backend, VkCommandBuffer command_buffer, u32 image_index);
void vulkan_backend_draw_frame(VulkanBackend* vulkan_backend);

// VULKAN MEMORY

u32 vulkan_backend_locate_memory_type(
    VulkanBackend* vulkan_backend,
    u32 type_filter,
    VkMemoryPropertyFlags properties
);

void vulkan_backend_create_buffer(
    VulkanBackend* vulkan_backend,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer,
    VkDeviceMemory* memory
);

void vulkan_backend_create_image(
    VulkanBackend* vulkan_backend,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage* image,
    VkDeviceMemory* memory
);

VkImageView vulkan_backend_create_image_view(
    VulkanBackend* vulkan_backend,
    VkImage image,
    VkFormat format
);

VkSampler vulkan_backend_create_sampler(VulkanBackend* vulkan_backend);

void vulkan_backend_transition_image_layout(
    VulkanBackend* vulkan_backend,
    VkImage image,
    VkFormat format,
    VkImageLayout old_layout,
    VkImageLayout new_layout
);

void vulkan_backend_copy_buffer(
    VulkanBackend* vulkan_backend,
    VkBuffer src_buffer,
    VkBuffer dst_buffer,
    VkDeviceSize size
);

void vulkan_backend_copy_buffer_to_image(
    VulkanBackend* vulkan_backend,
    VkBuffer buffer,
    VkImage image,
    u32 width,
    u32 height
);

void vulkan_backend_create_texture_from_pixels(
    VulkanBackend* vulkan_backend,
    const void* pixels,
    u32 width,
    u32 height,
    VkImage* image,
    VkDeviceMemory* image_memory,
    VkImageView* image_view,
    VkSampler* sampler
);

void vulkan_backend_create_texture_from_file(
    VulkanBackend* vulkan_backend,
    const char* path,
    VkImage* image,
    VkDeviceMemory* image_memory,
    VkImageView* image_view,
    VkSampler* sampler
);

// VULKAN COMMANDS

VkCommandBuffer vulkan_backend_begin_single_time_commands(VulkanBackend* backend);
void vulkan_backend_end_single_time_commands(VulkanBackend* vulkan_backend, VkCommandBuffer command_buffer);

void vulkan_backend_copy_buffer_to_image(
    VulkanBackend* vulkan_backend,
    VkBuffer buffer,
    VkImage image,
    u32 width,
    u32 height
);

#endif