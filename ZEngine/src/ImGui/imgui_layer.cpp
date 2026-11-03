#include "imgui_layer.h"

#include <stdexcept>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace z_engine
{
    ZImguiLayer::ZImguiLayer(ZDevice& device, VkFormat color_format, VkFormat depth_format, GLFWwindow* window) : _device(device)
    {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window, true);

        VkPipelineRenderingCreateInfo pipeline_rendering_info {};
        pipeline_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipeline_rendering_info.colorAttachmentCount = 1;
        pipeline_rendering_info.pColorAttachmentFormats = &color_format;
        pipeline_rendering_info.depthAttachmentFormat = depth_format;

        ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
        pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineInfo.PipelineRenderingCreateInfo = pipeline_rendering_info; 
        
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = device.instance();
        initInfo.PhysicalDevice = device.PhysicalDevice();
        initInfo.Device = device.device();
        initInfo.QueueFamily = device.FindPhysicalQueueFamilies().graphics_family;
        initInfo.Queue = device.GraphicsQueue();
        // initInfo.DescriptorPool = _imguiPool;
        initInfo.DescriptorPoolSize = 64;
        initInfo.MinImageCount = 3;
        initInfo.ImageCount = 3;
        initInfo.UseDynamicRendering = true;
        initInfo.PipelineInfoMain = pipelineInfo;

        ImGui_ImplVulkan_Init(&initInfo);
    }

    ZImguiLayer::~ZImguiLayer()
    {
        vkDeviceWaitIdle(_device.device());
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
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
}
