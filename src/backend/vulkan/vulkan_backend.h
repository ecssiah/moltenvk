#ifndef VULKAN_BACKEND_H
#define VULKAN_BACKEND_H 1

#include "platform/platform.h"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct VulkanSwapchainContext VulkanSwapchainContext;
typedef struct VulkanPipelineContext VulkanPipelineContext;
typedef struct VulkanFrame VulkanFrame;
typedef struct VulkanFrameContext VulkanFrameContext;
typedef struct VulkanDeviceContext VulkanDeviceContext;

typedef struct VulkanBackend VulkanBackend;

VulkanBackend* vulkan_backend_create(Platform* platform);
void vulkan_backend_draw(VulkanBackend* backend);
void vulkan_backend_destroy(VulkanBackend* backend);

#endif