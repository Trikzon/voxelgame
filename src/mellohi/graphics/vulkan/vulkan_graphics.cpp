#include "mellohi/graphics/vulkan/vulkan_graphics.hpp"

#include "mellohi/core/asset.hpp"
#include "mellohi/core/logger.hpp"

namespace mellohi
{
    VulkanGraphics::VulkanGraphics(const Config &config, std::shared_ptr<Platform> platform_ptr)
    {
        m_device_ptr = std::make_shared<Device>(config, *platform_ptr);
        create_render_pass();
        m_swapchain_ptr = std::make_shared<Swapchain>(platform_ptr, m_device_ptr, m_render_pass);
        create_graphics_pipeline();
        create_command_pool();
        create_command_buffers();
    }
    
    VulkanGraphics::~VulkanGraphics()
    {
        m_device_ptr->wait_idle();
        
        m_device_ptr->destroy_command_pool(m_command_pool);
        m_device_ptr->destroy_pipeline(m_graphics_pipeline);
        m_device_ptr->destroy_pipeline_layout(m_pipeline_layout);
        m_device_ptr->destroy_render_pass(m_render_pass);
    }
    
    void VulkanGraphics::draw_frame()
    {
        auto image_index = m_swapchain_ptr->acquire_next_image_index();
        if (!image_index.has_value())
        {
            // The swapchain is out of date and had to be recreated.
            return;
        }
        
        m_command_buffers[m_swapchain_ptr->get_current_frame_index()].reset();
        
        record_command_buffer(image_index.value());
        
        m_swapchain_ptr->present(image_index.value(), m_command_buffers[m_swapchain_ptr->get_current_frame_index()]);
    }
    
    void VulkanGraphics::create_render_pass()
    {
        const auto preferred_surface_format = m_device_ptr->get_preferred_surface_format();
        
        const vk::AttachmentDescription color_attachment
        {
            .format = preferred_surface_format.format,
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
    
    void VulkanGraphics::record_command_buffer(u32 image_index)
    {
        const auto current_command_buffer = m_command_buffers[m_swapchain_ptr->get_current_frame_index()];
        
        const vk::CommandBufferBeginInfo command_buffer_begin_info
        {
            .flags = {},
            .pInheritanceInfo = nullptr,
        };
        
        const auto begin_result = current_command_buffer.begin(command_buffer_begin_info);
        MH_ASSERT(begin_result == vk::Result::eSuccess, "Failed to begin recording Vulkan command buffer.");
        
        const vk::ClearValue clear_value
        {
            .color = vk::ClearColorValue
            {
                .float32 = std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f},
            }
        };
        
        const auto swapchain_extent = m_swapchain_ptr->get_extent();
        
        const vk::RenderPassBeginInfo render_pass_begin_info
        {
            .renderPass = m_render_pass,
            .framebuffer = m_swapchain_ptr->get_framebuffer(image_index),
            .renderArea = vk::Rect2D
            {
                .offset = {0, 0},
                .extent = swapchain_extent,
            },
            .clearValueCount = 1,
            .pClearValues = &clear_value,
        };
        
        current_command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
        
        current_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphics_pipeline);
        
        const vk::Viewport viewport
        {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapchain_extent.width),
            .height = static_cast<float>(swapchain_extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        current_command_buffer.setViewport(0, 1, &viewport);
        
        const vk::Rect2D scissor
        {
            .offset = {0, 0},
            .extent = swapchain_extent,
        };
        current_command_buffer.setScissor(0, 1, &scissor);
        
        current_command_buffer.draw(3, 1, 0, 0);
        
        current_command_buffer.endRenderPass();
        
        const auto end_result = current_command_buffer.end();
        MH_ASSERT(end_result == vk::Result::eSuccess, "Failed to end recording Vulkan command buffer.");
    }
}
