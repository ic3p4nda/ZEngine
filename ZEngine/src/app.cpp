#include "app.h"

#include "camera.h"
#include "keyboard_movement_controller.h"
#include "buffer.h"

#include <imgui.h>

#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>

namespace ZEngine
{
    struct GlobalUbo
    {
        glm::mat4 projectionView{1.0f};
        glm::vec3 lightDirection = glm::vec3(1.0f, -3.0f, -1.0f);
    };
    
    ZApp::ZApp()
    {
        LoadGameObjects();
    }

    ZApp::~ZApp(){}

    void ZApp::run()
    {
        std::vector<std::unique_ptr<ZBuffer>> uboBuffers(ZSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<ZBuffer>(
                Device,
                    sizeof(GlobalUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            uboBuffers[i]->map();
        }
        
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
                int frameIndex = Renderer.getFrameIndex();
                FrameInfo frameInfo{
                frameIndex,
                frameTime,
                commandBuffer,
                camera
                };
                
                // Update
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                // uboBuffers[frameIndex]->flush();
                
                //Render
                Renderer.beginSwapchainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
                
                // ImGui
                ImguiLayer.newFrame();
                ImGui::Begin("Stats");
                ImGui::Text("FrameTime = %f", frameTime);
                ImGui::DragFloat("Mouse Sensitivity: ", &cameraController.mouseSensitivity, 0.01f);
                for (size_t i = 0; i < gameObjects.size(); i++) {
                    ImGui::PushID((int)i);
                    ImGui::Text("Object %zu", i);
                    ImGui::DragFloat3("Position", &gameObjects[i].transform.translation.x, 0.005f);
                    glm::vec3 rotationDegrees = glm::degrees(gameObjects[i].transform.rotation);
                    if (ImGui::DragFloat3("Rotation", &rotationDegrees.x, 1.0f)) {
                        gameObjects[i].transform.rotation = glm::radians(rotationDegrees);
                    }
                    ImGui::PopID();
                }
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
        std::shared_ptr<ZModel> lveModel =
        ZModel::createModelFromFile(Device, "C:/_dev/ZEngine/models/flat_vase.obj");
        auto flatVase = ZGameObject::createGameObject();
        flatVase.model = lveModel;
        flatVase.transform.translation = {-.5f, .5f, 0.f};
        flatVase.transform.scale = glm::vec3(2.5f);
        gameObjects.push_back(std::move(flatVase));

        lveModel = ZModel::createModelFromFile(Device, "C:/_dev/ZEngine/models/smooth_vase.obj");
        auto smoothVase = ZGameObject::createGameObject();
        smoothVase.model = lveModel;
        smoothVase.transform.translation = {.5f, .5f, 0.f};
        smoothVase.transform.scale = glm::vec3(2.5f);
        gameObjects.push_back(std::move(smoothVase));
    }
}
