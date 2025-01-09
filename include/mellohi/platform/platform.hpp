#pragma once

#include <memory>

#include "mellohi/core/assets/config_assets.hpp"
#ifdef MH_GRAPHICS_VULKAN
    #include "mellohi/graphics/vulkan/vulkan.hpp"
#endif

namespace mellohi
{
    class Platform
    {
    public:
        virtual ~Platform() = default;
        
        virtual void process_events() = 0;
        
        [[nodiscard]]
        virtual bool close_requested() const = 0;
        [[nodiscard]]
        virtual uvec2 get_framebuffer_size() const = 0;
        [[nodiscard]]
        virtual std::vector<const char *> get_required_vulkan_instance_extensions() const = 0;
        #ifdef MH_GRAPHICS_VULKAN
            [[nodiscard]]
            virtual vk::SurfaceKHR create_vulkan_surface(vk::Instance vk_instance) const = 0;
        #endif
    };
    
    std::shared_ptr<Platform> init_platform(std::shared_ptr<EngineConfigAsset> engine_config_ptr);
}
