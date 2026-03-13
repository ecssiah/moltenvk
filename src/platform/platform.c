#include "platform/platform.h"

#include <stdlib.h>

#include "core/log/log.h"

static void platform_input_init(Platform* platform);
static void platform_window_init(Platform* platform);

Platform* platform_create(void)
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
    platform_input_init(platform);
    platform_window_init(platform);
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

void glfw_framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
    Platform* platform = glfwGetWindowUserPointer(window);

    if (width == 0 || height == 0)
    {
        return;
    }

    platform->platform_window.width  = (u32)width;
    platform->platform_window.height = (u32)height;

    if (platform->framebuffer_resize_callback)
    {
        platform->framebuffer_resize_callback(platform, width, height);
    }
}

static void platform_input_init(Platform* platform)
{
    for (int key = 0; key < GLFW_KEY_LAST + 1; ++key)
    {
        platform->platform_input.current_key_array[key] = false;
        platform->platform_input.previous_key_array[key] = false;
    }
    
    for (int button = 0; button < GLFW_MOUSE_BUTTON_LAST + 1; ++button)
    {
        platform->platform_input.current_mouse_array[button] = false;
        platform->platform_input.previous_mouse_array[button] = false;
    }

    platform->platform_input.current_mouse_x = 0.0;
    platform->platform_input.current_mouse_y = 0.0;
    platform->platform_input.previous_mouse_x = 0.0;
    platform->platform_input.previous_mouse_y = 0.0;

    LOG_INFO("Platform Input initialized");
}

static void platform_window_init(Platform* platform)
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

    platform->platform_window.glfw_window = glfwCreateWindow(
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        window_title, 
        NULL, 
        NULL
    );

    glfwSetWindowUserPointer(platform->platform_window.glfw_window, platform);

    glfwSetFramebufferSizeCallback(
        platform->platform_window.glfw_window,
        glfw_framebuffer_resize_callback
    );

    glfwSetInputMode(
        platform->platform_window.glfw_window, 
        GLFW_CURSOR, 
        GLFW_CURSOR_DISABLED
    );

    int width, height;
    glfwGetFramebufferSize(platform->platform_window.glfw_window, &width, &height);

    platform->platform_window.width  = (u32)width;
    platform->platform_window.height = (u32)height;

    platform->platform_window.close_requested = false;

    if (platform->platform_window.glfw_window)
    {
        LOG_INFO("GLFW window created");
    }
    else
    {
        LOG_FATAL("GLFW Failed to create window");
    }

    LOG_INFO("Platform Window initialized");
}