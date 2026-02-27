#include "renderer.h"

#include <fstream>

void Renderer::init()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(
        1024, 
        768, 
        "Vulkan Test", 
        nullptr, 
        nullptr
    );

    create_instance();
    create_surface(window);
    pick_physical_device();
    create_logical_device();
    create_swapchain();
    create_image_views();
    create_render_pass();
    create_graphics_pipeline();
    create_frame_buffers();
    create_command_pool();
    allocate_command_buffer();
    create_sync_objects();
}

void Renderer::wait_idle()
{

}

void Renderer::cleanup()
{
    vkDeviceWaitIdle(device);

    for (VkFramebuffer_T* framebuffer : frame_buffer_vec)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    
    vkDestroyPipeline(device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);

    vkDestroyRenderPass(device, render_pass, nullptr);

    for (VkImageView_T* view : image_view_vec)
    {
        vkDestroyImageView(device, view, nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, image_available_semaphore_vec[i], nullptr);
        vkDestroySemaphore(device, render_finished_semaphore_vec[i], nullptr);
        vkDestroyFence(device, in_flight_fence_vec[i], nullptr);
    }

    vkDestroyCommandPool(device, command_pool, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void Renderer::draw_frame()
{
    vkWaitForFences(device, 1, &in_flight_fence_vec[current_frame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;

    vkAcquireNextImageKHR(
        device, 
        swapchain, 
        UINT64_MAX, 
        image_available_semaphore_vec[current_frame], 
        VK_NULL_HANDLE, 
        &imageIndex
    );

    vkResetFences(device, 1, &in_flight_fence_vec[current_frame]);

    vkResetCommandBuffer(command_buffer_vec[current_frame], 0);

    record_command_buffer(command_buffer_vec[current_frame], imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &image_available_semaphore_vec[current_frame];

    VkPipelineStageFlags wait_stage_array[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submitInfo.pWaitDstStageMask = wait_stage_array;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer_vec[current_frame];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &render_finished_semaphore_vec[current_frame];

    vkQueueSubmit(
        graphics_queue,
        1,
        &submitInfo,
        in_flight_fence_vec[current_frame]
    );

    VkPresentInfoKHR presentInfo = {};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &render_finished_semaphore_vec[current_frame];

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(present_queue, &presentInfo);

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::create_instance()
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

    if (vkCreateInstance(&instance_create_info, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance");
    }
}

void Renderer::create_surface(GLFWwindow* window)
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create surface");
    }
}

void Renderer::pick_physical_device()
{
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    std::vector<VkPhysicalDevice> device_vec(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, device_vec.data());

    for (VkPhysicalDevice_T* device : device_vec)
    {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_family_vec(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_family_vec.data());

        for (uint32_t i = 0; i < queue_family_count; ++i)
        {
            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

            if ((queue_family_vec[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present_support)
            {
                physical_device = device;
                graphics_queue_familiy_index = i;

                return;
            }
        }
    }

    throw std::runtime_error("No suitable GPU found");
}

void Renderer::create_logical_device()
{
    float priority = 1.0f;

    VkDeviceQueueCreateInfo device_queue_info = {};
    device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_queue_info.queueFamilyIndex = graphics_queue_familiy_index;
    device_queue_info.queueCount = 1;
    device_queue_info.pQueuePriorities = &priority;

    const char* extension_array[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    instance_create_info.queueCreateInfoCount = 1;
    instance_create_info.pQueueCreateInfos = &device_queue_info;
    instance_create_info.enabledExtensionCount = 1;
    instance_create_info.ppEnabledExtensionNames = extension_array;

    if (vkCreateDevice(physical_device, &instance_create_info, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create device");
    }

    vkGetDeviceQueue(device, graphics_queue_familiy_index, 0, &graphics_queue);

    present_queue = graphics_queue;
}

void Renderer::create_swapchain()
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

    std::vector<VkSurfaceFormatKHR> format_vec(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, format_vec.data());

    VkSurfaceFormatKHR format = format_vec[0];
    swapchain_format = format.format;
    swapchain_extent = surface_capabilities.currentExtent;

    VkSwapchainCreateInfoKHR instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    instance_create_info.surface = surface;
    instance_create_info.minImageCount = surface_capabilities.minImageCount + 1;
    instance_create_info.imageFormat = format.format;
    instance_create_info.imageColorSpace = format.colorSpace;
    instance_create_info.imageExtent = swapchain_extent;
    instance_create_info.imageArrayLayers = 1;
    instance_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    instance_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    instance_create_info.preTransform = surface_capabilities.currentTransform;
    instance_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    instance_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    instance_create_info.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device, &instance_create_info, nullptr, &swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swapchain");
    }

    uint32_t image_count;
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);

    swapchain_image_vec.resize(image_count);
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_image_vec.data());
}

void Renderer::create_image_views()
{
    image_view_vec.resize(swapchain_image_vec.size());

    for (size_t i = 0; i < swapchain_image_vec.size(); ++i)
    {
        VkImageViewCreateInfo image_view_create_info = {};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = swapchain_image_vec[i];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = swapchain_format;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.layerCount = 1;

        vkCreateImageView(device, &image_view_create_info, nullptr, &image_view_vec[i]);
    }
}

void Renderer::create_render_pass()
{
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = swapchain_format;
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

    vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_pass);
}

std::vector<char> read_file(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("failed to open shader file");

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
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

void Renderer::create_graphics_pipeline()
{
    auto vert_shader_code = read_file("shaders/bin/test.vert.spv");
    auto frag_shader_code = read_file("shaders/bin/test.frag.spv");

    VkShaderModule vert_module = create_shader_module(device, vert_shader_code);
    VkShaderModule frag_module = create_shader_module(device, frag_shader_code);

    VkPipelineShaderStageCreateInfo vert_stage{};
    vert_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage.module = vert_module;
    vert_stage.pName = "main";

    VkPipelineShaderStageCreateInfo frag_stage{};
    frag_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage.module = frag_module;
    frag_stage.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = { vert_stage, frag_stage };

    VkPipelineVertexInputStateCreateInfo vertex_input{};
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapchain_extent.width;
    viewport.height = (float) swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain_extent;

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

    if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
    {
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
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline");
    }

    vkDestroyShaderModule(device, frag_module, nullptr);
    vkDestroyShaderModule(device, vert_module, nullptr);
}

void Renderer::create_frame_buffers()
{
    frame_buffer_vec.resize(image_view_vec.size());

    for (size_t i = 0; i < image_view_vec.size(); ++i)
    {
        VkFramebufferCreateInfo framebuffer_create_info = {};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = render_pass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = &image_view_vec[i];
        framebuffer_create_info.width = swapchain_extent.width;
        framebuffer_create_info.height = swapchain_extent.height;
        framebuffer_create_info.layers = 1;

        vkCreateFramebuffer(device, &framebuffer_create_info, nullptr, &frame_buffer_vec[i]);
    }
}

void Renderer::create_command_pool()
{
    VkCommandPoolCreateInfo command_pool_create_info = {};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = graphics_queue_familiy_index;

    vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool);
}

void Renderer::allocate_command_buffer()
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    command_buffer_vec.resize(MAX_FRAMES_IN_FLIGHT);

    vkAllocateCommandBuffers(
        device, 
        &command_buffer_allocate_info, 
        command_buffer_vec.data()
    );
}

void Renderer::create_sync_objects()
{
    image_available_semaphore_vec.resize(MAX_FRAMES_IN_FLIGHT);
    render_finished_semaphore_vec.resize(MAX_FRAMES_IN_FLIGHT);
    in_flight_fence_vec.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;  // Important!

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (
            vkCreateSemaphore(
                device, 
                &semaphore_create_info, 
                nullptr,
                &image_available_semaphore_vec[i]
            ) != VK_SUCCESS ||
            vkCreateSemaphore(
                device, 
                &semaphore_create_info, 
                nullptr,
                &render_finished_semaphore_vec[i]
            ) != VK_SUCCESS ||
            vkCreateFence(
                device, 
                &fence_create_info, 
                nullptr,
                &in_flight_fence_vec[i]
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("Failed to create sync objects.");
        }
    }
}

void Renderer::record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index)
{
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkClearValue clear_color{};
    clear_color.color = { { 0.1f, 0.1f, 0.2f, 1.0f } };

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = frame_buffer_vec[image_index];
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = swapchain_extent;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
    vkCmdDraw(command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);
    vkEndCommandBuffer(command_buffer);
}