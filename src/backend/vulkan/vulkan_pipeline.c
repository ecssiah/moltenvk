#include "vulkan_backend.h"
#include "vulkan_backend_internal.h"

#include <stdlib.h>
#include <stdio.h>

#include "core/core.h"

void vp_create_voxel_pipeline(VulkanBackend* vulkan_backend)
{
    vp_create_graphics_pipeline(vulkan_backend);
}

void vp_create_graphics_pipeline(VulkanBackend* vulkan_backend)
{
    VkShaderModule vert_module = 
        vp_create_shader_module(
            vulkan_backend->vulkan_device_context.device, 
            "assets/shaders/bin/test.vert.spv"
        );

    VkShaderModule frag_module = 
        vp_create_shader_module(
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

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage_info, frag_stage_info};

    VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = NULL,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = NULL,
    };

    VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

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
        .pNext = NULL,
        .flags = 0,
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
        .pNext = NULL,
        .flags = 0,
        .logicOpEnable = 0,
        .logicOp = 0,
        .attachmentCount = 1,
        .pAttachments = &pipeline_color_blend_attachment_state,
        .blendConstants = { 0, 0, 0, 0 },
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = 
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
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
        fprintf(stderr, "Failed to create Vulkan Pipeline layout\n");

        exit(EXIT_FAILURE);
    }

    VkGraphicsPipelineCreateInfo graphics_pipeline_info = 
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stageCount = 2,
        .pStages = shader_stages,
        .pVertexInputState = &pipeline_vertex_input_state_info,
        .pInputAssemblyState = &pipeline_input_assembly_state_info,
        .pTessellationState = NULL,
        .pViewportState = &pipeline_viewport_state_info,
        .pRasterizationState = &pipeline_rasterization_state_info,
        .pMultisampleState = &pipeline_multisampling_state_info,
        .pDepthStencilState = NULL,
        .pColorBlendState = &pipeline_color_blend_state_info,
        .pDynamicState = NULL,
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
        fprintf(stderr, "Failed to create Vulkan graphics pipeline\n");

        exit(EXIT_FAILURE);
    }

    vkDestroyShaderModule(vulkan_backend->vulkan_device_context.device, frag_module, NULL);
    vkDestroyShaderModule(vulkan_backend->vulkan_device_context.device, vert_module, NULL);
}

VkShaderModule vp_create_shader_module(VkDevice device, const char* filename)
{
    char* shader_src = NULL;
    size_t shader_src_size = c_read_file_binary(filename, &shader_src);

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

        fprintf(stderr, "Failed to create shader module\n");

        exit(EXIT_FAILURE);
    }

    free(shader_src);

    return shader_module;
}
