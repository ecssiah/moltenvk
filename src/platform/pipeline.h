#ifndef PIPELINE_H
#define PIPELINE_H

#include <vulkan/vulkan.h>

struct Pipeline
{
    VkRenderPass render_pass;
    VkPipelineLayout layout;
    VkPipeline pipeline;
};

#endif