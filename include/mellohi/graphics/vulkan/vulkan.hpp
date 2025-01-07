#pragma once

#ifndef MH_GRAPHICS_VULKAN
    #error vulkan.hpp can not be included when MH_GRAPHICS_VULKAN is not defined.
#endif

#include "mellohi/core/logger.hpp"

#define VULKAN_HPP_NO_CONSTRUCTORS   // Allows use of C++20 initializers for structs.
#define VULKAN_HPP_NO_EXCEPTIONS     // Don't throw exceptions, but instead return a result value.

#define VULKAN_HPP_ASSERT_ON_RESULT  // Disable asserting when the result is not a success. Handle it ourselves.

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

// Need to "export" multiple header files because of https://github.com/clangd/clangd/issues/1085.
#include <vulkan/vulkan.hpp> // IWYU pragma: export
#include <vulkan/vulkan_enums.hpp> // IWYU pragma: export
#include <vulkan/vulkan_handles.hpp> // IWYU pragma: export

#define MH_ASSERT_VK(vk_result, message, ...) \
    if (vk_result != vk::Result::eSuccess)    \
    {                                         \
        MH_FATAL(message, ##__VA_ARGS__);     \
        std::abort();                         \
    }
