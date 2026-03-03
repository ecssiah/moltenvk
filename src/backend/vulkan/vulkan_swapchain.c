#include "vulkan_backend.h"
#include "vulkan_backend_internal.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdio.h>

void vs_create_swapchain_context(VulkanBackend* vulkan_backend)
{
    vs_create_swapchain(vulkan_backend);
    vs_create_image_views(vulkan_backend);
    vs_create_render_pass(vulkan_backend);
    vs_create_frame_buffers(vulkan_backend);
}

void vs_create_swapchain(VulkanBackend* vulkan_backend)
{
    VkSurfaceCapabilitiesKHR surface_capabilities;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        vulkan_backend->vulkan_device_context.physical_device, 
        vulkan_backend->vulkan_device_context.surface, 
        &surface_capabilities
    );

    u32 format_count;

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        vulkan_backend->vulkan_device_context.physical_device, 
        vulkan_backend->vulkan_device_context.surface, 
        &format_count, 
        NULL
    );

    VkSurfaceFormatKHR* surface_format_array = malloc(sizeof (VkSurfaceFormatKHR) * format_count);

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        vulkan_backend->vulkan_device_context.physical_device,
        vulkan_backend->vulkan_device_context.surface,
        &format_count,
        surface_format_array
    );

    VkSurfaceFormatKHR surface_format = surface_format_array[0];
    vulkan_backend->vulkan_swapchain_context.format = surface_format.format;
    vulkan_backend->vulkan_swapchain_context.extent = surface_capabilities.currentExtent;

    free(surface_format_array);

    VkSwapchainCreateInfoKHR instance_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .surface = vulkan_backend->vulkan_device_context.surface,
        .minImageCount = 0,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = vulkan_backend->vulkan_swapchain_context.extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = NULL,
    };

    u32 min_image_count = surface_capabilities.minImageCount + 1;

    if (
        surface_capabilities.maxImageCount > 0 &&
        min_image_count > surface_capabilities.maxImageCount
    ) {
        min_image_count = surface_capabilities.maxImageCount;
    }

    instance_create_info.minImageCount = min_image_count;

    VkResult swapchain_result =
        vkCreateSwapchainKHR(
            vulkan_backend->vulkan_device_context.device, 
            &instance_create_info, 
            NULL, 
            &vulkan_backend->vulkan_swapchain_context.swapchain
        );

    if (swapchain_result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create swapchain");

        exit(EXIT_FAILURE);
    }

    u32 image_count = 0;

    vkGetSwapchainImagesKHR(
        vulkan_backend->vulkan_device_context.device, 
        vulkan_backend->vulkan_swapchain_context.swapchain, 
        &image_count, 
        NULL
    );
    
    vulkan_backend->vulkan_swapchain_context.image_array = malloc(sizeof (VkImage) * image_count);
    vulkan_backend->vulkan_swapchain_context.image_view_array = malloc(sizeof (VkImageView) * image_count);
    vulkan_backend->vulkan_swapchain_context.framebuffer_array = malloc(sizeof (VkFramebuffer) * image_count);

    vulkan_backend->vulkan_swapchain_context.image_count = image_count;

    vkGetSwapchainImagesKHR(
        vulkan_backend->vulkan_device_context.device, 
        vulkan_backend->vulkan_swapchain_context.swapchain, 
        &image_count, 
        vulkan_backend->vulkan_swapchain_context.image_array
    );
}

void vs_create_image_views(VulkanBackend* vulkan_backend)
{
    
    for (u32 image_index = 0; image_index < vulkan_backend->vulkan_swapchain_context.image_count; ++image_index)
    {
        VkComponentMapping component_mapping = 
        {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        };

        VkImageSubresourceRange image_subresource_range =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };

        VkImageViewCreateInfo image_view_create_info =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .image = vulkan_backend->vulkan_swapchain_context.image_array[image_index],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = vulkan_backend->vulkan_swapchain_context.format,
            .components = component_mapping,
            .subresourceRange = image_subresource_range,
        };

        vkCreateImageView(
            vulkan_backend->vulkan_device_context.device, 
            &image_view_create_info, 
            NULL, 
            &vulkan_backend->vulkan_swapchain_context.image_view_array[image_index]
        );
    }
}

void vs_create_render_pass(VulkanBackend* vulkan_backend)
{
    VkAttachmentDescription color_attachment =
    {
        .flags = 0,
        .format = vulkan_backend->vulkan_swapchain_context.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference attachment_reference =
    {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass_description =
    {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment_reference,
        .pResolveAttachments = NULL,
        .pDepthStencilAttachment = NULL,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = NULL,
    };

    VkRenderPassCreateInfo render_pass_info =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass_description,
        .dependencyCount = 0,
        .pDependencies = NULL,
    };

    vkCreateRenderPass(
        vulkan_backend->vulkan_device_context.device, 
        &render_pass_info, 
        NULL, 
        &vulkan_backend->voxel_pipeline_context.render_pass
    );
}

void vs_create_frame_buffers(VulkanBackend* vulkan_backend)
{
    
    for (u32 image_index = 0; image_index < vulkan_backend->vulkan_swapchain_context.image_count; ++image_index)
    {
        VkFramebufferCreateInfo framebuffer_info =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .renderPass = vulkan_backend->voxel_pipeline_context.render_pass,
            .attachmentCount = 1,
            .pAttachments = &vulkan_backend->vulkan_swapchain_context.image_view_array[image_index],
            .width = vulkan_backend->vulkan_swapchain_context.extent.width,
            .height = vulkan_backend->vulkan_swapchain_context.extent.height,
            .layers = 1,
        };

        vkCreateFramebuffer(
            vulkan_backend->vulkan_device_context.device, 
            &framebuffer_info, 
            NULL, 
            &vulkan_backend->vulkan_swapchain_context.framebuffer_array[image_index]
        );
    }
}

void vs_destroy_swapchain_context(VulkanBackend* vulkan_backend)
{
    for (u32 image_index = 0; image_index < vulkan_backend->vulkan_swapchain_context.image_count; ++image_index)
    {
        vkDestroyFramebuffer(
            vulkan_backend->vulkan_device_context.device,
            vulkan_backend->vulkan_swapchain_context.framebuffer_array[image_index],
            NULL
        );

        vkDestroyImageView(
            vulkan_backend->vulkan_device_context.device,
            vulkan_backend->vulkan_swapchain_context.image_view_array[image_index],
            NULL
        );
    }

    free(vulkan_backend->vulkan_swapchain_context.image_array);
    free(vulkan_backend->vulkan_swapchain_context.image_view_array);
    free(vulkan_backend->vulkan_swapchain_context.framebuffer_array);

    vkDestroySwapchainKHR(
        vulkan_backend->vulkan_device_context.device,
        vulkan_backend->vulkan_swapchain_context.swapchain,
        NULL
    );
}