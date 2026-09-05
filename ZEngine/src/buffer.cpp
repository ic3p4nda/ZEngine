#include "buffer.h"

// std
#include <cstring>

namespace z_engine
{
    /**
     * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
     *
     * @param instance_size The size of an instance
     * @param min_offset_alignment The minimum required alignment, in bytes, for the offset member (eg
     * minUniformBufferOffsetAlignment)
     *
     * @return VkResult of the buffer mapping call
     */
    VkDeviceSize ZBuffer::GetAlignment(const VkDeviceSize instance_size, const VkDeviceSize min_offset_alignment)
    {
        if (min_offset_alignment > 0)
        {
            return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
        }
        return instance_size;
    }

    ZBuffer::ZBuffer(
        ZDevice& device,
        VkDeviceSize instance_size,
        uint32_t instance_count,
        VkBufferUsageFlags usage_flags,
        VkMemoryPropertyFlags memory_property_flags,
        VkDeviceSize min_offset_alignment)
        : device_{device},
          instance_count_{instance_count},
          instance_size_{instance_size},
          usage_flags_{usage_flags},
          memoryPropertyFlags{memory_property_flags}
    {
        alignment_size_ = GetAlignment(instance_size, min_offset_alignment);
        buffer_size_ = alignment_size_ * instance_count;
        device.CreateBuffer(buffer_size_, usage_flags, memory_property_flags, buffer_, memory_);
    }

    ZBuffer::~ZBuffer()
    {
        unmap();
        vkDestroyBuffer(device_.device(), buffer_, nullptr);
        vkFreeMemory(device_.device(), memory_, nullptr);
    }

    /**
     * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
     *
     * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
     * buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the buffer mapping call
     */
    VkResult ZBuffer::map(VkDeviceSize size, VkDeviceSize offset)
    {
        assert(buffer_ && memory_ && "Called map on buffer before create");
        return vkMapMemory(device_.device(), memory_, offset, size, 0, &mapped_);
    }

    /**
     * Unmap a mapped memory range
     *
     * @note Does not return a result as vkUnmapMemory can't fail
     */
    void ZBuffer::unmap()
    {
        if (mapped_)
        {
            vkUnmapMemory(device_.device(), memory_);
            mapped_ = nullptr;
        }
    }

    /**
     * Copies the specified data to the mapped buffer. Default value writes whole buffer range
     *
     * @param data Pointer to the data to copy
     * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
     * range.
     * @param offset (Optional) Byte offset from beginning of mapped region
     *
     */
    void ZBuffer::WriteToBuffer(const void* data, VkDeviceSize size, VkDeviceSize offset) const
    {
        assert(mapped_ && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE)
        {
            memcpy(mapped_, data, buffer_size_);
        }
        else
        {
            auto mem_offset = static_cast<char*>(mapped_);
            mem_offset += offset;
            memcpy(mem_offset, data, size);
        }
    }

    /**
     * Flush a memory range of the buffer to make it visible to the device
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
     * complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the flush call
     */
    VkResult ZBuffer::flush(VkDeviceSize size, VkDeviceSize offset) const
    {
        VkMappedMemoryRange mapped_range = {};
        mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mapped_range.memory = memory_;
        mapped_range.offset = offset;
        mapped_range.size = size;
        return vkFlushMappedMemoryRanges(device_.device(), 1, &mapped_range);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
     * the complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the invalidate call
     */
    VkResult ZBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) const
    {
        VkMappedMemoryRange mapped_range = {};
        mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mapped_range.memory = memory_;
        mapped_range.offset = offset;
        mapped_range.size = size;
        return vkInvalidateMappedMemoryRanges(device_.device(), 1, &mapped_range);
    }

    /**
     * Create a buffer info descriptor
     *
     * @param size (Optional) Size of the memory range of the descriptor
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkDescriptorBufferInfo of specified offset and range
     */
    VkDescriptorBufferInfo ZBuffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset) const
    {
        return VkDescriptorBufferInfo{
            buffer_,
            offset,
            size,
        };
    }

    /**
     * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
     *
     * @param data Pointer to the data to copy
     * @param index Used in offset calculation
     *
     */
    void ZBuffer::WriteToIndex(const void* data, int index) const
    {
        WriteToBuffer(data, instance_size_, index * alignment_size_);
    }

    /**
     *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
     *
     * @param index Used in offset calculation
     *
     */
    VkResult ZBuffer::FlushIndex(int index) const { return flush(alignment_size_, index * alignment_size_); }

    /**
     * Create a buffer info descriptor
     *
     * @param index Specifies the region given by index * alignmentSize
     *
     * @return VkDescriptorBufferInfo for instance at index
     */
    VkDescriptorBufferInfo ZBuffer::DescriptorInfoForIndex(int index) const
    {
        return DescriptorInfo(alignment_size_, index * alignment_size_);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param index Specifies the region to invalidate: index * alignmentSize
     *
     * @return VkResult of the invalidate call
     */
    VkResult ZBuffer::InvalidateIndex(int index) const
    {
        return invalidate(alignment_size_, index * alignment_size_);
    }

    VkBuffer ZBuffer::GetBuffer() const
    {
        return buffer_;
    }
}
