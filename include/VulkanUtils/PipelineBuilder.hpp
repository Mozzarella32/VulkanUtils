#pragma once

#include "PipelineVertexBindingDescriptorBuilder.hpp"
#include "VkBindings/Structs.hpp"

#include <expected>
#include <filesystem>
#include <functional>
#include <span>

namespace VkUtils {
struct PipelineCacheManager {
    VkBindings::UniquePipelineCache pipelineCache;
    std::filesystem::path cache_file;

    void read(VkBindings::Device device, const std::filesystem::path &cache_file);
    void write(VkBindings::Device device);
    ~PipelineCacheManager();
};

struct PipelineBuilder {
  private:
    std::vector<std::pair<std::string, VkBindings::ShaderStageFlagBits>> shaders;
    VkBindings::PipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    VkBindings::PipelineTessellationStateCreateInfo tessellationState = {};
    VkBindings::PipelineViewportStateCreateInfo viewportState = {};
    VkBindings::PipelineRasterizationStateCreateInfo rasterizationState = {};
    VkBindings::PipelineMultisampleStateCreateInfo multisampleState = {};
    VkBindings::PipelineDepthStencilStateCreateInfo depthStencilState = {};
    VkBindings::PipelineColorBlendStateCreateInfo colorBlendState = {};
    VkBindings::PipelineColorBlendAttachmentState colorBlendAttachment{};
    std::vector<VkBindings::DynamicState> dynamicStates;
    VkBindings::PipelineDynamicStateCreateInfo dynamicState = {};
    std::vector<VkBindings::PushConstantRange> pushConstantRanges;
    std::vector<VkBindings::DescriptorSetLayout> descriptorSetLayouts;
    VkBindings::PipelineRenderingCreateInfo rendering = {};
    std::vector<VkBindings::Format> colorAttachments;

  public:
    void
    setShaderStages(std::vector<std::pair<std::string, VkBindings::ShaderStageFlagBits>> shaders);

    PipelineVertexBindingDescriptorBuilder vertexInputInfoBuilder;

    void setInputAssembly(VkBindings::PrimitiveTopology topology,
                          VkBindings::Bool32 primitiveRestartEnable = VkBindings::Constants::False);

    void setTessellation(uint32_t patchControlPoints);

    void setViewportDynamic(uint32_t viewportCount = 1, uint32_t scissorCount = 1);

    void setRasterization(VkBindings::PolygonMode polygonMode);

    void setRasterizationDepthPass();

    void setMultisample();

    void setDepthEnabled();

    void setStencilEnabled();

    void setNormalColorBlend();

    void addPushConstant(uint32_t offset, uint32_t size, VkBindings::ShaderStageFlags stages);

    void addDescriptorSetLayout(VkBindings::DescriptorSetLayout descriptorSetLayout);

    void setRenderingDepthAttachment(VkBindings::Format depthFormat);

    void setRenderingStencilAttachment(VkBindings::Format stencilFormat);

    void addRenderingColorAttachment(VkBindings::Format colorAttachmentFormat);

    std::expected<std::tuple<VkBindings::UniquePipelineLayout, VkBindings::UniquePipeline>,
                  VkBindings::Result>
    build(VkBindings::Device &device,
          std::function<std::span<const uint32_t>(const std::string &)> spirVGetter,
          VkBindings::PipelineCache pipelineCache = {}, const std::string &name = "");
};
} // namespace VkUtils
