#include "vulkan_backend.h"
#include "vulkan_backend_internal.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdio.h>

void vb_create_swapchain_context(VulkanBackend* vulkan_backend)
{
    vb_create_swapchain(vulkan_backend);
    vb_create_image_views(vulkan_backend);
    vb_create_render_pass(vulkan_backend);
    vb_create_frame_buffers(vulkan_backend);
}

void vb_create_swapchain(VulkanBackend* vulkan_backend)
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

    VkSwapchainCreateInfoKHR instance_create_info;
    instance_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    instance_create_info.pNext = NULL;
    instance_create_info.flags = 0;
    instance_create_info.surface = vulkan_backend->vulkan_device_context.surface;
    instance_create_info.minImageCount = 0;
    instance_create_info.imageFormat = surface_format.format;
    instance_create_info.imageColorSpace = surface_format.colorSpace;
    instance_create_info.imageExtent = vulkan_backend->vulkan_swapchain_context.extent;
    instance_create_info.imageArrayLayers = 1;
    instance_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    instance_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    instance_create_info.queueFamilyIndexCount = 0;
    instance_create_info.pQueueFamilyIndices = NULL;
    instance_create_info.preTransform = surface_capabilities.currentTransform;
    instance_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    instance_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    instance_create_info.clipped = VK_TRUE;
    instance_create_info.oldSwapchain = NULL;

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

void vb_create_image_views(VulkanBackend* vulkan_backend)
{
    u32 image_index;
    for (image_index = 0; image_index < vulkan_backend->vulkan_swapchain_context.image_count; ++image_index)
    {
        VkComponentMapping component_mapping;
        component_mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        component_mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        component_mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        component_mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        VkImageSubresourceRange image_subresource_range;
        image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_subresource_range.baseMipLevel = 0;
        image_subresource_range.levelCount = 1;
        image_subresource_range.baseArrayLayer = 0;
        image_subresource_range.layerCount = 1;

        VkImageViewCreateInfo image_view_create_info;
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.pNext = NULL;
        image_view_create_info.flags = 0;
        image_view_create_info.image = vulkan_backend->vulkan_swapchain_context.image_array[image_index];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = vulkan_backend->vulkan_swapchain_context.format;
        image_view_create_info.components = component_mapping;
        image_view_create_info.subresourceRange = image_subresource_range;

        vkCreateImageView(
            vulkan_backend->vulkan_device_context.device, 
            &image_view_create_info, 
            NULL, 
            &vulkan_backend->vulkan_swapchain_context.image_view_array[image_index]
        );
    }
}

void vb_create_render_pass(VulkanBackend* vulkan_backend)
{
    VkAttachmentDescription colovb_attachment;
    colovb_attachment.flags = 0;
    colovb_attachment.format = vulkan_backend->vulkan_swapchain_context.format;
    colovb_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colovb_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colovb_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colovb_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colovb_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colovb_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colovb_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachment_reference;
    attachment_reference.attachment = 0;
    attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description;
    subpass_description.flags = 0;
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.inputAttachmentCount = 0;
    subpass_description.pInputAttachments = NULL;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &attachment_reference;
    subpass_description.pResolveAttachments = NULL;
    subpass_description.pDepthStencilAttachment = NULL;
    subpass_description.preserveAttachmentCount = 0;
    subpass_description.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo rendevb_pass_info;
    rendevb_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rendevb_pass_info.pNext = NULL;
    rendevb_pass_info.flags = 0;
    rendevb_pass_info.attachmentCount = 1;
    rendevb_pass_info.pAttachments = &colovb_attachment;
    rendevb_pass_info.subpassCount = 1;
    rendevb_pass_info.pSubpasses = &subpass_description;
    rendevb_pass_info.dependencyCount = 0;
    rendevb_pass_info.pDependencies = NULL;

    vkCreateRenderPass(
        vulkan_backend->vulkan_device_context.device, 
        &rendevb_pass_info, 
        NULL, 
        &vulkan_backend->voxel_pipeline_context.render_pass
    );
}

void vb_create_frame_buffers(VulkanBackend* vulkan_backend)
{
    u32 image_index;
    for (image_index = 0; image_index < vulkan_backend->vulkan_swapchain_context.image_count; ++image_index)
    {
        VkFramebufferCreateInfo framebuffevb_info;
        framebuffevb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffevb_info.pNext = NULL;
        framebuffevb_info.flags = 0;
        framebuffevb_info.renderPass = vulkan_backend->voxel_pipeline_context.render_pass;
        framebuffevb_info.attachmentCount = 1;
        framebuffevb_info.pAttachments = &vulkan_backend->vulkan_swapchain_context.image_view_array[image_index];
        framebuffevb_info.width = vulkan_backend->vulkan_swapchain_context.extent.width;
        framebuffevb_info.height = vulkan_backend->vulkan_swapchain_context.extent.height;
        framebuffevb_info.layers = 1;

        vkCreateFramebuffer(
            vulkan_backend->vulkan_device_context.device, 
            &framebuffevb_info, 
            NULL, 
            &vulkan_backend->vulkan_swapchain_context.framebuffer_array[image_index]
        );
    }
}

void vb_destroy_swapchain_context(VulkanBackend* vulkan_backend)
{
    u32 image_index;
    for (image_index = 0; image_index < vulkan_backend->vulkan_swapchain_context.image_count; ++image_index)
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