#pragma once

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
    
    private:
        Config m_config;
        std::shared_ptr<Platform> m_platform_ptr;
    };
}
