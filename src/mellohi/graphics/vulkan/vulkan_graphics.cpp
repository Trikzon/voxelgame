#include "mellohi/graphics/vulkan/vulkan_graphics.hpp"

#include "mellohi/core/asset.hpp"
#include "mellohi/core/logger.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace mellohi
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_types,
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
    
    constexpr vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info
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
    
    QueueFamilyIndices::QueueFamilyIndices(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
    {
        const auto queue_families = physical_device.getQueueFamilyProperties();
        
        for (auto i = 0; i < queue_families.size(); ++i)
        {
            const auto &queue_family = queue_families[i];
            if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                graphics_family = i;
            }
            
            VkBool32 present_support = false;
            const auto _ = physical_device.getSurfaceSupportKHR(i, surface, &present_support);
            if (present_support)
            {
                present_family = i;
            }
        }
    }
    
    bool QueueFamilyIndices::is_complete() const
    {
        return graphics_family.has_value() && present_family.has_value();
    }
    
    std::set<u32> QueueFamilyIndices::get_unique_queue_families() const
    {
        return {graphics_family.value(), present_family.value()};
    }
    
    VulkanGraphics::VulkanGraphics(const Config &config, const Platform &platform)
    {
        VULKAN_HPP_DEFAULT_DISPATCHER.init();
        create_instance(config, platform);
        create_debug_utils_messenger();
        m_surface = platform.create_vulkan_surface(m_instance);
        choose_physical_device();
        create_logical_device();
        create_swapchain(platform);
        create_image_views();
        create_render_pass();
        create_graphics_pipeline();
        create_framebuffers();
        create_command_pool();
        create_command_buffers();
        create_sync_objects();
    }
    
    VulkanGraphics::~VulkanGraphics()
    {
        const auto result = m_device.waitIdle();
        MH_ASSERT(result == vk::Result::eSuccess, "Failed to wait for Vulkan device.");
        
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_device.destroyFence(m_in_flight_fences[i]);
            m_device.destroySemaphore(m_render_finished_semaphores[i]);
            m_device.destroySemaphore(m_image_available_semaphores[i]);
        }
        m_device.destroy(m_command_pool);
        for (const auto &swapchain_framebuffer : m_swapchain_framebuffers)
        {
            m_device.destroy(swapchain_framebuffer);
        }
        m_device.destroy(m_graphics_pipeline);
        m_device.destroy(m_pipeline_layout);
        m_device.destroy(m_render_pass);
        for (const auto &image_view : m_swapchain_image_views)
        {
            m_device.destroyImageView(image_view);
        }
        m_device.destroySwapchainKHR(m_swapchain);
        m_device.destroy();
        
        m_instance.destroySurfaceKHR(m_surface);
        if (m_debug_utils_messenger)
        {
            m_instance.destroyDebugUtilsMessengerEXT(m_debug_utils_messenger);
        }
        m_instance.destroy();
    }
    
    void VulkanGraphics::draw_frame()
    {
        auto result = m_device.waitForFences(1, &m_in_flight_fences[m_current_frame], vk::True,
                                             std::numeric_limits<u64>::max());
        MH_ASSERT(result == vk::Result::eSuccess, "Failed to wait for Vulkan fence.");
        
        result = m_device.resetFences(1, &m_in_flight_fences[m_current_frame]);
        MH_ASSERT(result == vk::Result::eSuccess, "Failed to reset Vulkan fence.");
        
        u32 image_index;
        result = m_device.acquireNextImageKHR(m_swapchain, std::numeric_limits<u64>::max(),
                                              m_image_available_semaphores[m_current_frame], {}, &image_index);
        MH_ASSERT(result == vk::Result::eSuccess, "Failed to acquire Vulkan swapchain image.");
        
        m_command_buffers[m_current_frame].reset();
        record_command_buffer(image_index);
        
        const vk::Semaphore wait_semaphores[] = {m_image_available_semaphores[m_current_frame]};
        const vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        const vk::Semaphore signal_semaphores[] = {m_render_finished_semaphores[m_current_frame]};
        
        const vk::SubmitInfo submit_info
        {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = wait_semaphores,
            .pWaitDstStageMask = wait_stages,
            .commandBufferCount = 1,
            .pCommandBuffers = &m_command_buffers[m_current_frame],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signal_semaphores,
        };
        
        result = m_graphics_queue.submit(1, &submit_info, m_in_flight_fences[m_current_frame]);
        MH_ASSERT(result == vk::Result::eSuccess, "Failed to submit Vulkan queue.");
        
        const vk::PresentInfoKHR present_info
        {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signal_semaphores,
            .swapchainCount = 1,
            .pSwapchains = &m_swapchain,
            .pImageIndices = &image_index,
            .pResults = nullptr,
        };
        
        result = m_present_queue.presentKHR(present_info);
        MH_ASSERT(result == vk::Result::eSuccess, "Failed to present Vulkan queue.");
        
        m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
    
    void VulkanGraphics::create_instance(const Config &config, const Platform &platform)
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
        if (MH_CONTAINS(required_extensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
        {
            flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
        }
        
        const void *next_ptr = nullptr;
        if (MH_CONTAINS(required_extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
        {
            next_ptr = &debug_utils_messenger_create_info;
        }
        
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
        
        const auto instance_result = vk::createInstance(instance_create_info);
        MH_ASSERT_VK(instance_result, "Failed to create Vulkan instance.");
        m_instance = instance_result.value;
        
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);
    }
    
    void VulkanGraphics::create_debug_utils_messenger()
    {
        #ifdef MH_DEBUG_MODE
            const auto result = m_instance.createDebugUtilsMessengerEXT(debug_utils_messenger_create_info);
            MH_ASSERT_VK(result, "Failed to create Vulkan debug utils messenger.");
            m_debug_utils_messenger = result.value;
        #endif
    }
    
    void VulkanGraphics::choose_physical_device()
    {
        const auto physical_devices_result = m_instance.enumeratePhysicalDevices();
        MH_ASSERT_VK(physical_devices_result, "Failed to enumerate Vulkan physical devices.");
        const auto physical_devices = physical_devices_result.value;
        
        MH_ASSERT(!physical_devices.empty(), "Failed to find GPUs with Vulkan support.");
        
        for (const auto &physical_device : physical_devices)
        {
            const QueueFamilyIndices queue_family_indices(physical_device, m_surface);
            
            const auto surface_format_results = physical_device.getSurfaceFormatsKHR(m_surface);
            if (surface_format_results.value.empty())
            {
                continue;
            }
            
            const auto present_modes = physical_device.getSurfacePresentModesKHR(m_surface);
            if (present_modes.value.empty())
            {
                continue;
            }
            
            if (queue_family_indices.is_complete())
            {
                m_physical_device = physical_device;
                
                const auto properties = m_physical_device.getProperties();
                MH_INFO("Selected GPU {}.", properties.deviceName);
                
                break;    
            }
        }
        
        MH_ASSERT(m_physical_device, "Failed to find a suitable GPU.");
    }
    
    void VulkanGraphics::create_logical_device()
    {
        const QueueFamilyIndices queue_family_indices(m_physical_device, m_surface);
        
        std::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos;
        const f32 queue_priority = 1.0f;
        for (const u32 queue_family : queue_family_indices.get_unique_queue_families())
        {
            device_queue_create_infos.push_back(vk::DeviceQueueCreateInfo
            {
                .queueFamilyIndex = queue_family,
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
        
        const auto device_result = m_physical_device.createDevice(device_create_info);
        MH_ASSERT_VK(device_result, "Failed to create Vulkan logical device.");
        m_device = device_result.value;
        
        m_device.getQueue(queue_family_indices.graphics_family.value(), 0, &m_graphics_queue);
        m_device.getQueue(queue_family_indices.present_family.value(), 0, &m_present_queue);
    }
    
    void VulkanGraphics::create_swapchain(const Platform &platform)
    {
        const auto available_surface_formats = m_physical_device.getSurfaceFormatsKHR(m_surface).value;
        auto surface_format = available_surface_formats[0];
        for (const auto &available_surface_format : available_surface_formats)
        {
            if (available_surface_format.format == vk::Format::eB8G8R8A8Srgb
                && available_surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                surface_format = available_surface_format;
                break;
            }
        }
        
        m_swapchain_image_format = surface_format.format;
        
        const auto available_present_modes = m_physical_device.getSurfacePresentModesKHR(m_surface).value;
        auto present_mode = vk::PresentModeKHR::eFifo;
        for (const auto &available_present_mode : available_present_modes)
        {
            if (available_present_mode == vk::PresentModeKHR::eMailbox)
            {
                present_mode = available_present_mode;
                break;
            }
        }
        
        const auto surface_capabilities_result = m_physical_device.getSurfaceCapabilitiesKHR(m_surface);
        MH_ASSERT_VK(surface_capabilities_result, "Failed to get Vulkan surface capabilities from physical device.");
        const auto surface_capabilities = surface_capabilities_result.value;
        
        m_swapchain_extent = surface_capabilities.currentExtent;
        if (surface_capabilities.currentExtent.width == std::numeric_limits<u32>::max())
        {
            const auto framebuffer_size = platform.get_framebuffer_size();
            m_swapchain_extent.width = std::clamp(framebuffer_size.x,
                                                  surface_capabilities.minImageExtent.width,
                                                  surface_capabilities.maxImageExtent.width);
            m_swapchain_extent.height = std::clamp(framebuffer_size.y,
                                                   surface_capabilities.minImageExtent.height,
                                                   surface_capabilities.maxImageExtent.height);
        }
        
        auto image_count = surface_capabilities.minImageCount + 1;
        if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount)
        {
            image_count = surface_capabilities.maxImageCount;
        }
        
        vk::SwapchainCreateInfoKHR swapchain_create_info
        {
            .surface = m_surface,
            .minImageCount = image_count,
            .imageFormat = surface_format.format,
            .imageColorSpace = surface_format.colorSpace,
            .imageExtent = m_swapchain_extent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .preTransform = surface_capabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = present_mode,
            .clipped = vk::True,
        };
        
        const QueueFamilyIndices queue_family_indices(m_physical_device, m_surface);
        const auto unique_queue_families = queue_family_indices.get_unique_queue_families();
        if (unique_queue_families.size() > 1)
        {
            const std::vector<u32> unique_queue_families_v(unique_queue_families.begin(), unique_queue_families.end());
            swapchain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
            swapchain_create_info.queueFamilyIndexCount = unique_queue_families_v.size();
            swapchain_create_info.pQueueFamilyIndices = unique_queue_families_v.data();
        }
        else
        {
            swapchain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
        }
        
        const auto swapchain_result = m_device.createSwapchainKHR(swapchain_create_info);
        MH_ASSERT_VK(swapchain_result, "Failed to create Vulkan swapchain.");
        m_swapchain = swapchain_result.value;
        
    }
    
    void VulkanGraphics::create_image_views()
    {
        const auto swapchain_images_result = m_device.getSwapchainImagesKHR(m_swapchain);
        MH_ASSERT_VK(swapchain_images_result, "Failed to get Vulkan swapchain images.");
        const auto swapchain_images = swapchain_images_result.value;
    
        for (const auto &swapchain_image : swapchain_images)
        {
            const vk::ImageViewCreateInfo image_view_create_info
            {
                .image = swapchain_image,
                .viewType = vk::ImageViewType::e2D,
                .format = m_swapchain_image_format,
                .components = vk::ComponentMapping
                {
                    .r = vk::ComponentSwizzle::eIdentity,
                    .g = vk::ComponentSwizzle::eIdentity,
                    .b = vk::ComponentSwizzle::eIdentity,
                    .a = vk::ComponentSwizzle::eIdentity,
                },
                .subresourceRange = vk::ImageSubresourceRange
                {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };
            
            const auto image_view_result = m_device.createImageView(image_view_create_info);
            MH_ASSERT_VK(image_view_result, "Failed to create Vulkan image view.");
            m_swapchain_image_views.push_back(image_view_result.value);
        }
    }
    
    void VulkanGraphics::create_render_pass()
    {
        const vk::AttachmentDescription color_attachment
        {
            .format = m_swapchain_image_format,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR,
        };
        
        const vk::AttachmentReference color_attachment_ref
        {
            .attachment = 0,
            .layout = vk::ImageLayout::eColorAttachmentOptimal,
        };
        
        const vk::SubpassDescription subpass_description
        {
            .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_ref,
        };
        
        const vk::SubpassDependency subpass_dependency
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .srcAccessMask = {},
            .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        };
        
        const vk::RenderPassCreateInfo render_pass_create_info
        {
            .attachmentCount = 1,
            .pAttachments = &color_attachment,
            .subpassCount = 1,
            .pSubpasses = &subpass_description,
            .dependencyCount = 1,
            .pDependencies = &subpass_dependency,
        };
        
        const auto render_pass_result = m_device.createRenderPass(render_pass_create_info);
        MH_ASSERT_VK(render_pass_result, "Failed to create Vulkan render pass.");
        m_render_pass = render_pass_result.value;
    }
    
    void VulkanGraphics::create_graphics_pipeline()
    {
        const auto vert_shader_code = Asset("sandbox:vert.spv").read_file_as_bytes();
        const auto frag_shader_code = Asset("sandbox:frag.spv").read_file_as_bytes();
        
        const vk::ShaderModuleCreateInfo vert_shader_module_create_info
        {
            .codeSize = vert_shader_code.size(),
            .pCode = reinterpret_cast<const u32 *>(vert_shader_code.data()),
        };
        const auto vert_shader_module_result = m_device.createShaderModule(vert_shader_module_create_info);
        MH_ASSERT_VK(vert_shader_module_result, "Failed to create Vulkan vertex shader module.");
        const auto vert_shader_module = vert_shader_module_result.value;
        
        const vk::ShaderModuleCreateInfo frag_shader_module_create_info
        {
            .codeSize = frag_shader_code.size(),
            .pCode = reinterpret_cast<const u32 *>(frag_shader_code.data()),
        };
        const auto frag_shader_module_result = m_device.createShaderModule(frag_shader_module_create_info);
        MH_ASSERT_VK(frag_shader_module_result, "Failed to create Vulkan fragment shader module.");
        const auto frag_shader_module = frag_shader_module_result.value;
        
        const vk::PipelineShaderStageCreateInfo shader_stages[]
        {
            {
                .stage = vk::ShaderStageFlagBits::eVertex,
                .module = vert_shader_module,
                .pName = "main",
            },
            {
                .stage = vk::ShaderStageFlagBits::eFragment,
                .module = frag_shader_module,
                .pName = "main",
            },
        };
        
        const std::vector dynamic_states
        {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor,
        };
        const vk::PipelineDynamicStateCreateInfo dynamic_state_create_info
        {
            .dynamicStateCount = static_cast<u32>(dynamic_states.size()),
            .pDynamicStates = dynamic_states.data(),
        };
        
        const vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info
        {
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr,
        };
        
        const vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info
        {
            .topology = vk::PrimitiveTopology::eTriangleList,
            .primitiveRestartEnable = vk::False,
        };
        
        const vk::PipelineViewportStateCreateInfo viewport_state_create_info
        {
            .viewportCount = 1,
            .scissorCount = 1,
        };
        
        const vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info
        {
            .depthClampEnable = vk::False,
            .rasterizerDiscardEnable = vk::False,
            .polygonMode = vk::PolygonMode::eFill,
            .lineWidth = 1.0f,
            .cullMode = vk::CullModeFlagBits::eBack,
            .frontFace = vk::FrontFace::eClockwise,
            .depthBiasEnable = vk::False,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
        };
        
        const vk::PipelineMultisampleStateCreateInfo multisample_state_create_info
        {
            .sampleShadingEnable = vk::False,
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = vk::False,
            .alphaToOneEnable = vk::False,
        };
        
        const vk::PipelineColorBlendAttachmentState color_blend_attachment_state
        {
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                            | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
            .blendEnable = vk::False,
            .srcColorBlendFactor = vk::BlendFactor::eOne,
            .dstColorBlendFactor = vk::BlendFactor::eZero,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eZero,
            .alphaBlendOp = vk::BlendOp::eAdd,
        };
        
        const vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info
        {
            .logicOpEnable = vk::False,
            .logicOp = vk::LogicOp::eCopy,
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment_state,
            .blendConstants = {},
        };
        
        const vk::PipelineLayoutCreateInfo pipeline_layout_create_info
        {
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
        };
        
        const auto pipeline_layout_result = m_device.createPipelineLayout(pipeline_layout_create_info);
        MH_ASSERT_VK(pipeline_layout_result, "Failed to create Vulkan pipeline layout.");
        m_pipeline_layout = pipeline_layout_result.value;
        
        const vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info
        {
            .stageCount = 2,
            .pStages = shader_stages,
            .pVertexInputState = &vertex_input_state_create_info,
            .pInputAssemblyState = &input_assembly_state_create_info,
            .pViewportState = &viewport_state_create_info,
            .pRasterizationState = &rasterization_state_create_info,
            .pMultisampleState = &multisample_state_create_info,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &color_blend_state_create_info,
            .pDynamicState = &dynamic_state_create_info,
            .layout = m_pipeline_layout,
            .renderPass = m_render_pass,
            .subpass = 0,
            .basePipelineHandle = nullptr,
            .basePipelineIndex = -1,
        };
        
        const auto graphics_pipeline_result = m_device.createGraphicsPipeline(nullptr, graphics_pipeline_create_info);
        MH_ASSERT_VK(graphics_pipeline_result, "Failed to create Vulkan graphics pipeline.");
        m_graphics_pipeline = graphics_pipeline_result.value;
        
        m_device.destroyShaderModule(frag_shader_module);
        m_device.destroyShaderModule(vert_shader_module);
    }
    
    void VulkanGraphics::create_framebuffers()
    {
        for (const auto &swapchain_image_view : m_swapchain_image_views)
        {
            const vk::FramebufferCreateInfo framebuffer_create_info
            {
                .renderPass = m_render_pass,
                .attachmentCount = 1,
                .pAttachments = &swapchain_image_view,
                .width = m_swapchain_extent.width,
                .height = m_swapchain_extent.height,
                .layers = 1,
            };
            
            const auto framebuffer_result = m_device.createFramebuffer(framebuffer_create_info);
            MH_ASSERT_VK(framebuffer_result, "Failed to create Vulkan framebuffer.");
            m_swapchain_framebuffers.push_back(framebuffer_result.value);
        }
    }
    
    void VulkanGraphics::create_command_pool()
    {
        const QueueFamilyIndices queue_family_indices(m_physical_device, m_surface);
        
        const vk::CommandPoolCreateInfo command_pool_create_info
        {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = queue_family_indices.graphics_family.value(),
        };
        
        const auto command_pool_result = m_device.createCommandPool(command_pool_create_info);
        MH_ASSERT_VK(command_pool_result, "Failed to create Vulkan command pool.");
        m_command_pool = command_pool_result.value;
    }
    
    void VulkanGraphics::create_command_buffers()
    {
        const vk::CommandBufferAllocateInfo command_buffer_allocate_info
        {
            .commandPool = m_command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = static_cast<u32>(MAX_FRAMES_IN_FLIGHT),
        };
        
        const auto command_buffer_result = m_device.allocateCommandBuffers(command_buffer_allocate_info);
        MH_ASSERT_VK(command_buffer_result, "Failed to allocate Vulkan command buffer.");
        m_command_buffers = command_buffer_result.value;
    }
    
    void VulkanGraphics::create_sync_objects()
    {
        const vk::SemaphoreCreateInfo semaphore_create_info;
        const vk::FenceCreateInfo fence_create_info
        {
            .flags = vk::FenceCreateFlagBits::eSignaled,
        };
        
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            auto semaphore_result = m_device.createSemaphore(semaphore_create_info);
            MH_ASSERT_VK(semaphore_result, "Failed to create Vulkan semaphore.");
            m_image_available_semaphores.push_back(semaphore_result.value);
            
            semaphore_result = m_device.createSemaphore(semaphore_create_info);
            MH_ASSERT_VK(semaphore_result, "Failed to create Vulkan semaphore.");
            m_render_finished_semaphores.push_back(semaphore_result.value);
            
            const auto fence_result = m_device.createFence(fence_create_info);
            MH_ASSERT_VK(fence_result, "Failed to create Vulkan fence.");
            m_in_flight_fences.push_back(fence_result.value);
        }
    }
    
    void VulkanGraphics::record_command_buffer(u32 image_index)
    {
        const vk::CommandBufferBeginInfo command_buffer_begin_info
        {
            .flags = {},
            .pInheritanceInfo = nullptr,
        };
        
        const auto begin_result = m_command_buffers[m_current_frame].begin(command_buffer_begin_info);
        MH_ASSERT(begin_result == vk::Result::eSuccess, "Failed to begin recording Vulkan command buffer.");
        
        const vk::ClearValue clear_value
        {
            .color = vk::ClearColorValue
            {
                .float32 = std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f},
            }
        };
        
        const vk::RenderPassBeginInfo render_pass_begin_info
        {
            .renderPass = m_render_pass,
            .framebuffer = m_swapchain_framebuffers[image_index],
            .renderArea = vk::Rect2D
            {
                .offset = {0, 0},
                .extent = m_swapchain_extent,
            },
            .clearValueCount = 1,
            .pClearValues = &clear_value,
        };
        
        m_command_buffers[m_current_frame].beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
        
        m_command_buffers[m_current_frame].bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphics_pipeline);
        
        const vk::Viewport viewport
        {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(m_swapchain_extent.width),
            .height = static_cast<float>(m_swapchain_extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        m_command_buffers[m_current_frame].setViewport(0, 1, &viewport);
        
        const vk::Rect2D scissor
        {
            .offset = {0, 0},
            .extent = m_swapchain_extent,
        };
        m_command_buffers[m_current_frame].setScissor(0, 1, &scissor);
        
        m_command_buffers[m_current_frame].draw(3, 1, 0, 0);
        
        m_command_buffers[m_current_frame].endRenderPass();
        
        const auto end_result = m_command_buffers[m_current_frame].end();
        MH_ASSERT(end_result == vk::Result::eSuccess, "Failed to end recording Vulkan command buffer.");
    }
    
    // TODO: Validate whether extensions and layers are available.
    std::vector<const char *> VulkanGraphics::get_required_instance_extensions(const Platform &platform)
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
    
    std::vector<const char *> VulkanGraphics::get_required_device_extensions()
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
    
    std::vector<const char *> VulkanGraphics::get_required_validation_layers()
    {
        std::vector<const char *> required_validation_layers;
        
        #ifdef MH_DEBUG_MODE
            required_validation_layers.push_back("VK_LAYER_KHRONOS_validation");
        #endif
        
        return required_validation_layers;
    }
}
