#include "Systems\render_system.h"

#include <array>
#include <iostream>
#include <stdexcept>

#include "app.h"

namespace ZEngine
{
    ZRenderSystem::ZRenderSystem(class ZDevice& device, VkRenderPass renderpass) : Device(device)
    {
        createPipelineLayout();
        createPipeline(renderpass);
    }

    ZRenderSystem::~ZRenderSystem()
    {
        vkDestroyPipelineLayout(Device.device(), pipelineLayout, nullptr);
    }
    
    void ZRenderSystem::createPipelineLayout()
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);
        
        VkPipelineLayoutCreateInfo layoutCreateInfo {};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.setLayoutCount = 0;
        layoutCreateInfo.pSetLayouts = nullptr;
        layoutCreateInfo.pushConstantRangeCount = 1;
        layoutCreateInfo.pPushConstantRanges = &pushConstantRange;
        
        if (vkCreatePipelineLayout(Device.device(), &layoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
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
            vertexShaderPath,
            fragShaderPath);
    }

    void ZRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<ZGameObject>& gameObjects, const ZCamera& camera)
    {
        pipeline->bind(commandBuffer);
        auto projectionView = camera.getProjection() * camera.getView();
        
        for (auto& object : gameObjects)
        {     
            SimplePushConstantData push{};
            auto modelMatrix = object.transform.mat4();
            push.transform = projectionView * modelMatrix;
            push.normalMatrix = object.transform.normalMatrix();
            
            vkCmdPushConstants(commandBuffer, 
               pipelineLayout,
               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
               0,
               sizeof(SimplePushConstantData),
               &push);
            object.model->bind(commandBuffer);
            object.model->draw(commandBuffer);
        }
    }
}
