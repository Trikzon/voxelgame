#pragma once

#include "mellohi/graphics/vulkan/swapchain.hpp"

namespace mellohi
{
    class RenderPass
    {
    public:
        RenderPass(std::shared_ptr<EngineConfigAsset> engine_config_ptr, std::shared_ptr<Device> device_ptr,
                   std::shared_ptr<Swapchain> swapchain_ptr);
        ~RenderPass();
        
        [[nodiscard]] bool begin();
        void bind_graphics_pipeline(vk::Pipeline graphics_pipeline);
        void draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance);
        void end();
        
        [[nodiscard]] vk::CommandBuffer get_current_command_buffer() const;
        [[nodiscard]] vk::RenderPass get_render_pass() const;
        
    private:
        std::shared_ptr<EngineConfigAsset> m_engine_config_ptr;
        std::shared_ptr<Device> m_device_ptr;
        std::shared_ptr<Swapchain> m_swapchain_ptr;
    
        vk::RenderPass m_render_pass;
        vk::CommandPool m_command_pool;
        std::vector<vk::CommandBuffer> m_command_buffers;
        std::optional<u32> m_current_image_index_opt;
        
        void create_render_pass();
        void create_command_pool();
        void create_command_buffers();
    };
}
