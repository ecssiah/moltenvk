#ifndef GPU_MESH_H
#define GPU_MESH_H

#include <vulkan/vulkan.h>

struct GpuMesh
{
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_memory;

    VkBuffer index_buffer;
    VkDeviceMemory index_memory;

    uint32_t index_count;
};

#endif