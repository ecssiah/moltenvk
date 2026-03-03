#include "platform/platform.h"
#include "platform/platform_internal.h"

#include <stdlib.h>

static void pi_init(PlatformInput* platform_input)
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
}

PlatformInput* platform_input_create()
{
    PlatformInput* platform_input = malloc(sizeof (PlatformInput));

    pi_init(platform_input);

    return platform_input;
}

void platform_input_destroy(PlatformInput *platform_input) 
{
    free(platform_input);
}