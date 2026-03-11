#include "backend/vulkan/vulkan_backend.h"
#include "vulkan_backend_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

VulkanBackend* vulkan_backend_create(Platform* platform)
{
    VulkanBackend* vulkan_backend = malloc(sizeof(*vulkan_backend));

    vulkan_backend_create_device_context(vulkan_backend, platform);
    vulkan_backend_create_swapchain_context(vulkan_backend);
    vulkan_backend_create_voxel_pipeline(vulkan_backend);
    vulkan_backend_create_frame_context(vulkan_backend);

    return vulkan_backend;
}

void vulkan_backend_init(VulkanBackend* vulkan_backend)
{
    vulkan_backend_create_voxel_mesh(vulkan_backend);
}

void vulkan_backend_destroy(VulkanBackend* vulkan_backend)
{
    vkDeviceWaitIdle(vulkan_backend->vulkan_device_context.device);

    vulkan_backend_destroy_swapchain_context(vulkan_backend);
    vulkan_backend_destroy_frame_context(vulkan_backend);
    vulkan_backend_destroy_voxel_pipeline(vulkan_backend);
    vulkan_backend_destroy_device_context(vulkan_backend);

    free(vulkan_backend);
}

void vulkan_backend_draw(VulkanBackend* vulkan_backend)
{
    vulkan_backend_draw_frame(vulkan_backend);
}