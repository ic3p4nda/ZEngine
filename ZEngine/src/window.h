#pragma once

#define GLFW_INCLUDE_VULKAN
#include <functional>
#include <GLFW/glfw3.h>

#include <string>
#include <functional>
#include <vector>
#include <glm/vec2.hpp>

namespace z_engine
{
    class ZWindow
    {
    public:
        ZWindow(int w, int h, std::string t);
        ~ZWindow();

        ZWindow(const ZWindow&) = delete;
        ZWindow& operator=(const ZWindow&) = delete;

        bool shouldClose() { return glfwWindowShouldClose(window); };
        VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
        bool wasWindowReized() { return frameBufferResized; }
        void resetWindowResizeFlag() { frameBufferResized = false; }
        GLFWwindow* getGLFWwindow() const { return window; };

        glm::vec2 getWindowExtent() { return glm::vec2(static_cast<float>(width), static_cast<float>(height)); }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

    private:
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        void initWindow();

        int width;
        int height;

        bool frameBufferResized = false;

        std::string windowTitle;

        GLFWwindow* window;
    };
}
