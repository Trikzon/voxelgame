#pragma once

#include "mellohi/graphics/graphics.hpp"
#include "mellohi/graphics/vulkan/swapchain.hpp"

namespace mellohi
{
    class VulkanGraphics final : public Graphics
    {
    public:
        const usize MAX_FRAMES_IN_FLIGHT = 2;
    
        explicit VulkanGraphics(const Config &config, std::shared_ptr<Platform> platform_ptr);
        ~VulkanGraphics() override;
        
        void draw_frame() override;

    private:
        std::shared_ptr<Device> m_device_ptr;
        std::shared_ptr<Swapchain> m_swapchain_ptr;
        vk::RenderPass m_render_pass;
        vk::PipelineLayout m_pipeline_layout;
        vk::Pipeline m_graphics_pipeline;
        vk::CommandPool m_command_pool;
        std::vector<vk::CommandBuffer> m_command_buffers;
        
        void create_render_pass();
        void create_graphics_pipeline();
        void create_command_pool();
        void create_command_buffers();
        void record_command_buffer(u32 image_index);
    };
}
