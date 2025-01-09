#include "mellohi/core/color.hpp"

#include <regex>
#include <sstream>

#include "mellohi/core/logger.hpp"

namespace mellohi
{
    Color::Color() : Color(0.0f, 0.0f, 0.0f)
    {
        
    }
    
    Color::Color(const f32 r, const f32 g, const f32 b, const f32 a) : r(r), g(g), b(b), a(a)
    {
        
    }
    
    Color::Color(const std::string &hex_code)
    {
        static const std::regex hex_regex("^#([A-Fa-f0-9]+)$");
        
        MH_ASSERT(std::regex_match(hex_code, hex_regex), "Cannot create Color with invalid hex code {}.", hex_code);
        
        std::string full_hex_code = hex_code;
        if (hex_code.length() == 4 || hex_code.length() == 5)
        {
            full_hex_code = {'#', hex_code[1], hex_code[1], hex_code[2], hex_code[2], hex_code[3], hex_code[3]};
            
            if (hex_code.length() == 5)
            {
                full_hex_code += {hex_code[4], hex_code[4]};
            }
        }
        
        MH_ASSERT(full_hex_code.length() == 7 || full_hex_code.length() == 9,
                  "Cannot create Color with invalid hex code {}.", hex_code);
        
        std::stringstream ss;
        
        u32 r;
        ss << std::hex << full_hex_code.substr(1, 2);
        ss >> r;
        ss.clear();
        
        u32 g;
        ss << std::hex << full_hex_code.substr(3, 2);
        ss >> g;
        ss.clear();
        
        u32 b;
        ss << std::hex << full_hex_code.substr(5, 2);
        ss >> b;
        ss.clear();
        
        u32 a = 255;
        if (full_hex_code.length() == 9)
        {
            ss << std::hex << full_hex_code.substr(7, 2);
            ss >> a;
        }
        
        this->r = r / 255.0f;
        this->g = g / 255.0f;
        this->b = b / 255.0f;
        this->a = a / 255.0f;
    }
    
    bool Color::operator==(const Color &other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
    
    bool Color::operator!=(const Color &other) const
    {
        return !(*this == other);
    }
    
    std::ostream & operator<<(std::ostream &os, const Color &color)
    {
        return os << '[' << color.r << ", " << color.g << ", " << color.b << ", " << color.a << ']';
    }
    
    Color Color::srgb_to_linear() const
    {
        return Color(
            r < 0.04045f ? r * (1.0f / 12.92f) : std::pow((r + 0.055) / 1.055, 2.4f),
            g < 0.04045f ? g * (1.0f / 12.92f) : std::pow((g + 0.055) / 1.055, 2.4f),
            b < 0.04045f ? b * (1.0f / 12.92f) : std::pow((b + 0.055) / 1.055, 2.4f),
            a
        );
    }
    
    Color Color::linear_to_srgb() const
    {
        return Color(
            r < 0.0031308f ? r * 12.92f : 1.055 * std::pow(r, 1.0f / 2.4f) - 0.055,
            g < 0.0031308f ? g * 12.92f : 1.055 * std::pow(g, 1.0f / 2.4f) - 0.055,
            b < 0.0031308f ? b * 12.92f : 1.055 * std::pow(b, 1.0f / 2.4f) - 0.055,
            a
        );
    }
    
    std::array<f32, 4> Color::as_array() const
    {
        return {r, g, b, a};
    }
}
