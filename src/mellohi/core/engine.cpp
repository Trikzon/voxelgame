#include "mellohi/core/engine.hpp"

namespace mellohi
{
    Engine::Engine()
    {
        m_asset_manager_ptr = std::make_shared<AssetManager>();
        m_engine_config_ptr = m_asset_manager_ptr->load<EngineConfigAsset>(AssetId(":engine.toml"));
        m_platform_ptr = init_platform(m_engine_config_ptr);
        m_graphics_ptr = init_graphics(m_asset_manager_ptr, m_platform_ptr);
    }
    
    void Engine::run(Game &game)
    {
        game.init(*this);
        
        while (!m_platform_ptr->close_requested())
        {
            m_platform_ptr->process_events();
            
            game.process(*this);
            
            m_graphics_ptr->draw_frame();
        }
    }
    
    std::shared_ptr<AssetManager> Engine::get_asset_manager_ptr() const
    {
        return m_asset_manager_ptr;
    }
    
    std::shared_ptr<EngineConfigAsset> Engine::get_engine_config_ptr() const
    {
        return m_engine_config_ptr;
    }
    
    std::shared_ptr<Graphics> Engine::get_graphics_ptr() const
    {
        return m_graphics_ptr;
    }
    
    std::shared_ptr<Platform> Engine::get_platform_ptr() const
    {
        return m_platform_ptr;
    }
}
