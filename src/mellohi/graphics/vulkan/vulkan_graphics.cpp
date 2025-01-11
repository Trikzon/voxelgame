#include "mellohi/graphics/vulkan/vulkan_graphics.hpp"

namespace mellohi
{
    VulkanGraphics::VulkanGraphics(const std::shared_ptr<AssetManager> asset_manager_ptr,
                                   const std::shared_ptr<Platform> platform_ptr)
        : m_asset_manager_ptr(asset_manager_ptr)
    {
        const auto engine_config_ptr = asset_manager_ptr->load<EngineConfigAsset>(AssetId(":engine.toml"));
        
        m_device_ptr = std::make_shared<Device>(*engine_config_ptr, *platform_ptr);
        m_swapchain_ptr = std::make_shared<Swapchain>(engine_config_ptr, platform_ptr, m_device_ptr);
        m_render_pass_ptr = std::make_shared<RenderPass>(engine_config_ptr, m_device_ptr, m_swapchain_ptr);
        
        m_triangle_material_ptr = asset_manager_ptr->load<VulkanMaterial>(
            AssetId("sandbox:materials/triangle.toml"), m_device_ptr, m_render_pass_ptr
        );
    }
    
    VulkanGraphics::~VulkanGraphics()
    {
        
    }
    
    void VulkanGraphics::draw_frame()
    {
        if (m_render_pass_ptr->begin())
        {
            m_triangle_material_ptr->bind();
            m_render_pass_ptr->draw(3, 1, 0, 0);
            
            m_render_pass_ptr->end();
        }
    }
}
