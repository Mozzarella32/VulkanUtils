#include "DescriptorSetLayoutBuilder.hpp"
#include "VkBindings/Defines.hpp"
#include "VkBindings/Enums.hpp"
#include "VkBindings/Objects.hpp"
#include "VkBindings/ObjectsForward.hpp"
#include "VkBindings/Structs.hpp"
#include "VkBindings/private/StructTemplatesInterface.hpp"
#include <cassert>

namespace VkUtils {
void DescriptorSetLayoutBuilder::addImmutableImageSampler(VkBindings::ShaderStageFlags stageFlags,
                                                          VkBindings::Sampler sampler) {
    immutableSamplers.emplace_back(sampler);
    assert(immutableSamplers.back() != VK_BINDINGS_NULL_HANDLE);

    VkBindings::DescriptorSetLayoutBinding binding;
    binding.descriptorType = VkBindings::DescriptorType::eCombinedImageSampler;
    binding.binding = currentBinding++;
    binding.stageFlags = stageFlags;
    binding.descriptorCount = 1;
    bindings.emplace_back(binding);
}
void DescriptorSetLayoutBuilder::addDescriptor(VkBindings::DescriptorSetLayoutBinding binding) {
    binding.binding = currentBinding++;
    binding.descriptorCount = 1;
    bindings.emplace_back(binding);
}
void DescriptorSetLayoutBuilder::addDescriptorArray(VkBindings::DescriptorSetLayoutBinding binding,
                                                    uint32_t count) {
    binding.binding = currentBinding++;
    binding.descriptorCount = count;
    bindings.emplace_back(binding);
}

auto DescriptorSetLayoutBuilder::build(VkBindings::Device device)
    -> std::expected<VkBindings::UniqueDescriptorSetLayout, VkBindings::Result> {
    VkBindings::DescriptorSetLayoutCreateInfo createInfo = {};

    size_t sampler = 0;
    for (auto &binding : bindings) {
        if (binding.descriptorType != VkBindings::DescriptorType::eCombinedImageSampler)
            continue;

        assert(sampler <= immutableSamplers.size());

        binding.immutableSamplers() = {immutableSamplers[sampler]};

        sampler += 1;

        assert(binding.pImmutableSamplers != nullptr);
    }

    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    createInfo.pBindings = bindings.data();
    return device.createDescriptorSetLayout(createInfo);
}
auto DescriptorSetLayoutBuilder::buildReset(VkBindings::Device device)
    -> std::expected<VkBindings::UniqueDescriptorSetLayout, VkBindings::Result> {
    auto layoutRes = build(device);
    bindings.clear();
    immutableSamplers.clear();
    currentBinding = 0;
    return layoutRes;
}
}; // namespace VkUtils
