#include "vulkan_backend_internal.h"


u32 vm_find_memory_type(
    VulkanBackend* vulkan_backend,
    u32 type_filter,
    VkMemoryPropertyFlags properties
) {
    
}

void vm_create_buffer(
    VulkanBackend* backend,
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
        backend->vulkan_device_context.device,
        &buffer_info,
        NULL,
        buffer
    );

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(
        backend->vulkan_device_context.device,
        *buffer,
        &mem_requirements
    );

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(
        backend->vulkan_device_context.physical_device,
        &mem_properties
    );

    u32 memory_type_index = UINT32_MAX;
    
    for (u32 i = 0; i < mem_properties.memoryTypeCount; ++i)
    {
        if (
            (mem_requirements.memoryTypeBits & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties
        ) {
            memory_type_index = i;
            break;
        }
    }

    VkMemoryAllocateInfo alloc_info = 
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = memory_type_index
    };

    vkAllocateMemory(
        backend->vulkan_device_context.device,
        &alloc_info,
        NULL,
        memory
    );

    vkBindBufferMemory(
        backend->vulkan_device_context.device,
        *buffer,
        *memory,
        0
    );
}

void vm_create_image(
    VulkanBackend* vulkan_backend,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage* image,
    VkDeviceMemory* memory
) {

}

VkImageView vm_create_image_view(
    VulkanBackend* vulkan_backend,
    VkImage image,
    VkFormat format
) {

}

VkSampler vm_create_sampler(VulkanBackend* vulkan_backend)
{
    
}
