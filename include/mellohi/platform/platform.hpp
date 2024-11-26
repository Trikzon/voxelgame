#pragma once

#include <memory>

#include "mellohi/core/config.hpp"

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
        virtual std::vector<const char *> get_required_vulkan_instance_extensions() const = 0;
    };
    
    std::shared_ptr<Platform> init_platform(const Config &config);
}
