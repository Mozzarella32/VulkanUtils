#include "PipelineBuilder.hpp"
#include "VulkanBindings.hpp"
#include "VulkanUtils.hpp"
#include <fstream>
#include <iostream>
#include <vector>

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

void PipelineCacheManager::read(VulkanBindings::UniqueVkDevice &device,
                                const std::filesystem::path &supplyed_cache_file) {
    cache_file = supplyed_cache_file;
    if (std::filesystem::exists(cache_file)) {
        std::ifstream i(cache_file);
        size_t size = 0;
        i.read((char *)&size, sizeof(size_t));
        std::vector<uint8_t> data(size);
        i.read((char *)data.data(), data.size());
        std::cout << "Read Pipline Cache: " << format_bytes(data.size()) << "\n";

        auto createInfo = VulkanBindings::Init<VkPipelineCacheCreateInfo>();
        createInfo.initialDataSize = data.size();
        createInfo.pInitialData = data.data();

        // fallback if cache is bad
        auto resPipelineCache = device.createPipelineCache(&createInfo);
        if (resPipelineCache) {
            pipelineCache = std::move(resPipelineCache.value());
            return;
        }
        std::cerr << "Cache was bad, falling back to new one: "
                  << VulkanBindings::impl::VkResultToString(resPipelineCache.error());
    }

    auto createInfo = VulkanBindings::Init<VkPipelineCacheCreateInfo>();
    createInfo.initialDataSize = 0;
    createInfo.pInitialData = nullptr;
    auto _ = device.createPipelineCache(&createInfo)
                 .transform_error(VkUtils::printFailedFunction("createPiplineCache"))
                 .transform(
                     [&](auto &&resPipelineCache) { pipelineCache = std::move(resPipelineCache); });
}

void PipelineCacheManager::write(VulkanBindings::UniqueVkDevice &device) {
    if (!pipelineCache)
        return;
    auto _ = device.getPiplineCacheData(pipelineCache)
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
    std::vector<std::pair<std::string, VkShaderStageFlagBits>> supplyed_shaders) {
    shaders = supplyed_shaders;
}

void PipelineBuilder::setInputAssembly(VkPrimitiveTopology topology,
                                       VkBool32 primitiveRestartEnable) {
    inputAssemblyState.topology = topology;
    inputAssemblyState.primitiveRestartEnable = primitiveRestartEnable;
}

void PipelineBuilder::setTessellation(uint32_t patchControlPoints) {
    tessellationState.patchControlPoints = patchControlPoints;
}

void PipelineBuilder::setViewportDynamic(uint32_t viewportCount, uint32_t scissorCount) {
    viewportState.viewportCount = viewportCount;
    viewportState.scissorCount = scissorCount;
    dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
}

void PipelineBuilder::setRasterization(VkPolygonMode polygonMode) {
    rasterizationState.polygonMode = polygonMode;
    rasterizationState.lineWidth = 1.0f;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

void PipelineBuilder::setRasterizationDepthPass() {
    rasterizationState.depthBiasEnable = VK_TRUE;
    rasterizationState.depthBiasConstantFactor = 1.75f;
    rasterizationState.depthBiasClamp = 0.0f;
    rasterizationState.depthBiasSlopeFactor = 3.00f;
}

void PipelineBuilder::setMultisample() {
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
}

void PipelineBuilder::setDepthEnabled() {
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
}

void PipelineBuilder::setStencilEnabled() { depthStencilState.stencilTestEnable = VK_TRUE; }

void PipelineBuilder::setNormalColorBlend() {
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachment;
}

void PipelineBuilder::addPushConstant(uint32_t offset, uint32_t size, VkShaderStageFlags stages) {
    pushConstantRanges.push_back({.stageFlags = stages, .offset = offset, .size = size});
}

void PipelineBuilder::addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) {
    descriptorSetLayouts.push_back(descriptorSetLayout);
}

void PipelineBuilder::setRenderingDepthAttachment(VkFormat depthFormat) {
    rendering.depthAttachmentFormat = depthFormat;
}

void PipelineBuilder::setRenderingStencilAttachment(VkFormat stencilFormat) {
    rendering.stencilAttachmentFormat = stencilFormat;
}

void PipelineBuilder::addRenderingColorAttachment(VkFormat colorAttachmentFormat) {
    colorAttachments.push_back(colorAttachmentFormat);
}

std::expected<std::tuple<VulkanBindings::UniqueVkPipelineLayout, VulkanBindings::UniqueVkPipeline>, VkResult>
PipelineBuilder::build(VulkanBindings::UniqueVkDevice &device, std::function<std::vector<uint32_t>&(std::string)> spirVGetter,
                       VkPipelineCache pipelineCache, const std::string &name) {
    auto pipelineLayoutInfo = VulkanBindings::Init<VkPipelineLayoutCreateInfo>();
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

    VulkanBindings::UniqueVkPipelineLayout pipelineLayout;
    return device.createPipelineLayout(&pipelineLayoutInfo)
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

            auto pipelineInfo = VulkanBindings::Init<VkGraphicsPipelineCreateInfo>();
            pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
            pipelineInfo.pStages = shaderStages.data();
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
            return device.createGraphicsPipeline(&pipelineInfo, pipelineCache);
        })
        .and_then(
            [&](auto &&pipeline)
                -> std::expected<std::tuple<VulkanBindings::UniqueVkPipelineLayout, VulkanBindings::UniqueVkPipeline>, VkResult> {
                if (name != "") {
                    device.nameObject(pipeline, name);
                    device.nameObject(pipelineLayout, name);
                }
                return std::make_tuple(std::move(pipelineLayout), std::move(pipeline));
            });
}
