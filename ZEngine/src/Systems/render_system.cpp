#include "Systems/render_system.h"

#include <array>
#include <iostream>
#include <stdexcept>

#include "app.h"

namespace z_engine
{
    ZRenderSystem::ZRenderSystem(class ZDevice& device, VkFormat color_format, VkFormat depth_format, VkDescriptorSetLayout globalSetLayout,
                                 VkDescriptorSetLayout textureSetLayout) : device_(device)
    {
        CreatePipelineLayout(globalSetLayout, textureSetLayout);
        CreatePipeline(color_format, depth_format);
    }

    ZRenderSystem::~ZRenderSystem()
    {
        vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
    }

    void ZRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout global_set_layout,
                                             VkDescriptorSetLayout texture_set_layout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{global_set_layout, texture_set_layout};

        VkPipelineLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        layoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
        layoutCreateInfo.pushConstantRangeCount = 1;
        layoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(device_.device(), &layoutCreateInfo, nullptr, &pipeline_layout_) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");
    }

    void ZRenderSystem::CreatePipeline(VkFormat color_format, VkFormat depth_format)
    {
        assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        ZPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.color_attachement_format = color_format;
        pipelineConfig.depth_attachement_format = depth_format;
        pipelineConfig.pipeline_layout = pipeline_layout_;
        pipeline_ = std::make_unique<ZPipeline>(device_,
                                               pipelineConfig,
                                               vertexshaderpath_,
                                               fragshaderpath_);
    }

    void ZRenderSystem::render(FrameInfo& frameinfo, VkDescriptorSet textureDescriptorSet)
    {
        pipeline_->bind(frameinfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameinfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout_,
            0, 1,
            &frameinfo.globalDescriptorSet,
            0, nullptr
        );

        vkCmdBindDescriptorSets(
            frameinfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout_,
            1, 1,
            &textureDescriptorSet,
            0, nullptr
        );

        for (auto& kv : frameinfo.gameObjects)
        {
            auto& object = kv.second;
            if (object.model == nullptr) continue;
            SimplePushConstantData push{};
            push.modelMatrix = object.transform.mat4();
            push.normalMatrix = object.transform.normalMatrix();

            vkCmdPushConstants(frameinfo.commandBuffer,
                               pipeline_layout_,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(SimplePushConstantData),
                               &push);
            object.model->bind(frameinfo.commandBuffer);
            object.model->draw(frameinfo.commandBuffer);
        }
    }
}
