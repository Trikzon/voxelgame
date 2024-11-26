#pragma once

#include <string>

#include "mellohi/core/types.hpp"

namespace mellohi
{
    struct Config
    {
        struct
        {
            std::string name;
            std::string package;
        } engine;
        
        struct
        {
            std::string name;
            std::string package;
        } game;
        
        struct
        {
            std::string title;
            u32 width, height;
        } window;
        
        Config();
    };
}
