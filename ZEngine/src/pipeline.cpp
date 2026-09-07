#include "pipeline.h"
#include "model.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cassert>

namespace z_engine
{
    ZPipeline::ZPipeline(ZDevice& device,
                         const PipelineConfigInfo& configInfo,
                         const std::string& vertFilepath,
                         const std::string& fragFilepath) : device{device}
    {
        createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
    }

    ZPipeline::~ZPipeline()
    {
        vkDestroyShaderModule(device.device(), vertShaderModule, nullptr);
        vkDestroyShaderModule(device.device(), fragShaderModule, nullptr);
        vkDestroyPipeline(device.device(), graphicsPipeline, nullptr);
    }

    void ZPipeline::bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }

    void ZPipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo)
    {
        configInfo.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.input_assembly_info.primitiveRestartEnable = VK_FALSE;

        configInfo.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewport_info.viewportCount = 1;
        configInfo.viewport_info.pViewports = nullptr;
        configInfo.viewport_info.scissorCount = 1;
        configInfo.viewport_info.pScissors = nullptr;

        configInfo.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterization_info.depthClampEnable = VK_FALSE;
        configInfo.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterization_info.lineWidth = 1.0f;
        configInfo.rasterization_info.cullMode = VK_CULL_MODE_NONE;
        configInfo.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        configInfo.rasterization_info.depthBiasEnable = VK_FALSE;
        configInfo.rasterization_info.depthBiasConstantFactor = 0.0f;
        configInfo.rasterization_info.depthBiasClamp = 0.0f;
        configInfo.rasterization_info.depthBiasSlopeFactor = 0.0f;

        configInfo.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisample_info.sampleShadingEnable = VK_FALSE;
        configInfo.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisample_info.minSampleShading = 1.0f;
        configInfo.multisample_info.pSampleMask = nullptr;
        configInfo.multisample_info.alphaToCoverageEnable = VK_FALSE;
        configInfo.multisample_info.alphaToOneEnable = VK_FALSE;

        configInfo.color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        configInfo.color_blend_attachment.blendEnable = VK_FALSE;
        configInfo.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        configInfo.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        configInfo.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        configInfo.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

        configInfo.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.color_blend_info.logicOpEnable = VK_FALSE;
        configInfo.color_blend_info.logicOp = VK_LOGIC_OP_COPY;
        configInfo.color_blend_info.attachmentCount = 1;
        configInfo.color_blend_info.pAttachments = &configInfo.color_blend_attachment;
        configInfo.color_blend_info.blendConstants[0] = 0.0f;
        configInfo.color_blend_info.blendConstants[1] = 0.0f;
        configInfo.color_blend_info.blendConstants[2] = 0.0f;
        configInfo.color_blend_info.blendConstants[3] = 0.0f;

        configInfo.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depth_stencil_info.depthTestEnable = VK_TRUE;
        configInfo.depth_stencil_info.depthWriteEnable = VK_TRUE;
        configInfo.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
        configInfo.depth_stencil_info.minDepthBounds = 0.0f;
        configInfo.depth_stencil_info.maxDepthBounds = 1.0f;
        configInfo.depth_stencil_info.stencilTestEnable = VK_FALSE;
        configInfo.depth_stencil_info.front = {};
        configInfo.depth_stencil_info.back = {};

        configInfo.dynamic_state_enables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        configInfo.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.dynamic_state_info.pDynamicStates = configInfo.dynamic_state_enables.data();
        configInfo.dynamic_state_info.dynamicStateCount =
            static_cast<uint32_t>(configInfo.dynamic_state_enables.size());
        configInfo.dynamic_state_info.flags = 0;

        configInfo.binding_descriptions = ZModel::Vertex::getBindingDescriptions();
        configInfo.attribute_descriptions = ZModel::Vertex::getAttributeDescriptions();
    }

    void ZPipeline::enableAlphaBlending(PipelineConfigInfo& configInfo)
    {
        configInfo.color_blend_attachment.blendEnable = VK_TRUE;

        configInfo.color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        configInfo.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        configInfo.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        configInfo.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        configInfo.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        configInfo.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }

    std::vector<char> ZPipeline::readFile(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file " + filepath);
        }

        size_t fileSize = file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    void ZPipeline::createGraphicsPipeline(const std::string& vertFilepath,
                                           const std::string& fragFilepath,
                                           const PipelineConfigInfo& configInfo)
    {
        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);

        assert(
            configInfo.pipeline_layout != VK_NULL_HANDLE &&
            "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");

        createShaderModule(vertCode, &vertShaderModule);
        createShaderModule(fragCode, &fragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2];
        // VertexShader
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;
        // FragmentShader
        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        auto& bindingDescriptions = configInfo.binding_descriptions;
        auto& attributeDescriptions = configInfo.attribute_descriptions;
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

        VkPipelineRenderingCreateInfo renderingCreateInfo {};
        renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderingCreateInfo.colorAttachmentCount = 1;
        renderingCreateInfo.pColorAttachmentFormats = &configInfo.color_attachement_format;
        renderingCreateInfo.depthAttachmentFormat = configInfo.depth_attachement_format;
        
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = &renderingCreateInfo;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.input_assembly_info;
        pipelineInfo.pViewportState = &configInfo.viewport_info;
        pipelineInfo.pRasterizationState = &configInfo.rasterization_info;
        pipelineInfo.pMultisampleState = &configInfo.multisample_info;
        pipelineInfo.pColorBlendState = &configInfo.color_blend_info;
        pipelineInfo.pDepthStencilState = &configInfo.depth_stencil_info;
        pipelineInfo.pDynamicState = &configInfo.dynamic_state_info;
        
        pipelineInfo.layout = configInfo.pipeline_layout;
        pipelineInfo.renderPass = VK_NULL_HANDLE;
        pipelineInfo.subpass = configInfo.subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create graphics pipeline");
        }
    }

    void ZPipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create shader module");
        }
    }
}
