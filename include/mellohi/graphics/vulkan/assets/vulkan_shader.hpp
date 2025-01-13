#pragma once

#include "mellohi/graphics/assets/shader.hpp"
#include "mellohi/graphics/vulkan/device.hpp"

namespace mellohi
{
    class VulkanShader : public Shader
    {
    public:
        VulkanShader(std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id,
                     std::shared_ptr<Device> device_ptr);
        virtual ~VulkanShader() override;
        
        vk::ShaderModule get_shader_module() const;
        
    private:
        std::shared_ptr<Device> m_device_ptr;
        
        vk::ShaderModule m_shader_module;
        
        void load() override;
    };
}
