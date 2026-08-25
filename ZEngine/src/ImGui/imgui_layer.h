#pragma once

#include "device.h"
#include "swapchain.h"

namespace ZEngine
{
    class ZImguiLayer
    {
    public:
        ZImguiLayer(ZDevice& device, VkRenderPass renderpass, GLFWwindow* window);
        ~ZImguiLayer();
        
        ZImguiLayer(const ZImguiLayer &) = delete;
        ZImguiLayer &operator=(const ZImguiLayer &) = delete;

        void newFrame();
        void render(VkCommandBuffer commandBuffer);
    
    private:
        void createDescriptorPool();
        
        ZDevice& _device;
        VkDescriptorPool _imguiPool;
    };
}
