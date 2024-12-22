#include "mellohi/graphics/graphics.hpp"

#ifdef MH_GRAPHICS_VULKAN
    #include "mellohi/graphics/vulkan/vulkan_graphics.hpp"
#endif

namespace mellohi
{
    std::shared_ptr<class Graphics> init_graphics(const Config &config, std::shared_ptr<Platform> platform_ptr)
    {
        #ifdef MH_GRAPHICS_VULKAN
            return std::make_shared<VulkanGraphics>(config, platform_ptr);
        #else
            MH_ASSERT(false, "No graphics backend has been selected.");
            return nullptr;
        #endif
    }
}
