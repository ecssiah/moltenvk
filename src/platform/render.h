#ifndef RENDER_H
#define RENDER_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "core/types.h"
#include "pipeline.h"
#include "vulkan_context.h"
#include "frame_context.h"
#include "swapchain_context.h"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct
{
    VulkanContext vulkan_context;
    
    SwapchainContext swapchain_context;

    uint32 frame_index;
    FrameContext frame_context_array[MAX_FRAMES_IN_FLIGHT];

    Pipeline voxel_pipeline;
}
Render;

void render_init(Render* render, GLFWwindow* window);
void render_frame(Render* render);
void render_resize(Render* render);
void render_destroy(Render* render);

void create_swapchain_context(Render* render);
void destroy_swapchain_context(Render* render);

void create_voxel_pipeline(Render* render);

void create_instance(Render* render);
void create_surface(Render* render, GLFWwindow* window);
void pick_physical_device(Render* render);
void create_logical_device(Render* render);
void create_frame_contexts(Render* render);

void create_swapchain(Render* render);
void create_image_views(Render* render);
void create_render_pass(Render* render);
void create_graphics_pipeline(Render* render);
void create_frame_buffers(Render* render);

void create_command_pool(Render* render);
void record_command_buffer(Render* render, VkCommandBuffer command_buffer, uint32 image_index);

#endif