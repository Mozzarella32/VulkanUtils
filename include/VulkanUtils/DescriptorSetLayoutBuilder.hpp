#pragma once

#include "VulkanObjects.hpp"

namespace VkUtils {

class DescriptorSetLayoutBuilder {
  private:
    uint32_t currentBinding = 0;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<VkSampler> immutableSamplers;

  public:
    void addImmutableImageSampler(VkShaderStageFlags stageFlags,
                                  VkBindings::UniqueVkSampler &sampler);
    void addDescriptor(VkDescriptorSetLayoutBinding binding);
    void addDescriptorArray(VkDescriptorSetLayoutBinding binding, uint32_t count);
    [[nodiscard]] std::expected<VkBindings::UniqueVkDescriptorSetLayout, VkResult>
    build(VkBindings::UniqueVkDevice &device);
    [[nodiscard]] std::expected<VkBindings::UniqueVkDescriptorSetLayout, VkResult>
    buildReset(VkBindings::UniqueVkDevice &device);
};

}; // namespace VkUtils
