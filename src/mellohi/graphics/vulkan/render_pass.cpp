#include "mellohi/graphics/vulkan/render_pass.hpp"

namespace mellohi
{
    RenderPass::RenderPass(const std::shared_ptr<EngineConfigAsset> engine_config_ptr,
                           const std::shared_ptr<Device> device_ptr, const std::shared_ptr<Swapchain> swapchain_ptr)
        : m_engine_config_ptr(engine_config_ptr), m_device_ptr(device_ptr), m_swapchain_ptr(swapchain_ptr)
    {
        create_render_pass();
        swapchain_ptr->init_with_render_pass(m_render_pass);
        create_command_pool();
        create_command_buffers();
    }
    
    RenderPass::~RenderPass()
    {
        resolve_frame_ended_callbacks();
        
        m_device_ptr->wait_idle();
        
        m_device_ptr->destroy_command_pool(m_command_pool);
        m_device_ptr->destroy_render_pass(m_render_pass);
    }
    
    bool RenderPass::begin()
    {
        m_current_image_index_opt = m_swapchain_ptr->acquire_next_image_index();
        
        if (!m_current_image_index_opt.has_value())
        {
            return false;
        }
        
        const auto command_buffer = get_current_command_buffer();
        
        command_buffer.reset();
        
        const vk::CommandBufferBeginInfo command_buffer_begin_info
        {
            .flags = {},
            .pInheritanceInfo = nullptr,
        };
        
        const auto result = command_buffer.begin(command_buffer_begin_info);
        MH_ASSERT_VK(result, "Failed to begin recording Vulkan command buffer.");
        
        const vk::ClearValue clear_value
        {
            .color = vk::ClearColorValue
            {
                .float32 = m_engine_config_ptr->get_window_clear_color().srgb_to_linear().as_array()
            },
        };
        
        const auto swapchain_extent = m_swapchain_ptr->get_extent();
        
        const vk::RenderPassBeginInfo render_pass_begin_info
        {
            .renderPass = m_render_pass,
            .framebuffer = m_swapchain_ptr->get_framebuffer(m_current_image_index_opt.value()),
            .renderArea = vk::Rect2D
            {
                .offset = {0, 0},
                .extent = swapchain_extent,
            },
            .clearValueCount = 1,
            .pClearValues = &clear_value,
        };
        
        command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
        
        const vk::Viewport viewport
        {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapchain_extent.width),
            .height = static_cast<float>(swapchain_extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        command_buffer.setViewport(0, 1, &viewport);
        
        const vk::Rect2D scissor
        {
            .offset = {0, 0},
            .extent = swapchain_extent,
        };
        command_buffer.setScissor(0, 1, &scissor);
        
        return true;
    }
    
    void RenderPass::bind_graphics_pipeline(const vk::Pipeline graphics_pipeline)
    {
        get_current_command_buffer().bindPipeline(vk::PipelineBindPoint::eGraphics, graphics_pipeline);
    }
    
    void RenderPass::draw(const u32 vertex_count, const u32 instance_count,
                          const u32 first_vertex, const u32 first_instance)
    {
        get_current_command_buffer().draw(vertex_count, instance_count, first_vertex, first_instance);
    }
    
    void RenderPass::end()
    {
        MH_ASSERT(m_current_image_index_opt.has_value(), "Render Pass must have begun to end it.");
        
        const auto command_buffer = get_current_command_buffer();
        
        command_buffer.endRenderPass();
        
        const auto result = command_buffer.end();
        MH_ASSERT_VK(result, "Failed to end recording Vulkan command buffer.");
        
        m_swapchain_ptr->present(m_current_image_index_opt.value(), command_buffer);
        
        m_current_image_index_opt = std::nullopt;
        
        if (!m_frame_ended_callbacks.empty())
        {
            resolve_frame_ended_callbacks();
        }
    }
    
    void RenderPass::defer_until_frame_ended(const std::function<void()> &callback)
    {
        m_frame_ended_callbacks.push_back(callback);
    }
    
    vk::CommandBuffer RenderPass::get_current_command_buffer() const
    {
        return m_command_buffers[m_swapchain_ptr->get_current_frame_index()];
    }
    
    vk::RenderPass RenderPass::get_render_pass() const
    {
        return m_render_pass;
    }
    
    void RenderPass::create_render_pass()
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
    
    void RenderPass::create_command_pool()
    {
        const vk::CommandPoolCreateInfo command_pool_create_info
        {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = m_device_ptr->get_queue_family_index(QueueCapability::Graphics),
        };
        
        m_command_pool = m_device_ptr->create_command_pool(command_pool_create_info);
    }
    
    void RenderPass::create_command_buffers()
    {
        const vk::CommandBufferAllocateInfo command_buffer_allocate_info
        {
            .commandPool = m_command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = static_cast<u32>(Swapchain::MAX_FRAMES_IN_FLIGHT),
        };
        
        m_command_buffers = m_device_ptr->allocate_command_buffers(command_buffer_allocate_info);
    }
    
    void RenderPass::resolve_frame_ended_callbacks()
    {
        m_device_ptr->wait_idle();
        
        const auto frame_ended_callbacks = m_frame_ended_callbacks;
        m_frame_ended_callbacks.clear();
        for (const auto &callback : frame_ended_callbacks)
        {
            callback();
        }
    }
}
