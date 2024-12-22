#include "mellohi/graphics/vulkan/vulkan_graphics.hpp"

#include "mellohi/core/asset.hpp"
#include "mellohi/core/logger.hpp"

namespace mellohi
{
    VulkanGraphics::VulkanGraphics(const Config &config, std::shared_ptr<Platform> platform_ptr)
    {
        m_device_ptr = std::make_shared<Device>(config, *platform_ptr);
        create_swapchain(*platform_ptr);
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
        m_device_ptr->wait_idle();
        
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_device_ptr->destroy_fence(m_in_flight_fences[i]);
            m_device_ptr->destroy_semaphore(m_render_finished_semaphores[i]);
            m_device_ptr->destroy_semaphore(m_image_available_semaphores[i]);
        }
        m_device_ptr->destroy_command_pool(m_command_pool);
        for (const auto &swapchain_framebuffer : m_swapchain_framebuffers)
        {
            m_device_ptr->destroy_framebuffer(swapchain_framebuffer);
        }
        m_device_ptr->destroy_pipeline(m_graphics_pipeline);
        m_device_ptr->destroy_pipeline_layout(m_pipeline_layout);
        m_device_ptr->destroy_render_pass(m_render_pass);
        for (const auto &image_view : m_swapchain_image_views)
        {
            m_device_ptr->destroy_image_view(image_view);
        }
        m_device_ptr->destroy_swapchain(m_swapchain);
    }
    
    void VulkanGraphics::draw_frame()
    {
        m_device_ptr->wait_for_fence(m_in_flight_fences[m_current_frame]);
        m_device_ptr->reset_fence(m_in_flight_fences[m_current_frame]);
        
        u32 image_index;
        auto result = m_device_ptr->get_device().acquireNextImageKHR(m_swapchain, std::numeric_limits<u64>::max(),
                                                                m_image_available_semaphores[m_current_frame], {},
                                                                &image_index);
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
        
        result = m_device_ptr->get_queue(QueueCapability::Graphics)
            .submit(1, &submit_info, m_in_flight_fences[m_current_frame]);
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
        
        result = m_device_ptr->get_queue(QueueCapability::Present).presentKHR(present_info);
        MH_ASSERT(result == vk::Result::eSuccess, "Failed to present Vulkan queue.");
        
        m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
    
    void VulkanGraphics::create_swapchain(const Platform &platform)
    {
        const auto available_surface_formats = m_device_ptr->get_surface_formats();
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
        
        const auto available_present_modes = m_device_ptr->get_surface_present_modes();
        auto present_mode = vk::PresentModeKHR::eFifo;
        for (const auto &available_present_mode : available_present_modes)
        {
            if (available_present_mode == vk::PresentModeKHR::eMailbox)
            {
                present_mode = available_present_mode;
                break;
            }
        }
        
        const auto surface_capabilities = m_device_ptr->get_surface_capabilities();
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
            .surface = m_device_ptr->get_surface(),
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
        
        const auto unique_queue_families = m_device_ptr->get_unique_queue_family_indices();
        if (unique_queue_families.size() > 1)
        {
            swapchain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
            swapchain_create_info.queueFamilyIndexCount = unique_queue_families.size();
            swapchain_create_info.pQueueFamilyIndices = unique_queue_families.data();
        }
        else
        {
            swapchain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
        }
        
        m_swapchain = m_device_ptr->create_swapchain(swapchain_create_info);
    }
    
    void VulkanGraphics::create_image_views()
    {
        const auto swapchain_images = m_device_ptr->get_swapchain_images(m_swapchain);
    
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
            
            m_swapchain_image_views.push_back(m_device_ptr->create_image_view(image_view_create_info));
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
        
        m_render_pass = m_device_ptr->create_render_pass(render_pass_create_info);
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
        const auto vert_shader_module = m_device_ptr->create_shader_module(vert_shader_module_create_info);
        
        const vk::ShaderModuleCreateInfo frag_shader_module_create_info
        {
            .codeSize = frag_shader_code.size(),
            .pCode = reinterpret_cast<const u32 *>(frag_shader_code.data()),
        };
        const auto frag_shader_module = m_device_ptr->create_shader_module(frag_shader_module_create_info);
        
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
        
        m_pipeline_layout = m_device_ptr->create_pipeline_layout(pipeline_layout_create_info);
        
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
        
        m_graphics_pipeline = m_device_ptr->create_graphics_pipeline(graphics_pipeline_create_info);
        
        m_device_ptr->destroy_shader_module(frag_shader_module);
        m_device_ptr->destroy_shader_module(vert_shader_module);
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
            
            m_swapchain_framebuffers.push_back(m_device_ptr->create_framebuffer(framebuffer_create_info));
        }
    }
    
    void VulkanGraphics::create_command_pool()
    {
        const vk::CommandPoolCreateInfo command_pool_create_info
        {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = m_device_ptr->get_queue_family_index(QueueCapability::Graphics),
        };
        
        m_command_pool = m_device_ptr->create_command_pool(command_pool_create_info);
    }
    
    void VulkanGraphics::create_command_buffers()
    {
        const vk::CommandBufferAllocateInfo command_buffer_allocate_info
        {
            .commandPool = m_command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = static_cast<u32>(MAX_FRAMES_IN_FLIGHT),
        };
        
        m_command_buffers = m_device_ptr->allocate_command_buffers(command_buffer_allocate_info);
    }
    
    void VulkanGraphics::create_sync_objects()
    {
        const vk::FenceCreateInfo fence_create_info
        {
            .flags = vk::FenceCreateFlagBits::eSignaled,
        };
        
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_image_available_semaphores.push_back(m_device_ptr->create_semaphore({}));
            m_render_finished_semaphores.push_back(m_device_ptr->create_semaphore({}));
            m_in_flight_fences.push_back(m_device_ptr->create_fence(fence_create_info));
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
}
