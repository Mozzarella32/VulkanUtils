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
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace VkUtils {
struct PipelineBuilder {
  private:
    // Have to do a copy, as the initlizer_list constructor will prb be a temp
    std::vector<std::pair<std::string_view, VkBindings::ShaderStageBits>> shaders;
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
    auto setShaderStages(
        std::span<const std::pair<std::string_view, VkBindings::ShaderStageBits>> shaders)
        -> PipelineBuilder;

    auto setShaderStages(
        std::initializer_list<std::pair<std::string_view, VkBindings::ShaderStageBits>> shaders)
        -> PipelineBuilder;

    auto setInputAssembly(VkBindings::PrimitiveTopology topology,
                          VkBindings::Bool32 primitiveRestartEnable = VkBindings::Constants::False)
        -> PipelineBuilder;

    auto setVertexInputBuilder(PipelineVertexBindingDescriptorBuilder vertexInputBuilder)
        -> PipelineBuilder;

    auto setTessellation(uint32_t patchControlPoints) -> PipelineBuilder;

    struct ViewportScissorDynamic {
        uint32_t viewportCount;
        uint32_t scissorCount;
    };
    auto setViewportScissorDynamic(ViewportScissorDynamic viewportScissorDynamic = {
                                       .viewportCount = 1, .scissorCount = 1}) -> PipelineBuilder;

    auto setRasterization(VkBindings::PolygonMode polygonMode) -> PipelineBuilder;

    struct BiasConfig {
        constexpr static const float defaultConstantFactor = 1.75F;
        constexpr static const float defaultSlopeFactor = 3.00F;

        float constantFactor = defaultConstantFactor;
        float clamp = 0.0F;
        float slopeFactor = defaultSlopeFactor;
    };
    auto setRasterizationDepthPass(BiasConfig biasConfig) -> PipelineBuilder;

    auto setMultisample() -> PipelineBuilder;

    auto setDepthEnabled() -> PipelineBuilder;

    auto setStencilEnabled() -> PipelineBuilder;

    auto setNormalColorBlend() -> PipelineBuilder;

    auto addPushConstant(uint32_t offset, uint32_t size, VkBindings::ShaderStageFlags stages)
        -> PipelineBuilder;

    auto addDescriptorSetLayout(const VkBindings::DescriptorSetLayout &descriptorSetLayout)
        -> PipelineBuilder;

    auto setRenderingDepthAttachment(VkBindings::Format depthFormat) -> PipelineBuilder;

    auto setRenderingStencilAttachment(VkBindings::Format stencilFormat) -> PipelineBuilder;

    auto addRenderingColorAttachment(VkBindings::Format colorAttachmentFormat) -> PipelineBuilder;

    auto build(VkBindings::Device device,
               std::function<std::span<const uint32_t>(std::string_view)> spirVGetter,
               VkBindings::PipelineCache pipelineCache = {}, const std::string &name = "")
        -> std::expected<std::tuple<VkBindings::UniquePipelineLayout, VkBindings::UniquePipeline>,
                         VkBindings::Result>;
};
} // namespace VkUtils
