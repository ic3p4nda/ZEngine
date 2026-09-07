#pragma once
#include "device.h"

namespace z_engine
{
    class ZTexture
    {
    public:
        ZTexture(ZDevice& device, const std::string& filepath);
        ~ZTexture();

        ZTexture(const ZTexture&) = delete;
        ZTexture& operator=(const ZTexture&) = delete;

        VkImageView GetImageView() const { return view_; };
        VkSampler GetSampler() const { return sampler_; };
        VkImageLayout GetImageLayout() const { return image_layout_; };

        VkDescriptorImageInfo DescriptorInfo() const
        {
            return VkDescriptorImageInfo{sampler_, view_, image_layout_};
        }

    private:
        void CreateTextureImage(const std::string& filepath);
        void CreateTextureImageView();
        void CreateTextureSampler();
        void TransitionImageLayout(VkImageLayout old_layout, VkImageLayout new_layout);

        ZDevice& device_;

        VkImage image_;
        VmaAllocation allocation_;
        VkImageView view_;
        VkSampler sampler_;
        VkFormat image_format_;
        VkImageLayout image_layout_;

        int width_, height_, mip_levels_ = 1;
    };
}
