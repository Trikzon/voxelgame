#pragma once

#include <memory>

#include "mellohi/core/assets/asset_manager.hpp"
#include "mellohi/platform/platform.hpp"

namespace mellohi
{
    class Graphics
    {
    public:
        virtual ~Graphics() = default;
        
        virtual void draw_frame() = 0;
    };
    
    std::shared_ptr<Graphics> init_graphics(std::shared_ptr<AssetManager> asset_manager_ptr, 
                                            std::shared_ptr<Platform> platform_ptr);
}
