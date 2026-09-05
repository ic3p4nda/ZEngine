#include "Systems/render_system.h"

#include <array>
#include <iostream>
#include <stdexcept>

#include "app.h"

namespace z_engine
{
    ZRenderSystem::ZRenderSystem(class ZDevice& device, VkRenderPass renderpass, VkDescriptorSetLayout globalSetLayout,
                                 VkDescriptorSetLayout textureSetLayout) : Device(device)
    {
        createPipelineLayout(globalSetLayout, textureSetLayout);
        createPipeline(renderpass);
    }

    ZRenderSystem::~ZRenderSystem()
    {
        vkDestroyPipelineLayout(Device.device(), pipelineLayout, nullptr);
    }

    void ZRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout,
                                             VkDescriptorSetLayout textureSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout, textureSetLayout};

        VkPipelineLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        layoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
        layoutCreateInfo.pushConstantRangeCount = 1;
        layoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(Device.device(), &layoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");
    }

    void ZRenderSystem::createPipeline(VkRenderPass renderpass)
    {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        ZPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderpass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<ZPipeline>(Device,
                                               pipelineConfig,
                                               VERTEXSHADERPATH,
                                               FRAGSHADERPATH);
    }

    void ZRenderSystem::render(FrameInfo& frameinfo, VkDescriptorSet textureDescriptorSet)
    {
        pipeline->bind(frameinfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameinfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1,
            &frameinfo.globalDescriptorSet,
            0, nullptr
        );

        vkCmdBindDescriptorSets(
            frameinfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
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
                               pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(SimplePushConstantData),
                               &push);
            object.model->bind(frameinfo.commandBuffer);
            object.model->draw(frameinfo.commandBuffer);
        }
    }
}
