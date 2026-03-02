#ifndef PIPELINE_H
#define PIPELINE_H

#include <vulkan/vulkan.h>

typedef struct
{
    VkRenderPass render_pass;
    VkPipelineLayout layout;
    VkPipeline pipeline;
}
Pipeline;

#endif