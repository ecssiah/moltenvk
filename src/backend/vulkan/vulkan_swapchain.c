#include "core/log/log.h"
#include "vulkan_backend.h"
#include "vulkan_backend_internal.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdio.h>

void vulkan_backend_create_swapchain_context(VulkanBackend* vulkan_backend)
{
    vulkan_backend_create_swapchain(vulkan_backend);
    vulkan_backend_create_image_views(vulkan_backend);
    vulkan_backend_create_render_pass(vulkan_backend);
    vulkan_backend_create_depth_resources(vulkan_backend);
    vulkan_backend_create_frame_buffers(vulkan_backend);
}

void vulkan_backend_create_swapchain(VulkanBackend* vulkan_backend)
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
        LOG_FATAL("Failed to create swapchain");
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

void vulkan_backend_create_image_views(VulkanBackend* vulkan_backend)
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

void vulkan_backend_create_render_pass(VulkanBackend* vulkan_backend)
{
    VkAttachmentDescription color_attachment =
    {
        .format = vulkan_backend->vulkan_swapchain_context.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentDescription depth_attachment =
    {
        .format = VK_FORMAT_D32_SFLOAT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference color_ref =
    {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depth_ref =
    {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass_description =
    {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_ref,
        .pResolveAttachments = NULL,
        .pDepthStencilAttachment = &depth_ref,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = NULL,
    };

    VkAttachmentDescription attachments[2] = { color_attachment, depth_attachment };

    VkRenderPassCreateInfo render_pass_info =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .attachmentCount = 2,
        .pAttachments = attachments,
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

void vulkan_backend_create_depth_resources(VulkanBackend* vulkan_backend)
{
    VkFormat depth_format = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo image_info =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent =
        {
            vulkan_backend->vulkan_swapchain_context.extent.width,
            vulkan_backend->vulkan_swapchain_context.extent.height,
            1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = depth_format,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    vkCreateImage(
        vulkan_backend->vulkan_device_context.device,
        &image_info,
        NULL,
        &vulkan_backend->vulkan_swapchain_context.depth_image
    );

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(
        vulkan_backend->vulkan_device_context.device,
        vulkan_backend->vulkan_swapchain_context.depth_image,
        &mem_requirements
    );

    VkMemoryAllocateInfo alloc_info =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = 0
    };

    vkAllocateMemory(
        vulkan_backend->vulkan_device_context.device,
        &alloc_info,
        NULL,
        &vulkan_backend->vulkan_swapchain_context.depth_memory
    );

    vkBindImageMemory(
        vulkan_backend->vulkan_device_context.device,
        vulkan_backend->vulkan_swapchain_context.depth_image,
        vulkan_backend->vulkan_swapchain_context.depth_memory,
        0
    );

    VkImageViewCreateInfo view_info =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = vulkan_backend->vulkan_swapchain_context.depth_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = depth_format,
        .subresourceRange =
        {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    vkCreateImageView(
        vulkan_backend->vulkan_device_context.device,
        &view_info,
        NULL,
        &vulkan_backend->vulkan_swapchain_context.depth_image_view
    );
}

void vulkan_backend_create_frame_buffers(VulkanBackend* vulkan_backend)
{
    for (u32 image_index = 0; image_index < vulkan_backend->vulkan_swapchain_context.image_count; ++image_index)
    {
        VkImageView attachments[2] =
        {
            vulkan_backend->vulkan_swapchain_context.image_view_array[image_index],
            vulkan_backend->vulkan_swapchain_context.depth_image_view
        };

        VkFramebufferCreateInfo framebuffer_info =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .renderPass = vulkan_backend->voxel_pipeline_context.render_pass,
            .attachmentCount = 2,
            .pAttachments = attachments,
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

void vulkan_backend_destroy_swapchain_context(VulkanBackend* vulkan_backend)
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

    vkDestroyImageView(
        vulkan_backend->vulkan_device_context.device,
        vulkan_backend->vulkan_swapchain_context.depth_image_view,
        NULL
    );

    vkDestroyImage(
        vulkan_backend->vulkan_device_context.device,
        vulkan_backend->vulkan_swapchain_context.depth_image,
        NULL
    );

    vkFreeMemory(
        vulkan_backend->vulkan_device_context.device,
        vulkan_backend->vulkan_swapchain_context.depth_memory,
        NULL
    );

    free(vulkan_backend->vulkan_swapchain_context.image_array);
    free(vulkan_backend->vulkan_swapchain_context.image_view_array);
    free(vulkan_backend->vulkan_swapchain_context.framebuffer_array);

    vkDestroySwapchainKHR(
        vulkan_backend->vulkan_device_context.device,
        vulkan_backend->vulkan_swapchain_context.swapchain,
        NULL
    );
}