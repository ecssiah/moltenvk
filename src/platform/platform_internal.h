#ifndef PLATFORM_INTERNAL_H
#define PLATFORM_INTERNAL_H 1

#include "platform/platform.h"

#include <GLFW/glfw3.h>

#include "core/types.h"

#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   768

struct PlatformInput
{
    bool current_key_array[GLFW_KEY_LAST + 1];
    bool previous_key_array[GLFW_KEY_LAST + 1];
    
    bool current_mouse_array[GLFW_MOUSE_BUTTON_LAST + 1];
    bool previous_mouse_array[GLFW_MOUSE_BUTTON_LAST + 1];

    f64 current_mouse_x;
    f64 current_mouse_y;
    f64 previous_mouse_x;
    f64 previous_mouse_y;
};

struct PlatformWindow
{
    struct GLFWwindow* glfw_window;

    u32 width;
    u32 height;
    bool close_requested;
};

struct Platform
{
    struct PlatformInput platform_input;
    struct PlatformWindow platform_window;
};

void platform_poll_events(struct PlatformWindow* platform_window);
void platform_record_input(struct PlatformInput* platform_input, struct PlatformWindow* platform_window);

#endif