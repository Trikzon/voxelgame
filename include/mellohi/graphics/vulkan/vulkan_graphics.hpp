#pragma once

#include "mellohi/graphics/graphics.hpp"
#include "mellohi/graphics/vulkan/assets/vulkan_material.hpp"

namespace mellohi
{
    class VulkanGraphics final : public Graphics
    {
    public:
        VulkanGraphics(std::shared_ptr<AssetManager> asset_manager_ptr, std::shared_ptr<Platform> platform_ptr);
        ~VulkanGraphics() override;
        
        void draw_frame() override;

    private:
        std::shared_ptr<AssetManager> m_asset_manager_ptr;
        std::shared_ptr<Device> m_device_ptr;
        std::shared_ptr<Swapchain> m_swapchain_ptr;
        std::shared_ptr<RenderPass> m_render_pass_ptr;
        std::shared_ptr<VulkanMaterial> m_triangle_material_ptr;
        
        void create_graphics_pipeline();
    };
}
