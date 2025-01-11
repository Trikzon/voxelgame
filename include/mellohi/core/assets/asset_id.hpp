#pragma once

#include <filesystem>
#include <string>

#include "mellohi/core/types.hpp"

namespace mellohi
{
    class AssetId
    {
    public:
        explicit AssetId(std::string_view fully_qualified_path);
        AssetId(std::string_view package, std::string_view path);
        
        bool operator==(const AssetId &other) const;
        bool operator!=(const AssetId &other) const;
        bool operator<(const AssetId &other) const;
        bool operator>(const AssetId &other) const;
        bool operator<=(const AssetId &other) const;
        bool operator>=(const AssetId &other) const;
        friend std::ostream & operator<<(std::ostream &os, const AssetId &asset_id);
        
        const std::string & get_package() const;
        const std::string & get_path() const;
        std::string get_fully_qualified_path() const;
        
        std::filesystem::path as_file_path() const;
        bool file_exists() const;
        std::string read_file_as_string() const;
        std::vector<u8> read_file_as_bytes() const;
        
    private:
        std::string m_package, m_path;
    };
}

template<>
struct std::hash<mellohi::AssetId>
{
    mellohi::usize operator()(const mellohi::AssetId &asset_id) const noexcept
    {
        return std::hash<string>{}(asset_id.get_fully_qualified_path());
    }
};
