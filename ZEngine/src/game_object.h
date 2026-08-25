#pragma once

#include "model.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace ZEngine
{
    
    struct TransformComponent
    {
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{};
        
        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };
    
    class ZGameObject
    {
    public:
        using id_t = unsigned int;
        
        static ZGameObject createGameObject();
        
        // GameObject(const GameObject &) = delete;
        // GameObject &operator=(const GameObject &) = delete;
        // GameObject(GameObject &&) = default;
        // GameObject &operator=(GameObject &&) = default;
        
        id_t getId() { return id; };
        
        std::shared_ptr<ZModel> model{};
        glm::vec3 color{};
        TransformComponent transform{};
        
        private:
        ZGameObject(id_t objId) : id(objId) {};
        
        id_t id;
    };
}
