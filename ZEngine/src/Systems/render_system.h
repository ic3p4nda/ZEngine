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
        ZRenderSystem(ZDevice& device, VkFormat color_format, VkFormat depth_format, VkDescriptorSetLayout globalSetLayout,
                      VkDescriptorSetLayout textureSetLayout);
        ~ZRenderSystem();

        ZRenderSystem(const ZRenderSystem&) = delete;
        ZRenderSystem& operator=(const ZRenderSystem&) = delete;

        void render(FrameInfo& frameinfo, VkDescriptorSet textureDescriptorSet);

    private:
        const std::string vertexshaderpath_ = "C:/_dev/ZEngine/shaders/shader.vert.spv";
        const std::string fragshaderpath_ = "C:/_dev/ZEngine/shaders/shader.frag.spv";

        void CreatePipelineLayout(VkDescriptorSetLayout global_set_layout, VkDescriptorSetLayout texture_set_layout);
        void CreatePipeline(VkFormat color_format, VkFormat depth_format);

        ZDevice& device_;

        std::unique_ptr<ZPipeline> pipeline_;
        VkPipelineLayout pipeline_layout_;
    };
}
