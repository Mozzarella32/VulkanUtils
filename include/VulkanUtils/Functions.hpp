#pragma once

#include <VulkanObjects.hpp>

#include <expected>
#include <functional>
#include <iostream>
#include <optional>
#include <set>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <tuple>

#include "CommandBufferContext.hpp"

namespace VkUtils {

[[nodiscard]] inline auto printFailedFunction(const std::string &func) {
    return [func] [[nodiscard]] (VkResult res) {
        std::cerr << func << " failed with: " << VkBindings::impl::VkResultToString(res) << "\n";
        return res;
    };
}

[[nodiscard]] inline auto
throwFailed(const std::string &func,
            const std::source_location location = std::source_location::current()) {
    return [func, location](VkResult res) -> VkResult {
        std::stringstream str;
        str << std::string("in ") << location.file_name() << ": " << location.function_name()
            << ": " << std::to_string(location.column()) << ": " << func
            << ":\nfailed with: " << VkBindings::impl::VkResultToString(res) << "\n";

        throw std::runtime_error(str.str());
    };
}

template <typename T>
T unwrap(std::expected<T, VkResult> &&expected, const std::string &func,
         const std::source_location location = std::source_location::current()) {
    auto e = std::move(expected).transform_error(throwFailed(func, location));
    T tmp = std::move(e).value();
    return tmp;
}

inline void unwrap(std::expected<void, VkResult> &&expected, const std::string &func,
                   const std::source_location location = std::source_location::current()) {
    std::move(expected).transform_error(throwFailed(func, location)).value();
}

bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers);

// returnes set of unsupported extensions
std::set<std::string>
checkDeviceExtensionSupport(const VkBindings::HandleVkPhysicalDevice &queryDevice,
                            const std::vector<const char *> &requiredExtensions);

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete();
};

QueueFamilyIndices findQueueFamilies(const VkBindings::HandleVkPhysicalDevice &queryDevice,
                                     VkBindings::UniqueVkSurfaceKHR &surface);

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities = {};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

[[nodiscard]] std::expected<SwapChainSupportDetails, VkResult>
querySwapChainSupport(const VkBindings::HandleVkPhysicalDevice &queryDevice,
                      VkBindings::UniqueVkSurfaceKHR &surface);

[[nodiscard]] std::expected<std::tuple<std::vector<VkBindings::UniqueVkShaderModule>,
                                       std::vector<VkPipelineShaderStageCreateInfo>>,
                            VkResult>
createShaderStages(VkBindings::UniqueVkDevice &device,
                   std::function<std::span<const uint32_t>(const std::string &)> spirVGetter,
                   const std::vector<std::pair<std::string, VkShaderStageFlagBits>> &shaders);

VkFormat findSupportedFormat(const std::vector<VkFormat> &candiates, VkImageTiling tiling,
                             VkFormatFeatureFlagBits features);

VkFormat findSupportedFormat(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                             const std::vector<VkFormat> &candiates, VkImageTiling tiling,
                             VkFormatFeatureFlagBits features);

[[nodiscard]] std::expected<VkBindings::UniqueVkImageView, VkResult>
createImageView(VkBindings::UniqueVkDevice &device, VkImage image, VkFormat format,
                VkImageAspectFlags aspectFlags);

[[nodiscard]] std::expected<std::tuple<VkBindings::UniqueVkImage, VkBindings::UniqueVkDeviceMemory>,
                            VkResult>
createImage(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
            VkBindings::UniqueVkDevice &device, VkExtent2D extent, VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

uint32_t findMemoryType(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                        uint32_t typeFilter, VkMemoryPropertyFlags properties);

bool hasStencilComponent(VkFormat format);

[[nodiscard]] std::expected<
    std::tuple<VkBindings::UniqueVkBuffer, VkBindings::UniqueVkDeviceMemory>, VkResult>
createBuffer(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
             VkBindings::UniqueVkDevice &device, VkDeviceSize size, VkBufferUsageFlags usage,
             VkMemoryPropertyFlags properties);

[[nodiscard]] std::expected<VkBindings::UniqueVkCommandBuffers, VkResult>
beginSingleTimeCommands(VkBindings::UniqueVkDevice &device,
                        VkBindings::UniqueVkCommandPool &commandPool);

[[nodiscard]] std::expected<void, VkResult>
endSingleTimeCommands(VkBindings::HandleVkQueue &graphicsQueue,
                      VkBindings::UniqueVkCommandBuffers &oneShotCommandBuffers);

void copyBuffer(CommandBufferContext &CBctx, VkBuffer srcBuffer, VkBuffer destBuffer,
                VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);

void copyBufferToImage(CommandBufferContext &CBctx, VkBuffer buffer, VkImage image,
                       VkExtent2D extent);
void copyImageToBuffer(CommandBufferContext &CBctx, VkImage image, VkBuffer buffer, uint32_t width,
                       uint32_t height);

[[nodiscard]] std::expected<
    std::tuple<VkBindings::UniqueVkBuffer, VkBindings::UniqueVkDeviceMemory>, VkResult>
createInitilisedBuffer(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                       VkBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
                       VkDeviceSize size, uint8_t *data, VkBufferUsageFlagBits type);

[[nodiscard]] std::expected<void, VkResult>
initiliseBuffer(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                VkBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
                VkBindings::UniqueVkBuffer &buffer, VkDeviceSize offset, VkDeviceSize size,
                const uint8_t *data);

[[nodiscard]] std::expected<std::tuple<std::vector<VkBindings::UniqueVkBuffer>,
                                       std::vector<VkBindings::UniqueVkDeviceMemory>>,
                            VkResult>
createInitilisedBuffers(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                        VkBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
                        size_t count, VkDeviceSize size, uint8_t *data, VkBufferUsageFlags type);

VkDeviceSize getAlignedOffset(VkDeviceSize offset, VkDeviceSize alignment);

void transitionImageLayout(CommandBufferContext &CBctx, VkBindings::UniqueVkImage &image,
                           VkFormat format, VkImageLayout newLayout);

[[nodiscard]] std::expected<std::tuple<VkBindings::UniqueVkImage, VkBindings::UniqueVkDeviceMemory>,
                            VkResult>
createTextureImage(
    CommandBufferContext &CBctx, VkBindings::UniqueVkDevice &device,
    VkBindings::HandleVkPhysicalDevice physicalDevice,
    std::function<std::tuple<std::pair<uint32_t, uint32_t>, std::span<const unsigned char>>(
        const std::string &)> textureGetter,
    const std::string &imageName);

}; // namespace VkUtils
