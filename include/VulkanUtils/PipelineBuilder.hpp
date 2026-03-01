#pragma once

#include "VulkanObjects.hpp"
#include "PipelineVertexBindingDescriptorBuilder.hpp"

#include <filesystem>
#include <functional>

namespace VkUtils {
struct PipelineCacheManager {
    VkBindings::UniqueVkPipelineCache pipelineCache;
    std::filesystem::path cache_file;

    void read(VkBindings::UniqueVkDevice &device, const std::filesystem::path &cache_file);
    void write(VkBindings::UniqueVkDevice &device);
    ~PipelineCacheManager();
};

struct PipelineBuilder {
  private:
    std::vector<std::pair<std::string, VkShaderStageFlagBits>> shaders;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
        VkBindings::Init<VkPipelineInputAssemblyStateCreateInfo>();
    VkPipelineTessellationStateCreateInfo tessellationState =
        VkBindings::Init<VkPipelineTessellationStateCreateInfo>();
    VkPipelineViewportStateCreateInfo viewportState =
        VkBindings::Init<VkPipelineViewportStateCreateInfo>();
    VkPipelineRasterizationStateCreateInfo rasterizationState =
        VkBindings::Init<VkPipelineRasterizationStateCreateInfo>();
    VkPipelineMultisampleStateCreateInfo multisampleState =
        VkBindings::Init<VkPipelineMultisampleStateCreateInfo>();
    VkPipelineDepthStencilStateCreateInfo depthStencilState =
        VkBindings::Init<VkPipelineDepthStencilStateCreateInfo>();
    VkPipelineColorBlendStateCreateInfo colorBlendState =
        VkBindings::Init<VkPipelineColorBlendStateCreateInfo>();
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    std::vector<VkDynamicState> dynamicStates;
    VkPipelineDynamicStateCreateInfo dynamicState =
        VkBindings::Init<VkPipelineDynamicStateCreateInfo>();
    std::vector<VkPushConstantRange> pushConstantRanges;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    VkPipelineRenderingCreateInfo rendering = VkBindings::Init<VkPipelineRenderingCreateInfo>();
    std::vector<VkFormat> colorAttachments;

  public:
    void setShaderStages(std::vector<std::pair<std::string, VkShaderStageFlagBits>> shaders);

    PipelineVertexBindingDescriptorBuilder vertexInputInfoBuilder;

    void setInputAssembly(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable = VK_FALSE);

    void setTessellation(uint32_t patchControlPoints);

    void setViewportDynamic(uint32_t viewportCount = 1, uint32_t scissorCount = 1);

    void setRasterization(VkPolygonMode polygonMode);

    void setRasterizationDepthPass();

    void setMultisample();

    void setDepthEnabled();

    void setStencilEnabled();

    void setNormalColorBlend();

    void addPushConstant(uint32_t offset, uint32_t size, VkShaderStageFlags stages);

    void addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);

    void setRenderingDepthAttachment(VkFormat depthFormat);

    void setRenderingStencilAttachment(VkFormat stencilFormat);

    void addRenderingColorAttachment(VkFormat colorAttachmentFormat);

    std::expected<std::tuple<VkBindings::UniqueVkPipelineLayout, VkBindings::UniqueVkPipeline>,
                  VkResult>
    build(VkBindings::UniqueVkDevice &device,
          std::function<std::span<const uint32_t>(const std::string &)> spirVGetter,
          VkPipelineCache pipelineCache = VK_NULL_HANDLE, const std::string &name = "");
};
} // namespace VkUtils
