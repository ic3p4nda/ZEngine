#pragma once
#include "device.h"

namespace ZEngine{
    class ZTexture
    {
    public:
        ZTexture(ZDevice& device, const std::string& filepath);
        ~ZTexture();
        
        ZTexture(const ZTexture&) = delete;
        ZTexture &operator=(const ZTexture&) = delete;
        
        VkImageView getImageView() const {return view;};
        VkSampler getSampler() const {return sampler;};
        VkImageLayout getImageLayout() const {return imageLayout;};
        
        VkDescriptorImageInfo descriptorInfo() const
        {
            return VkDescriptorImageInfo{ sampler, view, imageLayout};
        }
        
    private:
        void createTextureImage(const std::string& filepath);
        void createTextureImageView();
        void createTextureSampler();
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        
        ZDevice& device;
        
        VkImage image;
        VkDeviceMemory memory;
        VkImageView view;
        VkSampler sampler;
        VkFormat imageFormat;
        VkImageLayout imageLayout;
        
        int width, height, mipLevels = 1;
    };
}
