#include "keyboard_movement_controller.h"

#include <iostream>

namespace ZEngine
{
    ZKeyboardMovementController::ZKeyboardMovementController(GLFWwindow* window)
    {
        if (glfwRawMouseMotionSupported())
        {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_FALSE);
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
    }

    void ZKeyboardMovementController::moveInPlaneXZ(ZWindow& window, float deltaTime, ZGameObject& gameObject)
    {
        GLFWwindow* glfwWindow = window.getGLFWwindow();
        double xpos, ypos;
        glfwGetCursorPos(glfwWindow, &xpos, &ypos);
        
        glm::vec3 rotate{0};
        if (glfwGetKey(glfwWindow, keys.lookRight) == GLFW_PRESS) rotate.y += 1.0f;
        if (glfwGetKey(glfwWindow, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.0f;
        if (glfwGetKey(glfwWindow, keys.lookUp) == GLFW_PRESS) rotate.x += 1.0f;
        if (glfwGetKey(glfwWindow, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.0f;
        
        // HandleMouseInput(window, xpos, ypos, rotate);
        
        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()){
            gameObject.transform.rotation += lookSpeed * deltaTime * glm::normalize(rotate);
        }
        
        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
        
        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{sin(yaw), 0.0f, cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.0f, -forwardDir.x};
        const glm::vec3 upDir{0.0f, -1.0f, 0.0f};
        
        glm::vec3 moveDir{0.0f};
        if (glfwGetKey(glfwWindow, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if (glfwGetKey(glfwWindow, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if (glfwGetKey(glfwWindow, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(glfwWindow, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if (glfwGetKey(glfwWindow, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if (glfwGetKey(glfwWindow, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;
        
        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()){
            gameObject.transform.translation += lookSpeed * deltaTime * glm::normalize(moveDir);
        }
        
        glfwSetCursorPos(glfwWindow, xpos, ypos);
    }
    
    // void ZKeyboardMovementController::HandleMouseInput(ZWindow& window, float xpos, float ypos, glm::vec3& rotate)
    // {
    //     glm::vec2 windowExtent = window.getWindowExtent();
    //     float centerX = windowExtent.x / 2;
    //     float centerY = windowExtent.y / 2;
    //     
    //     // Look Right
    //     if (xpos > centerX) rotate.y += 0.1f;
    //     // Look Left
    //     if (xpos < centerX ) rotate.y -= 0.1f;
    //     // Look down
    //     if (ypos < centerY) rotate.x += 0.1f;
    //     // Look Up
    //     if (ypos  > centerY) rotate.x -= 0.1f;
    //     
    //     glfwSetCursorPos(window.getGLFWwindow(), centerX, centerY);
    // }
}
