#pragma once

#include "device.h"

namespace z_engine
{
    class ZBuffer
    {
    public:
        ZBuffer(
            ZDevice& device,
            VkDeviceSize instance_size,
            uint32_t instance_count,
            VkBufferUsageFlags usage_flags,
            VkMemoryPropertyFlags memory_property_flags,
            VkDeviceSize min_offset_alignment = 1);
        ~ZBuffer();

        ZBuffer(const ZBuffer&) = delete;
        ZBuffer& operator=(const ZBuffer&) = delete;

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void unmap();

        void WriteToBuffer(const void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

        void WriteToIndex(const void* data, int index) const;
        VkResult FlushIndex(int index) const;
        VkDescriptorBufferInfo DescriptorInfoForIndex(int index) const;
        VkResult InvalidateIndex(int index) const;

        VkBuffer GetBuffer() const;
        void* GetMappedMemory() const { return mapped_; }
        uint32_t GetInstanceCount() const { return instance_count_; }
        VkDeviceSize GetInstanceSize() const { return instance_size_; }
        VkDeviceSize GetAlignmentSize() const { return alignment_size_; }
        VkBufferUsageFlags GetUsageFlags() const { return usage_flags_; }
        VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return memoryPropertyFlags; }
        VkDeviceSize GetBufferSize() const { return buffer_size_; }

    private:
        static VkDeviceSize GetAlignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment);

        ZDevice& device_;
        void* mapped_ = nullptr;
        VkBuffer buffer_ = VK_NULL_HANDLE;
        VkDeviceMemory memory_ = VK_NULL_HANDLE;

        VkDeviceSize buffer_size_;
        uint32_t instance_count_;
        VkDeviceSize instance_size_;
        VkDeviceSize alignment_size_;
        VkBufferUsageFlags usage_flags_;
        VkMemoryPropertyFlags memoryPropertyFlags;
    };
}
