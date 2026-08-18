#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Constants.hpp>
#include <VkBindings/EnumToString.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Structs.hpp>

#include "Errorhandling.hpp"
#include "Functions.hpp"
#include "NameObject.hpp"
#include "PipelineBuilder.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <ios>
#include <iostream>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace VkUtils {

namespace {
auto format_bytes(size_t bytes) -> std::string {
    std::array units = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    auto size = static_cast<double>(bytes);

    constinit static const double base = 1024.0;

    while (size >= base && unit_index < 4) {
        size /= base;
        ++unit_index;
    }

    return std::format("{:.2f} {}", size, units.at(unit_index));
}
} // namespace

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
void PipelineCacheManager::read(const VkBindings::Device &device,
                                const std::filesystem::path &supplyed_cache_file) {
    cache_file = supplyed_cache_file;
    if (std::filesystem::exists(cache_file)) {
        std::ifstream inFile(cache_file, std::ios::binary);
        size_t size = 0;
        inFile.read(reinterpret_cast<char *>(&size), sizeof(size_t));
        std::vector<uint8_t> data(size);
        inFile.read(reinterpret_cast<char *>(data.data()),
                    static_cast<std::streamsize>(data.size()));
        std::cout << "Read Pipline Cache: " << format_bytes(data.size()) << "\n";

        VkBindings::PipelineCacheCreateInfo createInfo;
        createInfo.initialDataSize = data.size();
        createInfo.pInitialData = data.data();

        // fallback if cache is bad
        auto resPipelineCache = device.createPipelineCache(createInfo);
        if (resPipelineCache) {
            pipelineCache = std::move(resPipelineCache.value());
            return;
        }
        std::cerr << "Cache was bad, falling back to new one: "
                  << VkBindings::Reflections::enumToString(resPipelineCache.error());
    }

    VkBindings::PipelineCacheCreateInfo createInfo;
    createInfo.initialDataSize = 0;
    createInfo.pInitialData = nullptr;
    std::ignore = device.createPipelineCache(createInfo)
                      .transform_error(VkUtils::printFailedFunction("createPiplineCache"))
                      .transform([&](VkBindings::UniquePipelineCache &&resPipelineCache) -> void {
                          pipelineCache = std::move(resPipelineCache);
                      });
}

void PipelineCacheManager::write(const VkBindings::Device &device) {
    if (!pipelineCache)
        return;
    std::ignore = device.getPipelineCacheData(pipelineCache)
                      .transform_error(VkUtils::printFailedFunction("getPiplineCacheData"))
                      .transform([&](auto data) -> void {
                          size_t size = data.size();
                          std::ofstream outFile(cache_file, std::ios::binary);
                          outFile.write(reinterpret_cast<char *>(&size), sizeof(size_t));
                          outFile.write(reinterpret_cast<char *>(data.data()),
                                        static_cast<std::streamsize>(data.size()));
                          pipelineCache.cleanup();
                          std::cout << "Wrote Pipline Cache: " << format_bytes(data.size()) << "\n";
                      });
}
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

PipelineCacheManager::~PipelineCacheManager() {
    if (pipelineCache) {
        std::cerr << "Forgot to write back PiplineCacheData!\n";
    }
}

void PipelineBuilder::setShaderStages(
    std::vector<std::pair<std::string, VkBindings::ShaderStageBits>> supplyed_shaders) {
    shaders = std::move(supplyed_shaders);
}

void PipelineBuilder::setInputAssembly(VkBindings::PrimitiveTopology topology,
                                       VkBindings::Bool32 primitiveRestartEnable) {
    inputAssemblyState.topology = topology;
    inputAssemblyState.primitiveRestartEnable = primitiveRestartEnable;
}

void PipelineBuilder::setTessellation(uint32_t patchControlPoints) {
    tessellationState.patchControlPoints = patchControlPoints;
}

void PipelineBuilder::setViewportScissorDynamic(ViewportScissorDynamic viewportScissorDynamic) {
    viewportState.viewportCount = viewportScissorDynamic.viewportCount;
    viewportState.scissorCount = viewportScissorDynamic.scissorCount;
    dynamicStates.push_back(VkBindings::DynamicState::Viewport);
    dynamicStates.push_back(VkBindings::DynamicState::Scissor);
}
void PipelineBuilder::setRasterization(VkBindings::PolygonMode polygonMode) {
    rasterizationState.polygonMode = polygonMode;
    rasterizationState.lineWidth = 1.0F;
    rasterizationState.cullMode = VkBindings::CullModeBits::Back;
    rasterizationState.frontFace = VkBindings::FrontFace::CounterClockwise;
}

void PipelineBuilder::setRasterizationDepthPass(BiasConfig biasConfig) {
    rasterizationState.depthBiasEnable = VkBindings::Constants::True;
    rasterizationState.depthBiasConstantFactor = biasConfig.constantFactor;
    rasterizationState.depthBiasClamp = biasConfig.clamp;
    rasterizationState.depthBiasSlopeFactor = biasConfig.slopeFactor;
}

void PipelineBuilder::setMultisample() {
    multisampleState.rasterizationSamples = VkBindings::SampleCountBits::v1;
}

void PipelineBuilder::setDepthEnabled() {
    depthStencilState.depthTestEnable = VkBindings::Constants::True;
    depthStencilState.depthWriteEnable = VkBindings::Constants::True;
    depthStencilState.depthCompareOp = VkBindings::CompareOp::Less;
}

void PipelineBuilder::setStencilEnabled() {
    depthStencilState.stencilTestEnable = VkBindings::Constants::True;
}

void PipelineBuilder::setNormalColorBlend() {
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
}

void PipelineBuilder::addPushConstant(uint32_t offset, uint32_t size,
                                      VkBindings::ShaderStageFlags stages) {
    pushConstantRanges.push_back({.stageFlags = stages, .offset = offset, .size = size});
}

void PipelineBuilder::addDescriptorSetLayout(
    const VkBindings::DescriptorSetLayout &descriptorSetLayout) {
    descriptorSetLayouts.push_back(descriptorSetLayout);
}

void PipelineBuilder::setRenderingDepthAttachment(VkBindings::Format depthFormat) {
    rendering.depthAttachmentFormat = depthFormat;
}

void PipelineBuilder::setRenderingStencilAttachment(VkBindings::Format stencilFormat) {
    rendering.stencilAttachmentFormat = stencilFormat;
}

void PipelineBuilder::addRenderingColorAttachment(VkBindings::Format colorAttachmentFormat) {
    colorAttachments.push_back(colorAttachmentFormat);
}

auto PipelineBuilder::build(
    VkBindings::Device device,
    std::function<std::span<const uint32_t>(const std::string &)> spirVGetter,
    VkBindings::PipelineCache pipelineCache, const std::string &name)
    -> std::expected<std::tuple<VkBindings::UniquePipelineLayout, VkBindings::UniquePipeline>,
                     VkBindings::Result> {
    VkBindings::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.setLayouts() = descriptorSetLayouts;
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

    VkBindings::UniquePipelineLayout pipelineLayout;
    return device.createPipelineLayout(pipelineLayoutInfo)
        .and_then([&](VkBindings::UniquePipelineLayout &&pipelineLayoutRes) {
            pipelineLayout = std::move(pipelineLayoutRes);
            return VkUtils::createShaderStages(device, spirVGetter, shaders);
        })
        .and_then([&](std::tuple<std::vector<VkBindings::UniqueShaderModule>,
                                 std::vector<VkBindings::PipelineShaderStageCreateInfo>> &&tuple) {
            auto [_, shaderStages] = std::move(tuple);

            auto vertexInputState = vertexInputInfoBuilder.getVertexInputInfo();
            dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
            dynamicState.pDynamicStates = dynamicStates.data();

            rendering.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
            rendering.pColorAttachmentFormats = colorAttachments.data();

            VkBindings::GraphicsPipelineCreateInfo pipelineInfo;
            pipelineInfo.stages() = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputState;
            pipelineInfo.pInputAssemblyState = &inputAssemblyState;
            pipelineInfo.pTessellationState = &tessellationState;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizationState;
            pipelineInfo.pMultisampleState = &multisampleState;
            pipelineInfo.pDepthStencilState = &depthStencilState;
            pipelineInfo.pColorBlendState = &colorBlendState;
            pipelineInfo.pDynamicState = &dynamicState;
            pipelineInfo.layout = pipelineLayout;
            pipelineInfo.pNext = &rendering;
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
