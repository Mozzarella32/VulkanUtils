#pragma once

#include "PipelineVertexBindingDescriptorBuilder.hpp"

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Constants.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Structs.hpp>

#include <cstdint>
#include <expected>
#include <functional>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace VkUtils {
struct PipelineBuilder {
  private:
    std::vector<std::pair<std::string, VkBindings::ShaderStageBits>> shaders;
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
    PipelineVertexBindingDescriptorBuilder vertexInputBuilder;

  public:
    void setShaderStages(std::vector<std::pair<std::string, VkBindings::ShaderStageBits>> shaders);

    void setInputAssembly(VkBindings::PrimitiveTopology topology,
                          VkBindings::Bool32 primitiveRestartEnable = VkBindings::Constants::False);

    auto getVertexInputBuilder() -> PipelineVertexBindingDescriptorBuilder &;

    void setTessellation(uint32_t patchControlPoints);

    struct ViewportScissorDynamic {
        uint32_t viewportCount;
        uint32_t scissorCount;
    };
    void setViewportScissorDynamic(ViewportScissorDynamic viewportScissorDynamic = {
                                       .viewportCount = 1, .scissorCount = 1});

    void setRasterization(VkBindings::PolygonMode polygonMode);

    struct BiasConfig {
        constexpr static const float defaultConstantFactor = 1.75F;
        constexpr static const float defaultSlopeFactor = 3.00F;

        float constantFactor = defaultConstantFactor;
        float clamp = 0.0F;
        float slopeFactor = defaultSlopeFactor;
    };
    void setRasterizationDepthPass(BiasConfig biasConfig);

    void setMultisample();

    void setDepthEnabled();

    void setStencilEnabled();

    void setNormalColorBlend();

    void addPushConstant(uint32_t offset, uint32_t size, VkBindings::ShaderStageFlags stages);

    void addDescriptorSetLayout(const VkBindings::DescriptorSetLayout &descriptorSetLayout);

    void setRenderingDepthAttachment(VkBindings::Format depthFormat);

    void setRenderingStencilAttachment(VkBindings::Format stencilFormat);

    void addRenderingColorAttachment(VkBindings::Format colorAttachmentFormat);

    auto build(VkBindings::Device device,
               std::function<std::span<const uint32_t>(const std::string &)> spirVGetter,
               VkBindings::PipelineCache pipelineCache = {}, const std::string &name = "")
        -> std::expected<std::tuple<VkBindings::UniquePipelineLayout, VkBindings::UniquePipeline>,
                         VkBindings::Result>;
};
} // namespace VkUtils
