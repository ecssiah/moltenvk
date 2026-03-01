#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "frame_context.h"
#include "swapchain_context.h"

static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

struct Render
{
    // ------------------------------------------------------------
    // Platform
    // ------------------------------------------------------------
    GLFWwindow* window = nullptr;

    // ------------------------------------------------------------
    // Frame contexts (per-frame in flight resources)
    // ------------------------------------------------------------
    std::vector<FrameContext> frame_context_vec;
    uint32_t current_frame = 0;

    // ------------------------------------------------------------
    // Device lifetime (destroyed only at shutdown)
    // ------------------------------------------------------------
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue present_queue = VK_NULL_HANDLE;
    uint32_t graphics_queue_familiy_index = UINT32_MAX;

    VkCommandPool command_pool = VK_NULL_HANDLE;

    // ------------------------------------------------------------
    // Swapchain lifetime (destroyed on resize)
    // ------------------------------------------------------------
    SwapchainContext swapchain_context;

    // ------------------------------------------------------------
    // Pipeline (swapchain dependent for now)
    // ------------------------------------------------------------
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPipeline graphics_pipeline = VK_NULL_HANDLE;
};

void render_init(Render* render);
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
void record_command_buffer(Render* render, VkCommandBuffer command_buffer, uint32_t image_index);

