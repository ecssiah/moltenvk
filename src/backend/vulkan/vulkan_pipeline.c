#include "vulkan_backend.h"
#include "vulkan_backend_internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "core/core.h"
#include "core/log/log.h"
#include "render/render_internal.h"

void vulkan_backend_create_voxel_pipeline(VulkanBackend* vulkan_backend)
{
    vulkan_backend_create_graphics_pipeline(vulkan_backend);

    vulkan_backend_create_texture_from_file(
        vulkan_backend,
        "assets/textures/lion.png",
        &vulkan_backend->voxel_pipeline_context.vulkan_texture.image,
        &vulkan_backend->voxel_pipeline_context.vulkan_texture.image_memory,
        &vulkan_backend->voxel_pipeline_context.vulkan_texture.image_view,
        &vulkan_backend->voxel_pipeline_context.vulkan_texture.sampler
    );

    vulkan_backend_update_texture_descriptor(
        vulkan_backend,
        vulkan_backend->voxel_pipeline_context.vulkan_texture.image_view,
        vulkan_backend->voxel_pipeline_context.vulkan_texture.sampler
    );
}

void vulkan_backend_create_graphics_pipeline(VulkanBackend* vulkan_backend)
{
    VkShaderModule vert_module = 
        vulkan_backend_create_shader_module(
            vulkan_backend->vulkan_device_context.device, 
            "assets/shaders/bin/test.vert.spv"
        );

    VkShaderModule frag_module = 
        vulkan_backend_create_shader_module(
            vulkan_backend->vulkan_device_context.device, 
            "assets/shaders/bin/test.frag.spv"
        );

    VkPipelineShaderStageCreateInfo vert_stage_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_module,
        .pName = "main",
        .pSpecializationInfo = NULL,
    };

    VkPipelineShaderStageCreateInfo frag_stage_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_module,
        .pName = "main",
        .pSpecializationInfo = NULL,
    };

    VkPipelineShaderStageCreateInfo shader_stage_array[] = {vert_stage_info, frag_stage_info};

    VkVertexInputBindingDescription vertex_input_binding =
    {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription vertex_input_attribute_array[2] =
    {
        {
            .binding = 0,
            .location = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, position)
        },
        {
            .binding = 0,
            .location = 1,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, uv)
        },
    };

    VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertex_input_binding,
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = vertex_input_attribute_array,
    };

    VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;

    VkDeviceSize buffer_size = sizeof(quad_vertex_array);

    vulkan_backend_create_buffer(
        vulkan_backend,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer,
        &staging_memory
    );

    void* data;

    vkMapMemory(
        vulkan_backend->vulkan_device_context.device,
        staging_memory,
        0,
        buffer_size,
        0,
        &data
    );

    memcpy(data, quad_vertex_array, buffer_size);

    vkUnmapMemory(
        vulkan_backend->vulkan_device_context.device,
        staging_memory
    );

    vulkan_backend_create_buffer(
        vulkan_backend,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &vulkan_backend->voxel_pipeline_context.vertex_buffer,
        &vulkan_backend->voxel_pipeline_context.vertex_memory
    );

    vulkan_backend_copy_buffer(
        vulkan_backend,
        staging_buffer,
        vulkan_backend->voxel_pipeline_context.vertex_buffer,
        buffer_size
    );

    vkDestroyBuffer(vulkan_backend->vulkan_device_context.device, staging_buffer, NULL);
    vkFreeMemory(vulkan_backend->vulkan_device_context.device, staging_memory, NULL);

    VkViewport viewport =
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = (f32)vulkan_backend->vulkan_swapchain_context.extent.width,
        .height = (f32)vulkan_backend->vulkan_swapchain_context.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor_rect =
    {
        .offset = {
            .x = 0,
            .y = 0,
        },
        .extent = vulkan_backend->vulkan_swapchain_context.extent,
    };

    VkPipelineViewportStateCreateInfo pipeline_viewport_state_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor_rect,
    };

    VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo pipeline_multisampling_state_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state =
    {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = 0,
        .dstColorBlendFactor = 0,
        .colorBlendOp = 0,
        .srcAlphaBlendFactor = 0,
        .dstAlphaBlendFactor = 0,
        .alphaBlendOp = 0,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &pipeline_color_blend_attachment_state,
        .blendConstants = { 0, 0, 0, 0 },
    };

    VkDescriptorSetLayoutBinding descriptor_set_layout_binding = 
    {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = NULL,
    };

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .bindingCount = 1,
        .pBindings = &descriptor_set_layout_binding
    };

    VkResult descriptor_set_layout_result = 
        vkCreateDescriptorSetLayout(
            vulkan_backend->vulkan_device_context.device,
            &descriptor_set_layout_info,
            NULL,
            &vulkan_backend->voxel_pipeline_context.descriptor_set_layout
        );

    if (descriptor_set_layout_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create descriptor set layout");
    }

    VkDescriptorPoolSize pool_size =
    {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 64
    };

    VkDescriptorPoolCreateInfo pool_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .maxSets = 64,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size
    };

    VkResult descriptor_pool_result =
        vkCreateDescriptorPool(
            vulkan_backend->vulkan_device_context.device,
            &pool_info,
            NULL,
            &vulkan_backend->voxel_pipeline_context.descriptor_pool
        );

    if (descriptor_pool_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create descriptor pool");
    }

    VkDescriptorSetAllocateInfo alloc_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = vulkan_backend->voxel_pipeline_context.descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &vulkan_backend->voxel_pipeline_context.descriptor_set_layout
    };

    VkResult descriptor_set_result =
        vkAllocateDescriptorSets(
            vulkan_backend->vulkan_device_context.device,
            &alloc_info,
            &vulkan_backend->voxel_pipeline_context.descriptor_set
        );

    if (descriptor_set_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to allocate descriptor set");
    }

    VkPipelineLayoutCreateInfo pipeline_layout_info = 
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &vulkan_backend->voxel_pipeline_context.descriptor_set_layout,
    };

    VkResult pipeline_layout_result = 
        vkCreatePipelineLayout(
            vulkan_backend->vulkan_device_context.device, 
            &pipeline_layout_info, 
            NULL,
            &vulkan_backend->voxel_pipeline_context.layout
        );

    if (pipeline_layout_result != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create Vulkan Pipeline layout");
    }

    VkGraphicsPipelineCreateInfo graphics_pipeline_info = 
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shader_stage_array,
        .pVertexInputState = &pipeline_vertex_input_state_info,
        .pInputAssemblyState = &pipeline_input_assembly_state_info,
        .pViewportState = &pipeline_viewport_state_info,
        .pRasterizationState = &pipeline_rasterization_state_info,
        .pMultisampleState = &pipeline_multisampling_state_info,
        .pColorBlendState = &pipeline_color_blend_state_info,
        .layout = vulkan_backend->voxel_pipeline_context.layout,
        .renderPass = vulkan_backend->voxel_pipeline_context.render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    VkResult graphics_pipeline_result = 
        vkCreateGraphicsPipelines(
            vulkan_backend->vulkan_device_context.device, 
            VK_NULL_HANDLE, 
            1, 
            &graphics_pipeline_info, 
            NULL, 
            &vulkan_backend->voxel_pipeline_context.pipeline
        );

    if (graphics_pipeline_result != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create Vulkan graphics pipeline");
    }

    vkDestroyShaderModule(vulkan_backend->vulkan_device_context.device, frag_module, NULL);
    vkDestroyShaderModule(vulkan_backend->vulkan_device_context.device, vert_module, NULL);
}

VkShaderModule vulkan_backend_create_shader_module(VkDevice device, const char* filename)
{
    char* shader_src = NULL;
    size_t shader_src_size = read_file_binary(filename, &shader_src);

    VkShaderModuleCreateInfo shader_module_info =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .codeSize = shader_src_size,
        .pCode = (const u32*)shader_src,
    };

    VkShaderModule shader_module;

    VkResult shader_module_result = 
        vkCreateShaderModule(
            device, 
            &shader_module_info, 
            NULL, 
            &shader_module
        );

    if (shader_module_result != VK_SUCCESS)
    {
        free(shader_src);

        LOG_FATAL("Failed to create shader module");
    }

    free(shader_src);

    return shader_module;
}

void vulkan_backend_update_texture_descriptor(
    VulkanBackend* vulkan_backend,
    VkImageView image_view,
    VkSampler sampler
) {
    VkDescriptorImageInfo image_info =
    {
        .sampler = vulkan_backend->voxel_pipeline_context.vulkan_texture.sampler,
        .imageView = vulkan_backend->voxel_pipeline_context.vulkan_texture.image_view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet write_descriptor_set =
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = vulkan_backend->voxel_pipeline_context.descriptor_set,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImageInfo = &image_info,
    };

    vkUpdateDescriptorSets(
        vulkan_backend->vulkan_device_context.device,
        1,
        &write_descriptor_set,
        0,
        NULL
    );
}
