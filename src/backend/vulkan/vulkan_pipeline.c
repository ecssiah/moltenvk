#include "vulkan_backend.h"
#include "vulkan_backend_internal.h"

#include <stdlib.h>
#include <stdio.h>

#include "core/core.h"

void vb_create_voxel_pipeline(VulkanBackend* vulkan_backend)
{
    vb_create_graphics_pipeline(vulkan_backend);
}

void vb_create_graphics_pipeline(VulkanBackend* vulkan_backend)
{
    VkShaderModule vert_module = 
        vb_create_shader_module(
            vulkan_backend->vulkan_device_context.device, 
            "assets/shaders/bin/test.vert.spv"
        );

    VkShaderModule frag_module = 
        vb_create_shader_module(
            vulkan_backend->vulkan_device_context.device, 
            "assets/shaders/bin/test.frag.spv"
        );

    VkPipelineShaderStageCreateInfo vert_stage_info;
    vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage_info.pNext = NULL;
    vert_stage_info.flags = 0;
    vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage_info.module = vert_module;
    vert_stage_info.pName = "main";
    vert_stage_info.pSpecializationInfo = NULL;

    VkPipelineShaderStageCreateInfo frag_stage_info;
    frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage_info.pNext = NULL;
    frag_stage_info.flags = 0;
    frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage_info.module = frag_module;
    frag_stage_info.pName = "main";
    frag_stage_info.pSpecializationInfo = NULL;

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage_info, frag_stage_info};

    VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_info;
    pipeline_vertex_input_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipeline_vertex_input_state_info.pNext = NULL;
    pipeline_vertex_input_state_info.flags = 0;
    pipeline_vertex_input_state_info.vertexBindingDescriptionCount = 0;
    pipeline_vertex_input_state_info.pVertexBindingDescriptions = NULL;
    pipeline_vertex_input_state_info.vertexAttributeDescriptionCount = 0;
    pipeline_vertex_input_state_info.pVertexAttributeDescriptions = NULL;

    VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_info;
    pipeline_input_assembly_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipeline_input_assembly_state_info.pNext = NULL;
    pipeline_input_assembly_state_info.flags = 0;
    pipeline_input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeline_input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32)vulkan_backend->vulkan_swapchain_context.extent.width;
    viewport.height = (f32)vulkan_backend->vulkan_swapchain_context.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor_rect;
    scissor_rect.offset.x = 0;
    scissor_rect.offset.y = 0;
    scissor_rect.extent = vulkan_backend->vulkan_swapchain_context.extent;

    VkPipelineViewportStateCreateInfo pipeline_viewport_state_info;
    pipeline_viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipeline_viewport_state_info.pNext = NULL;
    pipeline_viewport_state_info.flags = 0;
    pipeline_viewport_state_info.viewportCount = 1;
    pipeline_viewport_state_info.pViewports = &viewport;
    pipeline_viewport_state_info.scissorCount = 1;
    pipeline_viewport_state_info.pScissors = &scissor_rect;

    VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_info;
    pipeline_rasterization_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipeline_rasterization_state_info.pNext = NULL;
    pipeline_rasterization_state_info.flags = 0;
    pipeline_rasterization_state_info.depthClampEnable = VK_FALSE;
    pipeline_rasterization_state_info.rasterizerDiscardEnable = VK_FALSE;
    pipeline_rasterization_state_info.polygonMode = VK_POLYGON_MODE_FILL;
    pipeline_rasterization_state_info.cullMode = VK_CULL_MODE_BACK_BIT;
    pipeline_rasterization_state_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipeline_rasterization_state_info.depthBiasEnable = VK_FALSE;
    pipeline_rasterization_state_info.depthBiasConstantFactor = 0.0f;
    pipeline_rasterization_state_info.depthBiasClamp = 0.0f;
    pipeline_rasterization_state_info.depthBiasSlopeFactor = 0.0f;
    pipeline_rasterization_state_info.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo pipeline_multisampling_state_info;
    pipeline_multisampling_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipeline_multisampling_state_info.pNext = NULL;
    pipeline_multisampling_state_info.flags = 0;
    pipeline_multisampling_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipeline_multisampling_state_info.sampleShadingEnable = VK_FALSE;
    pipeline_multisampling_state_info.minSampleShading = 0.0f;
    pipeline_multisampling_state_info.pSampleMask = NULL;
    pipeline_multisampling_state_info.alphaToCoverageEnable = VK_FALSE;
    pipeline_multisampling_state_info.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state;
    pipeline_color_blend_attachment_state.blendEnable = VK_FALSE;
    pipeline_color_blend_attachment_state.srcColorBlendFactor = 0;
    pipeline_color_blend_attachment_state.dstColorBlendFactor = 0;
    pipeline_color_blend_attachment_state.colorBlendOp = 0;
    pipeline_color_blend_attachment_state.srcAlphaBlendFactor = 0;
    pipeline_color_blend_attachment_state.dstAlphaBlendFactor = 0;
    pipeline_color_blend_attachment_state.alphaBlendOp = 0;
    pipeline_color_blend_attachment_state.colorWriteMask =         
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_info;
    pipeline_color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipeline_color_blend_state_info.pNext = NULL;
    pipeline_color_blend_state_info.flags = 0;
    pipeline_color_blend_state_info.logicOpEnable = 0;
    pipeline_color_blend_state_info.logicOp = 0;
    pipeline_color_blend_state_info.attachmentCount = 1;
    pipeline_color_blend_state_info.pAttachments = &pipeline_color_blend_attachment_state;
    pipeline_color_blend_state_info.blendConstants[4] = 0;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

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

    VkGraphicsPipelineCreateInfo graphics_pipeline_info;
    graphics_pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_info.pNext = NULL;
    graphics_pipeline_info.flags = 0;
    graphics_pipeline_info.stageCount = 2;
    graphics_pipeline_info.pStages = shader_stages;
    graphics_pipeline_info.pVertexInputState = &pipeline_vertex_input_state_info;
    graphics_pipeline_info.pInputAssemblyState = &pipeline_input_assembly_state_info;
    graphics_pipeline_info.pTessellationState = NULL;
    graphics_pipeline_info.pViewportState = &pipeline_viewport_state_info;
    graphics_pipeline_info.pRasterizationState = &pipeline_rasterization_state_info;
    graphics_pipeline_info.pMultisampleState = &pipeline_multisampling_state_info;
    graphics_pipeline_info.pDepthStencilState = NULL;
    graphics_pipeline_info.pColorBlendState = &pipeline_color_blend_state_info;
    graphics_pipeline_info.pDynamicState = NULL;
    graphics_pipeline_info.layout = vulkan_backend->voxel_pipeline_context.layout;
    graphics_pipeline_info.renderPass = vulkan_backend->voxel_pipeline_context.render_pass;
    graphics_pipeline_info.subpass = 0;
    graphics_pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_info.basePipelineIndex = -1;

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

VkShaderModule vb_create_shader_module(VkDevice device, const char* filename)
{
    char* shader_src = NULL;
    size_t shader_src_size = c_read_file_binary(filename, &shader_src);

    VkShaderModuleCreateInfo shader_module_info;
    shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_info.pNext = NULL;
    shader_module_info.flags = 0;
    shader_module_info.codeSize = shader_src_size;
    shader_module_info.pCode = (const u32*)shader_src;

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
