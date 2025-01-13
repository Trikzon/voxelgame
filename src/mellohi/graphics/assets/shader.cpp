#include "mellohi/graphics/assets/shader.hpp"

namespace mellohi
{
    Shader::Shader(std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id)
        : TomlAsset(asset_manager_ptr, asset_id)
    {
        
    }
}
