#include "keyboard_movement_controller.h"

#include <imgui.h>
#include <iostream>

namespace z_engine
{
    ZKeyboardMovementController::ZKeyboardMovementController(GLFWwindow* window)
    {
        // if (glfwRawMouseMotionSupported())
        // {
        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        //     glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        // }
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

        bool rmbHeld = glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

        if (rmbHeld && !ImGui::GetIO().WantCaptureMouse)
        {
            double xpos, ypos;
            glfwGetCursorPos(glfwWindow, &xpos, &ypos);

            if (!isLooking)
            {
                glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                lastMouseX = xpos;
                lastMouseY = ypos;
                isLooking = true;
            }

            double deltaX = xpos - lastMouseX;
            double deltaY = ypos - lastMouseY;
            lastMouseX = xpos;
            lastMouseY = ypos;

            rotate.y += static_cast<float>(deltaX) * mouseSensitivity;
            rotate.x -= static_cast<float>(deltaY) * mouseSensitivity;
        }
        else if (isLooking)
        {
            // just released RMB — give cursor back
            glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            isLooking = false;
        }

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
        {
            gameObject.transform.rotation += mouseSensitivity * deltaTime * glm::normalize(rotate);
        }

        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{sin(yaw), 0.0f, cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.0f, -forwardDir.x};
        constexpr glm::vec3 upDir{0.0f, -1.0f, 0.0f};

        glm::vec3 moveDir{0.0f};
        if (glfwGetKey(glfwWindow, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if (glfwGetKey(glfwWindow, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if (glfwGetKey(glfwWindow, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(glfwWindow, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if (glfwGetKey(glfwWindow, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if (glfwGetKey(glfwWindow, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
        {
            gameObject.transform.translation += moveSpeed * deltaTime * glm::normalize(moveDir);
        }

        glfwSetCursorPos(glfwWindow, xpos, ypos);
    }
}
