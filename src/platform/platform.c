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

void platform_update(Platform* platform)
{
    p_poll_events(&platform->platform_input, &platform->platform_window);
    p_handle_inputs(&platform->platform_input, &platform->platform_window);
}

void platform_destroy(Platform* platform)
{
    free(platform);
}

boolean platform_is_active(Platform* platform)
{
    return !glfwWindowShouldClose(platform->platform_window.handle);
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
