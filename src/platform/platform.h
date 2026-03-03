#ifndef PLATFORM_H
#define PLATFORM_H 1

#include <vulkan/vulkan.h>

#include "core/types.h"

typedef struct PlatformWindow PlatformWindow;

PlatformWindow* platform_window_create();
void platform_window_init(PlatformWindow* window, const char* title);
void platform_window_destroy(PlatformWindow* window);
void platform_window_poll_events(PlatformWindow* window);
boolean platform_window_should_close(PlatformWindow* window);
VkSurfaceKHR platform_window_create_vulkan_surface(PlatformWindow* window, VkInstance instance);

#endif