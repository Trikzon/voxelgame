#include "mellohi/core/asset.hpp"

#include <fstream>

#include "mellohi/core/logger.hpp"

namespace mellohi
{
    Asset::Asset(const std::string_view fully_qualified_id)
    {
        const auto delimiter_pos = fully_qualified_id.find(":");
        MH_ASSERT_DEBUG(delimiter_pos != std::string_view::npos,
                        "Asset cannot be constructed from an invalid fully qualified id: {}",
                        fully_qualified_id);
        
        m_package = fully_qualified_id.substr(0, delimiter_pos);
        m_relative_id = fully_qualified_id.substr(delimiter_pos + 1);
        
        if (!file_exists())
        {
            MH_WARN("Asset {} does not point to an existing file.", *this);
        }
    }
    
    Asset::Asset(const std::string_view package, const std::string_view relative_id)
    {
        m_package = package;
        m_relative_id = relative_id;
        
        if (!file_exists())
        {
            MH_WARN("Asset {} does not point to an existing file.", *this);
        }
    }
    
    bool Asset::operator==(const Asset &other) const
    {
        return m_package == other.m_package && m_relative_id == other.m_relative_id;
    }
    
    bool Asset::operator!=(const Asset &other) const
    {
        return !(*this == other);
    }
    
    std::ostream & operator<<(std::ostream &os, const Asset &asset)
    {
        return os << asset.get_fully_qualified_id();
    }
    
    const std::string & Asset::get_package() const
    {
        return m_package;
    }
    
    const std::string & Asset::get_relative_id() const
    {
        return m_relative_id;
    }
    
    std::string Asset::get_fully_qualified_id() const
    {
        return m_package + ":" + m_relative_id;
    }
    
    bool Asset::file_exists() const
    {
        return std::filesystem::exists(get_file_path());
    }
    
    std::filesystem::path Asset::get_file_path() const
    {
        const auto path = std::filesystem::path(MH_GAME_ASSETS_DIR) / m_package / m_relative_id;
        if (std::filesystem::exists(path))
        {
            return path;
        }
        
        return std::filesystem::path(MH_ENGINE_ASSETS_DIR) / m_package / m_relative_id;
    }
    
    std::string Asset::read_file_as_string() const
    {
        MH_ASSERT(file_exists(), "Asset {} points to a file that does not exist. Cannot read as string.", *this);
        
        std::ifstream ifs(get_file_path());
        std::string content((std::istreambuf_iterator(ifs)), (std::istreambuf_iterator<char>()));
        
        return content;
    }
    
    std::vector<u8> Asset::read_file_as_bytes() const
    {
        MH_ASSERT(file_exists(), "Asset {} points to a file that does not exist. Cannot read as bytes.", *this);
        
        std::ifstream ifs(get_file_path(), std::ios::binary | std::ios::ate);
        
        const auto size = ifs.tellg();
        
        ifs.seekg(0, std::ios::beg);
        std::vector<u8> content(size);
        ifs.read(reinterpret_cast<char *>(content.data()), size);
        
        return content;
    }
}
