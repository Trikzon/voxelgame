#include "mellohi/graphics/vulkan/assets/vulkan_shader.hpp"

#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>

namespace mellohi
{
    VulkanShader::VulkanShader(std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id,
                 std::shared_ptr<Device> device_ptr)
        : Shader(asset_manager_ptr, asset_id), m_device_ptr(device_ptr)
    {
        load();
    }
    
    VulkanShader::~VulkanShader()
    {
        m_device_ptr->push_to_deletion_queue(
            std::bind(&Device::destroy_shader_module, m_device_ptr, m_shader_module)
        );
    }
    
    vk::ShaderModule VulkanShader::get_shader_module() const
    {
        return m_shader_module;
    }
    
    void VulkanShader::load()
    {
        if (m_shader_module)
        {
            m_device_ptr->push_to_deletion_queue(
                std::bind(&Device::destroy_shader_module, m_device_ptr, m_shader_module)
            );
        }
        
        const auto shader_src_code = get_id().read_file_as_string();
        
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        
        // TODO: Infer from file extension instead.
        const bool vert = get_id().get_fully_qualified_path().contains("vert");
        
        const auto result = compiler.CompileGlslToSpv(shader_src_code, 
                                                      vert ? shaderc_vertex_shader : shaderc_fragment_shader,
                                                      get_id().get_fully_qualified_path().c_str(),
                                                      options);
        
        MH_ASSERT(result.GetCompilationStatus() == shaderc_compilation_status_success,
                  "Shader {} compilation failed: {}", get_id(), result.GetErrorMessage());
        
        const std::vector<u32> shader_spirv_code(result.cbegin(), result.cend());
        
        const vk::ShaderModuleCreateInfo shader_module_create_info
        {
            .codeSize = shader_spirv_code.size() * sizeof(u32),
            .pCode = shader_spirv_code.data(),
        };
        m_shader_module = m_device_ptr->create_shader_module(shader_module_create_info);
    }
}
