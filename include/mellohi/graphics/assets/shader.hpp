#pragma once

#include "mellohi/core/assets/toml_asset.hpp"

namespace mellohi
{
    class Shader : public TomlAsset
    {
    public:
        Shader(std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id);
        
    private:
    };
}
