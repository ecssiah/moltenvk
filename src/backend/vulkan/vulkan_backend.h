#ifndef VULKAN_BACKEND_H
#define VULKAN_BACKEND_H 1

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct VulkanBackend VulkanBackend;

typedef struct VulkanSwapchainContext VulkanSwapchainContext;
typedef struct VulkanPipelineContext VulkanPipelineContext;
typedef struct VulkanFrame VulkanFrame;
typedef struct VulkanFrameContext VulkanFrameContext;
typedef struct VulkanDeviceContext VulkanDeviceContext;

typedef struct VulkanTexture VulkanTexture;

typedef struct Platform Platform;
typedef struct Image Image;

VulkanBackend* vulkan_backend_create(Platform* platform);
void vulkan_backend_init(VulkanBackend* vulkan_backend);
void vulkan_backend_destroy(VulkanBackend* vulkan_backend);

void vulkan_backend_draw(VulkanBackend* vulkan_backend);

#endif