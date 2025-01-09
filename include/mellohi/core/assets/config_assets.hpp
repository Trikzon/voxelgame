#pragma once

#include "mellohi/core/assets/asset.hpp"

namespace mellohi
{
    class GameConfigAsset : public Asset
    {
    public:
        explicit GameConfigAsset(const AssetId &asset_id);
        
        std::string get_game_name() const;
        std::string get_game_package() const;
        
        std::optional<std::string> get_window_title_opt() const;
        std::optional<uvec2> get_window_initial_size_opt() const;
    
    private:
        struct
        {
            std::string name;
            std::string package;
        } m_game;
    
        struct
        {
            std::optional<std::string> title_opt;
            std::optional<uvec2> initial_size_opt;
        } m_window;
    
        void load() override;
    };
    
    class EngineConfigAsset : public Asset
    {
    public:
        explicit EngineConfigAsset(const AssetId &asset_id);
        
        std::string get_engine_name() const;
        std::string get_engine_package() const;
        
        std::string get_game_name() const;
        std::string get_game_package() const;
        
        std::string get_window_title() const;
        uvec2 get_window_initial_size() const;
        
    private:
        struct
        {
            std::string name;
            std::string package;
        } m_engine;
        
        struct
        {
            std::string title;
            uvec2 initial_size;
        } m_window;
        
        GameConfigAsset m_game_config;
        
        void load() override;
    };
}
