#ifndef PLATFORM_H
#define PLATFORM_H 1

#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct PlatformInput PlatformInput;
typedef struct PlatformWindow PlatformWindow;
typedef struct Platform Platform;

Platform* platform_create();
void platform_destroy(Platform* platform);

void platform_init(Platform* platform);
void platform_update(Platform* platform);

bool platform_is_active(Platform* platform);

bool platform_is_key_down(PlatformInput* platform_input, int key);
bool platform_is_key_pressed(PlatformInput* platform_input, int key);
bool platform_is_key_released(PlatformInput* platform_input, int key);

bool platform_is_mouse_down(PlatformInput* platform_input, int button);
bool platform_is_mouse_pressed(PlatformInput* platform_input, int button);
bool platform_is_mouse_released(PlatformInput* platform_input, int button);

VkSurfaceKHR platform_create_vulkan_surface(Platform* platform, VkInstance instance);

#endif