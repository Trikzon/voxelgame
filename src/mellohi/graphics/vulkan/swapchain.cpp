#include "mellohi/graphics/vulkan/swapchain.hpp"

namespace mellohi
{
    Swapchain::Swapchain(const std::shared_ptr<EngineConfigAsset> engine_config_ptr,
                         const std::shared_ptr<Platform> platform_ptr, const std::shared_ptr<Device> device_ptr)
        : m_engine_config_ptr(engine_config_ptr), m_platform_ptr(platform_ptr), m_device_ptr(device_ptr)
    {
        m_engine_config_reloaded_callback_id = engine_config_ptr->register_reload_callback(
            std::bind(&Swapchain::on_engine_config_reloaded, this)
        );
        
        create_swapchain();
        create_image_views();
        create_sync_objects();
    }

    Swapchain::~Swapchain()
    {
        m_engine_config_ptr->deregister_reload_callback(m_engine_config_reloaded_callback_id);
        
        destroy();
        
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_device_ptr->destroy_fence(m_in_flight_fences[i]);
            m_device_ptr->destroy_semaphore(m_render_finished_semaphores[i]);
            m_device_ptr->destroy_semaphore(m_image_available_semaphores[i]);
        }
    }
    
    void Swapchain::init_with_render_pass(const vk::RenderPass render_pass)
    {
        m_render_pass = render_pass;
        
        create_framebuffers();
    }
    
    std::optional<u32> Swapchain::acquire_next_image_index()
    {
        m_device_ptr->wait_for_fence(m_in_flight_fences[m_current_frame_index]);
        
        u32 image_index;
        auto result = m_device_ptr->get_device()
            .acquireNextImageKHR(m_swapchain, std::numeric_limits<u64>::max(),
                                 m_image_available_semaphores[m_current_frame_index], {}, &image_index);
        
        if (result == vk::Result::eErrorOutOfDateKHR)
        {
            recreate();
            return std::nullopt;
        }
        else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
        {
            MH_ASSERT(false, "Failed to acquire Vulkan swapchain image.");
        }
        
        m_device_ptr->reset_fence(m_in_flight_fences[m_current_frame_index]);
        
        return image_index;
    }
    
    void Swapchain::present(const u32 image_index, const vk::CommandBuffer command_buffer)
    {
        const vk::Semaphore wait_semaphores[] = {m_image_available_semaphores[m_current_frame_index]};
        const vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        const vk::Semaphore signal_semaphores[] = {m_render_finished_semaphores[m_current_frame_index]};
        
        const vk::SubmitInfo submit_info
        {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = wait_semaphores,
            .pWaitDstStageMask = wait_stages,
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signal_semaphores,
        };
        
        const auto graphics_queue = m_device_ptr->get_queue(QueueCapability::Graphics);
        
        auto result = graphics_queue.submit(1, &submit_info, m_in_flight_fences[m_current_frame_index]);
        MH_ASSERT_VK(result, "Failed to submit Vulkan queue.");
        
        const vk::PresentInfoKHR present_info
        {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signal_semaphores,
            .swapchainCount = 1,
            .pSwapchains = &m_swapchain,
            .pImageIndices = &image_index,
            .pResults = nullptr,
        };
        
        result = graphics_queue.presentKHR(present_info);
        if (m_should_be_recreated || result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
        {
            recreate();
        }
        else if (result != vk::Result::eSuccess)
        {
            MH_ASSERT(false, "Failed to present Vulkan swapchain image.");
        }
        
        m_current_frame_index = (m_current_frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
    }
    
    usize Swapchain::get_current_frame_index() const
    {
        return m_current_frame_index;
    }

    vk::Extent2D Swapchain::get_extent() const
    {
        return m_extent;
    }

    vk::SwapchainKHR Swapchain::get_swapchain() const
    {
        return m_swapchain;
    }

    const std::vector<vk::ImageView> & Swapchain::get_image_views() const
    {
        return m_image_views;
    }
    
    vk::Framebuffer Swapchain::get_framebuffer(const u32 image_index) const
    {
        return m_framebuffers[image_index];
    }
    
    void Swapchain::create_swapchain()
    {
        const auto available_present_modes = m_device_ptr->get_surface_present_modes();
        auto present_mode = vk::PresentModeKHR::eFifo;
        for (const auto &available_present_mode : available_present_modes)
        {
            if (m_engine_config_ptr->get_window_vsync() && available_present_mode == vk::PresentModeKHR::eMailbox
                || !m_engine_config_ptr->get_window_vsync() && available_present_mode == vk::PresentModeKHR::eImmediate)
            {
                present_mode = available_present_mode;
                break;
            }
        }
        
        const auto surface_capabilities = m_device_ptr->get_surface_capabilities();
        m_extent = surface_capabilities.currentExtent;
        if (surface_capabilities.currentExtent.width == std::numeric_limits<u32>::max())
        {
            const auto framebuffer_size = m_platform_ptr->get_framebuffer_size();
            m_extent.width = std::clamp(framebuffer_size.x,
                                        surface_capabilities.minImageExtent.width,
                                        surface_capabilities.maxImageExtent.width);
            m_extent.height = std::clamp(framebuffer_size.x,
                                         surface_capabilities.minImageExtent.height,
                                         surface_capabilities.maxImageExtent.height);
        }
        
        auto image_count = surface_capabilities.minImageCount + 1;
        if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount)
        {
            image_count = surface_capabilities.maxImageCount;
        }
        
        const auto surface_format = m_device_ptr->get_preferred_surface_format();
        
        vk::SwapchainCreateInfoKHR swapchain_create_info
        {
            .surface = m_device_ptr->get_surface(),
            .minImageCount = image_count,
            .imageFormat = surface_format.format,
            .imageColorSpace = surface_format.colorSpace,
            .imageExtent = m_extent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .preTransform = surface_capabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = present_mode,
            .clipped = vk::True,
        };
        
        const auto unique_queue_families = m_device_ptr->get_unique_queue_family_indices();
        if (unique_queue_families.size() > 1)
        {
            swapchain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
            swapchain_create_info.queueFamilyIndexCount = unique_queue_families.size();
            swapchain_create_info.pQueueFamilyIndices = unique_queue_families.data();
        }
        else
        {
            swapchain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
        }
        
        m_swapchain = m_device_ptr->create_swapchain(swapchain_create_info);
    }
    
    void Swapchain::create_image_views()
    {
        const auto swapchain_images = m_device_ptr->get_swapchain_images(m_swapchain);
        const auto surface_format = m_device_ptr->get_preferred_surface_format();
        
        for (const auto &swapchain_image : swapchain_images)
        {
            const vk::ImageViewCreateInfo image_view_create_info
            {
                .image = swapchain_image,
                .viewType = vk::ImageViewType::e2D,
                .format = surface_format.format,
                .components = vk::ComponentMapping
                {
                    .r = vk::ComponentSwizzle::eIdentity,
                    .g = vk::ComponentSwizzle::eIdentity,
                    .b = vk::ComponentSwizzle::eIdentity,
                    .a = vk::ComponentSwizzle::eIdentity,
                },
                .subresourceRange = vk::ImageSubresourceRange
                {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };
            
            m_image_views.push_back(m_device_ptr->create_image_view(image_view_create_info));
        }
    }
    
    void Swapchain::create_framebuffers()
    {
        for (const auto &image_view : m_image_views)
        {
            const vk::FramebufferCreateInfo framebuffer_create_info
            {
                .renderPass = m_render_pass,
                .attachmentCount = 1,
                .pAttachments = &image_view,
                .width = m_extent.width,
                .height = m_extent.height,
                .layers = 1,
            };
            
            m_framebuffers.push_back(m_device_ptr->create_framebuffer(framebuffer_create_info));
        }
    }
    
    void Swapchain::create_sync_objects()
    {
        const vk::FenceCreateInfo fence_create_info
        {
            .flags = vk::FenceCreateFlagBits::eSignaled,
        };
        
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_image_available_semaphores.push_back(m_device_ptr->create_semaphore({}));
            m_render_finished_semaphores.push_back(m_device_ptr->create_semaphore({}));
            m_in_flight_fences.push_back(m_device_ptr->create_fence(fence_create_info));
        }
    }
    
    void Swapchain::recreate()
    {
        destroy();
        create_swapchain();
        create_image_views();
        create_framebuffers();
        
        m_should_be_recreated = false;
    }
    
    void Swapchain::destroy()
    {
        m_device_ptr->wait_idle();
        
        for (const auto &framebuffer : m_framebuffers)
        {
            m_device_ptr->destroy_framebuffer(framebuffer);
        }
        m_framebuffers.clear();
        
        for (const auto &image_view : m_image_views)
        {
            m_device_ptr->destroy_image_view(image_view);
        }
        m_image_views.clear();
        
        m_device_ptr->destroy_swapchain(m_swapchain);
        m_swapchain = nullptr;
    }
    
    void Swapchain::on_engine_config_reloaded()
    {
        m_should_be_recreated = true;
    }
}
