#include "platform/platform_internal.h"

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

void platform_handle_input(PlatformInput* platform_input, PlatformWindow* platform_window)
{
    if (platform_input->current_key_array[GLFW_KEY_ESCAPE])
    {
        glfwSetWindowShouldClose(platform_window->glfw_window, GLFW_TRUE);
    }
}