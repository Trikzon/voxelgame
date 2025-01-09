#pragma once

#include <mutex>

#include "mellohi/core/assets/asset_id.hpp"

namespace mellohi
{
    class Asset
    {
    public:
        explicit Asset(const AssetId &asset_id);
        virtual ~Asset();
        
        void reload();
        usize register_reload_callback(const std::function<void()> &callback);
        void deregister_reload_callback(usize callback_id);
        
        const AssetId & get_id() const;
    
    private:
        AssetId m_id;
        
        std::unordered_map<usize, std::function<void()>> m_reload_callbacks{};
        usize m_next_reload_callback_id{};
        std::mutex m_reload_callback_mutex{};
        
        virtual void load() = 0;
    };
    
    class TextAsset : public Asset
    {
    public:
        explicit TextAsset(const AssetId &asset_id);
        
        const std::string & get_text() const;
        
    private:
        std::string m_text;
        
        void load() override;
    };
}
