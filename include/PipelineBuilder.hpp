#pragma once

// #include "ResourcePreprocessor.hpp"
#include "VulkanBindings.hpp"
#include "VulkanUtils.hpp"

#include <filesystem>

struct PipelineCacheManager {
    VulkanBindings::UniqueVkPipelineCache pipelineCache;
    std::filesystem::path cache_file;

    void read(VulkanBindings::UniqueVkDevice &device, const std::filesystem::path &cache_file);
    void write(VulkanBindings::UniqueVkDevice &device);
    ~PipelineCacheManager();
};

struct PipelineBuilder {
  private:
    std::vector<std::pair<std::string, VkShaderStageFlagBits>> shaders;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
        VulkanBindings::Init<VkPipelineInputAssemblyStateCreateInfo>();
    VkPipelineTessellationStateCreateInfo tessellationState =
        VulkanBindings::Init<VkPipelineTessellationStateCreateInfo>();
    VkPipelineViewportStateCreateInfo viewportState =
        VulkanBindings::Init<VkPipelineViewportStateCreateInfo>();
    VkPipelineRasterizationStateCreateInfo rasterizationState =
        VulkanBindings::Init<VkPipelineRasterizationStateCreateInfo>();
    VkPipelineMultisampleStateCreateInfo multisampleState =
        VulkanBindings::Init<VkPipelineMultisampleStateCreateInfo>();
    VkPipelineDepthStencilStateCreateInfo depthStencilState =
        VulkanBindings::Init<VkPipelineDepthStencilStateCreateInfo>();
    VkPipelineColorBlendStateCreateInfo colorBlendState =
        VulkanBindings::Init<VkPipelineColorBlendStateCreateInfo>();
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    std::vector<VkDynamicState> dynamicStates;
    VkPipelineDynamicStateCreateInfo dynamicState =
        VulkanBindings::Init<VkPipelineDynamicStateCreateInfo>();
    std::vector<VkPushConstantRange> pushConstantRanges;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    VkPipelineRenderingCreateInfo rendering = VulkanBindings::Init<VkPipelineRenderingCreateInfo>();
    std::vector<VkFormat> colorAttachments;

  public:
    void setShaderStages(std::vector<std::pair<std::string, VkShaderStageFlagBits>> shaders);

    VkUtils::PipelineVertexBindingDescriptorBuilder vertexInputInfoBuilder;

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

    std::expected<std::tuple<VulkanBindings::UniqueVkPipelineLayout, VulkanBindings::UniqueVkPipeline>, VkResult>
    build(VulkanBindings::UniqueVkDevice &device, std::function<std::span<const uint32_t>(const std::string&)> spirVGetter,
          VkPipelineCache pipelineCache = VK_NULL_HANDLE, const std::string &name = "");
};
