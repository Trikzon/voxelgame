#pragma once

#include "mellohi/core/assets/toml_asset.hpp"
#include "mellohi/core/color.hpp"

namespace mellohi
{
    class GameConfigAsset : public TomlAsset
    {
    public:
        GameConfigAsset(std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id);
        
        std::string get_game_name() const;
        std::string get_game_package() const;
        
        std::optional<Color> get_window_clear_color_opt() const;
        std::optional<uvec2> get_window_initial_size_opt() const;
        std::optional<bool> get_window_resizable_opt() const;
        std::optional<std::string> get_window_title_opt() const;
        std::optional<bool> get_window_vsync_opt() const;
    
    private:
        struct
        {
            std::string name;
            std::string package;
        } m_game{};
    
        struct
        {
            std::optional<Color> clear_color_opt;
            std::optional<uvec2> initial_size_opt;
            std::optional<bool> resizable_opt;
            std::optional<std::string> title_opt;
            std::optional<bool> vsync_opt;
        } m_window{};
    
        void load() override;
    };
    
    class EngineConfigAsset : public TomlAsset
    {
    public:
        EngineConfigAsset(std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id);
        
        std::string get_engine_name() const;
        std::string get_engine_package() const;
        
        std::string get_game_name() const;
        std::string get_game_package() const;
        
        Color get_window_clear_color() const;
        uvec2 get_window_initial_size() const;
        bool get_window_resizable() const;
        std::string get_window_title() const;
        bool get_window_vsync() const;
        
    private:
        struct
        {
            std::string name;
            std::string package;
        } m_engine{};
        
        struct
        {
            Color clear_color;
            uvec2 initial_size;
            bool resizable;
            std::string title;
            bool vsync;
        } m_window{};
        
        std::shared_ptr<GameConfigAsset> m_game_config;
        
        void load() override;
    };
}
