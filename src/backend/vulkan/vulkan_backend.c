#include "vulkan_backend_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "core/log.h"

VulkanBackend* vulkan_backend_create(Platform* platform)
{
    VulkanBackend* vulkan_backend = malloc(sizeof (VulkanBackend));

    // Device lifetime
    vb_create_instance(vulkan_backend);
    vb_create_surface(vulkan_backend, platform);
    vb_pick_physical_device(vulkan_backend);
    vb_create_logical_device(vulkan_backend);
    vb_create_command_pool(vulkan_backend);
    vb_create_frame_context(vulkan_backend);

    LOG_INFO("Vulkan Device Initialized");

    // Swapchain lifetime
    vb_create_swapchain_context(vulkan_backend);

    LOG_INFO("Vulkan Swapchain Initialized");

    // Engine resources
    vb_create_voxel_pipeline(vulkan_backend);

    LOG_INFO("Vulkan Pipeline Initialized");

    return vulkan_backend;
}

void vulkan_backend_destroy(VulkanBackend* vulkan_backend)
{
    vkDeviceWaitIdle(vulkan_backend->vulkan_device_context.device);

    vb_destroy_swapchain_context(vulkan_backend);

    u32 frame_index;
    for (frame_index = 0; frame_index < MAX_FRAMES_IN_FLIGHT; ++frame_index)
    {
        VulkanFrame* frame = &vulkan_backend->vulkan_frame_context.frame_array[frame_index];

        vkDestroyFence(vulkan_backend->vulkan_device_context.device, frame->in_flight, NULL);
        vkDestroySemaphore(vulkan_backend->vulkan_device_context.device, frame->image_available, NULL);
        vkDestroySemaphore(vulkan_backend->vulkan_device_context.device, frame->render_finished, NULL);
    }

    vkDestroyPipeline(vulkan_backend->vulkan_device_context.device, vulkan_backend->voxel_pipeline_context.pipeline, NULL);
    vkDestroyPipelineLayout(vulkan_backend->vulkan_device_context.device, vulkan_backend->voxel_pipeline_context.layout, NULL);
    vkDestroyRenderPass(vulkan_backend->vulkan_device_context.device, vulkan_backend->voxel_pipeline_context.render_pass, NULL);

    vkDestroyCommandPool(vulkan_backend->vulkan_device_context.device, vulkan_backend->vulkan_device_context.command_pool, NULL);
    vkDestroyDevice(vulkan_backend->vulkan_device_context.device, NULL);
    vkDestroySurfaceKHR(vulkan_backend->vulkan_device_context.instance, vulkan_backend->vulkan_device_context.surface, NULL);
    vkDestroyInstance(vulkan_backend->vulkan_device_context.instance, NULL);

    free(vulkan_backend);
}