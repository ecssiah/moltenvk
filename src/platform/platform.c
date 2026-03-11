#include "platform/platform_internal.h"

#include <stdlib.h>

#include "core/log/log.h"

static void platform_input_init(PlatformInput* platform_input);
static void platform_window_init(PlatformWindow* platform_window);

Platform* platform_create()
{
    Platform* platform = malloc(sizeof(*platform));

    return platform;
}

void platform_destroy(Platform* platform)
{
    glfwDestroyWindow(platform->platform_window.glfw_window);
    glfwTerminate();

    free(platform);
}

void platform_init(Platform* platform)
{
    platform_input_init(&platform->platform_input);
    platform_window_init(&platform->platform_window);
}

bool platform_is_active(Platform* platform)
{
    return !glfwWindowShouldClose(platform->platform_window.glfw_window);
}

VkSurfaceKHR platform_create_vulkan_surface(Platform* platform, VkInstance instance)
{
    VkSurfaceKHR surface;

    VkResult surface_result =
        glfwCreateWindowSurface(
            instance,
            platform->platform_window.glfw_window,
            NULL,
            &surface
        );

    if (surface_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create GLFW surface");
    }

    return surface;
}

void platform_update(Platform* platform)
{
    platform_poll_events(&platform->platform_window);
    platform_record_input(&platform->platform_input, &platform->platform_window);

    if (platform_is_key_released(&platform->platform_input, GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(platform->platform_window.glfw_window, GLFW_TRUE);
    }
}

static void platform_input_init(PlatformInput* platform_input)
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

static void platform_window_init(PlatformWindow* platform_window)
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