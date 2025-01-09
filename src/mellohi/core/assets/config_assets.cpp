#include "mellohi/core/assets/config_assets.hpp"

#include <toml++/toml.hpp>

#include "mellohi/core/logger.hpp"

namespace mellohi
{
    template<typename T>
    static std::optional<T> parse_opt(const toml::table &toml, const std::string &category, const std::string &name)
    {
        return toml[category][name].value<T>();
    }
    
    template<>
    std::optional<Color> parse_opt(const toml::table &toml, const std::string &category, const std::string &name)
    {
        const auto value_array = toml[category][name].as_array();
        if (value_array && value_array->size() >= 3 && value_array->size() <= 4)
        {
            const auto value_r_opt = value_array->at(0).value<f32>();
            const auto value_g_opt = value_array->at(1).value<f32>();
            const auto value_b_opt = value_array->at(2).value<f32>();
            
            if (value_r_opt.has_value() && value_g_opt.has_value() && value_b_opt.has_value())
            {
                const auto value_a_ptr = value_array->get(3);
                
                if (value_a_ptr)
                {
                    const auto value_a_opt = value_a_ptr->value<f32>();
                    
                    if (value_a_opt.has_value())
                    {
                        return Color(value_r_opt.value(), value_g_opt.value(), value_b_opt.value(), value_a_opt.value());
                    }
                }
                
                return Color(value_r_opt.value(), value_g_opt.value(), value_b_opt.value());
            }
        }
        
        const auto value_string_opt = toml[category][name].value<std::string>();
        if (value_string_opt.has_value())
        {
            return Color(value_string_opt.value());
        }
        
        return std::nullopt;
    }
    
    template<>
    std::optional<uvec2> parse_opt(const toml::table &toml, const std::string &category, const std::string &name)
    {
        const auto value_array = toml[category][name].as_array();
        if (value_array && value_array->size() == 2)
        {
            const auto value_x_opt = value_array->at(0).value<u32>();
            const auto value_y_opt = value_array->at(1).value<u32>();
            
            if (value_x_opt.has_value() && value_y_opt.has_value())
            {
                return uvec2(value_x_opt.value(), value_y_opt.value());
            }
        }
        
        return std::nullopt;
    }
    
    template<typename T>
    static T parse(const std::string &config_name, const std::string &type_name, const toml::table &toml,
                   const std::string &category, const std::string &name)
    {
        const auto value_opt = parse_opt<T>(toml, category, name);
        MH_ASSERT(value_opt.has_value(), "{} config is missing {} {}.{}.", config_name, type_name, category, name);
        return value_opt.value();
    }
    
    GameConfigAsset::GameConfigAsset(const AssetId &asset_id) : Asset(asset_id)
    {
        load();
    }
    
    std::string GameConfigAsset::get_game_name() const
    {
        return m_game.name;
    }
    
    std::string GameConfigAsset::get_game_package() const
    {
        return m_game.package;
    }
    
    std::optional<Color> GameConfigAsset::get_window_clear_color_opt() const
    {
        return m_window.clear_color_opt;
    }
    
    std::optional<uvec2> GameConfigAsset::get_window_initial_size_opt() const
    {
        return m_window.initial_size_opt;
    }
    
    std::optional<bool> GameConfigAsset::get_window_resizable_opt() const
    {
        return m_window.resizable_opt;
    }
    
    std::optional<std::string> GameConfigAsset::get_window_title_opt() const
    {
        return m_window.title_opt;
    }
    
    std::optional<bool> GameConfigAsset::get_window_vsync_opt() const
    {
        return m_window.vsync_opt;
    }
    
    void GameConfigAsset::load()
    {
        const auto asset_id = get_id();
        
        MH_ASSERT(asset_id == AssetId(":game.toml"), "Game config must be at :game.toml, not {}.", asset_id);
        
        auto toml = toml::parse(asset_id.read_file_as_string());
        
        m_game.name = parse<std::string>("Game", "string", toml, "game", "name");
        m_game.package = parse<std::string>("Game", "string", toml, "game", "package");
        
        m_window.clear_color_opt = parse_opt<Color>(toml, "window", "clear_color");
        m_window.initial_size_opt = parse_opt<uvec2>(toml, "window", "initial_size");
        m_window.resizable_opt = parse_opt<bool>(toml, "window", "resizable");
        m_window.title_opt = parse_opt<std::string>(toml, "window", "title");
        m_window.vsync_opt = parse_opt<bool>(toml, "window", "vsync");
    }
    
    EngineConfigAsset::EngineConfigAsset(const AssetId &asset_id)
        : Asset(asset_id), m_game_config(AssetId(":game.toml"))
    {
        load();
    }
    
    std::string EngineConfigAsset::get_engine_name() const
    {
        return m_engine.name;
    }
    
    std::string EngineConfigAsset::get_engine_package() const
    {
        return m_engine.package;
    }
    
    std::string EngineConfigAsset::get_game_name() const
    {
        return m_game_config.get_game_name();
    }
    
    std::string EngineConfigAsset::get_game_package() const
    {
        return m_game_config.get_game_package();
    }
    
    Color EngineConfigAsset::get_window_clear_color() const
    {
        return m_game_config.get_window_clear_color_opt().value_or(m_window.clear_color);
    }
    
    uvec2 EngineConfigAsset::get_window_initial_size() const
    {
        return m_game_config.get_window_initial_size_opt().value_or(m_window.initial_size);
    }
    
    bool EngineConfigAsset::get_window_resizable() const
    {
        return m_game_config.get_window_resizable_opt().value_or(m_window.resizable);
    }
    
    std::string EngineConfigAsset::get_window_title() const
    {
        return m_game_config.get_window_title_opt().value_or(m_window.title);
    }
    
    bool EngineConfigAsset::get_window_vsync() const
    {
        return m_game_config.get_window_vsync_opt().value_or(m_window.vsync);
    }
    
    void EngineConfigAsset::load()
    {
        m_game_config.reload();
        
        const auto asset_id = get_id();
        
        MH_ASSERT(asset_id == AssetId(":engine.toml"), "Engine config must be at :engine.toml, not {}.", asset_id);
        
        auto toml = toml::parse(asset_id.read_file_as_string());
        
        m_engine.name = parse<std::string>("Engine", "string", toml, "engine", "name");
        m_engine.package = parse<std::string>("Engine", "string", toml, "engine", "package");
        
        m_window.clear_color = parse<Color>("Engine", "Color", toml, "window", "clear_color");
        m_window.initial_size = parse<uvec2>("Engine", "uvec2", toml, "window", "initial_size");
        m_window.resizable = parse<bool>("Engine", "bool", toml, "window", "resizable");
        m_window.title = parse<std::string>("Engine", "string", toml, "window", "title");
        m_window.vsync = parse<bool>("Engine", "bool", toml, "window", "vsync");
    }
}
