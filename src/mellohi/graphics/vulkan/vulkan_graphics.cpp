#include "mellohi/graphics/vulkan/vulkan_graphics.hpp"

#include "mellohi/core/asset.hpp"

namespace mellohi
{
    VulkanGraphics::VulkanGraphics(const Config &config, std::shared_ptr<Platform> platform_ptr)
    {
        m_device_ptr = std::make_shared<Device>(config, *platform_ptr);
        m_swapchain_ptr = std::make_shared<Swapchain>(platform_ptr, m_device_ptr);
        m_render_pass_ptr = std::make_shared<RenderPass>(m_device_ptr, m_swapchain_ptr);
        create_graphics_pipeline();
    }
    
    VulkanGraphics::~VulkanGraphics()
    {
        m_device_ptr->wait_idle();
        
        m_device_ptr->destroy_pipeline(m_graphics_pipeline);
        m_device_ptr->destroy_pipeline_layout(m_pipeline_layout);
    }
    
    void VulkanGraphics::draw_frame()
    {
        if (m_render_pass_ptr->begin())
        {
            m_render_pass_ptr->bind_graphics_pipeline(m_graphics_pipeline);
            m_render_pass_ptr->draw(3, 1, 0, 0);
            
            m_render_pass_ptr->end();
        }
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
            .renderPass = m_render_pass_ptr->get_render_pass(),
            .subpass = 0,
            .basePipelineHandle = nullptr,
            .basePipelineIndex = -1,
        };
        
        m_graphics_pipeline = m_device_ptr->create_graphics_pipeline(graphics_pipeline_create_info);
        
        m_device_ptr->destroy_shader_module(frag_shader_module);
        m_device_ptr->destroy_shader_module(vert_shader_module);
    }
}
