#pragma once

#include <memory>

#include "mellohi/core/config.hpp"
#include "mellohi/platform/platform.hpp"

namespace mellohi
{
    class Graphics
    {
    public:
        virtual ~Graphics() = default;
        
        virtual void draw_frame() = 0;
    };
    
    std::shared_ptr<Graphics> init_graphics(const Config &config, const Platform &platform);
}
