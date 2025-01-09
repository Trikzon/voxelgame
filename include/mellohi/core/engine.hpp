#pragma once

#include <memory>

#include "mellohi/core/assets/asset_manager.hpp"
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
        
        [[nodiscard]] std::shared_ptr<AssetManager> get_asset_manager_ptr() const;
        [[nodiscard]] std::shared_ptr<EngineConfigAsset> get_engine_config_ptr() const;
        [[nodiscard]] std::shared_ptr<Graphics> get_graphics_ptr() const;
        [[nodiscard]] std::shared_ptr<Platform> get_platform_ptr() const;
    
    private:
        std::shared_ptr<AssetManager> m_asset_manager_ptr;
        std::shared_ptr<EngineConfigAsset> m_engine_config_ptr;
        std::shared_ptr<Platform> m_platform_ptr;
        std::shared_ptr<Graphics> m_graphics_ptr;
    };
}
