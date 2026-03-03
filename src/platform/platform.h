#ifndef PLATFORM_H
#define PLATFORM_H 1

#include <vulkan/vulkan.h>

#include "core/types.h"

typedef struct PlatformInput PlatformInput;
typedef struct PlatformWindow PlatformWindow;

PlatformWindow* platform_window_create();
void platform_window_destroy(PlatformWindow* platform_window);

void platform_window_poll_events(PlatformWindow* platform_window);
boolean platform_window_should_close(PlatformWindow* platform_window);
VkSurfaceKHR platform_window_create_vulkan_surface(PlatformWindow* platform_window, VkInstance instance);

PlatformInput* platform_input_create();
void platform_input_destroy(PlatformInput* platform_input);

#endif