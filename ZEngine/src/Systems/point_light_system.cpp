#include "Systems\point_light_system.h"

#include <array>
#include <iostream>
#include <map>
#include <stdexcept>

#include "app.h"

namespace ZEngine
{
    struct PointLightPushConstants
    {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };
    
    ZPointLightSystem::ZPointLightSystem(ZDevice& device, VkRenderPass renderpass, VkDescriptorSetLayout globalSetLayout) : Device(device)
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderpass);
    }

    ZPointLightSystem::~ZPointLightSystem()
    {
        vkDestroyPipelineLayout(Device.device(), pipelineLayout, nullptr);
    }
    
    void ZPointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstants);
        
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};
        
        VkPipelineLayoutCreateInfo layoutCreateInfo {};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        layoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
        layoutCreateInfo.pushConstantRangeCount = 1;
        layoutCreateInfo.pPushConstantRanges = &pushConstantRange;
        
        if (vkCreatePipelineLayout(Device.device(), &layoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void ZPointLightSystem::createPipeline(VkRenderPass renderpass)
    {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
        
        PipelineConfigInfo pipelineConfig{};
        ZPipeline::defaultPipelineConfigInfo(pipelineConfig);
        ZPipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.renderPass = renderpass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<ZPipeline>(Device,
            pipelineConfig,
            VERTEXSHADERPATH,
            FRAGSHADERPATH);
    }

    void ZPointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo)
    {
        auto rotateLight = glm::rotate(glm::mat4(1.0f),
                frameInfo.frameTime,
                {0.0f, -1.0f, 0.0f});
        
        int lightIndex = 0;
        for (auto& kv : frameInfo.gameObjects)
        {
            auto& obj = kv.second;
            if (obj.pointLight == nullptr) continue;
            
            assert(lightIndex < MAX_LIGHTS && "Point Lights exceed maximum specified");
            
            obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.0f));
            
            ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.0f);
            ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            
            lightIndex++;
        }
        ubo.numLights = lightIndex;
    }

    void ZPointLightSystem::render(FrameInfo &frameinfo)
    {
        std::map<float, ZGameObject::id_t> sorted;
        for (auto& kv : frameinfo.gameObjects)
        {
            auto& obj = kv.second;
            if (obj.pointLight == nullptr) continue;
            
            auto offset = frameinfo.camera.getPosition() - obj.transform.translation;
            float disSquared = glm::dot(offset, offset);
            sorted[disSquared] = obj.getId();
        }
        
        pipeline->bind(frameinfo.commandBuffer);
        
        vkCmdBindDescriptorSets(
            frameinfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1,
            &frameinfo.globalDescriptorSet,
            0, nullptr
            );
     
        for (auto it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            auto& obj = frameinfo.gameObjects.at(it->second);
            
            PointLightPushConstants pushConstant{};
            pushConstant.position = glm::vec4(obj.transform.translation, 1.0f);
            pushConstant.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            pushConstant.radius = obj.transform.scale.x;
            
            vkCmdPushConstants(
                frameinfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0, sizeof(PointLightPushConstants),
                &pushConstant);
            
            vkCmdDraw(frameinfo.commandBuffer, 6, 1, 0, 0);
        }
    }
}
