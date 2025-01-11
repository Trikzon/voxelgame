#pragma once

#include "mellohi/core/assets/toml_asset.hpp"

namespace mellohi
{
    class Material : public TomlAsset
    {
    public:
        Material(std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id);
        
    private:
    };
}
