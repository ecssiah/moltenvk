#include "render.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void r_init(Render* render, GLFWwindow* window)
{
    // Device lifetime
    r_create_instance(render);
    r_create_surface(render, window);
    r_pick_physical_device(render);
    r_create_logical_device(render);
    r_create_command_pool(render);
    r_create_frame_contexts(render);

    // Swapchain lifetime
    r_create_swapchain_context(render);

    // Engine resources
    r_create_voxel_pipeline(render);
}

void r_quit(Render* render)
{
    vkDeviceWaitIdle(render->vulkan_context.device);

    r_destroy_swapchain_context(render);

    u32 frame_index;
    for (frame_index = 0; frame_index < MAX_FRAMES_IN_FLIGHT; ++frame_index)
    {
        FrameContext* frame = &render->frame_context_array[frame_index];

        vkDestroyFence(render->vulkan_context.device, frame->in_flight, NULL);
        vkDestroySemaphore(render->vulkan_context.device, frame->image_available, NULL);
        vkDestroySemaphore(render->vulkan_context.device, frame->render_finished, NULL);
    }

    vkDestroyPipeline(render->vulkan_context.device, render->voxel_pipeline.pipeline, NULL);
    vkDestroyPipelineLayout(render->vulkan_context.device, render->voxel_pipeline.layout, NULL);
    vkDestroyRenderPass(render->vulkan_context.device, render->voxel_pipeline.render_pass, NULL);

    vkDestroyCommandPool(render->vulkan_context.device, render->vulkan_context.command_pool, NULL);
    vkDestroyDevice(render->vulkan_context.device, NULL);
    vkDestroySurfaceKHR(render->vulkan_context.instance, render->vulkan_context.surface, NULL);
    vkDestroyInstance(render->vulkan_context.instance, NULL);
}

void r_render(Render* render)
{
    FrameContext* frame_context = &render->frame_context_array[render->frame_index];

    vkWaitForFences(
        render->vulkan_context.device, 
        1, 
        &frame_context->in_flight, 
        VK_TRUE, 
        UINT64_MAX
    );

    u32 image_index;

    vkAcquireNextImageKHR(
        render->vulkan_context.device, 
        render->swapchain_context.swapchain, 
        UINT64_MAX, 
        frame_context->image_available, 
        VK_NULL_HANDLE, 
        &image_index
    );

    vkResetFences(render->vulkan_context.device, 1, &frame_context->in_flight);
    vkResetCommandBuffer(frame_context->command_buffer, 0);

    r_record_command_buffer(render, frame_context->command_buffer, image_index);

    VkPipelineStageFlags wait_stage_array[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &frame_context->image_available;
    submit_info.pWaitDstStageMask = wait_stage_array;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &frame_context->command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &frame_context->render_finished;

    vkQueueSubmit(
        render->vulkan_context.graphics_queue,
        1,
        &submit_info,
        frame_context->in_flight
    );

    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = NULL;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &frame_context->render_finished;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &render->swapchain_context.swapchain;
    present_info.pImageIndices = &image_index;
    present_info.pResults = NULL;

    vkQueuePresentKHR(render->vulkan_context.present_queue, &present_info);

    render->frame_index = (render->frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

void r_create_swapchain_context(Render* render)
{
    r_create_swapchain(render);
    r_create_image_views(render);
    r_create_render_pass(render);
    r_create_frame_buffers(render);
}

void r_create_voxel_pipeline(Render* render)
{
    r_create_graphics_pipeline(render);
}

void r_create_frame_contexts(Render* render)
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info;
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.pNext = NULL;
    command_buffer_allocate_info.commandPool = render->vulkan_context.command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    VkCommandBuffer command_buffer_array[MAX_FRAMES_IN_FLIGHT];

    vkAllocateCommandBuffers(
        render->vulkan_context.device, 
        &command_buffer_allocate_info, 
        command_buffer_array
    );

    VkSemaphoreCreateInfo semaphore_create_info;
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = NULL;
    semaphore_create_info.flags = 0;

    VkFenceCreateInfo fence_create_info;
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.pNext = NULL;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    u32 frame_index;
    for (frame_index = 0; frame_index < MAX_FRAMES_IN_FLIGHT; ++frame_index)
    {
        FrameContext* frame_context = &render->frame_context_array[frame_index];
        
        frame_context->command_buffer = command_buffer_array[frame_index];

        vkCreateSemaphore(
            render->vulkan_context.device, 
            &semaphore_create_info, 
            NULL, 
            &frame_context->image_available
        );

        vkCreateSemaphore(
            render->vulkan_context.device, 
            &semaphore_create_info, 
            NULL, 
            &frame_context->render_finished
        );

        vkCreateFence(
            render->vulkan_context.device, 
            &fence_create_info, 
            NULL, 
            &frame_context->in_flight
        );
    }
}

void r_create_instance(Render* render)
{
    u32 extension_count = 0;
    const char** extension_array = glfwGetRequiredInstanceExtensions(&extension_count);
    const char** required_extension_array = malloc(sizeof (const char*) * (extension_count + 1));

    u32 extension_index;
    for (extension_index = 0; extension_index < extension_count; ++extension_index)
    {
        required_extension_array[extension_index] = extension_array[extension_index];
    }

    required_extension_array[extension_count] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

    VkApplicationInfo application_info;
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pNext = NULL;
    application_info.pApplicationName = "Vulkan Test";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "Vulkan Test Engine";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo instance_create_info;
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext = NULL;
    instance_create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instance_create_info.pApplicationInfo = &application_info;
    instance_create_info.enabledLayerCount = 0;
    instance_create_info.ppEnabledLayerNames = NULL;
    instance_create_info.enabledExtensionCount = extension_count + 1;
    instance_create_info.ppEnabledExtensionNames = required_extension_array;

    int instance_result = 
        vkCreateInstance(
            &instance_create_info, 
            NULL, 
            &render->vulkan_context.instance
        );

    if (instance_result != VK_SUCCESS) 
    {
        free(required_extension_array);

        fprintf(stderr, "Failed to create Vulkan instance\n");

        exit(EXIT_FAILURE);
    }

    free(required_extension_array);
}

void r_create_surface(Render* render, GLFWwindow* window)
{
    VkResult surface_result = 
        glfwCreateWindowSurface(
            render->vulkan_context.instance, 
            window, 
            NULL, 
            &render->vulkan_context.surface
        );

    if (surface_result != VK_SUCCESS) 
    {
        fprintf(stderr, "Failed to create GLFW Window Surface\n");

        exit(EXIT_FAILURE);
    }
}

void r_pick_physical_device(Render* render)
{
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(render->vulkan_context.instance, &device_count, NULL);

    if (device_count == 0)
    {
        fprintf(stderr, "No Vulkan physical devices found\n");

        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice* physical_device_array = malloc(sizeof (VkPhysicalDevice) * device_count);
    vkEnumeratePhysicalDevices(render->vulkan_context.instance, &device_count, physical_device_array);

    u32 device_index;
    for (device_index = 0; device_index < device_count; ++device_index)
    {
        VkPhysicalDevice device = physical_device_array[device_index];

        u32 queue_family_count = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(
            device, 
            &queue_family_count, 
            NULL
        );

        VkQueueFamilyProperties* queue_family_properties_array = malloc(sizeof (VkQueueFamilyProperties) * queue_family_count);

        vkGetPhysicalDeviceQueueFamilyProperties(
            device, 
            &queue_family_count, 
            queue_family_properties_array
        );

        u32 queue_family_index;
        for (queue_family_index = 0; queue_family_index < queue_family_count; ++queue_family_index)
        {
            VkBool32 present_support = VK_FALSE;

            vkGetPhysicalDeviceSurfaceSupportKHR(
                device, 
                queue_family_index, 
                render->vulkan_context.surface, 
                &present_support
            );

            if (
                (queue_family_properties_array[queue_family_index].queueFlags & VK_QUEUE_GRAPHICS_BIT) && 
                present_support
            ) {
                render->vulkan_context.physical_device = device;
                render->vulkan_context.graphics_queue_family_index = queue_family_index;

                free(queue_family_properties_array);
                free(physical_device_array);

                return;
            }
        }

        free(queue_family_properties_array);
    }

    free(physical_device_array);

    fprintf(stderr, "No suitable GPU found");

    exit(EXIT_FAILURE);
}

void r_create_logical_device(Render* render)
{
    f32 queue_priority = 1.0f;

    VkDeviceQueueCreateInfo device_queue_info;
    device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_queue_info.pNext = NULL;
    device_queue_info.flags = 0;
    device_queue_info.queueFamilyIndex = render->vulkan_context.graphics_queue_family_index;
    device_queue_info.queueCount = 1;
    device_queue_info.pQueuePriorities = &queue_priority;

    const char* extension_array[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo device_info;
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pNext = NULL;
    device_info.flags = 0;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &device_queue_info;
    device_info.enabledLayerCount = 0;
    device_info.ppEnabledLayerNames = NULL;
    device_info.enabledExtensionCount = 1;
    device_info.ppEnabledExtensionNames = extension_array;
    device_info.pEnabledFeatures = NULL;

    VkResult device_result = 
        vkCreateDevice(
            render->vulkan_context.physical_device, 
            &device_info, 
            NULL, 
            &render->vulkan_context.device
        );

    if (device_result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create Vulkan device");

        exit(EXIT_FAILURE);
    }

    vkGetDeviceQueue(
        render->vulkan_context.device, 
        render->vulkan_context.graphics_queue_family_index, 
        0, 
        &render->vulkan_context.graphics_queue
    );

    render->vulkan_context.present_queue = render->vulkan_context.graphics_queue;
}

void r_create_swapchain(Render* render)
{
    VkSurfaceCapabilitiesKHR surface_capabilities;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        render->vulkan_context.physical_device, 
        render->vulkan_context.surface, 
        &surface_capabilities
    );

    u32 format_count;

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        render->vulkan_context.physical_device, 
        render->vulkan_context.surface, 
        &format_count, 
        NULL
    );

    VkSurfaceFormatKHR* surface_format_array = malloc(sizeof (VkSurfaceFormatKHR) * format_count);

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        render->vulkan_context.physical_device,
        render->vulkan_context.surface,
        &format_count,
        surface_format_array
    );

    VkSurfaceFormatKHR surface_format = surface_format_array[0];
    render->swapchain_context.format = surface_format.format;
    render->swapchain_context.extent = surface_capabilities.currentExtent;

    free(surface_format_array);

    VkSwapchainCreateInfoKHR instance_create_info;
    instance_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    instance_create_info.pNext = NULL;
    instance_create_info.flags = 0;
    instance_create_info.surface = render->vulkan_context.surface;
    instance_create_info.minImageCount = 0;
    instance_create_info.imageFormat = surface_format.format;
    instance_create_info.imageColorSpace = surface_format.colorSpace;
    instance_create_info.imageExtent = render->swapchain_context.extent;
    instance_create_info.imageArrayLayers = 1;
    instance_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    instance_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    instance_create_info.queueFamilyIndexCount = 0;
    instance_create_info.pQueueFamilyIndices = NULL;
    instance_create_info.preTransform = surface_capabilities.currentTransform;
    instance_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    instance_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    instance_create_info.clipped = VK_TRUE;
    instance_create_info.oldSwapchain = NULL;

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
            render->vulkan_context.device, 
            &instance_create_info, 
            NULL, 
            &render->swapchain_context.swapchain
        );

    if (swapchain_result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create swapchain");

        exit(EXIT_FAILURE);
    }

    u32 image_count = 0;

    vkGetSwapchainImagesKHR(
        render->vulkan_context.device, 
        render->swapchain_context.swapchain, 
        &image_count, 
        NULL
    );
    
    render->swapchain_context.image_array = malloc(sizeof (VkImage) * image_count);
    render->swapchain_context.image_view_array = malloc(sizeof (VkImageView) * image_count);
    render->swapchain_context.framebuffer_array = malloc(sizeof (VkFramebuffer) * image_count);

    render->swapchain_context.image_count = image_count;

    vkGetSwapchainImagesKHR(
        render->vulkan_context.device, 
        render->swapchain_context.swapchain, 
        &image_count, 
        render->swapchain_context.image_array
    );
}

void r_create_image_views(Render* render)
{
    u32 image_index;
    for (image_index = 0; image_index < render->swapchain_context.image_count; ++image_index)
    {
        VkComponentMapping component_mapping;
        component_mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        component_mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        component_mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        component_mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        VkImageSubresourceRange image_subresource_range;
        image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_subresource_range.baseMipLevel = 0;
        image_subresource_range.levelCount = 1;
        image_subresource_range.baseArrayLayer = 0;
        image_subresource_range.layerCount = 1;

        VkImageViewCreateInfo image_view_create_info;
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.pNext = NULL;
        image_view_create_info.flags = 0;
        image_view_create_info.image = render->swapchain_context.image_array[image_index];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = render->swapchain_context.format;
        image_view_create_info.components = component_mapping;
        image_view_create_info.subresourceRange = image_subresource_range;

        vkCreateImageView(
            render->vulkan_context.device, 
            &image_view_create_info, 
            NULL, 
            &render->swapchain_context.image_view_array[image_index]
        );
    }
}

void r_create_render_pass(Render* render)
{
    VkAttachmentDescription color_attachment;
    color_attachment.flags = 0;
    color_attachment.format = render->swapchain_context.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachment_reference;
    attachment_reference.attachment = 0;
    attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description;
    subpass_description.flags = 0;
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.inputAttachmentCount = 0;
    subpass_description.pInputAttachments = NULL;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &attachment_reference;
    subpass_description.pResolveAttachments = NULL;
    subpass_description.pDepthStencilAttachment = NULL;
    subpass_description.preserveAttachmentCount = 0;
    subpass_description.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo render_pass_info;
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.pNext = NULL;
    render_pass_info.flags = 0;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass_description;
    render_pass_info.dependencyCount = 0;
    render_pass_info.pDependencies = NULL;

    vkCreateRenderPass(
        render->vulkan_context.device, 
        &render_pass_info, 
        NULL, 
        &render->voxel_pipeline.render_pass
    );
}

size_t r_read_file_binary(const char* filename, char** out_buffer)
{
    FILE* file = fopen(filename, "rb");

    if (!file)
    {
        fprintf(stderr, "Could not open: %s\n", filename);

        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size);

    if (!buffer)
    {
        fclose(file);

        fprintf(stderr, "Failed to allocate shader buffer\n");

        exit(EXIT_FAILURE);
    }

    fread(buffer, 1, file_size, file);
    fclose(file);

    *out_buffer = buffer;

    return file_size;
}

VkShaderModule r_create_shader_module(VkDevice device, const char* filename)
{
    char* shader_src = NULL;
    size_t shader_src_size = r_read_file_binary(filename, &shader_src);

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

void r_create_graphics_pipeline(Render* render)
{
    VkShaderModule vert_module = 
        r_create_shader_module(
            render->vulkan_context.device, 
            "assets/shaders/bin/test.vert.spv"
        );

    VkShaderModule frag_module = 
        r_create_shader_module(
            render->vulkan_context.device, 
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
    viewport.width = (f32)render->swapchain_context.extent.width;
    viewport.height = (f32)render->swapchain_context.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor_rect;
    scissor_rect.offset.x = 0;
    scissor_rect.offset.y = 0;
    scissor_rect.extent = render->swapchain_context.extent;

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
            render->vulkan_context.device, 
            &pipeline_layout_info, 
            NULL,
            &render->voxel_pipeline.layout
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
    graphics_pipeline_info.layout = render->voxel_pipeline.layout;
    graphics_pipeline_info.renderPass = render->voxel_pipeline.render_pass;
    graphics_pipeline_info.subpass = 0;
    graphics_pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_info.basePipelineIndex = -1;

    VkResult graphics_pipeline_result = 
        vkCreateGraphicsPipelines(
            render->vulkan_context.device, 
            VK_NULL_HANDLE, 
            1, 
            &graphics_pipeline_info, 
            NULL, 
            &render->voxel_pipeline.pipeline
        );

    if (graphics_pipeline_result != VK_SUCCESS) 
    {
        fprintf(stderr, "Failed to create Vulkan graphics pipeline\n");

        exit(EXIT_FAILURE);
    }

    vkDestroyShaderModule(render->vulkan_context.device, frag_module, NULL);
    vkDestroyShaderModule(render->vulkan_context.device, vert_module, NULL);
}

void r_create_frame_buffers(Render* render)
{
    u32 image_index;
    for (image_index = 0; image_index < render->swapchain_context.image_count; ++image_index)
    {
        VkFramebufferCreateInfo framebuffer_info;
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.pNext = NULL;
        framebuffer_info.flags = 0;
        framebuffer_info.renderPass = render->voxel_pipeline.render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = &render->swapchain_context.image_view_array[image_index];
        framebuffer_info.width = render->swapchain_context.extent.width;
        framebuffer_info.height = render->swapchain_context.extent.height;
        framebuffer_info.layers = 1;

        vkCreateFramebuffer(
            render->vulkan_context.device, 
            &framebuffer_info, 
            NULL, 
            &render->swapchain_context.framebuffer_array[image_index]
        );
    }
}

void r_create_command_pool(Render* render)
{
    VkCommandPoolCreateInfo command_pool_info;
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.pNext = NULL;
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_info.queueFamilyIndex = render->vulkan_context.graphics_queue_family_index;

    vkCreateCommandPool(
        render->vulkan_context.device, 
        &command_pool_info, 
        NULL, 
        &render->vulkan_context.command_pool
    );
}

void r_record_command_buffer(Render* render, VkCommandBuffer command_buffer, u32 image_index)
{
    VkCommandBufferBeginInfo command_buffer_info;
    command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_info.pNext = NULL;
    command_buffer_info.flags = 0;
    command_buffer_info.pInheritanceInfo = NULL;

    vkBeginCommandBuffer(command_buffer, &command_buffer_info);

    VkClearValue clear_color;
    clear_color.color.float32[0] = 0.1f;
    clear_color.color.float32[1] = 0.1f;
    clear_color.color.float32[2] = 0.2f;
    clear_color.color.float32[3] = 1.0f;

    VkRect2D render_area;
    render_area.offset.x = 0;
    render_area.offset.y = 0;
    render_area.extent = render->swapchain_context.extent;

    VkRenderPassBeginInfo render_pass_begin_info;
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.pNext = NULL;
    render_pass_begin_info.renderPass = render->voxel_pipeline.render_pass;
    render_pass_begin_info.framebuffer = render->swapchain_context.framebuffer_array[image_index];
    render_pass_begin_info.renderArea = render_area;
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_begin_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        command_buffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        render->voxel_pipeline.pipeline
    );
    
    vkCmdDraw(command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);
    vkEndCommandBuffer(command_buffer);
}

void r_destroy_swapchain_context(Render* render)
{
    u32 image_index;
    for (image_index = 0; image_index < render->swapchain_context.image_count; ++image_index)
    {
        vkDestroyFramebuffer(
            render->vulkan_context.device,
            render->swapchain_context.framebuffer_array[image_index],
            NULL
        );

        vkDestroyImageView(
            render->vulkan_context.device,
            render->swapchain_context.image_view_array[image_index],
            NULL
        );
    }

    free(render->swapchain_context.image_array);
    free(render->swapchain_context.image_view_array);
    free(render->swapchain_context.framebuffer_array);

    vkDestroySwapchainKHR(
        render->vulkan_context.device,
        render->swapchain_context.swapchain,
        NULL
    );
}