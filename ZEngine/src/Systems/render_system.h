#pragma once

#include "pipeline.h"
#include "camera.h"
#include "game_object.h"
#include "device.h"
#include "model.h"
#include "frame_info.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>
#include <vector>

namespace z_engine
{
    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };

    class ZRenderSystem
    {
    public:
        ZRenderSystem(ZDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout,
                      VkDescriptorSetLayout textureSetLayout);
        ~ZRenderSystem();

        ZRenderSystem(const ZRenderSystem&) = delete;
        ZRenderSystem& operator=(const ZRenderSystem&) = delete;

        void render(FrameInfo& frameinfo, VkDescriptorSet textureDescriptorSet);

    private:
        const std::string VERTEXSHADERPATH = "C:/_dev/ZEngine/shaders/shader.vert.spv";
        const std::string FRAGSHADERPATH = "C:/_dev/ZEngine/shaders/shader.frag.spv";

        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
        void createPipeline(VkRenderPass renderPass);

        ZDevice& Device;

        std::unique_ptr<ZPipeline> pipeline;
        VkPipelineLayout pipelineLayout;
    };
}
