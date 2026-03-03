#include "core/log.h"
#include "vulkan_backend_internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <GLFW/glfw3.h>

void vb_create_instance(VulkanBackend* vulkan_backend)
{
    u32 extension_count = 0;
    const char** extension_array = glfwGetRequiredInstanceExtensions(&extension_count);

    const char** required_extension_array = malloc(sizeof (const char*) * (extension_count + 1));
    
    u32 extension_index;
    for (extension_index = 0; extension_index < extension_count; ++extension_index)
    {
        required_extension_array[extension_index] = extension_array[extension_index];
    }

    required_extension_array[extension_count] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

    VkApplicationInfo application_info;
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pNext = NULL;
    application_info.pApplicationName = "Vulkan Test";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "Vulkan Test Engine";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo instance_create_info;
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext = NULL;
    instance_create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instance_create_info.pApplicationInfo = &application_info;
    instance_create_info.enabledLayerCount = 0;
    instance_create_info.ppEnabledLayerNames = NULL;
    instance_create_info.enabledExtensionCount = extension_count + 1;
    instance_create_info.ppEnabledExtensionNames = required_extension_array;

    int instance_result = 
        vkCreateInstance(
            &instance_create_info, 
            NULL, 
            &vulkan_backend->vulkan_device_context.instance
        );

    if (instance_result != VK_SUCCESS) 
    {
        free(required_extension_array);

        LOG_FATAL("Failed to create Vulkan instance");
    }

    free(required_extension_array);
}

void vb_create_surface(VulkanBackend* vulkan_backend, Platform* platform)
{
    vulkan_backend->vulkan_device_context.surface = 
        platform_create_vulkan_surface(
            platform, 
            vulkan_backend->vulkan_device_context.instance
        );
}

void vb_pick_physical_device(VulkanBackend* vulkan_backend)
{
    u32 device_count = 0;

    vkEnumeratePhysicalDevices(
        vulkan_backend->vulkan_device_context.instance, 
        &device_count, 
        NULL
    );

    if (device_count == 0)
    {
        fprintf(stderr, "No Vulkan physical devices found\n");

        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice* physical_device_array = malloc(sizeof (VkPhysicalDevice) * device_count);

    vkEnumeratePhysicalDevices(
        vulkan_backend->vulkan_device_context.instance, 
        &device_count, 
        physical_device_array
    );

    u32 device_index;
    for (device_index = 0; device_index < device_count; ++device_index)
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

        u32 queue_family_index;
        for (queue_family_index = 0; queue_family_index < queue_family_count; ++queue_family_index)
        {
            VkBool32 present_support = VK_FALSE;

            vkGetPhysicalDeviceSurfaceSupportKHR(
                device, 
                queue_family_index, 
                vulkan_backend->vulkan_device_context.surface, 
                &present_support
            );

            if (
                (queue_family_properties_array[queue_family_index].queueFlags & VK_QUEUE_GRAPHICS_BIT) && 
                present_support
            ) {
                vulkan_backend->vulkan_device_context.physical_device = device;
                vulkan_backend->vulkan_device_context.graphics_queue_family_index = queue_family_index;

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

void vb_create_logical_device(VulkanBackend* vulkan_backend)
{
    f32 queue_priority = 1.0f;

    VkDeviceQueueCreateInfo device_queue_info;
    device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_queue_info.pNext = NULL;
    device_queue_info.flags = 0;
    device_queue_info.queueFamilyIndex = vulkan_backend->vulkan_device_context.graphics_queue_family_index;
    device_queue_info.queueCount = 1;
    device_queue_info.pQueuePriorities = &queue_priority;

    const char* extension_array[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo device_info;
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pNext = NULL;
    device_info.flags = 0;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &device_queue_info;
    device_info.enabledLayerCount = 0;
    device_info.ppEnabledLayerNames = NULL;
    device_info.enabledExtensionCount = 1;
    device_info.ppEnabledExtensionNames = extension_array;
    device_info.pEnabledFeatures = NULL;

    VkResult device_result = 
        vkCreateDevice(
            vulkan_backend->vulkan_device_context.physical_device, 
            &device_info, 
            NULL, 
            &vulkan_backend->vulkan_device_context.device
        );

    if (device_result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create Vulkan device");

        exit(EXIT_FAILURE);
    }

    vkGetDeviceQueue(
        vulkan_backend->vulkan_device_context.device, 
        vulkan_backend->vulkan_device_context.graphics_queue_family_index, 
        0, 
        &vulkan_backend->vulkan_device_context.graphics_queue
    );

    vulkan_backend->vulkan_device_context.present_queue = vulkan_backend->vulkan_device_context.graphics_queue;
}

void vb_create_command_pool(VulkanBackend* vulkan_backend)
{
    VkCommandPoolCreateInfo command_pool_info;
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.pNext = NULL;
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_info.queueFamilyIndex = vulkan_backend->vulkan_device_context.graphics_queue_family_index;

    vkCreateCommandPool(
        vulkan_backend->vulkan_device_context.device, 
        &command_pool_info, 
        NULL, 
        &vulkan_backend->vulkan_device_context.command_pool
    );
}

void vulkan_backend_draw(VulkanBackend* vulkan_backend)
{
    vb_render_frame(vulkan_backend);
}
