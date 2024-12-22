#pragma once

#include "mellohi/graphics/vulkan/vulkan.hpp"
#include "mellohi/platform/platform.hpp"

namespace mellohi
{
    enum QueueCapability
    {
        Graphics,
        Present,
    };
    
    class Device
    {
    public:
        Device(const Config &config, const Platform &platform);
        ~Device();
        
        void reset_fence(vk::Fence fence) const;
        void wait_for_fence(vk::Fence fence, u64 timeout = std::numeric_limits<u64>::max()) const;
        void wait_idle() const;
        
        [[nodiscard]] std::vector<vk::CommandBuffer> allocate_command_buffers(
            const vk::CommandBufferAllocateInfo &allocate_info) const;
        [[nodiscard]] vk::CommandPool create_command_pool(const vk::CommandPoolCreateInfo &create_info) const;
        [[nodiscard]] vk::Fence create_fence(const vk::FenceCreateInfo &create_info) const;
        [[nodiscard]] vk::Framebuffer create_framebuffer(const vk::FramebufferCreateInfo &create_info) const;
        [[nodiscard]] vk::Pipeline create_graphics_pipeline(const vk::GraphicsPipelineCreateInfo &create_info) const;
        [[nodiscard]] vk::ImageView create_image_view(const vk::ImageViewCreateInfo &create_info) const;
        [[nodiscard]] vk::PipelineLayout create_pipeline_layout(const vk::PipelineLayoutCreateInfo &create_info) const;
        [[nodiscard]] vk::RenderPass create_render_pass(const vk::RenderPassCreateInfo &create_info) const;
        [[nodiscard]] vk::Semaphore create_semaphore(const vk::SemaphoreCreateInfo &create_info) const;
        [[nodiscard]] vk::ShaderModule create_shader_module(const vk::ShaderModuleCreateInfo &create_info) const;
        [[nodiscard]] vk::SwapchainKHR create_swapchain(const vk::SwapchainCreateInfoKHR &create_info) const;
        
        void destroy_command_pool(vk::CommandPool command_pool) const;
        void destroy_fence(vk::Fence fence) const;
        void destroy_framebuffer(vk::Framebuffer framebuffer) const;
        void destroy_image_view(vk::ImageView image_view) const;
        void destroy_pipeline(vk::Pipeline pipeline) const;
        void destroy_pipeline_layout(vk::PipelineLayout pipeline_layout) const;
        void destroy_render_pass(vk::RenderPass render_pass) const;
        void destroy_semaphore(vk::Semaphore semaphore) const;
        void destroy_shader_module(vk::ShaderModule shader_module) const;
        void destroy_swapchain(vk::SwapchainKHR swapchain) const;
        
        [[nodiscard]] vk::Device get_device() const;
        [[nodiscard]] vk::Instance get_instance() const;
        [[nodiscard]] vk::PhysicalDevice get_physical_device() const;
        [[nodiscard]] vk::Queue get_queue(QueueCapability capability) const;
        [[nodiscard]] u32 get_queue_family_index(QueueCapability capability) const;
        [[nodiscard]] vk::SurfaceKHR get_surface() const;
        [[nodiscard]] vk::SurfaceCapabilitiesKHR get_surface_capabilities() const;
        [[nodiscard]] std::vector<vk::SurfaceFormatKHR> get_surface_formats() const;
        [[nodiscard]] std::vector<vk::PresentModeKHR> get_surface_present_modes() const;
        [[nodiscard]] std::vector<vk::Image> get_swapchain_images(vk::SwapchainKHR swapchain) const;
        [[nodiscard]] std::vector<u32> get_unique_queue_family_indices() const;
    
    private:
        vk::Instance m_instance;
        std::optional<vk::DebugUtilsMessengerEXT> m_debug_utils_messenger;
        vk::SurfaceKHR m_surface;
        vk::PhysicalDevice m_physical_device;
        vk::Device m_device;
        std::unordered_map<QueueCapability, u32> m_queue_family_indices;
        std::unordered_map<u32, vk::Queue> m_queues;
        
        void create_instance(const Config &config, const Platform &platform);
        void create_debug_utils_messenger();
        void choose_physical_device();
        void create_device();
        
        static std::vector<const char *> get_required_instance_extensions(const Platform &platform);
        static std::vector<const char *> get_required_device_extensions();
        static std::vector<const char *> get_required_validation_layers();
    };
};
