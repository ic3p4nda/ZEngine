#pragma once

#include "camera.h"

#include <vulkan/vulkan.h>

namespace ZEngine
{
    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        ZCamera &camera;
    };
}