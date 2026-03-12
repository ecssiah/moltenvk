#include "platform/platform.h"

#include <stdlib.h>

void platform_poll_events(PlatformWindow* platform_window)
{
    glfwPollEvents();

    if (glfwWindowShouldClose(platform_window->glfw_window))
    {
        platform_window->close_requested = true;
    }
}

void platform_record_input(PlatformInput* platform_input, PlatformWindow* platform_window)
{
    GLFWwindow* window = platform_window->glfw_window;
    
    for (int key = 0; key < GLFW_KEY_LAST + 1; ++key)
    {
        platform_input->previous_key_array[key] = platform_input->current_key_array[key];
        platform_input->current_key_array[key] = glfwGetKey(window, key) == GLFW_PRESS;
    }

    for (int button = 0; button < GLFW_MOUSE_BUTTON_LAST + 1; ++button)
    {
        platform_input->previous_mouse_array[button] = platform_input->current_mouse_array[button];
        platform_input->current_mouse_array[button] = glfwGetMouseButton(window, button) == GLFW_PRESS;
    }

    platform_input->previous_mouse_x = platform_input->current_mouse_x;
    platform_input->previous_mouse_y = platform_input->current_mouse_y;

    glfwGetCursorPos(
        window,
        &platform_input->current_mouse_x,
        &platform_input->current_mouse_y
    );
}

bool platform_is_key_down(PlatformInput* platform_input, int key)
{
    return platform_input->current_key_array[key];
}

bool platform_is_key_pressed(PlatformInput* platform_input, int key)
{
    return platform_input->current_key_array[key] && !platform_input->previous_key_array[key];
}

bool platform_is_key_released(PlatformInput* platform_input, int key)
{
    return !platform_input->current_key_array[key] && platform_input->previous_key_array[key];
}

bool platform_is_mouse_down(PlatformInput* platform_input, int button)
{
    return platform_input->current_mouse_array[button];
}

bool platform_is_mouse_pressed(PlatformInput* platform_input, int button)
{
    return platform_input->current_mouse_array[button] && !platform_input->previous_mouse_array[button];
}

bool platform_is_mouse_released(PlatformInput* platform_input, int button)
{
    return !platform_input->current_mouse_array[button] && platform_input->previous_mouse_array[button];
}

f64 platform_mouse_delta_x(PlatformInput* platform_input)
{
    return platform_input->current_mouse_x - platform_input->previous_mouse_x;
}

f64 platform_mouse_delta_y(PlatformInput* platform_input)
{
    return platform_input->current_mouse_y - platform_input->previous_mouse_y;
}

