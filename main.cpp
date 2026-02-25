#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

struct Renderer
{
    VkInstance instance = {};
    VkSurfaceKHR surface = {};
    VkPhysicalDevice physical_device = {VK_NULL_HANDLE};
    VkDevice device = {};
    VkQueue graphics_queue = {};
    uint32_t graphics_queue_familiy_index = {UINT32_MAX};

    VkSwapchainKHR swapchain = {};
    VkFormat swapchain_format = {};
    VkExtent2D swapchain_extent = {};
    std::vector<VkImage> swapchain_image_vec;
    std::vector<VkImageView> image_view_vec;

    VkRenderPass render_pass = {};
    std::vector<VkFramebuffer> frame_buffer_vec;

    VkCommandPool command_pool = {};
    VkCommandBuffer command_buffer = {};

    void init(GLFWwindow* window)
    {
        create_instance();
        create_surface(window);
        pick_physical_device();
        create_logical_device();
        create_swapchain();
        create_image_views();
        create_render_pass();
        create_frame_buffers();
        create_command_pool();
        allocate_command_buffer();
    }

    void cleanup()
    {
        vkDeviceWaitIdle(device);

        for (auto framebuffer : frame_buffer_vec)
        {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        vkDestroyRenderPass(device, render_pass, nullptr);

        for (auto view : image_view_vec)
        {
            vkDestroyImageView(device, view, nullptr);
        }

        vkDestroyCommandPool(device, command_pool, nullptr);
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    void draw()
    {
        uint32_t imageIndex;

        vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, VK_NULL_HANDLE, VK_NULL_HANDLE, &imageIndex);

        VkCommandBufferBeginInfo command_buffer_begin_info = {};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

        VkClearValue clear_value = {};
        clear_value.color = {{0.1f, 0.2f, 0.4f, 1.0f}};

        VkRenderPassBeginInfo render_pass_begin_info = {};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = render_pass;
        render_pass_begin_info.framebuffer = frame_buffer_vec[imageIndex];
        render_pass_begin_info.renderArea.extent = swapchain_extent;
        render_pass_begin_info.clearValueCount = 1;
        render_pass_begin_info.pClearValues = &clear_value;

        vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdEndRenderPass(command_buffer);
        vkEndCommandBuffer(command_buffer);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphics_queue);

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &imageIndex;

        vkQueuePresentKHR(graphics_queue, &present_info);
    }

private:

    void create_instance()
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

    void create_surface(GLFWwindow* window)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create surface");
        }
    }

    void pick_physical_device()
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

        std::vector<VkPhysicalDevice> device_vec(device_count);
        vkEnumeratePhysicalDevices(instance, &device_count, device_vec.data());

        for (auto device : device_vec)
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

    void create_logical_device()
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
    }

    void create_swapchain()
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

    void create_image_views()
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

    void create_render_pass()
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

    void create_frame_buffers()
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

    void create_command_pool()
    {
        VkCommandPoolCreateInfo command_pool_create_info = {};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.queueFamilyIndex = graphics_queue_familiy_index;

        vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool);
    }

    void allocate_command_buffer()
    {
        VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = command_pool;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = 1;

        vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &command_buffer);
    }
};

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(
        1024, 
        768, 
        "Vulkan Test", 
        nullptr, 
        nullptr
    );

    Renderer renderer;
    renderer.init(window);
    renderer.draw();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    renderer.cleanup();

    glfwDestroyWindow(window);
    glfwTerminate();
}