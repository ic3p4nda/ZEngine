#pragma once

#include "window.h"
#include "device.h"
#include "swapchain.h"
#include "model.h"

#include <memory>
#include <vector>
#include <cassert>

namespace z_engine
{
    class ZRenderer
    {
    public:
        ZRenderer(ZWindow& window, ZDevice& device);
        ~ZRenderer();

        ZRenderer(const ZRenderer&) = delete;
        ZRenderer& operator=(const ZRenderer&) = delete;

        VkFormat GetDepthFormat() const { return swapchain_->FindDepthFormat(); };
        VkFormat GetImageFormat() const { return swapchain_->GetSwapChainImageFormat(); };
        float GetAspectRatio() const { return swapchain_->ExtentAspectRatio(); };
        bool IsFrameInProgress() const { return is_frame_started_; };

        VkCommandBuffer GetCurrentCommandBuffer() const
        {
            assert(is_frame_started_ && "Cannot get command buffer when frame not in progress");
            return command_buffers_[current_frame_index_];
        }

        int GetFrameIndex() const
        {
            assert(is_frame_started_ && "Cannot get Frame index when frame not in progress");
            return current_frame_index_;
        }

        
        void TransitionImageLayout(VkCommandBuffer command_buffer,
        VkImage image,
        VkImageLayout old_layout,
        VkImageLayout new_layout,
        VkImageAspectFlags aspect_mask);
        
        VkCommandBuffer BeginFrame();
        void EndFrame();
        void BeginSwapchainRenderPass(VkCommandBuffer command_buffer);
        void EndSwapchainRenderPass(VkCommandBuffer command_buffer);

    private:
        void CreateCommandBuffers();
        void FreeCommandBuffers();
        void RecreateSwapchain();

        ZWindow& window_;
        ZDevice& device_;
        std::unique_ptr<ZSwapChain> swapchain_;
        std::vector<VkCommandBuffer> command_buffers_;

        uint32_t current_image_index_ = 0;
        int current_frame_index_ = 0;
        bool is_frame_started_ = false;
    };
}
