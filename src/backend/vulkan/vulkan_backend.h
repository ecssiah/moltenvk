#ifndef VULKAN_BACKEND_H
#define VULKAN_BACKEND_H 1

#include "platform/platform.h"
#include "renderer/renderer.h"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct VulkanBackend VulkanBackend;

typedef struct VulkanSwapchainContext VulkanSwapchainContext;
typedef struct VulkanPipelineContext VulkanPipelineContext;
typedef struct VulkanFrame VulkanFrame;
typedef struct VulkanFrameContext VulkanFrameContext;
typedef struct VulkanDeviceContext VulkanDeviceContext;

typedef struct VulkanTexture VulkanTexture;

VulkanBackend* vulkan_backend_create(Platform* platform);
void vulkan_backend_render(VulkanBackend* backend);
void vulkan_backend_destroy(VulkanBackend* backend);

void vulkan_backend_create_texture_from_image(VulkanBackend* backend, Image* image);

#endif