#pragma once

#include <filesystem>
#include <string>

#include "mellohi/core/types.hpp"

namespace mellohi
{
    class Asset
    {
    public:
        explicit Asset(std::string_view fully_qualified_id);
        Asset(std::string_view package, std::string_view relative_id);
        virtual ~Asset() = default;
        
        bool operator==(const Asset &other) const;
        bool operator!=(const Asset &other) const;
        friend std::ostream & operator<<(std::ostream &os, const Asset &asset);
        
        const std::string & get_package() const;
        const std::string & get_relative_id() const;
        std::string get_fully_qualified_id() const;
        
        bool file_exists() const;
        std::filesystem::path get_file_path() const;
        std::string read_file_as_string() const;
        std::vector<u8> read_file_as_bytes() const;
        
    private:
        std::string m_package, m_relative_id;
    };
}
