#include "mellohi/core/assets/config_assets.hpp"

#include <toml++/toml.hpp>

#include "mellohi/core/logger.hpp"

namespace mellohi
{
    GameConfigAsset::GameConfigAsset(const std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id)
        : TomlAsset(asset_manager_ptr, asset_id)
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
        MH_ASSERT(get_id() == AssetId(":game.toml"), "Game config must be at :game.toml, not {}.", get_id());

        auto table = parse_toml_table();

        m_game.name = parse<std::string>(table, "game.name", "string");
        m_game.package = parse<std::string>(table, "game.package", "string");

        m_window.clear_color_opt = parse_opt<Color>(table, "window.clear_color");
        m_window.initial_size_opt = parse_opt<uvec2>(table, "window.initial_size");
        m_window.resizable_opt = parse_opt<bool>(table, "window.resizable");
        m_window.title_opt = parse_opt<std::string>(table, "window.title");
        m_window.vsync_opt = parse_opt<bool>(table, "window.vsync");
    }

    EngineConfigAsset::EngineConfigAsset(const std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id)
        : TomlAsset(asset_manager_ptr, asset_id)
    {
        m_game_config = asset_manager_ptr->load<GameConfigAsset>(AssetId(":game.toml"));

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
        return m_game_config->get_game_name();
    }

    std::string EngineConfigAsset::get_game_package() const
    {
        return m_game_config->get_game_package();
    }

    Color EngineConfigAsset::get_window_clear_color() const
    {
        return m_game_config->get_window_clear_color_opt().value_or(m_window.clear_color);
    }

    uvec2 EngineConfigAsset::get_window_initial_size() const
    {
        return m_game_config->get_window_initial_size_opt().value_or(m_window.initial_size);
    }

    bool EngineConfigAsset::get_window_resizable() const
    {
        return m_game_config->get_window_resizable_opt().value_or(m_window.resizable);
    }

    std::string EngineConfigAsset::get_window_title() const
    {
        return m_game_config->get_window_title_opt().value_or(m_window.title);
    }

    bool EngineConfigAsset::get_window_vsync() const
    {
        return m_game_config->get_window_vsync_opt().value_or(m_window.vsync);
    }

    void EngineConfigAsset::load()
    {
        MH_ASSERT(get_id() == AssetId(":engine.toml"), "Engine config must be at :engine.toml, not {}.", get_id());

        auto table = parse_toml_table();

        m_engine.name = parse<std::string>(table, "engine.name", "string");
        m_engine.package = parse<std::string>(table, "engine.package", "string");

        m_window.clear_color = parse<Color>(table, "window.clear_color", "Color");
        m_window.initial_size = parse<uvec2>(table, "window.initial_size", "uvec2");
        m_window.resizable = parse<bool>(table, "window.resizable", "bool");
        m_window.title = parse<std::string>(table, "window.title", "string");
        m_window.vsync = parse<bool>(table, "window.vsync", "bool");
    }
}
