#ifndef PLATFORM_INTERNAL_H
#define PLATFORM_INTERNAL_H 1

#include <GLFW/glfw3.h>

#include "core/types.h"

#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   768

struct PlatformInput
{
    boolean current_key_array[GLFW_KEY_LAST + 1];
    boolean previous_key_array[GLFW_KEY_LAST + 1];
    
    boolean current_mouse_array[GLFW_MOUSE_BUTTON_LAST + 1];
    boolean previous_mouse_array[GLFW_MOUSE_BUTTON_LAST + 1];

    double current_mouse_x;
    double current_mouse_y;
    double previous_mouse_x;
    double previous_mouse_y;
};

struct PlatformWindow
{
    struct GLFWwindow* handle;
    u32 width;
    u32 height;
    boolean should_close;
};

struct Platform
{
    struct PlatformInput platform_input;
    struct PlatformWindow platform_window;
};

void pi_init(struct PlatformInput* platform_input);
void pw_init(struct PlatformWindow* platform_window);

void pi_record_inputs(struct PlatformInput* platform_input, struct PlatformWindow* platform_window);

#endif