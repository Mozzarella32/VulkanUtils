#pragma once

#include "CommandBufferContext.hpp"

#include <VkBindings/EnumToString.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Structs.hpp>

#include <expected>
#include <functional>
#include <optional>
#include <set>
#include <span>
#include <tuple>

namespace VkUtils {
bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers);

// returnes set of unsupported extensions
std::set<std::string>
checkDeviceExtensionSupport(VkBindings::PhysicalDevice queryDevice,
                            const std::vector<const char *> &requiredExtensions);

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete();
};

QueueFamilyIndices findQueueFamilies(VkBindings::PhysicalDevice queryDevice,
                                     VkBindings::SurfaceKHR surface);

struct SwapChainSupportDetails {
    VkBindings::SurfaceCapabilitiesKHR capabilities = {};
    std::vector<VkBindings::SurfaceFormatKHR> formats;
    std::vector<VkBindings::PresentModeKHR> presentModes;
};

[[nodiscard]] std::expected<SwapChainSupportDetails, VkBindings::Result>
querySwapChainSupport(VkBindings::PhysicalDevice queryDevice, VkBindings::SurfaceKHR surface);

[[nodiscard]] std::expected<std::tuple<std::vector<VkBindings::UniqueShaderModule>,
                                       std::vector<VkBindings::PipelineShaderStageCreateInfo>>,
                            VkBindings::Result>
createShaderStages(
    VkBindings::Device device,
    std::function<std::span<const uint32_t>(const std::string &)> spirVGetter,
    const std::vector<std::pair<std::string, VkBindings::ShaderStageFlagBits>> &shaders);

VkBindings::Format findSupportedFormat(const std::vector<VkBindings::Format> &candiates,
                                       VkBindings::ImageTiling tiling,
                                       VkBindings::FormatFeatureFlagBits features);

VkBindings::Format findSupportedFormat(VkBindings::PhysicalDevice physicalDevice,
                                       const std::vector<VkBindings::Format> &candiates,
                                       VkBindings::ImageTiling tiling,
                                       VkBindings::FormatFeatureFlagBits features);

[[nodiscard]] std::expected<VkBindings::UniqueImageView, VkBindings::Result>
createImageView(VkBindings::Device device, VkBindings::Image image, VkBindings::Format format,
                VkBindings::ImageAspectFlags aspectFlags);

[[nodiscard]] std::expected<std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory>,
                            VkBindings::Result>
createImage(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
            VkBindings::Extent2D extent, VkBindings::Format format, VkBindings::ImageTiling tiling,
            VkBindings::ImageUsageFlags usage, VkBindings::MemoryPropertyFlags properties);

uint32_t findMemoryType(VkBindings::PhysicalDevice physicalDevice, uint32_t typeFilter,
                        VkBindings::MemoryPropertyFlags properties);

bool hasStencilComponent(VkBindings::Format format);

[[nodiscard]] std::expected<std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
                            VkBindings::Result>
createBuffer(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
             VkBindings::DeviceSize size, VkBindings::BufferUsageFlags usage,
             VkBindings::MemoryPropertyFlags properties);

[[nodiscard]] std::expected<VkBindings::CommandBuffers, VkBindings::Result>
beginSingleTimeCommands(VkBindings::Device device, VkBindings::CommandPool commandPool);

[[nodiscard]] VkBindings::Result
endSingleTimeCommands(VkBindings::Queue graphicsQueue,
                      VkBindings::CommandBuffers &oneShotCommandBuffers);

void copyBuffer(CommandBufferContext &CBctx, VkBindings::Buffer srcBuffer,
                VkBindings::Buffer destBuffer, VkBindings::DeviceSize size,
                VkBindings::DeviceSize srcOffset = 0, VkBindings::DeviceSize dstOffset = 0);

void copyBufferToImage(CommandBufferContext &CBctx, VkBindings::Buffer buffer,
                       VkBindings::Image image, VkBindings::Extent2D extent);
void copyImageToBuffer(CommandBufferContext &CBctx, VkBindings::Image image,
                       VkBindings::Buffer buffer, uint32_t width, uint32_t height);

[[nodiscard]] std::expected<std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
                            VkBindings::Result>
createInitilisedBuffer(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
                       CommandBufferContext &CBctx, VkBindings::DeviceSize size, uint8_t *data,
                       VkBindings::BufferUsageFlagBits type);

[[nodiscard]] std::expected<void, VkBindings::Result>
initiliseBuffer(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
                CommandBufferContext &CBctx, VkBindings::Buffer buffer,
                VkBindings::DeviceSize offset, VkBindings::DeviceSize size, const uint8_t *data);

[[nodiscard]] std::expected<
    std::tuple<std::vector<VkBindings::UniqueBuffer>, std::vector<VkBindings::UniqueDeviceMemory>>,
    VkBindings::Result>
createInitilisedBuffers(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
                        CommandBufferContext &CBctx, size_t count, VkBindings::DeviceSize size,
                        uint8_t *data, VkBindings::BufferUsageFlags type);

VkBindings::DeviceSize getAlignedOffset(VkBindings::DeviceSize offset,
                                        VkBindings::DeviceSize alignment);

void transitionImageLayout(CommandBufferContext &CBctx, VkBindings::Image image,
                           VkBindings::Format format, VkBindings::ImageLayout &oldLayout,
                           VkBindings::ImageLayout newLayout);

[[nodiscard]] std::expected<
    std::tuple<std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory>,
               VkBindings::ImageLayout>,
    VkBindings::Result>
createTextureImage(
    CommandBufferContext &CBctx, VkBindings::Device device,
    VkBindings::PhysicalDevice physicalDevice,
    std::function<std::tuple<std::pair<uint32_t, uint32_t>, std::span<const unsigned char>>(
        const std::string &)> textureGetter,
    const std::string &imageName);

}; // namespace VkUtils
