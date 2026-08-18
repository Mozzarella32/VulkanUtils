#pragma once

#include <VkBindings/Enums.hpp>
#include <VkBindings/Handles.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/StructsForward.hpp>

#include <cstdint>
#include <expected>
#include <vector>

namespace VkUtils {

class DescriptorSetLayoutBuilder {
  private:
    uint32_t currentBinding = 0;
    std::vector<VkBindings::DescriptorSetLayoutBinding> bindings;
    std::vector<VkBindings::Handle::Sampler> immutableSamplers;

  public:
    void addImmutableImageSampler(VkBindings::ShaderStageFlags stageFlags,
                                  const VkBindings::Sampler &sampler);
    void addDescriptor(VkBindings::DescriptorSetLayoutBinding binding);
    void addDescriptorArray(VkBindings::DescriptorSetLayoutBinding binding, uint32_t count);
    [[nodiscard]] auto build(const VkBindings::Device &device)
        -> std::expected<VkBindings::UniqueDescriptorSetLayout, VkBindings::Result>;
    [[nodiscard]] auto buildReset(const VkBindings::Device &device)
        -> std::expected<VkBindings::UniqueDescriptorSetLayout, VkBindings::Result>;
};

}; // namespace VkUtils
