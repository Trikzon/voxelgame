#pragma once

#ifndef MH_PLATFORM_GLFW
    #error glfw_platform.hpp can not be included when MH_PLATFORM_GLFW is not defined.
#endif

#include <GLFW/glfw3.h>

#include "mellohi/platform/platform.hpp"

namespace mellohi
{
    class GlfwPlatform final : public Platform
    {
    public:
        explicit GlfwPlatform(std::shared_ptr<EngineConfigAsset> engine_config_ptr);
        ~GlfwPlatform() override;
        
        void process_events() override;
        
        [[nodiscard]]
        bool reload_pressed() const override;
        
        [[nodiscard]]
        bool close_requested() const override;
        [[nodiscard]]
        uvec2 get_framebuffer_size() const override;
        [[nodiscard]]
        std::vector<const char *> get_required_vulkan_instance_extensions() const override;
        #ifdef MH_GRAPHICS_VULKAN
            [[nodiscard]]
            vk::SurfaceKHR create_vulkan_surface(vk::Instance vk_instance) const override;
        #endif
        
    private:
        std::shared_ptr<EngineConfigAsset> m_engine_config_ptr{};
        usize m_engine_config_reloaded_callback_id{};
        GLFWwindow *m_window_ptr{};
        
        void on_engine_config_reloaded();
    };
}
