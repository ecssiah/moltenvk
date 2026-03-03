#include "platform/platform.h"
#include "platform/platform_internal.h"

#include <stdlib.h>

#include "core/log.h"

Platform* platform_create()
{
    Platform* platform = malloc(sizeof (Platform));

    pi_init(&platform->platform_input);
    pw_init(&platform->platform_window);

    return platform;
}

void platform_destroy(Platform* platform)
{
    free(platform);
}

boolean platform_should_close(Platform* platform)
{
    return glfwWindowShouldClose(platform->platform_window.handle);
}

void platform_poll_events(Platform* platform)
{
    glfwPollEvents();

    pi_record_inputs(&platform->platform_input, &platform->platform_window);
}

void platform_handle_inputs(Platform* platform)
{
    if (platform->platform_input.current_key_array[GLFW_KEY_ESCAPE])
    {
        glfwSetWindowShouldClose(platform->platform_window.handle, GLFW_TRUE);
    }
}

void platform_window_destroy(Platform* platform) 
{
    glfwDestroyWindow(platform->platform_window.handle);
    glfwTerminate();
}

VkSurfaceKHR platform_create_vulkan_surface(Platform* platform, VkInstance instance)
{
    VkSurfaceKHR surface;

    VkResult surface_result =
        glfwCreateWindowSurface(
            instance,
            platform->platform_window.handle,
            NULL,
            &surface
        );

    if (surface_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create GLFW surface");
    }

    return surface;
}
