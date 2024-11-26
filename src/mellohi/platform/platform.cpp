#include "mellohi/platform/platform.hpp"

#ifdef MH_PLATFORM_GLFW
    #include "mellohi/platform/glfw/glfw_platform.hpp"
#endif

namespace mellohi
{
    std::shared_ptr<Platform> init_platform(const Config &config)
    {
        #ifdef MH_PLATFORM_GLFW
            return std::make_shared<GLFWPlatform>(config);
        #else
            MH_ASSERT(false, "No platform backend has been selected.");
            return nullptr;
        #endif
    }
}
