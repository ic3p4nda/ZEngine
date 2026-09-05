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
        ZRenderer(ZWindow& Window, ZDevice& Device);
        ~ZRenderer();

        ZRenderer(const ZRenderer&) = delete;
        ZRenderer& operator=(const ZRenderer&) = delete;

        VkRenderPass getSwapchainRenderPass() const { return Swapchain->getRenderPass(); };
        float getAspectRatio() const { return Swapchain->extentAspectRatio(); };
        bool isFrameInProgress() const { return isFrameStarted; };

        VkCommandBuffer getCurrentCommandBuffer() const
        {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        };

        int getFrameIndex() const
        {
            assert(isFrameStarted && "Cannot get Frame index when frame not in progress");
            return currentFrameIndex;
        };

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapchainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapchainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapchain();

        ZWindow& Window;
        ZDevice& Device;
        std::unique_ptr<ZSwapChain> Swapchain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex = 0;
        int currentFrameIndex = 0;
        bool isFrameStarted = false;
    };
}
