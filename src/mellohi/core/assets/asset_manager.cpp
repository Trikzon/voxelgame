#include "mellohi/core/assets/asset_manager.hpp"

namespace mellohi
{
    AssetManager::AssetManager()
    {
        MH_INFO("Game Assets Dir: {}", MH_GAME_ASSETS_DIR);
        MH_INFO("Engine Assets Dir: {}", MH_ENGINE_ASSETS_DIR);
    }
}
