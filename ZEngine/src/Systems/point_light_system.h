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

namespace ZEngine {
    class ZPointLightSystem
    {
    public:
        ZPointLightSystem(ZDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~ZPointLightSystem();
        
        ZPointLightSystem(const ZPointLightSystem&) = delete;
        ZPointLightSystem &operator=(const ZPointLightSystem&) = delete;
    
        void update(FrameInfo& frameInfo, GlobalUbo& ubo);
        
        void render(FrameInfo &frameinfo);
    
    private:
        const std::string VERTEXSHADERPATH = "C:/_dev/ZEngine/shaders/point_light.vert.spv";
        const std::string FRAGSHADERPATH = "C:/_dev/ZEngine/shaders/point_light.frag.spv";
        
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);
        
        ZDevice& Device;
        
        std::unique_ptr<ZPipeline> pipeline;
        VkPipelineLayout pipelineLayout;
    };
    
}