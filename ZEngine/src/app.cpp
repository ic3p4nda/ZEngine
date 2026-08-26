#include "app.h"

#include "camera.h"
#include "keyboard_movement_controller.h"
#include "buffer.h"
#include "Systems/render_system.h"
#include "Systems/point_light_system.h"

#include <imgui.h>

#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>

namespace ZEngine
{
    
    
    ZApp::ZApp()
    {
        globalPool = ZDescriptorPool::Builder(Device)
            .setMaxSets(ZSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ZSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        
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
        
        auto globalSetLayout = ZDescriptorSetLayout::Builder(Device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();
        
        std::vector<VkDescriptorSet> globalDescriptorSets(ZSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            ZDescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
        }
        
        std::cout << "MaxPushConstantSize = " << Device.properties.limits.maxPushConstantsSize << "\n";
        
        ZRenderSystem simpleRenderSystem{Device,
            Renderer.getSwapchainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };
        ZPointLightSystem pointLightSystem{Device,
            Renderer.getSwapchainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };
        ZCamera camera{};
        camera.setViewTarget(glm::vec3(-1.0f, -2.0f, -2.0f), glm::vec3(0.0f, 0.0f, 2.5f));
        
        auto viewerObject = ZGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        ZKeyboardMovementController cameraController{Window.getGLFWwindow()};
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        
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
                camera,
                    globalDescriptorSets[frameIndex],
                    gameObjects
                };
                
                // Update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                // uboBuffers[frameIndex]->flush();
                
                //Render
                Renderer.beginSwapchainRenderPass(commandBuffer);
                simpleRenderSystem.render(frameInfo);
                pointLightSystem.render(frameInfo);
                
                // ImGui
                ImguiLayer.newFrame();
                ImGui::Begin("Stats");
                ImGui::Text("FrameTime = %f", frameTime);
                ImGui::DragFloat("Sens", &cameraController.mouseSensitivity, 0.01f);
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
        ZModel::createModelFromFile(Device, "C:/_dev/ZEngine/models/smooth_vase.obj");
        auto flatVase = ZGameObject::createGameObject();
        flatVase.model = lveModel;
        flatVase.transform.translation = {-.5f, .5f, 0.f};
        flatVase.transform.scale = glm::vec3(2.5f);
        gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        lveModel = ZModel::createModelFromFile(Device, "C:/_dev/ZEngine/models/smooth_vase.obj");
        auto smoothVase = ZGameObject::createGameObject();
        smoothVase.model = lveModel;
        smoothVase.transform.translation = {.5f, .5f, 0.f};
        smoothVase.transform.scale = glm::vec3(2.5f);
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));
        
        lveModel = ZModel::createModelFromFile(Device, "C:/_dev/ZEngine/models/quad.obj");
        auto floor = ZGameObject::createGameObject();
        floor.model = lveModel;
        floor.transform.translation = {0.0f, .5f, 0.f};
        floor.transform.scale = glm::vec3(3.0f);
        gameObjects.emplace(floor.getId(), std::move(floor));
        
        std::vector<glm::vec3> lightColors{
          {1.f, .1f, .1f},
          {.1f, .1f, 1.f},
          {.1f, 1.f, .1f},
          {1.f, 1.f, .1f},
          {.1f, 1.f, 1.f},
          {1.f, 1.f, 1.f}  //
        };
        
        for (int i = 0; i < lightColors.size(); i++)
        {
            auto pointLight = ZGameObject::makePointLight(0.5f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(glm::mat4(1.0f),
                (i * glm::two_pi<float>() ) / lightColors.size(),
                {0.0f, -1.0f, 0.0f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }
}
