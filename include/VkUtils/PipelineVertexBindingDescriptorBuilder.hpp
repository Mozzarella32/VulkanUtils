#pragma once

#include <VkBindings/Enums.hpp>
#include <VkBindings/Structs.hpp>

#include <cstdint>
#include <utility>
#include <vector>

namespace VkUtils {

class PipelineVertexBindingDescriptorBuilder {
  private:
    uint32_t nextBinding = 0;
    uint32_t currentBinding = 0;
    uint32_t currentLocation = 0;

    std::vector<VkBindings::VertexInputBindingDescription> bindingDescriptions;
    std::vector<VkBindings::VertexInputAttributeDescription> attributeDescriptions;

  public:
    auto addBinding(VkBindings::VertexInputBindingDescription bindingDescription)
        -> PipelineVertexBindingDescriptorBuilder;
    auto addAttribute(VkBindings::VertexInputAttributeDescription attributeDescription)
        -> PipelineVertexBindingDescriptorBuilder;

    // Can be chained
    template <typename T>
        requires requires(PipelineVertexBindingDescriptorBuilder desc) {
            T::addBinding(desc, std::declval<VkBindings::VertexInputRate>());
        }
    auto addVertex(VkBindings::VertexInputRate inputRate)
        -> PipelineVertexBindingDescriptorBuilder {
        T::addBinding(*this, inputRate);
        return *this;
    }

    [[nodiscard]] auto getVertexInputInfo() -> VkBindings::PipelineVertexInputStateCreateInfo;

    void print() const;
};

} // namespace VkUtils
