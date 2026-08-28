#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Constants.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Structs.hpp>

#include "Functions.hpp"
#include "NameObject.hpp"
#include "PipelineBuilder.hpp"
#include "PipelineVertexBindingDescriptorBuilder.hpp"

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
auto PipelineBuilder::setShaderStages(
    std::span<const std::pair<std::string_view, VkBindings::ShaderStageBits>> shaders)
    -> PipelineBuilder {
    this->shaders.assign_range(shaders);
    return *this;
}

auto PipelineBuilder::setShaderStages(
    std::initializer_list<std::pair<std::string_view, VkBindings::ShaderStageBits>> shaders)
    -> PipelineBuilder {
    return setShaderStages(std::span{shaders});
}

auto PipelineBuilder::setInputAssembly(VkBindings::PrimitiveTopology topology,
                                       VkBindings::Bool32 primitiveRestartEnable)
    -> PipelineBuilder {
    inputAssemblyState.topology = topology;
    inputAssemblyState.primitiveRestartEnable = primitiveRestartEnable;
    return *this;
}

auto PipelineBuilder::setVertexInputBuilder(
    PipelineVertexBindingDescriptorBuilder vertexInputBuilder) -> PipelineBuilder {
    this->vertexInputBuilder = std::move(vertexInputBuilder);
    return *this;
}

auto PipelineBuilder::setTessellation(uint32_t patchControlPoints) -> PipelineBuilder {
    tessellationState.patchControlPoints = patchControlPoints;
    return *this;
}

auto PipelineBuilder::setViewportScissorDynamic(ViewportScissorDynamic viewportScissorDynamic)
    -> PipelineBuilder {
    viewportState.viewportCount = viewportScissorDynamic.viewportCount;
    viewportState.scissorCount = viewportScissorDynamic.scissorCount;
    dynamicStates.push_back(VkBindings::DynamicState::Viewport);
    dynamicStates.push_back(VkBindings::DynamicState::Scissor);
    return *this;
}

auto PipelineBuilder::setRasterization(VkBindings::PolygonMode polygonMode) -> PipelineBuilder {
    rasterizationState.polygonMode = polygonMode;
    rasterizationState.lineWidth = 1.0F;
    rasterizationState.cullMode = VkBindings::CullModeBits::Back;
    rasterizationState.frontFace = VkBindings::FrontFace::CounterClockwise;
    return *this;
}

auto PipelineBuilder::setRasterizationDepthPass(BiasConfig biasConfig) -> PipelineBuilder {
    rasterizationState.depthBiasEnable = VkBindings::Constants::True;
    rasterizationState.depthBiasConstantFactor = biasConfig.constantFactor;
    rasterizationState.depthBiasClamp = biasConfig.clamp;
    rasterizationState.depthBiasSlopeFactor = biasConfig.slopeFactor;
    return *this;
}

auto PipelineBuilder::setMultisample() -> PipelineBuilder {
    multisampleState.rasterizationSamples = VkBindings::SampleCountBits::v1;
    return *this;
}

auto PipelineBuilder::setDepthEnabled() -> PipelineBuilder {
    depthStencilState.depthTestEnable = VkBindings::Constants::True;
    depthStencilState.depthWriteEnable = VkBindings::Constants::True;
    depthStencilState.depthCompareOp = VkBindings::CompareOp::Less;
    return *this;
}

auto PipelineBuilder::setStencilEnabled() -> PipelineBuilder {
    depthStencilState.stencilTestEnable = VkBindings::Constants::True;
    return *this;
}

auto PipelineBuilder::setNormalColorBlend() -> PipelineBuilder {
    colorBlendAttachment.colorWriteMask = VkBindings::ColorComponentBits::AllBits;
    colorBlendAttachment.blendEnable = VkBindings::Constants::True;
    colorBlendAttachment.srcColorBlendFactor = VkBindings::BlendFactor::SrcAlpha;
    colorBlendAttachment.dstColorBlendFactor = VkBindings::BlendFactor::OneMinusSrcAlpha;
    colorBlendAttachment.colorBlendOp = VkBindings::BlendOp::Add;
    colorBlendAttachment.srcAlphaBlendFactor = VkBindings::BlendFactor::One;
    colorBlendAttachment.dstAlphaBlendFactor = VkBindings::BlendFactor::Zero;
    colorBlendAttachment.alphaBlendOp = VkBindings::BlendOp::Add;

    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachment;
    return *this;
}

auto PipelineBuilder::addPushConstant(uint32_t offset, uint32_t size,
                                      VkBindings::ShaderStageFlags stages) -> PipelineBuilder {
    pushConstantRanges.push_back({.stageFlags = stages, .offset = offset, .size = size});
    return *this;
}

auto PipelineBuilder::addDescriptorSetLayout(
    const VkBindings::DescriptorSetLayout &descriptorSetLayout) -> PipelineBuilder {
    descriptorSetLayouts.push_back(descriptorSetLayout);
    return *this;
}

auto PipelineBuilder::setRenderingDepthAttachment(VkBindings::Format depthFormat)
    -> PipelineBuilder {
    rendering.depthAttachmentFormat = depthFormat;
    return *this;
}

auto PipelineBuilder::setRenderingStencilAttachment(VkBindings::Format stencilFormat)
    -> PipelineBuilder {
    rendering.stencilAttachmentFormat = stencilFormat;
    return *this;
}

auto PipelineBuilder::addRenderingColorAttachment(VkBindings::Format colorAttachmentFormat)
    -> PipelineBuilder {
    colorAttachments.push_back(colorAttachmentFormat);
    return *this;
}

auto PipelineBuilder::build(VkBindings::Device device,
                            std::function<std::span<const uint32_t>(std::string_view)> spirVGetter,
                            VkBindings::PipelineCache pipelineCache, const std::string &name)
    -> std::expected<std::tuple<VkBindings::UniquePipelineLayout, VkBindings::UniquePipeline>,
                     VkBindings::Result> {
    VkBindings::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayouts() = descriptorSetLayouts;
    pipelineLayoutInfo.pushConstantRanges() = pushConstantRanges;

    VkBindings::UniquePipelineLayout pipelineLayout;
    return device.createPipelineLayout(pipelineLayoutInfo)
        .and_then([&](VkBindings::UniquePipelineLayout &&pipelineLayoutRes) {
            pipelineLayout = std::move(pipelineLayoutRes);
            return VkUtils::createShaderStages(device, spirVGetter, shaders);
        })
        .and_then([&](std::tuple<std::vector<VkBindings::UniqueShaderModule>,
                                 std::vector<VkBindings::PipelineShaderStageCreateInfo>> &&tuple) {
            auto [_, shaderStages] = std::move(tuple);

            auto vertexInputState = vertexInputBuilder.getVertexInputInfo();
            dynamicState.dynamicStates() = dynamicStates;

            rendering.colorAttachmentFormats() = colorAttachments;

            VkBindings::GraphicsPipelineCreateInfo pipelineInfo{
                .pNext = &rendering,
                .pVertexInputState = &vertexInputState,
                .pInputAssemblyState = &inputAssemblyState,
                .pTessellationState = &tessellationState,
                .pViewportState = &viewportState,
                .pRasterizationState = &rasterizationState,
                .pMultisampleState = &multisampleState,
                .pDepthStencilState = &depthStencilState,
                .pColorBlendState = &colorBlendState,
                .pDynamicState = &dynamicState,
                .layout = pipelineLayout};
            pipelineInfo.stages() = shaderStages;
            return device.createGraphicsPipelines(pipelineCache, {pipelineInfo});
        })
        .transform([&](std::vector<VkBindings::UniquePipeline> &&pipelines) {
            auto &&pipeline = std::move(pipelines).at(0);
            nameObject(device, pipeline, name);
            nameObject(device, pipelineLayout, name);
            return std::make_tuple(std::move(pipelineLayout), std::move(pipeline));
        });
}

} // namespace VkUtils
