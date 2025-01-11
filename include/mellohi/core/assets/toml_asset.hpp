#pragma once

#include <toml++/toml.hpp>

#include "mellohi/core/assets/asset_manager.hpp"
#include "mellohi/core/color.hpp"
#include "mellohi/core/logger.hpp"

namespace mellohi
{
    class TomlAsset : public Asset
    {
    public:
        TomlAsset(std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id);
        
    protected:
        toml::table parse_toml_table() const;
        
        template<typename T>
        std::optional<T> parse_opt(const toml::table &table, std::string_view path) const;
        template<>
        std::optional<AssetId> parse_opt(const toml::table &table, std::string_view path) const;
        template<>
        std::optional<Color> parse_opt(const toml::table &table, std::string_view path) const;
        template<>
        std::optional<uvec2> parse_opt(const toml::table &table, std::string_view path) const;
        
        template<typename T>
        T parse(const toml::table &table, std::string_view path, std::string_view type_name) const;
    };
    
    template<typename T>
    std::optional<T> TomlAsset::parse_opt(const toml::table &table, std::string_view path) const
    {
        return table[toml::path(path)].value<T>();
    }
    
    template<typename T>
    T TomlAsset::parse(const toml::table &table, std::string_view path, std::string_view type_name) const
    {
        const auto value_opt = parse_opt<T>(table, path);
        MH_ASSERT(value_opt.has_value(), "{} is missing {} {}.", get_id(), type_name, path);
        return value_opt.value();
    }
}
