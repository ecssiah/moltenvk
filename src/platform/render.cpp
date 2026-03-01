#include "render.h"
#include "frame_context.h"

#include <fstream>
#include <GLFW/glfw3.h>

void render_init(Render* render)
{
    // Device lifetime
    create_instance(render);
    create_surface(render, render->window);
    pick_physical_device(render);
    create_logical_device(render);
    create_command_pool(render);
    create_frame_contexts(render);

    // Swapchain lifetime
    create_swapchain_context(render);

    // Engine resources
    create_voxel_pipeline(render);
}

void create_swapchain_context(Render* render)
{
    create_swapchain(render);
    create_image_views(render);
    create_render_pass(render);
    create_frame_buffers(render);
}

void create_voxel_pipeline(Render* render)
{
    create_graphics_pipeline(render);
}

void render_destroy(Render* render)
{
    vkDeviceWaitIdle(render->vulkan_context.device);

    // -----------------------------
    // Swapchain lifetime
    // -----------------------------
    destroy_swapchain_context(render);

    // -----------------------------
    // Frame lifetime
    // -----------------------------
    for (size_t i = 0; i < render->frame_context_vec.size(); ++i)
    {
        FrameContext& frame = render->frame_context_vec[i];

        vkDestroyFence(render->vulkan_context.device, frame.in_flight, nullptr);
        vkDestroySemaphore(render->vulkan_context.device, frame.image_available, nullptr);
        vkDestroySemaphore(render->vulkan_context.device, frame.render_finished, nullptr);
    }

    // -----------------------------
    // Pipeline
    // -----------------------------
    vkDestroyPipeline(render->vulkan_context.device, render->voxel_pipeline.pipeline, nullptr);
    vkDestroyPipelineLayout(render->vulkan_context.device, render->voxel_pipeline.layout, nullptr);
    vkDestroyRenderPass(render->vulkan_context.device, render->voxel_pipeline.render_pass, nullptr);

    // -----------------------------
    // Device lifetime
    // -----------------------------
    vkDestroyCommandPool(render->vulkan_context.device, render->vulkan_context.command_pool, nullptr);
    vkDestroyDevice(render->vulkan_context.device, nullptr);
    vkDestroySurfaceKHR(render->vulkan_context.instance, render->vulkan_context.surface, nullptr);
    vkDestroyInstance(render->vulkan_context.instance, nullptr);
}

void render_frame(Render* render)
{
    FrameContext& frame_context = render->frame_context_vec[render->current_frame];

    vkWaitForFences(
        render->vulkan_context.device, 
        1, 
        &frame_context.in_flight, 
        VK_TRUE, 
        UINT64_MAX
    );

    uint32_t image_index;

    vkAcquireNextImageKHR(
        render->vulkan_context.device, 
        render->swapchain_context.swapchain, 
        UINT64_MAX, 
        frame_context.image_available, 
        VK_NULL_HANDLE, 
        &image_index
    );

    vkResetFences(render->vulkan_context.device, 1, &frame_context.in_flight);
    vkResetCommandBuffer(frame_context.command_buffer, 0);

    record_command_buffer(render, frame_context.command_buffer, image_index);

    VkPipelineStageFlags wait_stage_array[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit_info = {};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &frame_context.image_available;
    submit_info.pWaitDstStageMask = wait_stage_array;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &frame_context.command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &frame_context.render_finished;

    vkQueueSubmit(
        render->vulkan_context.graphics_queue,
        1,
        &submit_info,
        frame_context.in_flight
    );

    VkPresentInfoKHR present_info = {};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &frame_context.render_finished;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &render->swapchain_context.swapchain;
    present_info.pImageIndices = &image_index;

    vkQueuePresentKHR(render->vulkan_context.present_queue, &present_info);

    render->current_frame = (render->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void create_frame_contexts(Render *render)
{
    render->frame_context_vec.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = render->vulkan_context.command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    std::vector<VkCommandBuffer> command_buffer_vec(MAX_FRAMES_IN_FLIGHT);

    vkAllocateCommandBuffers(
        render->vulkan_context.device, 
        &command_buffer_allocate_info, 
        command_buffer_vec.data()
    );

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        render->frame_context_vec[i].command_buffer = command_buffer_vec[i];
    }

    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        FrameContext* frame_context = &render->frame_context_vec[i];

        vkCreateSemaphore(
            render->vulkan_context.device, 
            &semaphore_create_info, 
            nullptr, 
            &frame_context->image_available
        );

        vkCreateSemaphore(
            render->vulkan_context.device, 
            &semaphore_create_info, 
            nullptr, 
            &frame_context->render_finished
        );

        vkCreateFence(
            render->vulkan_context.device, 
            &fence_create_info, 
            nullptr, 
            &frame_context->in_flight
        );
    }
}

void create_instance(Render* render)
{
    uint32_t extension_count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extension_count);

    std::vector<const char*> required_extension_vec(extensions, extensions + extension_count);
    required_extension_vec.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    VkApplicationInfo application_info = {};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "Vulkan Test";
    application_info.applicationVersion = VK_MAKE_VERSION(1,0,0);
    application_info.pEngineName = "No Engine";
    application_info.engineVersion = VK_MAKE_VERSION(1,0,0);
    application_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;
    instance_create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instance_create_info.enabledExtensionCount = static_cast<uint32_t>(required_extension_vec.size());
    instance_create_info.ppEnabledExtensionNames = required_extension_vec.data();

    if (
        vkCreateInstance(
            &instance_create_info, 
            nullptr, 
            &render->vulkan_context.instance
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("Failed to create instance");
    }
}

void create_surface(Render* render, GLFWwindow* window)
{
    if (
        glfwCreateWindowSurface(
            render->vulkan_context.instance, 
            window, 
            nullptr, 
            &render->vulkan_context.surface
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("Failed to create surface");
    }
}

void pick_physical_device(Render* render)
{
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(render->vulkan_context.instance, &device_count, nullptr);

    std::vector<VkPhysicalDevice> device_vec(device_count);
    vkEnumeratePhysicalDevices(render->vulkan_context.instance, &device_count, device_vec.data());

    for (VkPhysicalDevice_T* device : device_vec)
    {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_family_vec(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_family_vec.data());

        for (uint32_t i = 0; i < queue_family_count; ++i)
        {
            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, render->vulkan_context.surface, &present_support);

            if ((queue_family_vec[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present_support)
            {
                render->vulkan_context.physical_device = device;
                render->vulkan_context.graphics_queue_family_index = i;

                return;
            }
        }
    }

    throw std::runtime_error("No suitable GPU found");
}

void create_logical_device(Render* render)
{
    float priority = 1.0f;

    VkDeviceQueueCreateInfo device_queue_info = {};
    device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_queue_info.queueFamilyIndex = render->vulkan_context.graphics_queue_family_index;
    device_queue_info.queueCount = 1;
    device_queue_info.pQueuePriorities = &priority;

    const char* extension_array[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    instance_create_info.queueCreateInfoCount = 1;
    instance_create_info.pQueueCreateInfos = &device_queue_info;
    instance_create_info.enabledExtensionCount = 1;
    instance_create_info.ppEnabledExtensionNames = extension_array;

    if (vkCreateDevice(render->vulkan_context.physical_device, &instance_create_info, nullptr, &render->vulkan_context.device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create device");
    }

    vkGetDeviceQueue(render->vulkan_context.device, render->vulkan_context.graphics_queue_family_index, 0, &render->vulkan_context.graphics_queue);

    render->vulkan_context.present_queue = render->vulkan_context.graphics_queue;
}

void create_swapchain(Render* render)
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(render->vulkan_context.physical_device, render->vulkan_context.surface, &surface_capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(render->vulkan_context.physical_device, render->vulkan_context.surface, &format_count, nullptr);

    std::vector<VkSurfaceFormatKHR> format_vec(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(render->vulkan_context.physical_device, render->vulkan_context.surface, &format_count, format_vec.data());

    VkSurfaceFormatKHR format = format_vec[0];
    render->swapchain_context.format = format.format;
    render->swapchain_context.extent = surface_capabilities.currentExtent;

    VkSwapchainCreateInfoKHR instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    instance_create_info.surface = render->vulkan_context.surface;
    instance_create_info.minImageCount = surface_capabilities.minImageCount + 1;
    instance_create_info.imageFormat = format.format;
    instance_create_info.imageColorSpace = format.colorSpace;
    instance_create_info.imageExtent = render->swapchain_context.extent;
    instance_create_info.imageArrayLayers = 1;
    instance_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    instance_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    instance_create_info.preTransform = surface_capabilities.currentTransform;
    instance_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    instance_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    instance_create_info.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(render->vulkan_context.device, &instance_create_info, nullptr, &render->swapchain_context.swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swapchain");
    }

    uint32_t image_count;
    vkGetSwapchainImagesKHR(render->vulkan_context.device, render->swapchain_context.swapchain, &image_count, nullptr);

    render->swapchain_context.image_vec.resize(image_count);
    vkGetSwapchainImagesKHR(render->vulkan_context.device, render->swapchain_context.swapchain, &image_count, render->swapchain_context.image_vec.data());
}

void create_image_views(Render* render)
{
    render->swapchain_context.image_view_vec.resize(render->swapchain_context.image_vec.size());

    for (size_t i = 0; i < render->swapchain_context.image_view_vec.size(); ++i)
    {
        VkImageViewCreateInfo image_view_create_info = {};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = render->swapchain_context.image_vec[i];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = render->swapchain_context.format;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.layerCount = 1;

        vkCreateImageView(
            render->vulkan_context.device, 
            &image_view_create_info, 
            nullptr, 
            &render->swapchain_context.image_view_vec[i]
        );
    }
}

void create_render_pass(Render* render)
{
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = render->swapchain_context.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachment_reference = {};
    attachment_reference.attachment = 0;
    attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description = {};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &attachment_reference;

    VkRenderPassCreateInfo render_pass_create_info = {};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &color_attachment;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_description;

    vkCreateRenderPass(
        render->vulkan_context.device, 
        &render_pass_create_info, 
        nullptr, 
        &render->voxel_pipeline.render_pass
    );
}

std::vector<char> read_file(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open shader file");
    }

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer_vec(file_size);

    file.seekg(0);
    file.read(buffer_vec.data(), file_size);
    file.close();

    return buffer_vec;
}

VkShaderModule create_shader_module(VkDevice device, const std::vector<char>& code)
{
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module");
    }

    return shader_module;
}

void create_graphics_pipeline(Render* render)
{
    std::vector<char> vert_shader_code = read_file("shaders/bin/test.vert.spv");
    std::vector<char> frag_shader_code = read_file("shaders/bin/test.frag.spv");

    VkShaderModule vert_module = create_shader_module(render->vulkan_context.device, vert_shader_code);
    VkShaderModule frag_module = create_shader_module(render->vulkan_context.device, frag_shader_code);

    VkPipelineShaderStageCreateInfo vert_stage = {};
    vert_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage.module = vert_module;
    vert_stage.pName = "main";

    VkPipelineShaderStageCreateInfo frag_stage = {};
    frag_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage.module = frag_module;
    frag_stage.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage, frag_stage};

    VkPipelineVertexInputStateCreateInfo vertex_input{};
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) render->swapchain_context.extent.width;
    viewport.height = (float) render->swapchain_context.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = render->swapchain_context.extent;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (
        vkCreatePipelineLayout(
            render->vulkan_context.device, 
            &pipeline_layout_info, 
            nullptr,
            &render->voxel_pipeline.layout
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("failed to create pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = render->voxel_pipeline.layout;
    pipeline_info.renderPass = render->voxel_pipeline.render_pass;
    pipeline_info.subpass = 0;

    if (
        vkCreateGraphicsPipelines(
            render->vulkan_context.device, 
            VK_NULL_HANDLE, 
            1, 
            &pipeline_info, 
            nullptr, 
            &render->voxel_pipeline.pipeline
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("failed to create graphics pipeline");
    }

    vkDestroyShaderModule(render->vulkan_context.device, frag_module, nullptr);
    vkDestroyShaderModule(render->vulkan_context.device, vert_module, nullptr);
}

void create_frame_buffers(Render* render)
{
    render->swapchain_context.framebuffer_vec.resize(render->swapchain_context.image_vec.size());

    for (size_t i = 0; i < render->swapchain_context.image_vec.size(); ++i)
    {
        VkFramebufferCreateInfo framebuffer_create_info = {};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = render->voxel_pipeline.render_pass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = &render->swapchain_context.image_view_vec[i];
        framebuffer_create_info.width = render->swapchain_context.extent.width;
        framebuffer_create_info.height = render->swapchain_context.extent.height;
        framebuffer_create_info.layers = 1;

        vkCreateFramebuffer(
            render->vulkan_context.device, 
            &framebuffer_create_info, 
            nullptr, 
            &render->swapchain_context.framebuffer_vec[i]
        );
    }
}

void create_command_pool(Render* render)
{
    VkCommandPoolCreateInfo command_pool_create_info = {};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = render->vulkan_context.graphics_queue_family_index;

    vkCreateCommandPool(render->vulkan_context.device, &command_pool_create_info, nullptr, &render->vulkan_context.command_pool);
}

void record_command_buffer(Render* render, VkCommandBuffer command_buffer, uint32_t image_index)
{
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkClearValue clear_color = {};
    clear_color.color = {{0.1f, 0.1f, 0.2f, 1.0f}};

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render->voxel_pipeline.render_pass;
    render_pass_info.framebuffer = render->swapchain_context.framebuffer_vec[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = render->swapchain_context.extent;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render->voxel_pipeline.pipeline);
    vkCmdDraw(command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);
    vkEndCommandBuffer(command_buffer);
}

void destroy_swapchain_context(Render* render)
{
    for (size_t i = 0; i < render->swapchain_context.framebuffer_vec.size(); ++i)
    {
        vkDestroyFramebuffer(
            render->vulkan_context.device,
            render->swapchain_context.framebuffer_vec[i],
            nullptr
        );
    }

    for (size_t i = 0; i < render->swapchain_context.image_view_vec.size(); ++i)
    {
        vkDestroyImageView(
            render->vulkan_context.device,
            render->swapchain_context.image_view_vec[i],
            nullptr
        );
    }

    vkDestroySwapchainKHR(
        render->vulkan_context.device,
        render->swapchain_context.swapchain,
        nullptr
    );
}