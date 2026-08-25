#include "window.h"

#include <iostream>
#include <ostream>
#include <stdexcept>

namespace ZEngine
{
    ZWindow::ZWindow(int w, int h, std::string t) : width(w), height(h), windowTitle(t)
    {
        initWindow();
    }

    ZWindow::~ZWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void ZWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void ZWindow::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void ZWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto engineWindow = reinterpret_cast<ZWindow*>(glfwGetWindowUserPointer(window));
        engineWindow->frameBufferResized = true;
        engineWindow->width = width;
        engineWindow->height = height;
    }
}

