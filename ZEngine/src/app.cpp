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

namespace z_engine
{
    ZApp::ZApp()
    {
        global_pool_ = ZDescriptorPool::Builder(device_)
                     .SetMaxSets(ZSwapChain::MAX_FRAMES_IN_FLIGHT + 1)
                     .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ZSwapChain::MAX_FRAMES_IN_FLIGHT)
                     .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
                     .build();

        LoadGameObjects();
    }

    ZApp::~ZApp()
    {
    }

    void ZApp::run()
    {
        std::vector<std::unique_ptr<ZBuffer>> ubo_buffers(ZSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < ubo_buffers.size(); i++)
        {
            ubo_buffers[i] = std::make_unique<ZBuffer>(
                device_,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            ubo_buffers[i]->map();
        }

        auto global_set_layout = ZDescriptorSetLayout::Builder(device_)
                               .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                               .build();

        auto texture_set_layout = ZDescriptorSetLayout::Builder(device_)
                                .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                .build();

        auto brick_texture = std::make_shared<ZTexture>(device_, "C:/_dev/ZEngine/models/brick.jpg");

        VkDescriptorSet floor_texture_descriptor_set;
        auto image_info = brick_texture->descriptorInfo();
        ZDescriptorWriter(*texture_set_layout, *global_pool_)
            .WriteImage(0, &image_info)
            .build(floor_texture_descriptor_set);

        std::vector<VkDescriptorSet> global_descriptor_sets(ZSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < global_descriptor_sets.size(); i++)
        {
            auto buffer_info = ubo_buffers[i]->DescriptorInfo();
            ZDescriptorWriter(*global_set_layout, *global_pool_)
                .WriteBuffer(0, &buffer_info)
                .build(global_descriptor_sets[i]);
        }

        std::cout << "MaxPushConstantSize = " << device_.properties_.limits.maxPushConstantsSize << "\n";

        ZRenderSystem simple_render_system{
            device_,
            renderer_.getSwapchainRenderPass(),
            global_set_layout->GetDescriptorSetLayout(),
            texture_set_layout->GetDescriptorSetLayout()
        };
        ZPointLightSystem point_light_system{
            device_,
            renderer_.getSwapchainRenderPass(),
            global_set_layout->GetDescriptorSetLayout()
        };
        ZCamera camera{};
        camera.SetViewTarget(glm::vec3(-1.0f, -2.0f, -2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

        auto viewer_object = ZGameObject::createGameObject();
        viewer_object.transform.translation.z = -2.5f;
        ZKeyboardMovementController camera_controller{window_.getGLFWwindow()};

        auto current_time = std::chrono::high_resolution_clock::now();

        float fps_timer = 0.0f;
        float fps = 0.0f;

        while (!window_.shouldClose())
        {
            glfwPollEvents();

            auto new_time = std::chrono::high_resolution_clock::now();
            float frame_time = std::chrono::duration<float>(new_time - current_time).count();
            current_time = new_time;

            frame_time = std::min(frame_time, MAX_FRAME_TIME);

            camera_controller.moveInPlaneXZ(window_, frame_time, viewer_object);
            camera.SetViewYxz(viewer_object.transform.translation, viewer_object.transform.rotation);
            float aspect = renderer_.getAspectRatio();

            camera.SetPerspectiveProjection(glm::radians(60.0f), aspect, 0.1f, 100.0f);

            if (auto command_buffer = renderer_.beginFrame())
            {
                int frame_index = renderer_.getFrameIndex();
                FrameInfo frame_info{
                    frame_index,
                    frame_time,
                    command_buffer,
                    camera,
                    global_descriptor_sets[frame_index],
                    game_objects_
                };

                // Update
                GlobalUbo ubo{};
                ubo.projection = camera.GetProjection();
                ubo.view = camera.GetView();
                ubo.inverseView = camera.GetInverseView();
                point_light_system.update(frame_info, ubo);
                ubo_buffers[frame_index]->WriteToBuffer(&ubo);

                //Render
                renderer_.beginSwapchainRenderPass(command_buffer);
                simple_render_system.render(frame_info, floor_texture_descriptor_set);
                point_light_system.render(frame_info);

                fps_timer += frame_time;
                if (fps_timer >= 0.2f)
                {
                    fps = 1.0f / frame_time;
                    fps_timer = 0.0f;
                }

                // ImGui
                imgui_layer_.newFrame();
                ImGui::Begin("Stats");
                ImGui::Text("FPS = %f", fps);
                ImGui::Text("FrameTime = %f", frame_time);
                ImGui::DragFloat("Sens", &camera_controller.mouseSensitivity, 0.01f);
                ImGui::End();
                imgui_layer_.render(command_buffer);

                renderer_.endSwapchainRenderPass(command_buffer);
                renderer_.endFrame();
            }
        }

        vkDeviceWaitIdle(device_.device());
    }

    void ZApp::LoadGameObjects()
    {
        std::shared_ptr<ZModel> lve_model =
            ZModel::createModelFromFile(device_, "C:/_dev/ZEngine/models/FinalBaseMesh.obj");
        auto flat_vase = ZGameObject::createGameObject();
        flat_vase.model = lve_model;
        flat_vase.transform.translation = {-.5f, .5f, 0.f};
        flat_vase.transform.scale = glm::vec3(-0.06f);
        game_objects_.emplace(flat_vase.getId(), std::move(flat_vase));

        lve_model = ZModel::createModelFromFile(device_, "C:/_dev/ZEngine/models/smooth_vase.obj");
        auto smooth_vase = ZGameObject::createGameObject();
        smooth_vase.model = lve_model;
        smooth_vase.transform.translation = {.5f, .5f, 0.f};
        smooth_vase.transform.scale = glm::vec3(2.5f);
        game_objects_.emplace(smooth_vase.getId(), std::move(smooth_vase));

        lve_model = ZModel::createModelFromFile(device_, "C:/_dev/ZEngine/models/quad.obj");
        auto floor = ZGameObject::createGameObject();
        floor.model = lve_model;
        floor.transform.translation = {0.0f, .5f, 0.f};
        floor.transform.scale = glm::vec3(3.0f);
        game_objects_.emplace(floor.getId(), std::move(floor));

        std::vector<glm::vec3> light_colors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f} //
        };

        for (int i = 0; i < light_colors.size(); i++)
        {
            auto point_light = ZGameObject::makePointLight(0.5f);
            point_light.color = light_colors[i];
            auto rotate_light = glm::rotate(glm::mat4(1.0f),
                                           (i * glm::two_pi<float>()) / light_colors.size(),
                                           {0.0f, -1.0f, 0.0f});
            point_light.transform.translation = glm::vec3(rotate_light * glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));
            game_objects_.emplace(point_light.getId(), std::move(point_light));
        }
    }
}
