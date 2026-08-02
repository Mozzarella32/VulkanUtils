#include "PipelineBuilder.hpp"
#include "Errorhandling.hpp"
#include "Functions.hpp"
#include "NameObject.hpp"

#include "NameObject.hpp"
#include "VkBindings/Constants.hpp"
#include "VkBindings/EnumToString.hpp"
#include "VkBindings/Enums.hpp"
#include "VkBindings/ObjectsForward.hpp"

#include <fstream>
#include <iostream>
#include <vector>

namespace VkUtils {

std::string format_bytes(size_t bytes) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unit_index < 4) {
        size /= 1024.0;
        ++unit_index;
    }

    return std::format("{:.2f} {}", size, units[unit_index]);
}

void PipelineCacheManager::read(VkBindings::Device device,
                                const std::filesystem::path &supplyed_cache_file) {
    cache_file = supplyed_cache_file;
    if (std::filesystem::exists(cache_file)) {
        std::ifstream i(cache_file);
        size_t size = 0;
        i.read((char *)&size, sizeof(size_t));
        std::vector<uint8_t> data(size);
        i.read((char *)data.data(), data.size());
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
                      .transform([&](auto &&resPipelineCache) {
                          pipelineCache = std::move(resPipelineCache);
                      });
}

void PipelineCacheManager::write(VkBindings::Device device) {
    if (!pipelineCache)
        return;
    std::ignore = device.getPipelineCacheData(pipelineCache)
                      .transform_error(VkUtils::printFailedFunction("getPiplineCacheData"))
                      .transform([&](auto data) {
                          size_t size = data.size();
                          std::ofstream o(cache_file);
                          o.write((char *)&size, sizeof(size_t));
                          o.write((char *)data.data(), data.size());
                          pipelineCache.cleanup();
                          std::cout << "Wrote Pipline Cache: " << format_bytes(data.size()) << "\n";
                      });
}

PipelineCacheManager::~PipelineCacheManager() {
    if (pipelineCache) {
        std::cerr << "Forgot to write back PiplineCacheData!\n";
    }
}

void PipelineBuilder::setShaderStages(
    std::vector<std::pair<std::string, VkBindings::ShaderStageFlagBits>> supplyed_shaders) {
    shaders = supplyed_shaders;
}

void PipelineBuilder::setInputAssembly(VkBindings::PrimitiveTopology topology,
                                       VkBindings::Bool32 primitiveRestartEnable) {
    inputAssemblyState.topology = topology;
    inputAssemblyState.primitiveRestartEnable = primitiveRestartEnable;
}

void PipelineBuilder::setTessellation(uint32_t patchControlPoints) {
    tessellationState.patchControlPoints = patchControlPoints;
}

void PipelineBuilder::setViewportDynamic(uint32_t viewportCount, uint32_t scissorCount) {
    viewportState.viewportCount = viewportCount;
    viewportState.scissorCount = scissorCount;
    dynamicStates.push_back(VkBindings::DynamicState::eViewport);
    dynamicStates.push_back(VkBindings::DynamicState::eScissor);
}
void PipelineBuilder::setRasterization(VkBindings::PolygonMode polygonMode) {
    rasterizationState.polygonMode = polygonMode;
    rasterizationState.lineWidth = 1.0f;
    rasterizationState.cullMode = VkBindings::CullModeFlagBits::eBack;
    rasterizationState.frontFace = VkBindings::FrontFace::eCounterClockwise;
}

void PipelineBuilder::setRasterizationDepthPass() {
    rasterizationState.depthBiasEnable = VkBindings::Constants::True;
    rasterizationState.depthBiasConstantFactor = 1.75f;
    rasterizationState.depthBiasClamp = 0.0f;
    rasterizationState.depthBiasSlopeFactor = 3.00f;
}

void PipelineBuilder::setMultisample() {
    multisampleState.rasterizationSamples = VkBindings::SampleCountFlagBits::e1;
}

void PipelineBuilder::setDepthEnabled() {
    depthStencilState.depthTestEnable = VkBindings::Constants::True;
    depthStencilState.depthWriteEnable = VkBindings::Constants::True;
    depthStencilState.depthCompareOp = VkBindings::CompareOp::eLess;
}

void PipelineBuilder::setStencilEnabled() {
    depthStencilState.stencilTestEnable = VkBindings::Constants::True;
}

void PipelineBuilder::setNormalColorBlend() {
    colorBlendAttachment.colorWriteMask = VkBindings::ColorComponentFlagBits::eAllBits;
    colorBlendAttachment.blendEnable = VkBindings::Constants::True;
    colorBlendAttachment.srcColorBlendFactor = VkBindings::BlendFactor::eSrcAlpha;
    colorBlendAttachment.dstColorBlendFactor = VkBindings::BlendFactor::eOneMinusSrcAlpha;
    colorBlendAttachment.colorBlendOp = VkBindings::BlendOp::eAdd;
    colorBlendAttachment.srcAlphaBlendFactor = VkBindings::BlendFactor::eOne;
    colorBlendAttachment.dstAlphaBlendFactor = VkBindings::BlendFactor::eZero;
    colorBlendAttachment.alphaBlendOp = VkBindings::BlendOp::eAdd;

    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachment;
}

void PipelineBuilder::addPushConstant(uint32_t offset, uint32_t size,
                                      VkBindings::ShaderStageFlags stages) {
    pushConstantRanges.push_back({.stageFlags = stages, .offset = offset, .size = size});
}

void PipelineBuilder::addDescriptorSetLayout(VkBindings::DescriptorSetLayout descriptorSetLayout) {
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

std::expected<std::tuple<VkBindings::UniquePipelineLayout, VkBindings::UniquePipeline>,
              VkBindings::Result>
PipelineBuilder::build(VkBindings::Device &device,
                       std::function<std::span<const uint32_t>(const std::string &)> spirVGetter,
                       VkBindings::PipelineCache pipelineCache, const std::string &name) {
    VkBindings::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.setLayouts() = descriptorSetLayouts;
    // pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

    VkBindings::UniquePipelineLayout pipelineLayout;
    return device.createPipelineLayout(pipelineLayoutInfo)
        .and_then([&](auto &&pipelineLayoutRes) {
            pipelineLayout = std::move(pipelineLayoutRes);
            return VkUtils::createShaderStages(device, spirVGetter, shaders);
        })
        .and_then([&](auto &&tuple) {
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
        .transform([&](auto &&pipelines) {
            auto &&pipeline = std::move(pipelines[0]);
            nameObject(device, pipeline, name);
            nameObject(device, pipelineLayout, name);
            return std::make_tuple(std::move(pipelineLayout), std::move(pipeline));
        });
}

} // namespace VkUtils
