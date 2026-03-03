#ifndef PLATFORM_H
#define PLATFORM_H 1

#include <vulkan/vulkan.h>

#include "core/types.h"

typedef struct PlatformInput PlatformInput;
typedef struct PlatformWindow PlatformWindow;
typedef struct Platform Platform;

Platform* platform_create();
void platform_update(Platform* platform);
void platform_destroy(Platform* platform);

bool platform_is_active(Platform* platform);

VkSurfaceKHR platform_create_vulkan_surface(Platform* platform, VkInstance instance);

#endif