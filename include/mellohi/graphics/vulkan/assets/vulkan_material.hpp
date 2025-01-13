#pragma once

#include "mellohi/graphics/assets/material.hpp"
#include "mellohi/graphics/vulkan/assets/vulkan_shader.hpp"
#include "mellohi/graphics/vulkan/render_pass.hpp"

namespace mellohi
{
    class VulkanMaterial : public Material
    {
    public:
        VulkanMaterial(std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id,
                       std::shared_ptr<Device> device_ptr, std::shared_ptr<RenderPass> render_pass_ptr);
        virtual ~VulkanMaterial() override;
        
        void bind();
        
    private:
        std::shared_ptr<Device> m_device_ptr;
        std::shared_ptr<RenderPass> m_render_pass_ptr;
        
        std::shared_ptr<VulkanShader> m_vert_shader_ptr;
        std::shared_ptr<VulkanShader> m_frag_shader_ptr;
        usize m_on_vert_shader_reloaded_id, m_on_frag_shader_reloaded_id;
        bool m_should_be_recreated;
        
        vk::Pipeline m_graphics_pipeline;
        
        void load() override;
        
        void on_shader_reloaded();
    };
}
