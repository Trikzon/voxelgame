#include "mellohi/core/assets/asset.hpp"

#include "mellohi/core/assets/asset_manager.hpp"
#include "mellohi/core/logger.hpp"

namespace mellohi
{
    Asset::Asset(const std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id)
        : m_asset_manager_ptr(asset_manager_ptr), m_id(asset_id)
    {
        MH_TRACE("Asset {} constructed.", asset_id);
    }
    
    Asset::~Asset()
    {
        MH_TRACE("Asset {} destructed.", get_id());
    }
    
    void Asset::reload()
    {
        load();
        
        MH_TRACE("Asset {} reloaded.", get_id());
        
        // Callbacks can deregister themselves mid-loop, so we make a copy to keep the iteration valid.
        const auto reload_callbacks = m_reload_callbacks;
        for (const auto &[id, callback] : reload_callbacks)
        {
            callback();
        }
    }
    
    usize Asset::register_reload_callback(const std::function<void()> &callback)
    {
        const std::lock_guard<std::mutex> lock(m_reload_callback_mutex);
        m_reload_callbacks[m_next_reload_callback_id] = callback;
        return m_next_reload_callback_id++;
    }
    
    void Asset::deregister_reload_callback(usize callback_id)
    {
        const std::lock_guard<std::mutex> lock(m_reload_callback_mutex);
        m_reload_callbacks.erase(callback_id);
    }
    
    const AssetId & Asset::get_id() const
    {
        return m_id;
    }
    
    class AssetManager & Asset::get_asset_manager() const
    {
        return *m_asset_manager_ptr;
    }
    
    TextAsset::TextAsset(const std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id)
        : Asset(asset_manager_ptr, asset_id)
    {
        load();
    }
    
    std::string TextAsset::get_text() const
    {
        return m_text;
    }
    
    void TextAsset::load()
    {
        m_text = get_id().read_file_as_string();
    }
    
    BinaryAsset::BinaryAsset(const std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id)
        : Asset(asset_manager_ptr, asset_id)
    {
        load();
    }
    
    const std::vector<u8> & BinaryAsset::get_bytes() const
    {
        return m_bytes;
    }
    
    void BinaryAsset::load()
    {
        m_bytes = get_id().read_file_as_bytes();
    }
}
