#pragma once

#include "mellohi/core/assets/asset.hpp"
#include "mellohi/core/logger.hpp"

namespace mellohi
{
    class AssetManager : public std::enable_shared_from_this<AssetManager>
    {
    public:
        AssetManager();
    
        template<typename T>
        std::shared_ptr<T> load(const AssetId &asset_id);
    
    private:
        std::unordered_map<AssetId, std::weak_ptr<Asset>> m_assets;
    };
    
    template<typename T>
    std::shared_ptr<T> AssetManager::load(const AssetId &asset_id)
    {
        const auto asset_it = m_assets.find(asset_id);
        if (asset_it != m_assets.end())
        {
            if (const auto asset_ptr = asset_it->second.lock())
            {
                const auto asset = std::dynamic_pointer_cast<T>(asset_ptr);
                MH_ASSERT(asset, "Loaded asset '{}' cannot be cast to requested type.", asset_id);
                return asset;
            }
        }
        
        const auto asset = std::make_shared<T>(shared_from_this(), asset_id);
        m_assets[asset_id] = asset;
        return asset;
    }
}
