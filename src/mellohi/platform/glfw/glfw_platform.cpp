#include "mellohi/platform/glfw/glfw_platform.hpp"

#include "mellohi/core/logger.hpp"

namespace mellohi
{
    GlfwPlatform::GlfwPlatform(const std::shared_ptr<EngineConfigAsset> engine_config_ptr)
        : m_engine_config_ptr(engine_config_ptr)
    {
        m_engine_config_reloaded_callback_id = engine_config_ptr->register_reload_callback(
            std::bind(&GlfwPlatform::on_engine_config_reloaded, this)
        );
        
        MH_ASSERT(glfwInit(), "Failed to initialize GLFW.");
        
        #ifdef MH_GRAPHICS_VULKAN
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        #endif
        
        glfwWindowHint(GLFW_RESIZABLE, engine_config_ptr->get_window_resizable());
        
        const auto initial_size = engine_config_ptr->get_window_initial_size();
        
        m_window_ptr = glfwCreateWindow(initial_size.x, initial_size.y,
                                        engine_config_ptr->get_window_title().c_str(),
                                        nullptr, nullptr);
    }
    
    GlfwPlatform::~GlfwPlatform()
    {
        m_engine_config_ptr->deregister_reload_callback(m_engine_config_reloaded_callback_id);
        
        glfwDestroyWindow(m_window_ptr);
        
        glfwTerminate();
    }
    
    void GlfwPlatform::process_events()
    {
        glfwPollEvents();
    }
    
    bool GlfwPlatform::reload_pressed() const
    {
        return glfwGetKey(m_window_ptr, GLFW_KEY_R) == GLFW_PRESS;
    }
    
    bool GlfwPlatform::close_requested() const
    {
        return glfwWindowShouldClose(m_window_ptr);
    }
    
    uvec2 GlfwPlatform::get_framebuffer_size() const
    {
        i32 width, height;
        glfwGetFramebufferSize(m_window_ptr, &width, &height);
        
        MH_ASSERT_DEBUG(width >= 0 && height >= 0, "GLFW framebuffer size is negative ({}, {}).", width, height);
        
        return {width, height};
    }
    
    std::vector<const char *> GlfwPlatform::get_required_vulkan_instance_extensions() const
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
        vk::SurfaceKHR GlfwPlatform::create_vulkan_surface(const vk::Instance vk_instance) const
        {
            VkSurfaceKHR vk_surface;
            auto result = glfwCreateWindowSurface(vk_instance, m_window_ptr, nullptr, &vk_surface);
            MH_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan surface from GLFW window.");
            
            return vk_surface;
        }
    #endif
    
    void GlfwPlatform::on_engine_config_reloaded()
    {
        glfwSetWindowTitle(m_window_ptr, m_engine_config_ptr->get_window_title().c_str());
        glfwSetWindowAttrib(m_window_ptr, GLFW_RESIZABLE, m_engine_config_ptr->get_window_resizable());
    }
}
