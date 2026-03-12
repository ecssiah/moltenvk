#ifndef PLATFORM_H
#define PLATFORM_H 1

#include <stdbool.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/types.h"

#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   768

typedef struct PlatformInput
{
    bool current_key_array[GLFW_KEY_LAST + 1];
    bool previous_key_array[GLFW_KEY_LAST + 1];
    
    bool current_mouse_array[GLFW_MOUSE_BUTTON_LAST + 1];
    bool previous_mouse_array[GLFW_MOUSE_BUTTON_LAST + 1];

    f64 current_mouse_x;
    f64 current_mouse_y;
    f64 previous_mouse_x;
    f64 previous_mouse_y;
}
PlatformInput;

typedef struct PlatformWindow
{
    struct GLFWwindow* glfw_window;

    u32 width;
    u32 height;
    bool close_requested;
}
PlatformWindow;

typedef struct Platform
{
    struct PlatformInput platform_input;
    struct PlatformWindow platform_window;
}
Platform;

Platform* platform_create(void);
void platform_destroy(Platform* platform);

void platform_init(Platform* platform);
void platform_update(Platform* platform);

void platform_poll_events(struct PlatformWindow* platform_window);
void platform_record_input(struct PlatformInput* platform_input, struct PlatformWindow* platform_window);

bool platform_is_active(Platform* platform);

bool platform_is_key_down(PlatformInput* platform_input, int key);
bool platform_is_key_pressed(PlatformInput* platform_input, int key);
bool platform_is_key_released(PlatformInput* platform_input, int key);

bool platform_is_mouse_down(PlatformInput* platform_input, int button);
bool platform_is_mouse_pressed(PlatformInput* platform_input, int button);
bool platform_is_mouse_released(PlatformInput* platform_input, int button);

f64 platform_mouse_delta_x(PlatformInput* platform_input);
f64 platform_mouse_delta_y(PlatformInput* platform_input);

VkSurfaceKHR platform_create_vulkan_surface(Platform* platform, VkInstance instance);

#endif