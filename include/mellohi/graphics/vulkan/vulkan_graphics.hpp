#pragma once

#include <set>

#include "mellohi/graphics/graphics.hpp"
#include "mellohi/graphics/vulkan/vulkan.hpp"

namespace mellohi
{
    struct QueueFamilyIndices
    {
        std::optional<u32> graphics_family;
        std::optional<u32> present_family;
        
        QueueFamilyIndices() {}
        QueueFamilyIndices(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);
        
        bool is_complete() const;
        std::set<u32> get_unique_queue_families() const;
    };
    
    class VulkanGraphics final : public Graphics
    {
    public:
        const usize MAX_FRAMES_IN_FLIGHT = 2;
    
        explicit VulkanGraphics(const Config &config, const Platform &platform);
        ~VulkanGraphics() override;
        
        void draw_frame() override;

    private:
        vk::Instance m_instance;
        vk::DebugUtilsMessengerEXT m_debug_utils_messenger;
        vk::SurfaceKHR m_surface;
        vk::PhysicalDevice m_physical_device;
        vk::Device m_device;
        vk::Queue m_graphics_queue;
        vk::Queue m_present_queue;
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
        
        void create_instance(const Config &config, const Platform &platform);
        void create_debug_utils_messenger();
        void choose_physical_device();
        void create_logical_device();
        void create_swapchain(const Platform &platform);
        void create_image_views();
        void create_render_pass();
        void create_graphics_pipeline();
        void create_framebuffers();
        void create_command_pool();
        void create_command_buffers();
        void create_sync_objects();
        void record_command_buffer(u32 image_index);
        
        static std::vector<const char *> get_required_instance_extensions(const Platform &platform);
        static std::vector<const char *> get_required_device_extensions();
        static std::vector<const char *> get_required_validation_layers();
    };
}
