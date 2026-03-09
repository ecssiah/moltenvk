#include "vulkan_backend_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "core/log/log.h"

VulkanBackend* vulkan_backend_create(Platform* platform)
{
    VulkanBackend* vulkan_backend = malloc(sizeof(*vulkan_backend));

    // Device lifetime
    vulkan_backend_create_instance(vulkan_backend);
    vulkan_backend_create_surface(vulkan_backend, platform);
    vulkan_backend_choose_physical_device(vulkan_backend);
    vulkan_backend_create_logical_device(vulkan_backend);
    vulkan_backend_create_command_pool(vulkan_backend);

    LOG_INFO("Vulkan Device Initialized");

    // Swapchain lifetime
    vulkan_backend_create_swapchain_context(vulkan_backend);

    LOG_INFO("Vulkan Swapchain Initialized");

    // Engine resources
    vulkan_backend_create_voxel_pipeline(vulkan_backend);

    LOG_INFO("Vulkan Pipeline Initialized");

    vulkan_backend_create_fame_context(vulkan_backend);

    LOG_INFO("Vulkan Frame Initialized");

    return vulkan_backend;
}

void vulkan_backend_render(VulkanBackend* vulkan_backend)
{
    vulkan_backend_draw_frame(vulkan_backend);
}

void vulkan_backend_destroy(VulkanBackend* vulkan_backend)
{
    vkDeviceWaitIdle(vulkan_backend->vulkan_device_context.device);

    vulkan_backend_destroy_swapchain_context(vulkan_backend);

    for (u32 frame_index = 0; frame_index < MAX_FRAMES_IN_FLIGHT; ++frame_index)
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

void vulkan_backend_create_texture_from_image(VulkanBackend* backend, Image* image)
{

}
