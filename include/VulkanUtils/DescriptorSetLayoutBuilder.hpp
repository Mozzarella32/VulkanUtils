#pragma once

#include <expected>
#include <vector>

#include "VkBindings/Enums.hpp"
#include "VkBindings/Handles.hpp"
#include "VkBindings/ObjectsForward.hpp"
#include "VkBindings/StructsForward.hpp"

namespace VkUtils {

class DescriptorSetLayoutBuilder {
  private:
    uint32_t currentBinding = 0;
    std::vector<VkBindings::DescriptorSetLayoutBinding> bindings;
    std::vector<VkBindings::Handle::Sampler> immutableSamplers;

  public:
    void addImmutableImageSampler(VkBindings::ShaderStageFlags stageFlags,
                                  VkBindings::Sampler sampler);
    void addDescriptor(VkBindings::DescriptorSetLayoutBinding binding);
    void addDescriptorArray(VkBindings::DescriptorSetLayoutBinding binding, uint32_t count);
    [[nodiscard]] auto build(VkBindings::Device device)
        -> std::expected<VkBindings::UniqueDescriptorSetLayout, VkBindings::Result>;
    [[nodiscard]] auto buildReset(VkBindings::Device device)
        -> std::expected<VkBindings::UniqueDescriptorSetLayout, VkBindings::Result>;
};

}; // namespace VkUtils
