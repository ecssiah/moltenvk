#include "platform/platform.h"
#include "platform/platform_internal.h"

#include <stdlib.h>

#include "core/log.h"

void pi_init(PlatformInput* platform_input)
{
    for (int key = 0; key < GLFW_KEY_LAST + 1; ++key)
    {
        platform_input->current_key_array[key] = false;
        platform_input->previous_key_array[key] = false;
    }
    
    for (int button = 0; button < GLFW_MOUSE_BUTTON_LAST + 1; ++button)
    {
        platform_input->current_mouse_array[button] = false;
        platform_input->previous_mouse_array[button] = false;
    }

    platform_input->current_mouse_x = 0.0;
    platform_input->current_mouse_y = 0.0;
    platform_input->previous_mouse_x = 0.0;
    platform_input->previous_mouse_y = 0.0;

    LOG_INFO("Platform Input initialized");
}

void p_poll_events(PlatformInput* platform_input, PlatformWindow* platform_window)
{
    glfwPollEvents();

    if (glfwWindowShouldClose(platform_window->glfw_window))
    {
        platform_window->close_requested = true;
    }
}

void p_record_inputs(PlatformInput* platform_input, PlatformWindow* platform_window)
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

void p_handle_inputs(PlatformInput* platform_input, PlatformWindow* platform_window)
{
    if (platform_input->current_key_array[GLFW_KEY_ESCAPE])
    {
        glfwSetWindowShouldClose(platform_window->glfw_window, GLFW_TRUE);
    }
}