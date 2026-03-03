#include "platform/platform.h"
#include "platform/platform_internal.h"

#include <stdlib.h>

#include "core/log.h"

void pi_init(PlatformInput* platform_input)
{
    u32 key_index;
    for (key_index = 0; key_index < GLFW_KEY_LAST + 1; ++key_index)
    {
        platform_input->current_key_array[key_index] = false;
        platform_input->previous_key_array[key_index] = false;
    }

    u32 mouse_button_index;
    for (mouse_button_index = 0; mouse_button_index < GLFW_MOUSE_BUTTON_LAST + 1; ++mouse_button_index)
    {
        platform_input->current_mouse_array[mouse_button_index] = false;
        platform_input->previous_mouse_array[mouse_button_index] = false;
    }

    platform_input->current_mouse_x = 0.0;
    platform_input->current_mouse_y = 0.0;
    platform_input->previous_mouse_x = 0.0;
    platform_input->previous_mouse_y = 0.0;

    LOG_INFO("Platform Input initialized");
}

void pi_record_inputs(PlatformInput* platform_input, PlatformWindow* platform_window)
{
    GLFWwindow* window = platform_window->handle;

    int key;
    for (key = 0; key < GLFW_KEY_LAST + 1; ++key)
    {
        platform_input->previous_key_array[key] = platform_input->current_key_array[key];
        platform_input->current_key_array[key] = glfwGetKey(window, key) == GLFW_PRESS;
    }

    int button;
    for (button = 0; button < GLFW_MOUSE_BUTTON_LAST + 1; ++button)
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
