#include "backend/vulkan/vulkan_backend.h"
#include "vulkan_backend_internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "core/log/log.h"

void vulkan_backend_create_device_context(VulkanBackend* vulkan_backend, Platform* platform)
{
    vulkan_backend_create_instance(vulkan_backend);
    vulkan_backend_create_surface(vulkan_backend, platform);
    vulkan_backend_choose_physical_device(vulkan_backend);
    vulkan_backend_create_logical_device(vulkan_backend);
    vulkan_backend_create_command_pool(vulkan_backend);

    LOG_INFO("Vulkan Device Initialized");
}

void vulkan_backend_create_instance(VulkanBackend* vulkan_backend)
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
            &vulkan_backend->vulkan_device_context.instance
        );

    if (instance_result != VK_SUCCESS) 
    {
        free(required_extension_array);

        LOG_FATAL("Failed to create Vulkan instance");
    }

    free(required_extension_array);
}

void vulkan_backend_create_surface(VulkanBackend* vulkan_backend, Platform* platform)
{
    vulkan_backend->vulkan_device_context.surface = 
        platform_create_vulkan_surface(
            platform, 
            vulkan_backend->vulkan_device_context.instance
        );
}

void vulkan_backend_choose_physical_device(VulkanBackend* vulkan_backend)
{
    u32 device_count = 0;

    vkEnumeratePhysicalDevices(
        vulkan_backend->vulkan_device_context.instance, 
        &device_count, 
        NULL
    );

    if (device_count == 0)
    {
        LOG_FATAL("No Vulkan physical devices found");
    }

    VkPhysicalDevice* physical_device_array = malloc(sizeof (VkPhysicalDevice) * device_count);

    vkEnumeratePhysicalDevices(
        vulkan_backend->vulkan_device_context.instance, 
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

void vulkan_backend_create_logical_device(VulkanBackend* vulkan_backend)
{
    const f32 queue_priority = 1.0f;

    VkDeviceQueueCreateInfo device_queue_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = vulkan_backend->vulkan_device_context.graphics_queue_family_index,
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
            vulkan_backend->vulkan_device_context.physical_device, 
            &device_info, 
            NULL, 
            &vulkan_backend->vulkan_device_context.device
        );

    if (device_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create Vulkan device");
    }

    vkGetDeviceQueue(
        vulkan_backend->vulkan_device_context.device, 
        vulkan_backend->vulkan_device_context.graphics_queue_family_index, 
        0, 
        &vulkan_backend->vulkan_device_context.graphics_queue
    );

    vulkan_backend->vulkan_device_context.present_queue = vulkan_backend->vulkan_device_context.graphics_queue;
}

void vulkan_backend_create_command_pool(VulkanBackend* vulkan_backend)
{
    VkCommandPoolCreateInfo command_pool_info = 
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = vulkan_backend->vulkan_device_context.graphics_queue_family_index,
    };

    vkCreateCommandPool(
        vulkan_backend->vulkan_device_context.device, 
        &command_pool_info, 
        NULL, 
        &vulkan_backend->vulkan_device_context.command_pool
    );
}

void vulkan_backend_destroy_device_context(VulkanBackend* vulkan_backend)
{
    VkDevice device = vulkan_backend->vulkan_device_context.device;
    VkInstance instance = vulkan_backend->vulkan_device_context.instance;

    vkDestroyCommandPool(device, vulkan_backend->vulkan_device_context.command_pool, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, vulkan_backend->vulkan_device_context.surface, NULL);
    vkDestroyInstance(instance, NULL);
}
