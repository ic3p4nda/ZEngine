#pragma once

#include "device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>

namespace ZEngine
{
    class ZModel
    {
    public:
        
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec3 normal;
            glm::vec2 uv;
            
            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            
            bool operator==(const Vertex &other) const {return position == other.position && 
                color == other.color && normal == other.normal;}
        };
        
        struct Builder
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};
            
            void LoadModel(const std::string &filepath);
        };
        
        ZModel(ZDevice &device, const Builder &builder);
        ~ZModel();
        
        static std::unique_ptr<ZModel> createModelFromFile(ZDevice &device, const std::string &filepath);
        
        ZModel(const ZModel&) = delete;
        ZModel &operator=(const ZModel&) = delete;
        
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
        
    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);
        
        ZDevice& device;
        
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
        
        bool hasIndexBuffer = false;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        uint32_t indexCount;
    };
}
