#pragma once

#include <memory>

#include "mellohi/graphics/graphics.hpp"
#include "mellohi/platform/platform.hpp"

namespace mellohi
{
    class Game
    {
    public:
        virtual ~Game() = default;
        
        virtual void init(class Engine &engine) = 0;
        virtual void process(class Engine &engine) = 0;
    };
    
    class Engine
    {
    public:
        Engine();
        
        void run(Game &game);
        
        [[nodiscard]]
        const Config & get_config() const;
        [[nodiscard]]
        std::shared_ptr<Platform> get_platform_ptr() const;
        [[nodiscard]]
        std::shared_ptr<Graphics> get_graphics_ptr() const;
    
    private:
        Config m_config;
        std::shared_ptr<Platform> m_platform_ptr;
        std::shared_ptr<Graphics> m_graphics_ptr;
    };
}
