#include "mellohi/core/assets/asset_id.hpp"

#include <fstream>

#include "mellohi/core/logger.hpp"

namespace mellohi
{
    AssetId::AssetId(const std::string_view fully_qualified_path)
    {
        const auto delimiter_pos = fully_qualified_path.find(':');
        if (delimiter_pos == std::string_view::npos)
        {
            m_path = fully_qualified_path;
        }
        else
        {
            m_package = fully_qualified_path.substr(0, delimiter_pos);
            m_path = fully_qualified_path.substr(delimiter_pos + 1);
        }
        
        MH_ASSERT(file_exists(), "AssetId {} constructed, but its file does not exist.");
    }
    
    AssetId::AssetId(const std::string_view package, const std::string_view path)
        : m_package(package), m_path(path)
    {
        MH_ASSERT(file_exists(), "AssetId {} constructed, but its file does not exist.");
    }
    
    bool AssetId::operator==(const AssetId &other) const
    {
        return m_package == other.m_package && m_path == other.m_path;
    }
    
    bool AssetId::operator!=(const AssetId &other) const
    {
        return !(*this == other);
    }
    
    bool AssetId::operator<(const AssetId &other) const
    {
        return get_fully_qualified_path() < other.get_fully_qualified_path();
    }
    
    bool AssetId::operator>(const AssetId &other) const
    {
        return other < *this;
    }
    
    bool AssetId::operator<=(const AssetId &other) const
    {
        return !(other < *this);
    }
    
    bool AssetId::operator>=(const AssetId &other) const
    {
        return !(*this < other);
    }
    
    std::ostream & operator<<(std::ostream &os, const AssetId &asset_id)
    {
        return os << asset_id.get_fully_qualified_path();
    }
    
    const std::string & AssetId::get_package() const
    {
        return m_package;
    }
    
    const std::string & AssetId::get_path() const
    {
        return m_path;
    }
    
    std::string AssetId::get_fully_qualified_path() const
    {
        return m_package + ":" + m_path;
    }
    
    std::filesystem::path AssetId::as_file_path() const
    {
        const auto path = std::filesystem::path(MH_GAME_ASSETS_DIR) / m_package / m_path;
        if (std::filesystem::exists(path))
        {
            return path;
        }
        
        return std::filesystem::path(MH_ENGINE_ASSETS_DIR) / m_package / m_path;
    }
    
    bool AssetId::file_exists() const
    {
        return std::filesystem::exists(as_file_path());
    }
    
    std::string AssetId::read_file_as_string() const
    {
        MH_ASSERT(file_exists(), "AssetId {} points to a file that does not exist. Cannot read as string.", *this);
        
        std::ifstream ifs(as_file_path());
        std::string content((std::istreambuf_iterator(ifs)), (std::istreambuf_iterator<char>()));
        
        return content;
    }
    
    std::vector<u8> AssetId::read_file_as_bytes() const
    {
        MH_ASSERT(file_exists(), "AssetId {} points to a file that does not exist. Cannot read as bytes.", *this);
        
        std::ifstream ifs(as_file_path(), std::ios::binary | std::ios::ate);
        
        const auto size = ifs.tellg();
        
        ifs.seekg(0, std::ios::beg);
        std::vector<u8> content(size);
        ifs.read(reinterpret_cast<char *>(content.data()), size);
        
        return content;
    }
}
