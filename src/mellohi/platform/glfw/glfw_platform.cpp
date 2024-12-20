#include "mellohi/platform/glfw/glfw_platform.hpp"

#include "mellohi/core/logger.hpp"

namespace mellohi
{
    GLFWPlatform::GLFWPlatform(const Config &config)
    {
        MH_ASSERT(glfwInit(), "Failed to initialize GLFW.");
        
        #ifdef MH_GRAPHICS_VULKAN
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        #endif
        
        m_window_ptr = glfwCreateWindow(config.window.width, config.window.height,
                                        config.window.title.c_str(),
                                        nullptr, nullptr);
    }
    
    GLFWPlatform::~GLFWPlatform()
    {
        glfwDestroyWindow(m_window_ptr);
        
        glfwTerminate();
    }
    
    void GLFWPlatform::process_events()
    {
        glfwPollEvents();
    }
    
    bool GLFWPlatform::close_requested() const
    {
        return glfwWindowShouldClose(m_window_ptr);
    }
    
    uvec2 GLFWPlatform::get_framebuffer_size() const
    {
        i32 width, height;
        glfwGetFramebufferSize(m_window_ptr, &width, &height);
        
        MH_ASSERT_DEBUG(width >= 0 && height >= 0, "GLFW framebuffer size is negative ({}, {}).", width, height);
        
        return {width, height};
    }
    
    std::vector<const char *> GLFWPlatform::get_required_vulkan_instance_extensions() const
    {
        u32 extension_count = 0;
        const char **extensions = glfwGetRequiredInstanceExtensions(&extension_count);
        
        if (!extensions)
        {
            return {};
        }
        
        return {extensions, extensions + extension_count};
    }
    
    #ifdef MH_GRAPHICS_VULKAN
        [[nodiscard]]
        vk::SurfaceKHR GLFWPlatform::create_vulkan_surface(vk::Instance vk_instance) const
        {
            VkSurfaceKHR vk_surface;
            auto result = glfwCreateWindowSurface(vk_instance, m_window_ptr, nullptr, &vk_surface);
            MH_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan surface from GLFW window.");
            
            return vk_surface;
        }
    #endif
}
