#include "mellohi/graphics/graphics.hpp"

#ifdef MH_GRAPHICS_VULKAN
    #include "mellohi/graphics/vulkan/vulkan_graphics.hpp"
#endif

namespace mellohi
{
    std::shared_ptr<Graphics> init_graphics(const Config &config, const Platform &platform)
    {
        #ifdef MH_GRAPHICS_VULKAN
            return std::make_shared<VulkanGraphics>(config, platform);
        #else
            MH_ASSERT(false, "No graphics backend has been selected.");
            return nullptr;
        #endif
    }
}
