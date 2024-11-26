#include "mellohi/core/engine.hpp"

#include "mellohi/core/logger.hpp"

namespace mellohi
{
    Engine::Engine()
    {
        MH_INFO("Game Assets Dir: {}", MH_GAME_ASSETS_DIR);
        MH_INFO("Engine Assets Dir: {}", MH_ENGINE_ASSETS_DIR);
        
        m_platform_ptr = init_platform(m_config);
    }
    
    void Engine::run(Game &game)
    {
        game.init(*this);
        
        while (!m_platform_ptr->close_requested())
        {
            m_platform_ptr->process_events();
            
            game.process(*this);
        }
    }
    
    const Config & Engine::get_config() const
    {
        return m_config;
    }
    
    std::shared_ptr<Platform> Engine::get_platform_ptr() const
    {
        return m_platform_ptr;
    }
}
