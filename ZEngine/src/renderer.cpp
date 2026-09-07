#include "renderer.h"

#include <array>
#include <iostream>
#include <stdexcept>

namespace z_engine
{
    ZRenderer::ZRenderer(class ZWindow& window, class ZDevice& device) : window_(window), device_(device)
    {
        RecreateSwapchain();
        CreateCommandBuffers();
    }

    ZRenderer::~ZRenderer()
    {
        FreeCommandBuffers();
    }

    void ZRenderer::CreateCommandBuffers()
    {
        command_buffers_.resize(swapchain_->MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = device_.GetCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

        if (vkAllocateCommandBuffers(device_.device(), &allocInfo, command_buffers_.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void ZRenderer::FreeCommandBuffers()
    {
        vkFreeCommandBuffers(device_.device(), device_.GetCommandPool(),
                             static_cast<uint32_t>(command_buffers_.size()),
                             command_buffers_.data());

        command_buffers_.clear();
    }

    void ZRenderer::RecreateSwapchain()
    {
        auto extent = window_.getExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = window_.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device_.device());

        if (swapchain_ == nullptr)
        {
            swapchain_ = std::make_unique<ZSwapChain>(device_, extent);
        }
        else
        {
            std::shared_ptr<ZSwapChain> oldSwapChain = std::move(swapchain_);
            swapchain_ = std::make_unique<ZSwapChain>(device_, extent, oldSwapChain);

            if (!oldSwapChain->CompareSwapFormates(*swapchain_.get()))
            {
                throw std::runtime_error("Swao chain image or depth format has changed");
            }
        }
    }

    VkCommandBuffer ZRenderer::BeginFrame()
    {
        assert(!is_frame_started_ && "Cant call beginFrame while already in progress");
        auto result = swapchain_->AcquireNextImage(&current_image_index_);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapchain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swapchain image!");
        }

        is_frame_started_ = true;

        VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        return commandBuffer;
    }

    void ZRenderer::EndFrame()
    {
        assert(is_frame_started_ && "Cant call endFrame while not in progress");

        auto commandBuffer = GetCurrentCommandBuffer();
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }

        auto result = swapchain_->SubmitCommandBuffers(&commandBuffer, &current_image_index_);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window_.wasWindowReized())
        {
            window_.resetWindowResizeFlag();
            RecreateSwapchain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swapchain image!");
        }

        is_frame_started_ = false;
        current_frame_index_ = (current_frame_index_ + 1) % swapchain_->MAX_FRAMES_IN_FLIGHT;
    }

    void ZRenderer::BeginSwapchainRenderPass(VkCommandBuffer command_buffer)
    {
        assert(is_frame_started_ && "Cant call beginSwapchain if frame is not in progress");
        assert(command_buffer == GetCurrentCommandBuffer() &&
            "Cant begin render pass on command buffer from a different frame");
        
        TransitionImageLayout(
            command_buffer,
            swapchain_->GetSwapChainImage(current_image_index_),
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT
        );
        
        TransitionImageLayout(
            command_buffer,
            swapchain_->GetDepthImage(current_image_index_),
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_IMAGE_ASPECT_DEPTH_BIT
        );
        
        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = swapchain_->GetImageView(current_image_index_);
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = {0.01f, 0.01f, 0.01f, 1.0f};

        VkRenderingAttachmentInfo depthAttachment{};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = swapchain_->GetDepthImageView(current_image_index_);
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.clearValue.depthStencil = {1.0f, 0};

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea = {{0, 0}, swapchain_->GetSwapChainExtent()};
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachment;

        vkCmdBeginRendering(command_buffer, &renderingInfo);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchain_->GetSwapChainExtent().width);
        viewport.height = static_cast<float>(swapchain_->GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, swapchain_->GetSwapChainExtent()};
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    }

    void ZRenderer::EndSwapchainRenderPass(VkCommandBuffer command_buffer)
    {
        assert(is_frame_started_ && "Cant call endSwapchain if frame is not in progress");
        assert(command_buffer == GetCurrentCommandBuffer() &&
            "Cant end render pass on command buffer from a different frame");

        vkCmdEndRendering(command_buffer);

        TransitionImageLayout(command_buffer,
            swapchain_->GetSwapChainImage(current_image_index_),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_ASPECT_COLOR_BIT
            );
    }
    
    void ZRenderer::TransitionImageLayout(
        VkCommandBuffer command_buffer,
        VkImage image,
        VkImageLayout old_layout,
        VkImageLayout new_layout,
        VkImageAspectFlags aspect_mask)
    {
        VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspect_mask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;
        source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destination_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        command_buffer,
        source_stage, destination_stage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    }
}
