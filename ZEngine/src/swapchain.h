#pragma once

#include "device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace z_engine
{
    class ZSwapChain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        ZSwapChain(ZDevice& device_ref, VkExtent2D extent);
        ZSwapChain(ZDevice& device_ref, VkExtent2D extent, std::shared_ptr<ZSwapChain> previous);
        ~ZSwapChain();

        ZSwapChain(const ZSwapChain&) = delete;
        ZSwapChain& operator=(const ZSwapChain&) = delete;

        VkFramebuffer GetFrameBuffer(int index) { return swap_chain_framebuffers_[index]; }
        VkRenderPass GetRenderPass() { return render_pass_; }
        VkImage GetDepthImage(int index) { return depth_images_[index]; }
        VkImageView GetDepthImageView(int index) { return depth_image_views_[index]; }
        VkImage GetSwapChainImage(int index) { return swap_chain_images_[index]; }
        VkImageView GetImageView(int index) { return swap_chain_image_views_[index]; }
        size_t ImageCount() { return swap_chain_images_.size(); }
        VkFormat GetSwapChainImageFormat() { return swap_chain_image_format_; }
        VkExtent2D GetSwapChainExtent() { return swap_chain_extent_; }
        uint32_t width() { return swap_chain_extent_.width; }
        uint32_t height() { return swap_chain_extent_.height; }

        float ExtentAspectRatio()
        {
            return static_cast<float>(swap_chain_extent_.width) / static_cast<float>(swap_chain_extent_.height);
        }

        VkFormat FindDepthFormat();

        VkResult AcquireNextImage(uint32_t* image_index);
        VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* image_index);

        bool CompareSwapFormates(const ZSwapChain& swapchain) const
        {
            return swapchain.swap_chain_depth_format_ == swap_chain_depth_format_ &&
                swapchain.swap_chain_image_format_ == swap_chain_image_format_;
        }

    private:
        void init();
        void CreateSwapChain();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& available_formats);
        VkPresentModeKHR ChooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& available_present_modes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        VkFormat swap_chain_image_format_;
        VkFormat swap_chain_depth_format_;
        VkExtent2D swap_chain_extent_;

        std::vector<VkFramebuffer> swap_chain_framebuffers_;
        VkRenderPass render_pass_;

        std::vector<VkImage> depth_images_;
        std::vector<VmaAllocation> depth_image_allocation_;
        std::vector<VkImageView> depth_image_views_;
        std::vector<VkImage> swap_chain_images_;
        std::vector<VkImageView> swap_chain_image_views_;

        ZDevice& device_;
        VkExtent2D window_extent_;

        VkSwapchainKHR swap_chain_;
        std::shared_ptr<ZSwapChain> old_swap_chain_;

        std::vector<VkSemaphore> image_available_semaphores_;
        std::vector<VkSemaphore> render_finished_semaphores_;
        std::vector<VkFence> in_flight_fences_;
        std::vector<VkFence> images_in_flight_;
        size_t current_frame_ = 0;
    };
}
