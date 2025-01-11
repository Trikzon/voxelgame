#pragma once

#include <mutex>

#include "mellohi/core/assets/asset_id.hpp"

namespace mellohi
{
    class Asset
    {
    public:
        Asset(std::shared_ptr<class AssetManager> asset_manager_ptr, const AssetId &asset_id);
        virtual ~Asset();
        
        void reload();
        
        usize register_reload_callback(const std::function<void()> &callback);
        void deregister_reload_callback(usize callback_id);
        
        const AssetId & get_id() const;
        
    protected:
        class AssetManager & get_asset_manager() const;
        
    private:
        std::shared_ptr<class AssetManager> m_asset_manager_ptr;
        AssetId m_id;
        
        std::unordered_map<usize, std::function<void()>> m_reload_callbacks;
        usize m_next_reload_callback_id = 0;
        std::mutex m_reload_callback_mutex;
        
        virtual void load() = 0;
    };
    
    class TextAsset : public Asset
    {
    public:
        TextAsset(std::shared_ptr<class AssetManager> asset_manager, const AssetId &asset_id);
        
        std::string get_text() const;
        
    private:
        std::string m_text;
        
        void load() override;
    };
    
    class BinaryAsset : public Asset
    {
    public:
        BinaryAsset(std::shared_ptr<class AssetManager> asset_manager, const AssetId &asset_id);
        
        const std::vector<u8> & get_bytes() const;
        
    private:
        std::vector<u8> m_bytes;
        
        void load() override;
    };
}
