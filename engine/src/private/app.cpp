#include "app.h"

#include "camera.h"
#include "keyboard_movement_controller.h"

#include <imgui.h>

#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>

namespace ZEngine
{
    ZApp::ZApp()
    {
        LoadGameObjects();
    }

    ZApp::~ZApp(){}

    void ZApp::run()
    {
        std::cout << "MaxPushConstantSize = " << Device.properties.limits.maxPushConstantsSize << "\n";
        
        ZRenderSystem simpleRenderSystem{Device, Renderer.getSwapchainRenderPass()};
        ZCamera camera{};
        camera.setViewTarget(glm::vec3(-1.0f, -2.0f, -2.0f), glm::vec3(0.0f, 0.0f, 2.5f));
        
        auto viewerObject = ZGameObject::createGameObject();
        ZKeyboardMovementController cameraController{Window.getGLFWwindow()};
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        
        ImGui::CreateContext();
        
        while (!Window.shouldClose()){
            glfwPollEvents();
            
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float>(newTime - currentTime).count();
            currentTime = newTime;
        
            frameTime = std::min(frameTime, MAX_FRAME_TIME);
            
            cameraController.moveInPlaneXZ(Window, frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
            
            float aspect = Renderer.getAspectRatio();
            
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.0f);
            
            if (auto commandBuffer = Renderer.beginFrame())
            {
                Renderer.beginSwapchainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
                
                glm::vec3 clear_color;
                
                // ImGui
                ImguiLayer.newFrame();
                ImGui::Begin("Stats");
                ImGui::Text("FrameTime = %f", frameTime);
                ImGui::ColorEdit3("clear color", (float*)&clear_color);
                ImGui::
                ImGui::End();
                ImguiLayer.render(commandBuffer);
                
                Renderer.endSwapchainRenderPass(commandBuffer);
                Renderer.endFrame();
            }
        }
        
        vkDeviceWaitIdle(Device.device());
    }

    void ZApp::LoadGameObjects()
    {
        std::shared_ptr<ZModel> model = 
            ZModel::createModelFromFile(Device, "C:/_dev/engine/models/flat_vase.obj");
        
        auto gameObj = ZGameObject::createGameObject();
        gameObj.model = model;
        gameObj.transform.translation = {0.0f, 0.5f, 2.5f};
        gameObj.transform.scale = glm::vec3(1.5f);
        gameObjects.push_back(std::move(gameObj));
    }
}
