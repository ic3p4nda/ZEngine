#include "renderer.h"

#include <array>
#include <iostream>
#include <stdexcept>

namespace ZEngine
{
    ZRenderer::ZRenderer(class ZWindow& Window, class ZDevice& Device) : Window(Window), Device(Device)
    {
        recreateSwapchain();
        createCommandBuffers();
    }

    ZRenderer::~ZRenderer()
    {
        freeCommandBuffers();
    }

    void ZRenderer::createCommandBuffers()
    {
        commandBuffers.resize(Swapchain->MAX_FRAMES_IN_FLIGHT);
        
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = Device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
        
        if (vkAllocateCommandBuffers(Device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void ZRenderer::freeCommandBuffers()
    {
        vkFreeCommandBuffers(Device.device(), Device.getCommandPool(),
            static_cast<uint32_t>(commandBuffers.size()),
            commandBuffers.data());
        
        commandBuffers.clear();
    }

    void ZRenderer::recreateSwapchain()
    {
        auto extent = Window.getExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = Window.getExtent();
            glfwWaitEvents();
        }
        
        vkDeviceWaitIdle(Device.device());  
        
        if (Swapchain == nullptr)
        {
            Swapchain = std::make_unique<ZSwapChain>(Device, extent);
        }
        else
        {
            std::shared_ptr<ZSwapChain> oldSwapChain = std::move(Swapchain);
            Swapchain = std::make_unique<ZSwapChain>(Device, extent, oldSwapChain);
            
            if (!oldSwapChain->compareSwapFormates(*Swapchain.get()))
            {
                throw std::runtime_error("Swao chain image or depth format has changed");
            }
        }
    }
    
    VkCommandBuffer ZRenderer::beginFrame()
    {
        assert(!isFrameStarted && "Cant call beginFrame while already in progress");
        auto result = Swapchain->acquireNextImage(&currentImageIndex);
        
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapchain();
            return nullptr;
        }
        
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swapchain image!");
        }
           
        isFrameStarted = true;
        
        VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
        
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        
        return commandBuffer;
    }

    void ZRenderer::endFrame()
    {
        assert(isFrameStarted && "Cant call endFrame while not in progress");
        
        auto commandBuffer = getCurrentCommandBuffer();
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
        
        auto result = Swapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || Window.wasWindowReized())
        {
            Window.resetWindowResizeFlag();
            recreateSwapchain();
        } else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swapchain image!");
        }
        
        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % Swapchain->MAX_FRAMES_IN_FLIGHT;
    }

    void ZRenderer::beginSwapchainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Cant call beginSwapchain if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && 
            "Cant begin render pass on command buffer from a different frame");
        
        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = Swapchain->getRenderPass();
        renderPassBeginInfo.framebuffer = Swapchain->getFrameBuffer(currentImageIndex);
            
        renderPassBeginInfo.renderArea.offset = { 0, 0 };
        renderPassBeginInfo.renderArea.extent = Swapchain->getSwapChainExtent();
            
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();
            
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(Swapchain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(Swapchain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, Swapchain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        
    }

    void ZRenderer::endSwapchainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Cant call endSwapchain if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && 
            "Cant end render pass on command buffer from a different frame");
        
        vkCmdEndRenderPass(commandBuffer);
    }
}
