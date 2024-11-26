#pragma once

#ifndef MH_PLATFORM_GLFW
    #error glfw_platform.hpp can not be included when MH_PLATFORM_GLFW is not defined.
#endif

#include <GLFW/glfw3.h>

#include "mellohi/platform/platform.hpp"

namespace mellohi
{
    class GLFWPlatform final : public Platform
    {
    public:
        explicit GLFWPlatform(const Config &config);
        ~GLFWPlatform() override;
        
        void process_events() override;
        
        [[nodiscard]]
        bool close_requested() const override;
        [[nodiscard]]
        std::vector<const char *> get_required_vulkan_instance_extensions() const override;
        
    private:
        GLFWwindow *m_window_ptr = nullptr;
    };
}
