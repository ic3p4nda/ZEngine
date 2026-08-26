#pragma once
 
#include "device.h"
 
// std
#include <memory>
#include <unordered_map>
#include <vector>
 
namespace ZEngine {
 
class ZDescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(ZDevice &Device) : Device{Device} {}
 
    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<ZDescriptorSetLayout> build() const;
 
   private:
    ZDevice &Device;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };
 
  ZDescriptorSetLayout(
      ZDevice &Device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~ZDescriptorSetLayout();
  ZDescriptorSetLayout(const ZDescriptorSetLayout &) = delete;
  ZDescriptorSetLayout &operator=(const ZDescriptorSetLayout &) = delete;
 
  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
 
 private:
  ZDevice &Device;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
 
  friend class ZDescriptorWriter;
};
 
class ZDescriptorPool {
 public:
  class Builder {
   public:
    Builder(ZDevice &Device) : Device{Device} {}
 
    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<ZDescriptorPool> build() const;
 
   private:
    ZDevice &Device;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };
 
  ZDescriptorPool(
      ZDevice &Device,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~ZDescriptorPool();
  ZDescriptorPool(const ZDescriptorPool &) = delete;
  ZDescriptorPool &operator=(const ZDescriptorPool &) = delete;
 
  bool allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;
 
  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;
 
  void resetPool();
 
 private:
  ZDevice &Device;
  VkDescriptorPool descriptorPool;
 
  friend class ZDescriptorWriter;
};
 
class ZDescriptorWriter {
 public:
  ZDescriptorWriter(ZDescriptorSetLayout &setLayout, ZDescriptorPool &pool);
 
  ZDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  ZDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);
 
  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);
 
 private:
  ZDescriptorSetLayout &setLayout;
  ZDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};
 
}