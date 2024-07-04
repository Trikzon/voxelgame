#pragma once

#include <mellohi.hpp>

using namespace mellohi;

struct Camera
{
    struct Data
    {
        mat4x4f projection;
        mat4x4f view;
    };

    explicit Camera(const flecs::world &world);
};