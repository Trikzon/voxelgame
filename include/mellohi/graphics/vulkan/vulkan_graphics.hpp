#pragma once

#include "mellohi/graphics/graphics.hpp"
#include "mellohi/graphics/vulkan/device.hpp"

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
        vk::SwapchainKHR m_swapchain;
        vk::Format m_swapchain_image_format;
        vk::Extent2D m_swapchain_extent;
        std::vector<vk::ImageView> m_swapchain_image_views;
        vk::RenderPass m_render_pass;
        vk::PipelineLayout m_pipeline_layout;
        vk::Pipeline m_graphics_pipeline;
        std::vector<vk::Framebuffer> m_swapchain_framebuffers;
        vk::CommandPool m_command_pool;
        std::vector<vk::CommandBuffer> m_command_buffers;
        std::vector<vk::Semaphore> m_image_available_semaphores;
        std::vector<vk::Semaphore> m_render_finished_semaphores;
        std::vector<vk::Fence> m_in_flight_fences;
        usize m_current_frame = 0;
        
        void create_swapchain(const Platform &platform);
        void create_image_views();
        void create_render_pass();
        void create_graphics_pipeline();
        void create_framebuffers();
        void create_command_pool();
        void create_command_buffers();
        void create_sync_objects();
        void record_command_buffer(u32 image_index);
    };
}
