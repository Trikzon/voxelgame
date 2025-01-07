#include "mellohi/graphics/vulkan/device.hpp"
#include <vulkan/vulkan_structs.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace mellohi
{
    Device::Device(const Config &config, const Platform &platform)
    {
        VULKAN_HPP_DEFAULT_DISPATCHER.init();
        create_instance(config, platform);
        create_debug_utils_messenger();
        choose_physical_device();
        create_device();
        choose_preferred_surface_format();
    }
    
    Device::~Device()
    {
        m_device.destroy();
        
        m_instance.destroySurfaceKHR(m_surface);
        
        if (m_debug_utils_messenger)
        {
            m_instance.destroyDebugUtilsMessengerEXT(*m_debug_utils_messenger);
        }
        
        m_instance.destroy();
    }
    
    void Device::reset_fence(const vk::Fence fence) const
    {
        const auto result = m_device.resetFences(1, &fence);
        MH_ASSERT_VK(result, "Failed to reset Vulkan fence.");
    }
    
    void Device::wait_for_fence(const vk::Fence fence, const u64 timeout) const
    {
        const auto result = m_device.waitForFences(1, &fence, vk::True, timeout);
        MH_ASSERT_VK(result, "Failed to wait for Vulkan fence.");
    }
    
    void Device::wait_idle() const
    {
        const auto result = m_device.waitIdle();
        MH_ASSERT_VK(result, "Failed to wait for Vulkan device.");
    }
    
    std::vector<vk::CommandBuffer> Device::allocate_command_buffers(
        const vk::CommandBufferAllocateInfo &allocate_info) const
    {
        const auto resval = m_device.allocateCommandBuffers(allocate_info);
        MH_ASSERT_VK(resval.result, "Failed to allocate Vulkan command buffer.");
        return resval.value;
    }
    
    vk::CommandPool Device::create_command_pool(const vk::CommandPoolCreateInfo &create_info) const
    {
        const auto resval = m_device.createCommandPool(create_info);
        MH_ASSERT_VK(resval.result, "Failed to create Vulkan command pool.");
        return resval.value;
    }
    
    vk::Fence Device::create_fence(const vk::FenceCreateInfo &create_info) const
    {
        const auto resval = m_device.createFence(create_info);
        MH_ASSERT_VK(resval.result, "Failed to create Vulkan fence.");
        return resval.value;
    }
    
    vk::Framebuffer Device::create_framebuffer(const vk::FramebufferCreateInfo &create_info) const
    {
        const auto resval = m_device.createFramebuffer(create_info);
        MH_ASSERT_VK(resval.result, "Failed to create Vulkan framebuffer.");
        return resval.value;
    }
    
    vk::Pipeline Device::create_graphics_pipeline(const vk::GraphicsPipelineCreateInfo &create_info) const
    {
        const auto resval = m_device.createGraphicsPipeline(nullptr, create_info);
        MH_ASSERT_VK(resval.result, "Failed to create Vulkan graphics pipeline.");
        return resval.value;
    }
    
    vk::ImageView Device::create_image_view(const vk::ImageViewCreateInfo &create_info) const
    {
        const auto resval = m_device.createImageView(create_info);
        MH_ASSERT_VK(resval.result, "Failed to create Vulkan image view.");
        return resval.value;
    }
    
    vk::PipelineLayout Device::create_pipeline_layout(const vk::PipelineLayoutCreateInfo &create_info) const
    {
        const auto resval = m_device.createPipelineLayout(create_info);
        MH_ASSERT_VK(resval.result, "Failed to create Vulkan pipeline layout.");
        return resval.value;
    }
    
    vk::RenderPass Device::create_render_pass(const vk::RenderPassCreateInfo &create_info) const
    {
        const auto resval = m_device.createRenderPass(create_info);
        MH_ASSERT_VK(resval.result, "Failed to create Vulkan render pass.");
        return resval.value;
    }
    
    vk::Semaphore Device::create_semaphore(const vk::SemaphoreCreateInfo &create_info) const
    {
        const auto resval = m_device.createSemaphore(create_info);
        MH_ASSERT_VK(resval.result, "Failed to create Vulkan semaphore.");
        return resval.value;
    }
    
    vk::ShaderModule Device::create_shader_module(const vk::ShaderModuleCreateInfo &create_info) const
    {
        const auto resval = m_device.createShaderModule(create_info);
        MH_ASSERT_VK(resval.result, "Failed to create Vulkan shader module.");
        return resval.value;
    }
    
    vk::SwapchainKHR Device::create_swapchain(const vk::SwapchainCreateInfoKHR &create_info) const
    {
        const auto resval = m_device.createSwapchainKHR(create_info);
        MH_ASSERT_VK(resval.result, "Failed to create Vulkan swapchain.");
        return resval.value;
    }
    
    void Device::destroy_command_pool(const vk::CommandPool command_pool) const
    {
        m_device.destroyCommandPool(command_pool);
    }
    
    void Device::destroy_fence(const vk::Fence fence) const
    {
        m_device.destroyFence(fence);
    }
    
    void Device::destroy_framebuffer(const vk::Framebuffer framebuffer) const
    {
        m_device.destroyFramebuffer(framebuffer);
    }
    
    void Device::destroy_image_view(const vk::ImageView image_view) const
    {
        m_device.destroyImageView(image_view);
    }
    
    void Device::destroy_pipeline(const vk::Pipeline pipeline) const
    {
        m_device.destroyPipeline(pipeline);
    }
    
    void Device::destroy_pipeline_layout(const vk::PipelineLayout pipeline_layout) const
    {
        m_device.destroyPipelineLayout(pipeline_layout);
    }
    
    void Device::destroy_render_pass(const vk::RenderPass render_pass) const
    {
        m_device.destroyRenderPass(render_pass);
    }
    
    void Device::destroy_semaphore(const vk::Semaphore semaphore) const
    {
        m_device.destroySemaphore(semaphore);
    }
    
    void Device::destroy_shader_module(const vk::ShaderModule shader_module) const
    {
        m_device.destroyShaderModule(shader_module);
    }
    
    void Device::destroy_swapchain(const vk::SwapchainKHR swapchain) const
    {
        m_device.destroySwapchainKHR(swapchain);
    }
    
    vk::Device Device::get_device() const
    {
        return m_device;
    }
    
    vk::Instance Device::get_instance() const
    {
        return m_instance;
    }
    
    vk::PhysicalDevice Device::get_physical_device() const
    {
        return m_physical_device;
    }
    
    vk::SurfaceFormatKHR Device::get_preferred_surface_format() const
    {
        return m_preferred_surface_format;
    }
    
    vk::Queue Device::get_queue(const QueueCapability capability) const
    {
        const auto queue_it = m_queues.find(get_queue_family_index(capability));
        MH_ASSERT(queue_it != m_queues.end(), "Device is missing queue for capability {}.", capability);
        return queue_it->second;
    }
    
    u32 Device::get_queue_family_index(const QueueCapability capability) const
    {
        const auto queue_family_it = m_queue_family_indices.find(capability);
        MH_ASSERT(queue_family_it != m_queue_family_indices.end(), "Device is missing queue family for capability {}.", capability);
        
        return queue_family_it->second;
    }
    
    vk::SurfaceKHR Device::get_surface() const
    {
        return m_surface;
    }
    
    vk::SurfaceCapabilitiesKHR Device::get_surface_capabilities() const
    {
        const auto resval = m_physical_device.getSurfaceCapabilitiesKHR(m_surface);
        MH_ASSERT_VK(resval.result, "Failed to get surface capabilities from Vulkan physical device.");
        return resval.value;
    }
    
    std::vector<vk::SurfaceFormatKHR> Device::get_surface_formats() const
    {
        const auto resval = m_physical_device.getSurfaceFormatsKHR(m_surface);
        MH_ASSERT_VK(resval.result, "Failed to get surface formats from Vulkan physical device.");
        return resval.value;
    }
    
    std::vector<vk::PresentModeKHR> Device::get_surface_present_modes() const
    {
        const auto resval = m_physical_device.getSurfacePresentModesKHR(m_surface);
        MH_ASSERT_VK(resval.result, "Failed to get surface present modes from Vulkan physical device.");
        return resval.value;
    }
    
    std::vector<vk::Image> Device::get_swapchain_images(const vk::SwapchainKHR swapchain) const
    {
        const auto resval = m_device.getSwapchainImagesKHR(swapchain);
        MH_ASSERT_VK(resval.result, "Failed to get Vulkan swapchain images.");
        return resval.value;
    }
    
    std::vector<u32> Device::get_unique_queue_family_indices() const
    {
        const auto graphics_index = get_queue_family_index(QueueCapability::Graphics);
        const auto present_index = get_queue_family_index(QueueCapability::Present);
        
        if (graphics_index != present_index)
        {
            return {graphics_index, present_index};
        }
        else
        {
            return {graphics_index};
        }
    }
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
        const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        const VkDebugUtilsMessageTypeFlagsEXT message_types,
        const VkDebugUtilsMessengerCallbackDataEXT *callback_data_ptr,
        void *user_data_ptr)
    {
        switch (message_severity)
        {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            MH_ERROR(callback_data_ptr->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            MH_WARN(callback_data_ptr->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            MH_INFO(callback_data_ptr->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            MH_TRACE(callback_data_ptr->pMessage);
            break;
        }
        
        return VK_FALSE;
    }
    
    static constexpr vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info
    {
        .flags = {},
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                         | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
                         // | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
                         // | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                     | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                     | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
        .pfnUserCallback = vk_debug_callback,
        .pUserData = {},
    };
    
    void Device::create_instance(const Config &config, const Platform &platform)
    {
        const vk::ApplicationInfo app_info
        {
            .pApplicationName = config.game.name.c_str(),
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = config.engine.name.c_str(),
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_3,
        };
        
        const auto required_extensions = get_required_instance_extensions(platform);
        const auto required_validation_layers = get_required_validation_layers();
        
        vk::InstanceCreateFlagBits flags = {};
        #ifdef __APPLE__
            flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
        #endif
        
        const void *next_ptr = nullptr;
        #ifdef MH_DEBUG_MODE
            next_ptr = &debug_utils_messenger_create_info;
        #endif
        
        const vk::InstanceCreateInfo instance_create_info
        {
            .pNext = next_ptr,
            .flags = flags,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = static_cast<u32>(required_validation_layers.size()),
            .ppEnabledLayerNames = required_validation_layers.data(),
            .enabledExtensionCount = static_cast<u32>(required_extensions.size()),
            .ppEnabledExtensionNames = required_extensions.data(),
        };
        
        const auto instance_resval = vk::createInstance(instance_create_info);
        MH_ASSERT_VK(instance_resval.result, "Failed to create Vulkan instance.");
        m_instance = instance_resval.value;
        
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);
        
        m_surface = platform.create_vulkan_surface(m_instance);
    }
    
    void Device::create_debug_utils_messenger()
    {
        #ifdef MH_DEBUG_MODE
            const auto resval = m_instance.createDebugUtilsMessengerEXT(debug_utils_messenger_create_info);
            MH_ASSERT_VK(resval.result, "Failed to create Vulkan debug utils messenger.");
            m_debug_utils_messenger = resval.value;
        #endif
    }
    
    void Device::choose_physical_device()
    {
        const auto physical_devices_resval = m_instance.enumeratePhysicalDevices();
        MH_ASSERT_VK(physical_devices_resval.result, "Failed to enumerate Vulkan physical devices.");
        const auto physical_devices = physical_devices_resval.value;
        
        MH_ASSERT(!physical_devices.empty(), "Failed to find GPUs with Vulkan support.");
        
        for (const auto &physical_device : physical_devices)
        {
            const auto surface_format_resvals = physical_device.getSurfaceFormatsKHR(m_surface);
            if (surface_format_resvals.value.empty())
            {
                continue;
            }
            
            const auto present_modes = physical_device.getSurfacePresentModesKHR(m_surface);
            if (present_modes.value.empty())
            {
                continue;
            }
            
            const auto queue_families = physical_device.getQueueFamilyProperties();
            std::unordered_map<QueueCapability, u32> queue_family_indices;
            for (auto i = 0; i < queue_families.size(); ++i)
            {
                const auto &queue_family = queue_families[i];
                if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
                {
                    queue_family_indices.try_emplace(QueueCapability::Graphics, i);
                }
                
                VkBool32 present_support = false;
                const auto _ = physical_device.getSurfaceSupportKHR(i, m_surface, &present_support);
                if (present_support)
                {
                    queue_family_indices.try_emplace(QueueCapability::Present, i);
                }
            }
            
            if (queue_family_indices.contains(QueueCapability::Graphics)
                && queue_family_indices.contains(QueueCapability::Present))
            {
                m_queue_family_indices = queue_family_indices;
                m_physical_device = physical_device;
                
                const auto properties = m_physical_device.getProperties();
                MH_INFO("Selected GPU {}.", properties.deviceName);
                
                break;    
            }
        }
        
        MH_ASSERT(m_physical_device, "Failed to find a suitable GPU.");
    }
    
    void Device::create_device()
    {
        std::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos;
        const f32 queue_priority = 1.0f;
        for (const u32 queue_family_index : get_unique_queue_family_indices())
        {
            device_queue_create_infos.push_back(vk::DeviceQueueCreateInfo
            {
                .queueFamilyIndex = queue_family_index,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority,
            });
        }
        
        const vk::PhysicalDeviceFeatures physical_device_features;
        
        const auto required_device_extensions = get_required_device_extensions();
        const auto required_validation_layers = get_required_validation_layers();
        
        const vk::DeviceCreateInfo device_create_info
        {
            .queueCreateInfoCount = static_cast<u32>(device_queue_create_infos.size()),
            .pQueueCreateInfos = device_queue_create_infos.data(),
            .pEnabledFeatures = &physical_device_features,
            .enabledLayerCount = static_cast<u32>(required_validation_layers.size()),
            .ppEnabledLayerNames = required_validation_layers.data(),
            .enabledExtensionCount = static_cast<u32>(required_device_extensions.size()),
            .ppEnabledExtensionNames = required_device_extensions.data(),
        };
        
        const auto device_resval = m_physical_device.createDevice(device_create_info);
        MH_ASSERT_VK(device_resval.result, "Failed to create Vulkan logical device.");
        m_device = device_resval.value;
        
        for (const u32 queue_family_index : get_unique_queue_family_indices())
        {
            vk::Queue queue;
            m_device.getQueue(queue_family_index, 0, &queue);
            m_queues.try_emplace(queue_family_index, queue);
        }
    }
    
    void Device::choose_preferred_surface_format()
    {
        const auto available_formats = get_surface_formats();
        m_preferred_surface_format = available_formats[0];
        
        for (const auto &available_format : available_formats)
        {
            if (available_format.format == vk::Format::eB8G8R8A8Srgb
                && available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                m_preferred_surface_format = available_format;
                break;
            }
        }
    }
    
    // TODO: Validate whether extensions and layers are available.
    std::vector<const char *> Device::get_required_instance_extensions(const Platform &platform)
    {
        auto required_extensions = platform.get_required_vulkan_instance_extensions();
        
        #ifdef __APPLE__
            required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        #endif
        
        #ifdef MH_DEBUG_MODE
            required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        #endif
        
        return required_extensions;
    }
    
    std::vector<const char *> Device::get_required_device_extensions()
    {
        std::vector<const char *> required_extensions
        
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };
        
        #ifdef __APPLE__
            required_extensions.push_back("VK_KHR_portability_subset");
        #endif
        
        return required_extensions;
    }
    
    std::vector<const char *> Device::get_required_validation_layers()
    {
        std::vector<const char *> required_validation_layers;
        
        #ifdef MH_DEBUG_MODE
            required_validation_layers.push_back("VK_LAYER_KHRONOS_validation");
        #endif
        
        return required_validation_layers;
    }
}
