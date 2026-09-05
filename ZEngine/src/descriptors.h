#pragma once

#include "device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace z_engine
{
    class ZDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder(ZDevice& device) : device_{device}
            {
            }

            Builder& AddBinding(
                uint32_t binding,
                VkDescriptorType descriptor_type,
                VkShaderStageFlags stage_flags,
                uint32_t count = 1);
            std::unique_ptr<ZDescriptorSetLayout> build() const;

        private:
            ZDevice& device_;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_{};
        };

        ZDescriptorSetLayout(
            ZDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~ZDescriptorSetLayout();
        ZDescriptorSetLayout(const ZDescriptorSetLayout&) = delete;
        ZDescriptorSetLayout& operator=(const ZDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptor_set_layout_; }

    private:
        ZDevice& device_;
        VkDescriptorSetLayout descriptor_set_layout_;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_;

        friend class ZDescriptorWriter;
    };

    class ZDescriptorPool
    {
    public:
        class Builder
        {
        public:
            Builder(ZDevice& device) : device_{device}
            {
            }

            Builder& AddPoolSize(VkDescriptorType descriptor_type, uint32_t count);
            Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& SetMaxSets(uint32_t count);
            std::unique_ptr<ZDescriptorPool> build() const;

        private:
            ZDevice& device_;
            std::vector<VkDescriptorPoolSize> pool_sizes_{};
            uint32_t max_sets_ = 1000;
            VkDescriptorPoolCreateFlags pool_flags_ = 0;
        };

        ZDescriptorPool(
            ZDevice& device,
            uint32_t max_sets,
            VkDescriptorPoolCreateFlags pool_flags,
            const std::vector<VkDescriptorPoolSize>& pool_sizes);
        ~ZDescriptorPool();
        ZDescriptorPool(const ZDescriptorPool&) = delete;
        ZDescriptorPool& operator=(const ZDescriptorPool&) = delete;

        bool AllocateDescriptor(
            VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet& descriptor) const;

        void FreeDescriptors(const std::vector<VkDescriptorSet>& descriptors) const;

        void ResetPool() const;

    private:
        ZDevice& device_;
        VkDescriptorPool descriptor_pool_;

        friend class ZDescriptorWriter;
    };

    class ZDescriptorWriter
    {
    public:
        ZDescriptorWriter(ZDescriptorSetLayout& set_layout, ZDescriptorPool& pool);

        ZDescriptorWriter& WriteBuffer(uint32_t binding, const VkDescriptorBufferInfo* buffer_info);
        ZDescriptorWriter& WriteImage(uint32_t binding, const VkDescriptorImageInfo* image_info);

        bool build(VkDescriptorSet& set);
        void overwrite(const VkDescriptorSet& set);

    private:
        ZDescriptorSetLayout& set_layout_;
        ZDescriptorPool& pool_;
        std::vector<VkWriteDescriptorSet> writes_;
    };
}
