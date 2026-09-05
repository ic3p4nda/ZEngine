#include "descriptors.h"

// std
#include <cassert>
#include <stdexcept>

namespace z_engine
{
    // *************** Descriptor Set Layout Builder *********************

    ZDescriptorSetLayout::Builder& ZDescriptorSetLayout::Builder::AddBinding(
        uint32_t binding,
        VkDescriptorType descriptor_type,
        VkShaderStageFlags stage_flags,
        uint32_t count)
    {
        assert(bindings_.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layout_binding{};
        layout_binding.binding = binding;
        layout_binding.descriptorType = descriptor_type;
        layout_binding.descriptorCount = count;
        layout_binding.stageFlags = stage_flags;
        bindings_[binding] = layout_binding;
        return *this;
    }

    std::unique_ptr<ZDescriptorSetLayout> ZDescriptorSetLayout::Builder::build() const
    {
        return std::make_unique<ZDescriptorSetLayout>(device_, bindings_);
    }

    // *************** Descriptor Set Layout *********************

    ZDescriptorSetLayout::ZDescriptorSetLayout(
        ZDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : device_{device}, bindings_{bindings}
    {
        std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};
        for (auto kv : bindings)
        {
            set_layout_bindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
        descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
        descriptor_set_layout_info.pBindings = set_layout_bindings.data();

        if (vkCreateDescriptorSetLayout(
            device.device(),
            &descriptor_set_layout_info,
            nullptr,
            &descriptor_set_layout_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    ZDescriptorSetLayout::~ZDescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(device_.device(), descriptor_set_layout_, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    ZDescriptorPool::Builder& ZDescriptorPool::Builder::AddPoolSize(
        VkDescriptorType descriptor_type, uint32_t count)
    {
        pool_sizes_.push_back({descriptor_type, count});
        return *this;
    }

    ZDescriptorPool::Builder& ZDescriptorPool::Builder::SetPoolFlags(
        VkDescriptorPoolCreateFlags flags)
    {
        pool_flags_ = flags;
        return *this;
    }

    ZDescriptorPool::Builder& ZDescriptorPool::Builder::SetMaxSets(uint32_t count)
    {
        max_sets_ = count;
        return *this;
    }

    std::unique_ptr<ZDescriptorPool> ZDescriptorPool::Builder::build() const
    {
        return std::make_unique<ZDescriptorPool>(device_, max_sets_, pool_flags_, pool_sizes_);
    }

    // *************** Descriptor Pool *********************

    ZDescriptorPool::ZDescriptorPool(
        ZDevice& device,
        uint32_t max_sets,
        VkDescriptorPoolCreateFlags pool_flags,
        const std::vector<VkDescriptorPoolSize>& pool_sizes)
        : device_{device}
    {
        VkDescriptorPoolCreateInfo descriptor_pool_info{};
        descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        descriptor_pool_info.pPoolSizes = pool_sizes.data();
        descriptor_pool_info.maxSets = max_sets;
        descriptor_pool_info.flags = pool_flags;

        if (vkCreateDescriptorPool(device.device(), &descriptor_pool_info, nullptr, &descriptor_pool_) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    ZDescriptorPool::~ZDescriptorPool()
    {
        vkDestroyDescriptorPool(device_.device(), descriptor_pool_, nullptr);
    }

    bool ZDescriptorPool::AllocateDescriptor(
        const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet& descriptor) const
    {
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = descriptor_pool_;
        alloc_info.pSetLayouts = &descriptor_set_layout;
        alloc_info.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(device_.device(), &alloc_info, &descriptor) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }

    void ZDescriptorPool::FreeDescriptors(const std::vector<VkDescriptorSet>& descriptors) const
    {
        vkFreeDescriptorSets(
            device_.device(),
            descriptor_pool_,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void ZDescriptorPool::ResetPool() const
    {
        vkResetDescriptorPool(device_.device(), descriptor_pool_, 0);
    }

    // *************** Descriptor Writer *********************

    ZDescriptorWriter::ZDescriptorWriter(ZDescriptorSetLayout& set_layout, ZDescriptorPool& pool)
        : set_layout_{set_layout}, pool_{pool}
    {
    }

    ZDescriptorWriter& ZDescriptorWriter::WriteBuffer(
        uint32_t binding, const VkDescriptorBufferInfo* buffer_info)
    {
        assert(set_layout_.bindings_.count(binding) == 1 && "Layout does not contain specified binding");

        const auto& binding_description = set_layout_.bindings_[binding];

        assert(
            binding_description.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = binding_description.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = buffer_info;
        write.descriptorCount = 1;

        writes_.push_back(write);
        return *this;
    }

    ZDescriptorWriter& ZDescriptorWriter::WriteImage(
        uint32_t binding, const VkDescriptorImageInfo* image_info)
    {
        assert(set_layout_.bindings_.count(binding) == 1 && "Layout does not contain specified binding");

        const auto& binding_description = set_layout_.bindings_[binding];

        assert(
            binding_description.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = binding_description.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = image_info;
        write.descriptorCount = 1;

        writes_.push_back(write);
        return *this;
    }

    bool ZDescriptorWriter::build(VkDescriptorSet& set)
    {
        bool success = pool_.AllocateDescriptor(set_layout_.GetDescriptorSetLayout(), set);
        if (!success)
        {
            return false;
        }
        overwrite(set);
        return true;
    }

    void ZDescriptorWriter::overwrite(const VkDescriptorSet& set)
    {
        for (auto& write : writes_)
        {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool_.device_.device(), static_cast<uint32_t>(writes_.size()), writes_.data(), 0, nullptr);
    }
}
