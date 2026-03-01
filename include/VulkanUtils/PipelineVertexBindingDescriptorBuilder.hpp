#pragma once

#include <Vulkan.hpp>

#include <vector>

namespace VkUtils {

class PipelineVertexBindingDescriptorBuilder {
  private:
    uint32_t nextBinding = 0;
    uint32_t currentBinding = 0;
    uint32_t currentLocation = 0;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

  public:
    void addBinding(VkVertexInputBindingDescription bindingDescription);
    void addAttribute(VkVertexInputAttributeDescription attributeDescription);
    template <typename T>
        requires requires(PipelineVertexBindingDescriptorBuilder desc) {
            T::addBinding(desc, std::declval<VkVertexInputRate>());
        }
    inline void addVertex(VkVertexInputRate inputRate) {
        T::addBinding(*this, inputRate);
    }

    [[nodiscard]] VkPipelineVertexInputStateCreateInfo getVertexInputInfo();

    void print() const;
};

} // namespace VkUtils
