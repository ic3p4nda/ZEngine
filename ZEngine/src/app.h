#pragma once

#include "window.h"
#include "game_object.h"
#include "device.h"
#include "renderer.h"
#include "descriptors.h"
#include "ImGui/imgui_layer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>
#include <vector>


#define MAX_FRAME_TIME 0.5f

namespace z_engine
{
    class ZApp
    {
    public:
        static constexpr int width_ = 800;
        static constexpr int height_ = 600;

        ZApp();
        ~ZApp();

        ZApp(const ZApp&) = delete;
        ZApp& operator=(const ZApp&) = delete;

        void run();

    private:
        void LoadGameObjects();

        ZWindow window_{width_, height_, "HelloWorld"};
        ZDevice device_{window_};
        ZRenderer renderer_{window_, device_};
        ZImguiLayer imgui_layer_{device_, renderer_.getSwapchainRenderPass(), window_.getGLFWwindow()};

        std::unique_ptr<ZDescriptorPool> global_pool_{};
        ZGameObject::MAP game_objects_;
    };
}
