#pragma once

#include "mellohi/core/types.hpp"

namespace mellohi
{
    struct Color
    {
        f32 r, g, b, a;
        
        Color();
        Color(f32 r, f32 g, f32 b, f32 a = 1.0f);
        Color(const std::string &hex_code);
        
        bool operator==(const Color &other) const;
        bool operator!=(const Color &other) const;
        friend std::ostream & operator<<(std::ostream &os, const Color &color);
        
        Color srgb_to_linear() const;
        Color linear_to_srgb() const;
        
        std::array<f32, 4> as_array() const;
    };
}
