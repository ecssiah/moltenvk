#include <vulkan/vulkan_core.h>
#ifndef VULKAN_BACKEND_INTERNAL_H
#define VULKAN_BACKEND_INTERNAL_H 1

#include <vulkan/vulkan.h>

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
    VkImageView depth_view;

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

// VULKAN DEVICE

void vd_create_instance(VulkanBackend* vulkan_backend);
void vd_create_surface(VulkanBackend* vulkan_backend, Platform* platform);
void vd_pick_physical_device(VulkanBackend* vulkan_backend);
void vd_create_logical_device(VulkanBackend* vulkan_backend);
void vd_create_command_pool(VulkanBackend* vulkan_backend);

// VULKAN SWAPCHAIN

void vs_create_swapchain_context(VulkanBackend* vulkan_backend);
void vs_destroy_swapchain_context(VulkanBackend* vulkan_backend);
void vs_create_swapchain(VulkanBackend* vulkan_backend);
void vs_create_frame_buffers(VulkanBackend* vulkan_backend);
void vs_create_image_views(VulkanBackend* vulkan_backend);
void vs_create_render_pass(VulkanBackend* vulkan_backend);

// VULKAN PIPELINE

void vp_create_voxel_pipeline(VulkanBackend* vulkan_backend);
void vp_create_graphics_pipeline(VulkanBackend* vulkan_backend);
VkShaderModule vp_create_shader_module(VkDevice device, const char* filename);

void vp_update_texture_descriptor(
    VulkanBackend* vulkan_backend,
    VkImageView image_view,
    VkSampler sampler
);

// VULKAN FRAME

void vf_record_command_buffer(VulkanBackend* vulkan_backend, VkCommandBuffer command_buffer, u32 image_index);
void vf_create_frame_context(VulkanBackend* vulkan_backend);
void vf_render_frame(VulkanBackend* vulkan_backend);

// VULKAN MEMORY

u32 vm_find_memory_type(
    VulkanBackend* vulkan_backend,
    u32 type_filter,
    VkMemoryPropertyFlags properties
);

void vm_create_buffer(
    VulkanBackend* vulkan_backend,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer,
    VkDeviceMemory* memory
);

void vm_create_image(
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

VkImageView vm_create_image_view(
    VulkanBackend* vulkan_backend,
    VkImage image,
    VkFormat format
);

VkSampler vm_create_sampler(VulkanBackend* vulkan_backend);

void vm_transition_image_layout(
    VulkanBackend* vulkan_backend,
    VkImage image,
    VkFormat format,
    VkImageLayout old_layout,
    VkImageLayout new_layout
);

void vm_copy_buffer(
    VulkanBackend* vulkan_backend,
    VkBuffer src_buffer,
    VkBuffer dst_buffer,
    VkDeviceSize size
);

void vm_copy_buffer_to_image(
    VulkanBackend* vulkan_backend,
    VkBuffer buffer,
    VkImage image,
    u32 width,
    u32 height
);

void vm_create_texture_from_pixels(
    VulkanBackend* vulkan_backend,
    const void* pixels,
    u32 width,
    u32 height,
    VkImage* image,
    VkDeviceMemory* image_memory,
    VkImageView* image_view,
    VkSampler* sampler
);

void vm_create_texture_from_file(
    VulkanBackend* vulkan_backend,
    const char* path,
    VkImage* image,
    VkDeviceMemory* image_memory,
    VkImageView* image_view,
    VkSampler* sampler
);

// VULKAN COMMANDS

VkCommandBuffer vc_begin_single_time_commands(VulkanBackend* backend);
void vc_end_single_time_commands(VulkanBackend* vulkan_backend, VkCommandBuffer command_buffer);

void vc_copy_buffer_to_image(
    VulkanBackend* vulkan_backend,
    VkBuffer buffer,
    VkImage image,
    u32 width,
    u32 height
);

#endif