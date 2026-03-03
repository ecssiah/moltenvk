#include "platform/platform.h"
#include "platform/platform_internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "core/log.h"

void pw_init(PlatformWindow* platform_window)
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

    const char* window_title = "Vulkan Test";

    platform_window->glfw_window = glfwCreateWindow(
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        window_title, 
        NULL, 
        NULL
    );

    int width, height;
    glfwGetFramebufferSize(platform_window->glfw_window, &width, &height);

    platform_window->width  = (u32)width;
    platform_window->height = (u32)height;

    platform_window->close_requested = false;

    if (platform_window->glfw_window)
    {
        LOG_INFO("GLFW window created");
    }
    else
    {
        LOG_FATAL("GLFW Failed to create window");
    }

    LOG_INFO("Platform Window initialized");
}
