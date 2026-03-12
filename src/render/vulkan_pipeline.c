#include "render/render.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core/core.h"
#include "core/log/log.h"

VkShaderModule render_vulkan_create_shader_module(VkDevice device, const char* filename)
{
    char* shader_src = NULL;
    size_t shader_src_size = read_file_binary(filename, &shader_src);

    VkShaderModuleCreateInfo shader_module_info =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
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

void render_vulkan_update_texture_descriptor(
    Render* render,
    VkImageView image_view,
    VkSampler sampler
) {
    VkDescriptorImageInfo image_info =
    {
        .sampler = render->voxel_pipeline_context.vulkan_texture.sampler,
        .imageView = render->voxel_pipeline_context.vulkan_texture.image_view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet write_descriptor_set =
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = render->voxel_pipeline_context.descriptor_set,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImageInfo = &image_info,
    };

    vkUpdateDescriptorSets(
        render->vulkan_device_context.device,
        1,
        &write_descriptor_set,
        0,
        NULL
    );
}

void render_vulkan_create_and_init_voxel_pipeline(Render* render)
{
    VkShaderModule vert_module = 
        render_vulkan_create_shader_module(
            render->vulkan_device_context.device, 
            "assets/shaders/bin/voxel.vert.spv"
        );

    VkShaderModule frag_module = 
        render_vulkan_create_shader_module(
            render->vulkan_device_context.device, 
            "assets/shaders/bin/voxel.frag.spv"
        );

    VkPipelineShaderStageCreateInfo vert_stage_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_module,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo frag_stage_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_module,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo shader_stage_array[] = 
    {
        vert_stage_info, 
        frag_stage_info
    };

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
            .format = VK_FORMAT_R32G32B32_SFLOAT,
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

    VkViewport viewport =
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = (f32)render->vulkan_swapchain_context.extent.width,
        .height = (f32)render->vulkan_swapchain_context.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor_rect =
    {
        .offset = { 0, 0 },
        .extent = render->vulkan_swapchain_context.extent,
    };

    VkPipelineViewportStateCreateInfo pipeline_viewport_state_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor_rect,
    };

    VkDynamicState dynamic_state_array[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamic_state_array,
    };

    VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
    };

    VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state =
    {
        .blendEnable = VK_FALSE,
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
        .bindingCount = 1,
        .pBindings = &descriptor_set_layout_binding
    };

    VkResult descriptor_set_layout_result = 
        vkCreateDescriptorSetLayout(
            render->vulkan_device_context.device,
            &descriptor_set_layout_info,
            NULL,
            &render->voxel_pipeline_context.descriptor_set_layout
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
        .maxSets = 64,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size
    };

    VkResult descriptor_pool_result =
        vkCreateDescriptorPool(
            render->vulkan_device_context.device,
            &pool_info,
            NULL,
            &render->voxel_pipeline_context.descriptor_pool
        );

    if (descriptor_pool_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create descriptor pool");
    }

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = render->voxel_pipeline_context.descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &render->voxel_pipeline_context.descriptor_set_layout
    };

    VkResult descriptor_set_result =
        vkAllocateDescriptorSets(
            render->vulkan_device_context.device,
            &descriptor_set_allocate_info,
            &render->voxel_pipeline_context.descriptor_set
        );

    if (descriptor_set_result != VK_SUCCESS)
    {
        LOG_FATAL("Failed to allocate descriptor set");
    }

    VkPushConstantRange push_constant_range =
    {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(VoxelPushConstants),
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = 
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &render->voxel_pipeline_context.descriptor_set_layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant_range,
    };

    VkResult pipeline_layout_result = 
        vkCreatePipelineLayout(
            render->vulkan_device_context.device, 
            &pipeline_layout_info, 
            NULL,
            &render->voxel_pipeline_context.layout
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
        .pDepthStencilState = &pipeline_depth_stencil_state_info,
        .pColorBlendState = &pipeline_color_blend_state_info,
        .pDynamicState = &pipeline_dynamic_state_info,
        .layout = render->voxel_pipeline_context.layout,
        .renderPass = render->vulkan_swapchain_context.render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    VkResult graphics_pipeline_result = 
        vkCreateGraphicsPipelines(
            render->vulkan_device_context.device, 
            VK_NULL_HANDLE, 
            1, 
            &graphics_pipeline_info, 
            NULL, 
            &render->voxel_pipeline_context.pipeline
        );

    if (graphics_pipeline_result != VK_SUCCESS) 
    {
        LOG_FATAL("Failed to create Vulkan graphics pipeline");
    }

    vkDestroyShaderModule(render->vulkan_device_context.device, frag_module, NULL);
    vkDestroyShaderModule(render->vulkan_device_context.device, vert_module, NULL);

    LOG_INFO("Voxel Pipeline Initialized");
}

void render_vulkan_destroy_voxel_pipeline(Render* render)
{
    VkDevice device = render->vulkan_device_context.device;

    // Destroy texture resources
    vkDestroySampler(
        device,
        render->voxel_pipeline_context.vulkan_texture.sampler,
        NULL
    );

    vkDestroyImageView(
        device,
        render->voxel_pipeline_context.vulkan_texture.image_view,
        NULL
    );

    vkDestroyImage(
        device,
        render->voxel_pipeline_context.vulkan_texture.image,
        NULL
    );

    vkFreeMemory(
        device,
        render->voxel_pipeline_context.vulkan_texture.image_memory,
        NULL
    );

    // Destroy vertex buffer
    vkDestroyBuffer(
        device,
        render->voxel_pipeline_context.vertex_buffer,
        NULL
    );

    vkFreeMemory(
        device,
        render->voxel_pipeline_context.vertex_memory,
        NULL
    );

    // Destroy descriptor resources
    vkDestroyDescriptorPool(
        device,
        render->voxel_pipeline_context.descriptor_pool,
        NULL
    );

    vkDestroyDescriptorSetLayout(
        device,
        render->voxel_pipeline_context.descriptor_set_layout,
        NULL
    );

    // Destroy pipeline objects
    vkDestroyPipeline(
        device,
        render->voxel_pipeline_context.pipeline,
        NULL
    );

    vkDestroyPipelineLayout(
        device,
        render->voxel_pipeline_context.layout,
        NULL
    );

    vkDestroyRenderPass(
        device,
        render->vulkan_swapchain_context.render_pass,
        NULL
    );
}

void render_vulkan_create_and_init_nuklear_pipeline(Render* render)
{
    VkShaderModule vert_module =
        render_vulkan_create_shader_module(
            render->vulkan_device_context.device,
            "assets/shaders/bin/nuklear.vert.spv"
        );

    VkShaderModule frag_module =
        render_vulkan_create_shader_module(
            render->vulkan_device_context.device,
            "assets/shaders/bin/nuklear.frag.spv"
        );

    VkPipelineShaderStageCreateInfo vert_stage_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_module,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo frag_stage_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_module,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo shader_stage_array[] =
    {
        vert_stage_info,
        frag_stage_info
    };

    /* Nuklear vertex format: position (vec2), uv (vec2), color (rgba8) */
    VkVertexInputBindingDescription vertex_binding =
    {
        .binding = 0,
        .stride = sizeof(NkVertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription vertex_attributes[3] =
    {
        {
            .binding = 0,
            .location = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(NkVertex, position)
        },
        {
            .binding = 0,
            .location = 1,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(NkVertex, uv)
        },
        {
            .binding = 0,
            .location = 2,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .offset = offsetof(NkVertex, color)
        }
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertex_binding,
        .vertexAttributeDescriptionCount = 3,
        .pVertexAttributeDescriptions = vertex_attributes,
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport =
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = (f32)render->vulkan_swapchain_context.extent.width,
        .height = (f32)render->vulkan_swapchain_context.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor =
    {
        .offset = {0,0},
        .extent = render->vulkan_swapchain_context.extent
    };

    VkPipelineViewportStateCreateInfo viewport_state =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkDynamicState dynamic_states[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamic_states,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampling =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineDepthStencilStateCreateInfo depth =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable = VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_ALWAYS,
    };

    /* Alpha blending for UI */
    VkPipelineColorBlendAttachmentState blend_attachment =
    {
        .blendEnable = VK_TRUE,

        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,

        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alphaBlendOp = VK_BLEND_OP_ADD,

        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo blend_state =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &blend_attachment
    };

    VkDescriptorSetLayoutBinding sampler_binding =
    {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    VkDescriptorSetLayoutCreateInfo layout_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &sampler_binding
    };

    vkCreateDescriptorSetLayout(
        render->vulkan_device_context.device,
        &layout_info,
        NULL,
        &render->nuklear_pipeline_context.descriptor_set_layout
    );

    VkPushConstantRange push_constant_range =
    {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(NuklearPushConstants)
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant_range,
        .pSetLayouts = &render->nuklear_pipeline_context.descriptor_set_layout,
    };

    vkCreatePipelineLayout(
        render->vulkan_device_context.device,
        &pipeline_layout_info,
        NULL,
        &render->nuklear_pipeline_context.layout
    );

    VkGraphicsPipelineCreateInfo pipeline_info =
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shader_stage_array,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depth,
        .pColorBlendState = &blend_state,
        .pDynamicState = &dynamic_state,
        .layout = render->nuklear_pipeline_context.layout,
        .renderPass = render->vulkan_swapchain_context.render_pass,
        .subpass = 0
    };

    vkCreateGraphicsPipelines(
        render->vulkan_device_context.device,
        VK_NULL_HANDLE,
        1,
        &pipeline_info,
        NULL,
        &render->nuklear_pipeline_context.pipeline
    );

    vkDestroyShaderModule(render->vulkan_device_context.device, vert_module, NULL);
    vkDestroyShaderModule(render->vulkan_device_context.device, frag_module, NULL);

    LOG_INFO("Nuklear Pipeline Initialized");
}

void render_vulkan_destroy_nuklear_pipeline(Render* render)
{

}
