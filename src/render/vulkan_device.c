#include "render/render.h"

#include <stdlib.h>
#include <stdio.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/log/log.h"

void vulkan_backend_create_and_init_device_context(Render* render, Platform* platform)
{
    vulkan_backend_create_instance(render);
    vulkan_backend_create_surface(render, platform);
    vulkan_backend_choose_physical_device(render);
    vulkan_backend_create_logical_device(render);
    vulkan_backend_create_command_pool(render);

    LOG_INFO("Vulkan Device Initialized");
}

void vulkan_backend_create_instance(Render* render)
{
    u32 extension_count = 0;
    const char** extension_array = glfwGetRequiredInstanceExtensions(&extension_count);
    const char** required_extension_array = malloc(sizeof (const char*) * (extension_count + 1));
    
    for (u32 extension_index = 0; extension_index < extension_count; ++extension_index)
    {
        required_extension_array[extension_index] = extension_array[extension_index];
    }

    required_extension_array[extension_count] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

    const char* application_name = "Vulkan Test";
    const char* engine_name = "Vulkan Test Engine";

    VkApplicationInfo application_info = 
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = application_name,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = engine_name,
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_2,
    };

    const char* layer_name_array[] =
    {
        // "VK_LAYER_KHRONOS_validation"
    };

    VkInstanceCreateInfo instance_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = layer_name_array,
        .enabledExtensionCount = extension_count + 1,
        .ppEnabledExtensionNames = required_extension_array,
    };

    int instance_result = 
        vkCreateInstance(
            &instance_create_info, 
            NULL, 
            &render->vulkan_device_context.instance
        );

    if (instance_result != VK_SUCCESS) 
    {
        free(required_extension_array);

        LOG_FATAL("Failed to create Vulkan instance");
    }

    free(required_extension_array);
}

void vulkan_backend_create_surface(Render* render, Platform* platform)
{
    render->vulkan_device_context.surface = 
        platform_create_vulkan_surface(
            platform, 
            render->vulkan_device_context.instance
        );
}

void vulkan_backend_choose_physical_device(Render* render)
{
    u32 device_count = 0;

    vkEnumeratePhysicalDevices(
        render->vulkan_device_context.instance, 
        &device_count, 
        NULL
    );

    if (device_count == 0)
    {
        LOG_FATAL("No Vulkan physical devices found");
    }

    VkPhysicalDevice* physical_device_array = malloc(sizeof (VkPhysicalDevice) * device_count);

    vkEnumeratePhysicalDevices(
        render->vulkan_device_context.instance, 
        &device_count, 
        physical_device_array
    );

    for (u32 device_index = 0; device_index < device_count; ++device_index)
    {
        VkPhysicalDevice device = physical_device_array[device_index];

        u32 queue_family_count = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(
            device, 
            &queue_family_count, 
            NULL
        );

        VkQueueFamilyProperties* queue_family_properties_array = malloc(sizeof (VkQueueFamilyProperties) * queue_family_count);

        vkGetPhysicalDeviceQueueFamilyProperties(
            device, 
            &queue_family_count, 
            queue_family_properties_array
        );
        
        for (u32 queue_family_index = 0; queue_family_index < queue_family_count; ++queue_family_index)
        {
            VkBool32 present_support = VK_FALSE;

            vkGetPhysicalDeviceSurfaceSupportKHR(
                device, 
                queue_family_index, 
                render->vulkan_device_context.surface, 
                &present_support
            );

            if (
                (queue_family_properties_array[queue_family_index].queueFlags & VK_QUEUE_GRAPHICS_BIT) && 
                present_support
            ) {
                render->vulkan_device_context.physical_device = device;
                render->vulkan_device_context.graphics_queue_family_index = queue_family_index;

                free(queue_family_properties_array);
                free(physical_device_array);

                return;
            }
        }

        free(queue_family_properties_array);
    }

    free(physical_device_array);

    LOG_FATAL("No suitable GPU found");
}

void vulkan_backend_create_logical_device(Render* render)
{
    const f32 queue_priority = 1.0f;

    VkDeviceQueueCreateInfo device_queue_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = render->vulkan_device_context.graphics_queue_family_index,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    const char* extension_array[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        "VK_KHR_portability_subset"
    };

    VkDeviceCreateInfo device_info = 
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &device_queue_info,
        .enabledLayerCount = 0,
        .enabledExtensionCount = 2,
        .ppEnabledExtensionNames = extension_array,
    };

    VkResult device_result = 
        vkCreateDevice(
            render->vulkan_device_context.physical_device, 
            &device_info, 
            NULL, 
            &render->vulkan_device_context.device
        );

    if (device_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create Vulkan device");
    }

    vkGetDeviceQueue(
        render->vulkan_device_context.device, 
        render->vulkan_device_context.graphics_queue_family_index, 
        0, 
        &render->vulkan_device_context.graphics_queue
    );

    render->vulkan_device_context.present_queue = render->vulkan_device_context.graphics_queue;
}

void vulkan_backend_create_command_pool(Render* render)
{
    VkCommandPoolCreateInfo command_pool_info = 
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = render->vulkan_device_context.graphics_queue_family_index,
    };

    vkCreateCommandPool(
        render->vulkan_device_context.device, 
        &command_pool_info, 
        NULL, 
        &render->vulkan_device_context.command_pool
    );
}

void vulkan_backend_destroy_device_context(Render* render)
{
    VkDevice device = render->vulkan_device_context.device;
    VkInstance instance = render->vulkan_device_context.instance;

    vkDestroyCommandPool(device, render->vulkan_device_context.command_pool, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, render->vulkan_device_context.surface, NULL);
    vkDestroyInstance(instance, NULL);
}
