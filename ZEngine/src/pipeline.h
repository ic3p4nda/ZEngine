#pragma once

#include "device.h"

#include <string>
#include <vector>

namespace z_engine
{
    struct PipelineConfigInfo
    {
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        std::vector<VkVertexInputBindingDescription> binding_descriptions{};
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
        VkPipelineViewportStateCreateInfo viewport_info;
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
        VkPipelineRasterizationStateCreateInfo rasterization_info;
        VkPipelineMultisampleStateCreateInfo multisample_info;
        VkPipelineColorBlendAttachmentState color_blend_attachment;
        VkPipelineColorBlendStateCreateInfo color_blend_info;
        VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
        std::vector<VkDynamicState> dynamic_state_enables;
        VkPipelineDynamicStateCreateInfo dynamic_state_info;
        VkPipelineLayout pipeline_layout = nullptr;
        VkFormat color_attachement_format = VK_FORMAT_UNDEFINED;
        VkFormat depth_attachement_format = VK_FORMAT_UNDEFINED;
        uint32_t subpass = 0;
    };

    class ZPipeline
    {
    public:
        ZPipeline(ZDevice& device,
                  const PipelineConfigInfo& configInfo,
                  const std::string& vertFilepath,
                  const std::string& fragFilepath);

        ~ZPipeline();

        ZPipeline(const ZPipeline&) = delete;
        ZPipeline& operator=(const ZPipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
        static void enableAlphaBlending(PipelineConfigInfo& configInfo);

    private:
        static std::vector<char> readFile(const std::string& filepath);

        void createGraphicsPipeline(const std::string& vertFilepath,
                                    const std::string& fragFilepath,
                                    const PipelineConfigInfo& configInfo);

        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        ZDevice& device;
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
}
