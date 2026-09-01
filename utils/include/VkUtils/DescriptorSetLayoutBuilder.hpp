#pragma once

#include <VkBindings/Enums.hpp>
#include <VkBindings/Flags.hpp>
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
    auto addImmutableImageSampler(VkBindings::ShaderStageFlags stageFlags,
                                  const VkBindings::Sampler &sampler) -> DescriptorSetLayoutBuilder;
    auto addDescriptor(VkBindings::DescriptorSetLayoutBinding binding)
        -> DescriptorSetLayoutBuilder;
    auto addDescriptorArray(VkBindings::DescriptorSetLayoutBinding binding, uint32_t count)
        -> DescriptorSetLayoutBuilder;
    [[nodiscard]] auto build(const VkBindings::Device &device)
        -> std::expected<VkBindings::UniqueDescriptorSetLayout, VkBindings::Result>;
    [[nodiscard]] auto buildReset(const VkBindings::Device &device)
        -> std::expected<VkBindings::UniqueDescriptorSetLayout, VkBindings::Result>;
};

}; // namespace VkUtils
