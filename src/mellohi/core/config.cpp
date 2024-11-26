#include "mellohi/core/config.hpp"

#include <toml++/toml.hpp>

#include "mellohi/core/asset.hpp"
#include "mellohi/core/logger.hpp"

namespace mellohi
{
    Config::Config()
    {
        auto engine_config = toml::parse(Asset(":engine.toml").read_file_as_string());
        auto game_config = toml::parse(Asset(":game.toml").read_file_as_string());
        
        engine.name = engine_config["engine"]["name"].value_or<std::string>("");
        MH_ASSERT(!engine.name.empty(), "Engine config is missing string engine.name.");
        
        engine.package = engine_config["engine"]["package"].value_or<std::string>("");
        MH_ASSERT(!engine.package.empty(), "Engine config is missing string engine.package.");
        
        game.name = game_config["game"]["name"].value_or<std::string>("");
        MH_ASSERT(!game.name.empty(), "Game config is missing string game.name.");
        
        game.package = game_config["game"]["package"].value_or<std::string>("");
        MH_ASSERT(!game.package.empty(), "Game config is missing string game.package.");
        
        window.title = game_config["window"]["title"].value_or<std::string>("");
        if (window.title.empty())
        {
            window.title = engine_config["window"]["title"].value_or<std::string>("");
            MH_ASSERT(!window.title.empty(), "Game and engine configs are missing string window.title.");
        }
        
        window.width = game_config["window"]["width"].value_or<u32>(0);
        if (window.width == 0)
        {
            window.width = engine_config["window"]["width"].value_or<u32>(0);
            MH_ASSERT(window.width != 0, "Game and engine configs are missing u32 window.width.");
        }
        
        window.height = game_config["window"]["height"].value_or<u32>(0);
        if (window.height == 0)
        {
            window.height = engine_config["window"]["height"].value_or<u32>(0);
            MH_ASSERT(window.height != 0, "Game and engine configs are missing u32 window.height.");
        }
    }
}
