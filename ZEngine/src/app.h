#pragma once

#include "window.h"
#include "game_object.h"
#include "device.h"
#include "renderer.h"
#include "ImGui/imgui_layer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>
#include <vector>

#include "Systems/render_system.h"

#define MAX_FRAME_TIME 0.5f

namespace ZEngine {
    class ZApp
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        
        ZApp();
        ~ZApp();
        
        ZApp(const ZApp&) = delete;
        ZApp &operator=(const ZApp&) = delete;
        
        void run();
    
    private:
        void LoadGameObjects();
        
        ZWindow Window{WIDTH , HEIGHT , "HelloWorld"};
        ZDevice Device{Window};
        ZRenderer Renderer{Window, Device};
        ZImguiLayer ImguiLayer{Device, Renderer.getSwapchainRenderPass(), Window.getGLFWwindow()};
        
        std::vector<ZGameObject> gameObjects;
    };
    
}
