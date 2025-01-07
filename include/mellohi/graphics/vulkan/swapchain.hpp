#pragma once

#include "mellohi/graphics/vulkan/device.hpp"

namespace mellohi
{
    class Swapchain
    {
    public:
        const static usize MAX_FRAMES_IN_FLIGHT = 2;
    
        Swapchain(std::shared_ptr<Platform> platform_ptr, std::shared_ptr<Device> device_ptr);
        ~Swapchain();
        
        void init_with_render_pass(vk::RenderPass render_pass);
        
        [[nodiscard]] std::optional<u32> acquire_next_image_index();
        void present(u32 image_index, vk::CommandBuffer command_buffer);
        
        [[nodiscard]] usize get_current_frame_index() const;
        [[nodiscard]] vk::Extent2D get_extent() const;
        [[nodiscard]] vk::SwapchainKHR get_swapchain() const;
        [[nodiscard]] const std::vector<vk::ImageView> & get_image_views() const;
        [[nodiscard]] vk::Framebuffer get_framebuffer(u32 image_index) const;

    private:
        std::shared_ptr<Platform> m_platform_ptr;
        std::shared_ptr<Device> m_device_ptr;
        vk::RenderPass m_render_pass;

        vk::SwapchainKHR m_swapchain;
        vk::Extent2D m_extent;
        std::vector<vk::ImageView> m_image_views;
        std::vector<vk::Framebuffer> m_framebuffers;
        
        std::vector<vk::Semaphore> m_image_available_semaphores;
        std::vector<vk::Semaphore> m_render_finished_semaphores;
        std::vector<vk::Fence> m_in_flight_fences;
        usize m_current_frame_index = 0;

        void create_swapchain();
        void create_image_views();
        void create_framebuffers();
        void create_sync_objects();
        
        void recreate();
        void destroy();
    };
}
