#include "imgui_layer.h"

#include <stdexcept>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace ZEngine
{
    ZImguiLayer::ZImguiLayer(ZDevice& device, VkRenderPass renderpass, GLFWwindow* window) : _device(device)
    {
        createDescriptorPool();
        
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window, true);

        ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
        pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineInfo.RenderPass = renderpass;
        pipelineInfo.Subpass = 0;

        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = device.instance();
        initInfo.PhysicalDevice = device.physicalDevice();
        initInfo.Device = device.device();
        initInfo.QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily;
        initInfo.Queue = device.graphicsQueue();
        // initInfo.DescriptorPool = _imguiPool;
        initInfo.DescriptorPoolSize = 64;
        initInfo.MinImageCount = 3;
        initInfo.ImageCount = 3;
        initInfo.UseDynamicRendering = false;
        initInfo.PipelineInfoMain = pipelineInfo;

        ImGui_ImplVulkan_Init(&initInfo);
        
    }

    ZImguiLayer::~ZImguiLayer()
    {
        vkDeviceWaitIdle(_device.device());
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(_device.device(), _imguiPool, nullptr);
    }

    void ZImguiLayer::newFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ZImguiLayer::render(VkCommandBuffer commandBuffer)
    {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }

    void ZImguiLayer::createDescriptorPool()
    {
        VkDescriptorPoolSize poolSizes[]
        { {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }};
        
        VkDescriptorPoolCreateInfo poolinfo{};
        poolinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolinfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolinfo.maxSets = 1;
        poolinfo.poolSizeCount = 1;
        poolinfo.pPoolSizes = poolSizes;
        
        if (vkCreateDescriptorPool(_device.device(), &poolinfo, nullptr, &_imguiPool))
        {
            throw std::runtime_error("failed to create descriptor pool!");
        };
    }
    
}
