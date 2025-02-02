cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(mellohi)

option(MH_GRAPHICS_VULKAN "Enable Vulkan graphics backend" ON)
option(MH_PLATFORM_GLFW   "Enable GLFW platform backend"   ON)

set(INCLUDES
    include/mellohi/core/assets/asset.hpp
    include/mellohi/core/assets/asset_id.hpp
    include/mellohi/core/assets/asset_manager.hpp
    include/mellohi/core/assets/config_assets.hpp
    include/mellohi/core/assets/toml_asset.hpp
    include/mellohi/core/color.hpp
    include/mellohi/core/engine.hpp
    include/mellohi/core/logger.hpp
    include/mellohi/core/types.hpp
    include/mellohi/graphics/assets/material.hpp
    include/mellohi/graphics/assets/shader.hpp
    include/mellohi/graphics/vulkan/assets/vulkan_material.hpp
    include/mellohi/graphics/vulkan/assets/vulkan_shader.hpp
    include/mellohi/graphics/vulkan/device.hpp
    include/mellohi/graphics/vulkan/render_pass.hpp
    include/mellohi/graphics/vulkan/swapchain.hpp
    include/mellohi/graphics/vulkan/vulkan.hpp
    include/mellohi/graphics/vulkan/vulkan_graphics.hpp
    include/mellohi/graphics/graphics.hpp
    include/mellohi/platform/glfw/glfw_platform.hpp
    include/mellohi/platform/platform.hpp
)

set(SOURCES
    src/mellohi/core/assets/asset.cpp
    src/mellohi/core/assets/asset_id.cpp
    src/mellohi/core/assets/asset_manager.cpp
    src/mellohi/core/assets/config_assets.cpp
    src/mellohi/core/assets/toml_asset.cpp
    src/mellohi/core/color.cpp
    src/mellohi/core/engine.cpp
    src/mellohi/graphics/assets/material.cpp
    src/mellohi/graphics/assets/shader.cpp
    src/mellohi/graphics/vulkan/assets/vulkan_material.cpp
    src/mellohi/graphics/vulkan/assets/vulkan_shader.cpp
    src/mellohi/graphics/vulkan/device.cpp
    src/mellohi/graphics/vulkan/render_pass.cpp
    src/mellohi/graphics/vulkan/swapchain.cpp
    src/mellohi/graphics/vulkan/vulkan_graphics.cpp
    src/mellohi/graphics/graphics.cpp
    src/mellohi/platform/glfw/glfw_platform.cpp
    src/mellohi/platform/platform.cpp
)

add_library(mellohi STATIC ${SOURCES} ${INCLUDES})
target_include_directories(mellohi PUBLIC include)

target_compile_definitions(mellohi PUBLIC MH_ENGINE_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(mellohi PUBLIC MH_DEBUG_MODE)
endif()

if(MH_GRAPHICS_VULKAN)
    find_package(Vulkan REQUIRED)
    find_library(SHADERC_COMBINED_LIB shaderc_combined)
    if (NOT SHADERC_COMBINED_LIB)
        message(FATAL_ERROR "shaderc_combined not found.")
    endif()

    target_link_libraries(mellohi PUBLIC Vulkan::Vulkan ${SHADERC_COMBINED_LIB})
    target_compile_definitions(mellohi PUBLIC MH_GRAPHICS_VULKAN)
    target_compile_definitions(mellohi PUBLIC GLFW_INCLUDE_VULKAN)
endif()

if(MH_PLATFORM_GLFW)
    add_subdirectory(external/glfw)
    target_link_libraries(mellohi PUBLIC glfw)
    target_compile_definitions(mellohi PUBLIC MH_PLATFORM_GLFW)
endif()

add_subdirectory(external/glm)
target_link_libraries(mellohi PUBLIC glm)
target_compile_definitions(mellohi PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_ENABLE_EXPERIMENTAL)

add_subdirectory(external/tomlplusplus)
target_link_libraries(mellohi PUBLIC tomlplusplus::tomlplusplus)

add_subdirectory(games)
