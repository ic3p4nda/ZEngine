#pragma once

#include "camera.h"
#include "game_object.h"

#include <vulkan/vulkan.h>

namespace ZEngine
{
    
#define MAX_LIGHTS 10
    
    struct PointLight
    {
        glm::vec4 position{}; // ignore W
        glm::vec4 color{};
    };
    
    struct GlobalUbo
    {
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
        glm::vec4 ambientColor{1.0f, 1.0f, 1.0f, 0.02f};
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };
    
    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        ZCamera &camera;
        VkDescriptorSet globalDescriptorSet;
        ZGameObject::MAP &gameObjects;
    };
}