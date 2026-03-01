#include "DescriptorSetLayoutBuilder.hpp"

namespace VkUtils {
void DescriptorSetLayoutBuilder::addImmutableImageSampler(VkShaderStageFlags stageFlags,
                                                          VkBindings::UniqueVkSampler &sampler) {
    immutableSamplers.emplace_back(sampler);
    assert(immutableSamplers.back() != VK_NULL_HANDLE);
    VkDescriptorSetLayoutBinding binding;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.binding = currentBinding++;
    binding.pImmutableSamplers = (VkSampler *)-1;
    binding.stageFlags = stageFlags;
    binding.descriptorCount = 1;
    bindings.emplace_back(binding);
}
void DescriptorSetLayoutBuilder::addDescriptor(VkDescriptorSetLayoutBinding binding) {
    binding.binding = currentBinding++;
    binding.descriptorCount = 1;
    bindings.emplace_back(binding);
}
void DescriptorSetLayoutBuilder::addDescriptorArray(VkDescriptorSetLayoutBinding binding,
                                                    uint32_t count) {
    binding.binding = currentBinding++;
    binding.descriptorCount = count;
    bindings.emplace_back(binding);
}
std::expected<VkBindings::UniqueVkDescriptorSetLayout, VkResult>
DescriptorSetLayoutBuilder::build(VkBindings::UniqueVkDevice &device) {
    auto createInfo = VkBindings::Init<VkDescriptorSetLayoutCreateInfo>();
    size_t sampler = 0;
    for (auto &binding : bindings) {
        if (binding.pImmutableSamplers == nullptr)
            continue;
        assert(sampler <= immutableSamplers.size());
        binding.pImmutableSamplers = &immutableSamplers[sampler];
        assert(binding.pImmutableSamplers != nullptr);
    }
    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    createInfo.pBindings = bindings.data();
    return device.createDescriptorSetLayout(&createInfo);
}
std::expected<VkBindings::UniqueVkDescriptorSetLayout, VkResult>
DescriptorSetLayoutBuilder::buildReset(VkBindings::UniqueVkDevice &device) {
    auto layoutRes = build(device);
    bindings.clear();
    immutableSamplers.clear();
    currentBinding = 0;
    return layoutRes;
}
}; // namespace VkUtils
