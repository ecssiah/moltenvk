#include "render/render.h"

#include <string.h>
#include "stb/stb_image.h"

#include "core/log/log.h"

u32 vulkan_backend_locate_memory_type(
    Render* render,
    u32 type_filter,
    VkMemoryPropertyFlags properties
) {
    VkPhysicalDeviceMemoryProperties mem_properties;

    vkGetPhysicalDeviceMemoryProperties(
        render->vulkan_device_context.physical_device,
        &mem_properties
    );

    for (u32 i = 0; i < mem_properties.memoryTypeCount; i++)
    {
        if (
            (type_filter & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties
        ) {
            return i;
        }
    }

    return UINT32_MAX;
}

void vulkan_backend_create_buffer(
    Render* render,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer,
    VkDeviceMemory* memory
) {
    VkBufferCreateInfo buffer_info = 
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    vkCreateBuffer(
        render->vulkan_device_context.device,
        &buffer_info,
        NULL,
        buffer
    );

    VkMemoryRequirements mem_requirements;

    vkGetBufferMemoryRequirements(
        render->vulkan_device_context.device,
        *buffer,
        &mem_requirements
    );

    u32 memory_type_index = vulkan_backend_locate_memory_type(
        render,
        mem_requirements.memoryTypeBits,
        properties
    );

    if (memory_type_index == UINT32_MAX) 
    {
        LOG_FATAL("Failed to find suitable memory type for buffer");
    }

    VkMemoryAllocateInfo alloc_info = 
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = memory_type_index
    };

    vkAllocateMemory(
        render->vulkan_device_context.device,
        &alloc_info,
        NULL,
        memory
    );

    vkBindBufferMemory(
        render->vulkan_device_context.device,
        *buffer,
        *memory,
        0
    );
}

void vulkan_backend_create_image(
    Render* render,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage* image,
    VkDeviceMemory* memory
) {
    VkImageCreateInfo image_info =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent = {
            .width = width,
            .height = height,
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = format,
        .tiling = tiling,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = usage,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    vkCreateImage(
        render->vulkan_device_context.device,
        &image_info,
        NULL,
        image
    );

    VkMemoryRequirements mem_requirements;

    vkGetImageMemoryRequirements(
        render->vulkan_device_context.device,
        *image,
        &mem_requirements
    );

    u32 memory_type_index = vulkan_backend_locate_memory_type(
        render,
        mem_requirements.memoryTypeBits,
        properties
    );

    if (memory_type_index == UINT32_MAX) 
    {
        LOG_FATAL("Failed to find suitable memory type for image");
    }

    VkMemoryAllocateInfo alloc_info =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = memory_type_index
    };

    vkAllocateMemory(
        render->vulkan_device_context.device,
        &alloc_info,
        NULL,
        memory
    );

    vkBindImageMemory(
        render->vulkan_device_context.device,
        *image,
        *memory,
        0
    );
}

VkImageView vulkan_backend_create_image_view(
    Render* render,
    VkImage image,
    VkFormat format
) {
    VkImageViewCreateInfo view_info =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    VkImageView image_view;

    vkCreateImageView(
        render->vulkan_device_context.device,
        &view_info,
        NULL,
        &image_view
    );

    return image_view;
}

VkSampler vulkan_backend_create_sampler(Render* render)
{
    VkSamplerCreateInfo sampler_info =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,

        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,

        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,

        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_FALSE,
        .compareEnable = VK_FALSE,

        .minLod = 0.0f,
        .maxLod = 0.0f,

        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,

        .compareOp = VK_COMPARE_OP_ALWAYS,
    };

    VkSampler sampler;

    vkCreateSampler(
        render->vulkan_device_context.device,
        &sampler_info,
        NULL,
        &sampler
    );

    return sampler;
}

void vulkan_backend_transition_image_layout(
    Render* render,
    VkImage image,
    VkFormat format,
    VkImageLayout old_layout,
    VkImageLayout new_layout
) {
    VkCommandBuffer command_buffer = vulkan_backend_begin_single_time_commands(render);

    VkImageMemoryBarrier barrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (
            format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
            format == VK_FORMAT_D24_UNORM_S8_UINT
        ) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (
        old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    ) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (
        old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    ) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (
        old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    ) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }

    vkCmdPipelineBarrier(
        command_buffer,
        source_stage,
        destination_stage,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &barrier
    );

    vulkan_backend_end_single_time_commands(render, command_buffer);
}

void vulkan_backend_copy_buffer(
    Render* render,
    VkBuffer src_buffer,
    VkBuffer dst_buffer,
    VkDeviceSize size
) {
    VkCommandBuffer command_buffer = vulkan_backend_begin_single_time_commands(render);

    VkBufferCopy copy_region =
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    vkCmdCopyBuffer(
        command_buffer,
        src_buffer,
        dst_buffer,
        1,
        &copy_region
    );

    vulkan_backend_end_single_time_commands(render, command_buffer);
}

void vulkan_backend_copy_buffer_to_image(
    Render* render,
    VkBuffer buffer,
    VkImage image,
    u32 width,
    u32 height
) {
    VkCommandBuffer command_buffer = vulkan_backend_begin_single_time_commands(render);

    VkBufferImageCopy region =
    {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = {
            .x = 0,
            .y = 0,
            .z = 0
        },
        .imageExtent = {
            .width = width,
            .height = height,
            .depth = 1
        }
    };

    vkCmdCopyBufferToImage(
        command_buffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    vulkan_backend_end_single_time_commands(render, command_buffer);
}

void vulkan_backend_create_texture_from_pixels(
    Render* render,
    const void* pixels,
    u32 width,
    u32 height,
    VkImage* image,
    VkDeviceMemory* image_memory,
    VkImageView* image_view,
    VkSampler* sampler
) {
    VkDeviceSize image_size = width * height * 4;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;

    vulkan_backend_create_buffer(
        render,
        image_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer,
        &staging_memory
    );

    void* data;
    vkMapMemory(
        render->vulkan_device_context.device,
        staging_memory,
        0,
        image_size,
        0,
        &data
    );

    memcpy(data, pixels, (size_t)image_size);

    vkUnmapMemory(
        render->vulkan_device_context.device,
        staging_memory
    );

    vulkan_backend_create_image(
        render,
        width,
        height,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        image,
        image_memory
    );

    vulkan_backend_transition_image_layout(
        render,
        *image,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    vulkan_backend_copy_buffer_to_image(
        render,
        staging_buffer,
        *image,
        width,
        height
    );

    vulkan_backend_transition_image_layout(
        render,
        *image,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    *image_view = vulkan_backend_create_image_view(
        render,
        *image,
        VK_FORMAT_R8G8B8A8_UNORM
    );

    *sampler = vulkan_backend_create_sampler(render);

    vkDestroyBuffer(
        render->vulkan_device_context.device,
        staging_buffer,
        NULL
    );

    vkFreeMemory(
        render->vulkan_device_context.device,
        staging_memory,
        NULL
    );
}

void vulkan_backend_create_texture_from_file(
    Render* render,
    const char* path,
    VkImage* image,
    VkDeviceMemory* image_memory,
    VkImageView* image_view,
    VkSampler* sampler
) {
    int width;
    int height;
    int channels;

    stbi_set_flip_vertically_on_load(true);

    stbi_uc* pixel_array = stbi_load(
        path, 
        &width, 
        &height, 
        &channels, 
        STBI_rgb_alpha
    );

    if (!pixel_array) 
    {
        LOG_FATAL("Failed to load texture image: %s", path);
        return;
    }

    vulkan_backend_create_texture_from_pixels(
        render,
        pixel_array,
        (u32)width,
        (u32)height,
        image,
        image_memory,
        image_view,
        sampler
    );

    stbi_image_free(pixel_array);
}

void vulkan_backend_create_voxel_mesh(Render* render)
{
    VulkanTexture* vulkan_texture = &render->voxel_pipeline_context.vulkan_texture;

    vulkan_backend_create_texture_from_file(
        render,
        "assets/textures/lion.png",
        &vulkan_texture->image,
        &vulkan_texture->image_memory,
        &vulkan_texture->image_view,
        &vulkan_texture->sampler
    );

    vulkan_backend_update_texture_descriptor(
        render,
        vulkan_texture->image_view,
        vulkan_texture->sampler
    );

    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;

    VkDeviceSize buffer_size = sizeof(cube_vertex_array);

    vulkan_backend_create_buffer(
        render,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer,
        &staging_memory
    );

    void* data;

    vkMapMemory(
        render->vulkan_device_context.device,
        staging_memory,
        0,
        buffer_size,
        0,
        &data
    );

    memcpy(data, cube_vertex_array, buffer_size);

    vkUnmapMemory(
        render->vulkan_device_context.device,
        staging_memory
    );

    vulkan_backend_create_buffer(
        render,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &render->voxel_pipeline_context.vertex_buffer,
        &render->voxel_pipeline_context.vertex_memory
    );

    vulkan_backend_copy_buffer(
        render,
        staging_buffer,
        render->voxel_pipeline_context.vertex_buffer,
        buffer_size
    );

    vkDestroyBuffer(render->vulkan_device_context.device, staging_buffer, NULL);
    vkFreeMemory(render->vulkan_device_context.device, staging_memory, NULL);
}