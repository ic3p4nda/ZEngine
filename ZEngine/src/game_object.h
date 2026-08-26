#pragma once

#include "model.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

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
    
    struct PointLightComponent
    {
        float lightIntensity = 1.0f;
    };
    
    class ZGameObject
    {
    public:
        using id_t = unsigned int;
        using MAP = std::unordered_map<id_t, ZGameObject>;
        
        static ZGameObject createGameObject();
        
        static ZGameObject makePointLight(
            float lightIntensity = 5.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.0f));
        
        ZGameObject(const ZGameObject &) = delete;
        ZGameObject &operator=(const ZGameObject &) = delete;
        ZGameObject(ZGameObject &&) = default;
        ZGameObject &operator=(ZGameObject &&) = default;
        
        id_t getId() { return id; }
        
        glm::vec3 color{};
        TransformComponent transform{};
    
        // Optional poiters
        std::shared_ptr<ZModel> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;
    
        private:
        ZGameObject(id_t objId) : id(objId) {};
        
        id_t id;
    };
}
