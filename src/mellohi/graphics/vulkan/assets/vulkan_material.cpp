#include "mellohi/graphics/vulkan/assets/vulkan_material.hpp"

namespace mellohi
{
    VulkanMaterial::VulkanMaterial(const std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id,
                                   const std::shared_ptr<Device> device_ptr,
                                   const std::shared_ptr<RenderPass> render_pass_ptr)
        : Material(asset_manager_ptr, asset_id), m_device_ptr(device_ptr), m_render_pass_ptr(render_pass_ptr)
    {
        load();
    }
    
    VulkanMaterial::~VulkanMaterial()
    {
        m_frag_shader_ptr->deregister_reload_callback(m_on_frag_shader_reloaded_id);
        m_vert_shader_ptr->deregister_reload_callback(m_on_vert_shader_reloaded_id);
        
        m_device_ptr->push_to_deletion_queue(
            std::bind(&Device::destroy_pipeline, m_device_ptr, m_graphics_pipeline)
        );
    }
    
    void VulkanMaterial::bind()
    {
        if (m_should_be_recreated)
        {
            reload();
        }
        
        m_render_pass_ptr->bind_graphics_pipeline(m_graphics_pipeline);
    }
    
    void VulkanMaterial::load()
    {
        m_should_be_recreated = false;
        
        if (m_graphics_pipeline)
        {
            m_device_ptr->push_to_deletion_queue(
                std::bind(&Device::destroy_pipeline, m_device_ptr, m_graphics_pipeline)
            );
        }
        
        const auto table = parse_toml_table();
        
        const auto vert_shader_id = parse<AssetId>(table, "vert_shader", "AssetId");
        const auto frag_shader_id = parse<AssetId>(table, "frag_shader", "AssetId");
        
        auto &asset_manager = get_asset_manager();
        
        if (!m_vert_shader_ptr || m_vert_shader_ptr->get_id() != vert_shader_id)
        {
            if (m_vert_shader_ptr)
            {
                m_vert_shader_ptr->deregister_reload_callback(m_on_vert_shader_reloaded_id);
            }
            
            m_vert_shader_ptr = asset_manager.load<VulkanShader>(vert_shader_id, m_device_ptr);
            
            m_on_vert_shader_reloaded_id = m_vert_shader_ptr->register_reload_callback(
                std::bind(&VulkanMaterial::on_shader_reloaded, this)
            );
        }
        
        if (!m_frag_shader_ptr || m_frag_shader_ptr->get_id() != frag_shader_id)
        {
            if (m_frag_shader_ptr)
            {
                m_frag_shader_ptr->deregister_reload_callback(m_on_frag_shader_reloaded_id);
            }
            
            m_frag_shader_ptr = asset_manager.load<VulkanShader>(frag_shader_id, m_device_ptr);
            
            m_on_frag_shader_reloaded_id = m_frag_shader_ptr->register_reload_callback(
                std::bind(&VulkanMaterial::on_shader_reloaded, this)
            );
        }
        
        const vk::PipelineShaderStageCreateInfo shader_stages[]
        {
            {
                .stage = vk::ShaderStageFlagBits::eVertex,
                .module = m_vert_shader_ptr->get_shader_module(),
                .pName = "main",
            },
            {
                .stage = vk::ShaderStageFlagBits::eFragment,
                .module = m_frag_shader_ptr->get_shader_module(),
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
        
        const auto pipeline_layout = m_device_ptr->create_pipeline_layout(pipeline_layout_create_info);
        
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
            .layout = pipeline_layout,
            .renderPass = m_render_pass_ptr->get_render_pass(),
            .subpass = 0,
            .basePipelineHandle = nullptr,
            .basePipelineIndex = -1,
        };
        
        m_graphics_pipeline = m_device_ptr->create_graphics_pipeline(graphics_pipeline_create_info);
        
        m_device_ptr->destroy_pipeline_layout(pipeline_layout);
    }
    
    void VulkanMaterial::on_shader_reloaded()
    {
        m_should_be_recreated = true;
    }
}
