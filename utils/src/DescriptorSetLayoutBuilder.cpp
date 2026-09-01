#include "DescriptorSetLayoutBuilder.hpp"

#include <VkBindings/Defines.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/Flags.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Structs.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>

namespace VkUtils {
auto DescriptorSetLayoutBuilder::addImmutableImageSampler(VkBindings::ShaderStageFlags stageFlags,
                                                          const VkBindings::Sampler &sampler)
    -> DescriptorSetLayoutBuilder {
    assert(sampler != VK_BINDINGS_NULL_HANDLE);
    immutableSamplers.emplace_back(sampler);

    bindings.push_back({.binding = currentBinding++,
                        .descriptorType = VkBindings::DescriptorType::CombinedImageSampler,
                        .descriptorCount = 1,
                        .stageFlags = stageFlags});
    return *this;
}

auto DescriptorSetLayoutBuilder::addDescriptor(VkBindings::DescriptorSetLayoutBinding binding)
    -> DescriptorSetLayoutBuilder {
    binding.binding = currentBinding++;
    binding.descriptorCount = 1;
    bindings.emplace_back(binding);
    return *this;
}

auto DescriptorSetLayoutBuilder::addDescriptorArray(VkBindings::DescriptorSetLayoutBinding binding,
                                                    uint32_t count) -> DescriptorSetLayoutBuilder {
    binding.binding = currentBinding++;
    binding.descriptorCount = count;
    bindings.emplace_back(binding);
    return *this;
}

auto DescriptorSetLayoutBuilder::build(const VkBindings::Device &device)
    -> std::expected<VkBindings::UniqueDescriptorSetLayout, VkBindings::Result> {
    VkBindings::DescriptorSetLayoutCreateInfo createInfo = {};

    size_t sampler = 0;
    for (auto &binding : bindings) {
        if (binding.descriptorType != VkBindings::DescriptorType::CombinedImageSampler)
            continue;
        if (binding.pImmutableSamplers == nullptr)
            continue;

        binding.immutableSamplers() = {immutableSamplers.at(sampler)};

        sampler += 1;
    }

    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    createInfo.pBindings = bindings.data();
    return device.createDescriptorSetLayout(createInfo);
}
auto DescriptorSetLayoutBuilder::buildReset(const VkBindings::Device &device)
    -> std::expected<VkBindings::UniqueDescriptorSetLayout, VkBindings::Result> {
    auto layoutRes = build(device);
    bindings.clear();
    immutableSamplers.clear();
    currentBinding = 0;
    return layoutRes;
}
}; // namespace VkUtils
