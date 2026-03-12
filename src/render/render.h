#ifndef RENDER_H
#define RENDER_H 1

#include <cglm/cglm.h>
#define GLFW_INCLUDE_VULKAN

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear/nuklear.h"

#include "core/types.h"
#include "platform/platform.h"

#define MAX_FRAMES_IN_FLIGHT 2
#define CUBE_RADIUS 0.5f

#define NUKLEAR_MAX_VERTEX_BUFFER  (512 * 1024)
#define NUKLEAR_MAX_INDEX_BUFFER   (128 * 1024)

typedef struct Platform Platform;
typedef struct World World;

typedef struct Vertex
{
    vec3 position;
    vec2 uv;
}
Vertex;

static const Vertex cube_vertex_array[] =
{
    // +X
    {{ +CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +1.0f}},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS }, { +1.0f, +0.0f}},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +0.0f}},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +1.0f}},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +0.0f}},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS }, { +0.0f, +1.0f}},

    // -X
    {{ -CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +0.0f }},
    {{ -CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS }, { +0.0f, +1.0f }},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +1.0f }},
    {{ -CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +0.0f }},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +1.0f }},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS }, { +1.0f, +0.0f }},

    // +Y
    {{ -CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS }, { +0.0f, +1.0f }},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +1.0f }},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS }, { +1.0f, +0.0f }},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS }, { +0.0f, +1.0f }},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS }, { +1.0f, +0.0f }},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +0.0f }},

    // -Y
    {{ -CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +0.0f }},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS }, { +1.0f, +0.0f }},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +1.0f }},
    {{ -CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS }, { +0.0f, +0.0f }},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS }, { +1.0f, +1.0f }},
    {{ -CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS }, { +0.0f, +1.0f }},

    // +Z
    {{ -CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS}, { +0.0f, +1.0f }},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS}, { +0.0f, +0.0f }},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS}, { +1.0f, +0.0f }},
    {{ -CUBE_RADIUS, -CUBE_RADIUS, +CUBE_RADIUS}, { +0.0f, +1.0f }},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS}, { +1.0f, +0.0f }},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, +CUBE_RADIUS}, { +1.0f, +1.0f }},

    // -Z
    {{ +CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS}, { +0.0f, +1.0f}},
    {{ -CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS}, { +0.0f, +0.0f}},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS}, { +1.0f, +0.0f}},
    {{ +CUBE_RADIUS, -CUBE_RADIUS, -CUBE_RADIUS}, { +0.0f, +1.0f}},
    {{ -CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS}, { +1.0f, +0.0f}},
    {{ +CUBE_RADIUS, +CUBE_RADIUS, -CUBE_RADIUS}, { +1.0f, +1.0f}},
};

typedef struct Image
{
    u32 width;
    u32 height;
    u32 channels;
    u8* pixels;
}
Image;

typedef struct VulkanTexture
{
    VkImage image;
    VkImageView image_view;
    VkDeviceMemory image_memory;
    VkSampler sampler;

    u32 width;
    u32 height;
}
VulkanTexture;

typedef struct VulkanSwapchainContext
{
    VkSwapchainKHR swapchain;
    VkFormat format;
    VkExtent2D extent;

    VkRenderPass render_pass;

    u32 image_count;
    VkImage* image_array;
    VkImageView* image_view_array;

    VkFormat depth_format;
    VkImage depth_image;
    VkDeviceMemory depth_memory;
    VkImageView depth_image_view;

    VkSemaphore* render_finished_array;
    VkFramebuffer* framebuffer_array;
}
VulkanSwapchainContext;

typedef struct VulkanPipelineContext
{
    VkPipelineLayout layout;
    VkPipeline pipeline;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_set;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_memory;
    VulkanTexture vulkan_texture;
}
VulkanPipelineContext;

typedef struct VulkanFrame
{
    VkCommandBuffer command_buffer;
    VkSemaphore image_available;
    VkFence in_flight;
}
VulkanFrame;

typedef struct VulkanFrameContext
{
    u32 frame_index;
    VulkanFrame frame_array[MAX_FRAMES_IN_FLIGHT];
}
VulkanFrameContext;

typedef struct VulkanDeviceContext
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
VulkanDeviceContext;

typedef struct VoxelPushConstants
{
    mat4 projection_view_matrix;
}
VoxelPushConstants;

typedef struct NkVertex
{
    float position[2];
    float uv[2];

    nk_byte color[4];
}
NkVertex;

typedef struct NuklearPushConstants
{
    float scale[2];
    float translate[2];
} 
NuklearPushConstants;

typedef struct NuklearContext
{
    struct nk_context context;
    struct nk_font_atlas atlas;
    struct nk_font* font;

    struct nk_buffer commands;
    struct nk_buffer vertices;
    struct nk_buffer indices;

    struct nk_draw_null_texture null_texture;

    VkImage font_image;
    VkDeviceMemory font_image_memory;
    VkImageView font_image_view;
    VkSampler font_sampler;

    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;
    void* vertex_buffer_mapped;

    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;
    void* index_buffer_mapped;

    u32 vertex_count;
    u32 index_count;
}
NuklearContext;

typedef struct Render
{
    vec3 position;

    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 projection_view_matrix;

    VulkanDeviceContext vulkan_device_context;
    VulkanSwapchainContext vulkan_swapchain_context;
    
    VulkanPipelineContext voxel_pipeline_context;
    VulkanPipelineContext nuklear_pipeline_context;

    VulkanFrameContext vulkan_frame_context;

    NuklearContext nuklear_context;
}
Render;

Render* render_create(Platform* platform);
void render_destroy(Render* render);

void render_init(Render* render, Platform* platform);
void render_update(Render* render, World* world, f64 delta_time);

void render_begin_frame(Render* render);
void render_record_frame(Render* render);
void render_submit_frame(Render* render);
void render_present_frame(Render* render);

Image render_image_load(const char* path);
void render_image_destroy(Image* image);

// VULKAN DEVICE

void render_vulkan_create_and_init_device_context(Render* render, Platform* platform);
void render_vulkan_destroy_device_context(Render* render);

void render_vulkan_create_instance(Render* render);
void render_vulkan_create_surface(Render* render, Platform* platform);
void render_vulkan_choose_physical_device(Render* render);
void render_vulkan_create_logical_device(Render* render);
void render_vulkan_create_command_pool(Render* render);

// VULKAN SWAPCHAIN

void render_vulkan_create_and_init_swapchain_context(Render* render);
void render_vulkan_destroy_swapchain_context(Render* render);

void render_vulkan_create_swapchain(Render* render);
void render_vulkan_create_frame_buffers(Render* render);
void render_vulkan_create_image_views(Render* render);
void render_vulkan_create_render_pass(Render* render);
void render_vulkan_create_depth_resources(Render* render);

// VULKAN PIPELINE

VkShaderModule render_vulkan_create_shader_module(VkDevice device, const char* filename);

void render_vulkan_update_texture_descriptor(
    Render* render,
    VkImageView image_view,
    VkSampler sampler
);

void render_vulkan_create_and_init_voxel_pipeline(Render* render);
void render_vulkan_destroy_voxel_pipeline(Render* render);

void render_vulkan_create_and_init_nuklear_pipeline(Render* render);
void render_vulkan_destroy_nuklear_pipeline(Render* render);

// VULKAN FRAME

void render_vulkan_create_and_init_frame_context(Render* render);
void render_vulkan_destroy_frame_context(Render* render);

void render_vulkan_create_and_init_frame_context(Render* render);

void render_vulkan_record_command_buffer(Render* render, VkCommandBuffer command_buffer, u32 image_index);
void render_vulkan_draw_frame(Render* render);

// VULKAN MEMORY

u32 render_vulkan_locate_memory_type(
    Render* render,
    u32 type_filter,
    VkMemoryPropertyFlags properties
);

void render_vulkan_create_buffer(
    Render* render,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer,
    VkDeviceMemory* memory
);

void render_vulkan_create_image(
    Render* render,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage* image,
    VkDeviceMemory* memory
);

VkImageView render_vulkan_create_image_view(
    Render* render,
    VkImage image,
    VkFormat format
);

VkSampler render_vulkan_create_sampler(Render* render);

void render_vulkan_transition_image_layout(
    Render* render,
    VkImage image,
    VkFormat format,
    VkImageLayout old_layout,
    VkImageLayout new_layout
);

void render_vulkan_copy_buffer(
    Render* render,
    VkBuffer src_buffer,
    VkBuffer dst_buffer,
    VkDeviceSize size
);

void render_vulkan_copy_buffer_to_image(
    Render* render,
    VkBuffer buffer,
    VkImage image,
    u32 width,
    u32 height
);

void render_vulkan_create_texture_from_pixels(
    Render* render,
    const void* pixels,
    u32 width,
    u32 height,
    VkImage* image,
    VkDeviceMemory* image_memory,
    VkImageView* image_view,
    VkSampler* sampler
);

void render_vulkan_create_texture_from_file(
    Render* render,
    const char* path,
    VkImage* image,
    VkDeviceMemory* image_memory,
    VkImageView* image_view,
    VkSampler* sampler
);

void render_vulkan_create_voxel_mesh(Render* render);

// VULKAN COMMANDS

VkCommandBuffer render_vulkan_begin_single_time_commands(Render* render);
void render_vulkan_end_single_time_commands(Render* render, VkCommandBuffer command_buffer);

void render_vulkan_copy_buffer_to_image(
    Render* render,
    VkBuffer buffer,
    VkImage image,
    u32 width,
    u32 height
);

// NUKLEAR

void render_nuklear_init(Render* render);
void render_nuklear_convert(Render* render);
void render_nuklear_upload(Render* render);
void render_nuklear_record(Render* render, VkCommandBuffer cmd);
void render_nuklear_draw(Render* render);
void render_nuklear_shutdown(Render* render);

#endif