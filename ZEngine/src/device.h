#pragma once

#include "window.h"

// std lib headers
#include <string>
#include <vector>

#include <vk_mem_alloc.h>

namespace z_engine
{
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    struct QueueFamilyIndices
    {
        uint32_t graphics_family;
        uint32_t present_family;
        bool graphics_family_has_value = false;
        bool present_family_has_value = false;
        bool IsComplete() const { return graphics_family_has_value && present_family_has_value; }
    };

    class ZDevice
    {
    public:
#ifdef NDEBUG
        const bool enable_validation_layers_ = false;
#else
        const bool enable_validation_layers_ = true;
#endif

        ZDevice(ZWindow& window);
        ~ZDevice();

        // Not copyable or movable
        ZDevice(const ZDevice&) = delete;
        ZDevice operator=(const ZDevice&) = delete;
        ZDevice(ZDevice&&) = delete;
        ZDevice& operator=(ZDevice&&) = delete;

        VkCommandPool GetCommandPool() const { return command_pool_; }
        VkDevice device() const { return device_; }
        VkInstance instance() const { return instance_; }
        VkSurfaceKHR surface() const { return surface_; }
        VkQueue GraphicsQueue() const { return graphics_queue_; }
        VkQueue PresentQueue() const { return present_queue_; }
        VkPhysicalDevice PhysicalDevice() const { return physical_device_; }
        VmaAllocator GetAllocator() { return allocator_; }

        SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(physical_device_); }
        uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);
        QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(physical_device_); }
        VkFormat FindSupportedFormat(
            const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        // Buffer Helper Functions
        void CreateBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& buffer_memory);
        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer command_buffer);
        void CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
        void CopyBufferToImage(
            VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count);

        void CreateImageWithInfo(
            const VkImageCreateInfo& image_info,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& image_memory);

        VkPhysicalDeviceProperties properties_;

    private:
        void CreateInstance();
        void SetupDebugMessenger();
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateCommandPool();

        // helper functions
        bool IsDeviceSuitable(VkPhysicalDevice device);
        std::vector<const char*> GetRequiredExtensions();
        bool CheckValidationLayerSupport();
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info);
        void HasGflwRequiredInstanceExtensions();
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

        VkInstance instance_;
        VkDebugUtilsMessengerEXT debug_messenger_;
        VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
        ZWindow& window_;
        VkCommandPool command_pool_;

        VkDevice device_;
        VkSurfaceKHR surface_;
        VkQueue graphics_queue_;
        VkQueue present_queue_;
        VmaAllocator allocator_;

        const std::vector<const char*> validation_layers_ = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char*> device_extensions_ = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };
}
