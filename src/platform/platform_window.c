#include "platform/platform_internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "core/log.h"

static void pw_init(PlatformWindow* platform_window, const char* title)
{
    if (glfwInit())
    {
        LOG_INFO("GLFW initialized");
    }
    else
    {
        LOG_FATAL("GLFW initialization failed");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    platform_window->handle = glfwCreateWindow(
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        title, 
        NULL, 
        NULL
    );

    if (platform_window->handle)
    {
        LOG_INFO("GLFW window created");
    }
    else
    {
        LOG_FATAL("GLFW Failed to create window");
    }
}

PlatformWindow* platform_window_create()
{
    PlatformWindow* platform_window = malloc(sizeof (PlatformWindow));

    pw_init(platform_window, "Vulkan Test");

    return platform_window;
}

boolean platform_window_should_close(PlatformWindow *window)
{
    return glfwWindowShouldClose(window->handle);
}

void platform_window_poll_events(PlatformWindow *window)
{
    glfwPollEvents();
}

void platform_window_destroy(PlatformWindow *window) 
{
    glfwDestroyWindow(window->handle);
    glfwTerminate();
}

VkSurfaceKHR platform_window_create_vulkan_surface(PlatformWindow *window, VkInstance instance)
{
    VkSurfaceKHR surface;

    VkResult surface_result =
        glfwCreateWindowSurface(
            instance,
            window->handle,
            NULL,
            &surface
        );

    if (surface_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create GLFW surface");
    }

    return surface;
}