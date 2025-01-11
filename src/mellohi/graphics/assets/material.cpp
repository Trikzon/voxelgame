#include "mellohi/graphics/assets/material.hpp"

namespace mellohi
{
    Material::Material(std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id)
        : TomlAsset(asset_manager_ptr, asset_id)
    {
        
    }
}
