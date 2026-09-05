#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace z_engine
{
    class ZCamera
    {
    public:
        void SetOrthographicProjection(
            float left, float right, float bottom, float top, float near, float far);

        void SetPerspectiveProjection(
            float fovy, float aspect, float near, float far);

        void SetViewDirection(
            glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f));
        void SetViewTarget(
            glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f));
        void SetViewYxz(glm::vec3 position, glm::vec3 rotation);

        const glm::mat4& GetProjection() const { return projection_matrix_; }
        const glm::mat4& GetView() const { return view_matrix_; }
        const glm::mat4& GetInverseView() const { return inverse_view_matrix_; }
        glm::vec3 GetPosition() const { return glm::vec3(inverse_view_matrix_[3]); }

    private:
        glm::mat4 projection_matrix_{1.0f};
        glm::mat4 view_matrix_{1.0f};
        glm::mat4 inverse_view_matrix_{1.0f};
    };
}
