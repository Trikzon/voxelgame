#include "mellohi/platform/platform.hpp"

#ifdef MH_PLATFORM_GLFW
    #include "mellohi/platform/glfw/glfw_platform.hpp"
#endif

namespace mellohi
{
    std::shared_ptr<Platform> init_platform(const std::shared_ptr<EngineConfigAsset> engine_config_ptr)
    {
        #ifdef MH_PLATFORM_GLFW
            return std::make_shared<GlfwPlatform>(engine_config_ptr);
        #else
            MH_ASSERT(false, "No platform backend has been selected.");
            return nullptr;
        #endif
    }
}
