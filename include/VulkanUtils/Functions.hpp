#pragma once

#include "CommandBufferContext.hpp"
#include "VkBindings/Objects.hpp"

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
auto checkValidationLayerSupport(const std::vector<const char *> &validationLayers) -> bool;

// returnes set of unsupported extensions
auto checkDeviceExtensionSupport(VkBindings::PhysicalDevice queryDevice,
                                 const std::vector<const char *> &requiredExtensions)
    -> std::set<std::string>;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    auto isComplete() -> bool;
};

auto findQueueFamilies(VkBindings::PhysicalDevice queryDevice, VkBindings::SurfaceKHR surface)
    -> QueueFamilyIndices;

struct SwapChainSupportDetails {
    VkBindings::SurfaceCapabilitiesKHR capabilities = {};
    std::vector<VkBindings::SurfaceFormatKHR> formats;
    std::vector<VkBindings::PresentModeKHR> presentModes;
};

[[nodiscard]] auto querySwapChainSupport(VkBindings::PhysicalDevice queryDevice,
                                         VkBindings::SurfaceKHR surface)
    -> std::expected<SwapChainSupportDetails, VkBindings::Result>;

[[nodiscard]] auto createShaderStages(
    VkBindings::Device device,
    std::function<std::span<const uint32_t>(const std::string &)> spirVGetter,
    const std::vector<std::pair<std::string, VkBindings::ShaderStageFlagBits>> &shaders)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueShaderModule>,
                                std::vector<VkBindings::PipelineShaderStageCreateInfo>>,
                     VkBindings::Result>;

auto findSupportedFormat(const std::vector<VkBindings::Format> &candiates,
                         VkBindings::ImageTiling tiling, VkBindings::FormatFeatureFlagBits features)
    -> VkBindings::Format;

auto findSupportedFormat(VkBindings::PhysicalDevice physicalDevice,
                         const std::vector<VkBindings::Format> &candiates,
                         VkBindings::ImageTiling tiling, VkBindings::FormatFeatureFlagBits features)
    -> VkBindings::Format;

[[nodiscard]] auto createImageView(VkBindings::Device device, VkBindings::Image image,
                                   VkBindings::Format format,
                                   VkBindings::ImageAspectFlags aspectFlags)
    -> std::expected<VkBindings::UniqueImageView, VkBindings::Result>;

[[nodiscard]] auto createImage(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
                               VkBindings::Extent2D extent, VkBindings::Format format,
                               VkBindings::ImageTiling tiling, VkBindings::ImageUsageFlags usage,
                               VkBindings::MemoryPropertyFlags properties)
    -> std::expected<std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory>,
                     VkBindings::Result>;

auto findMemoryType(VkBindings::PhysicalDevice physicalDevice, uint32_t typeFilter,
                    VkBindings::MemoryPropertyFlags properties) -> uint32_t;

auto hasStencilComponent(VkBindings::Format format) -> bool;

[[nodiscard]] auto createBuffer(VkBindings::PhysicalDevice physicalDevice,
                                VkBindings::Device device, VkBindings::DeviceSize size,
                                VkBindings::BufferUsageFlags usage,
                                VkBindings::MemoryPropertyFlags properties)
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
                     VkBindings::Result>;

[[nodiscard]] auto beginSingleTimeCommands(VkBindings::Device device,
                                           VkBindings::CommandPool commandPool)
    -> std::expected<VkBindings::CommandBuffers, VkBindings::Result>;

[[nodiscard]] auto endSingleTimeCommands(VkBindings::Queue graphicsQueue,
                                         VkBindings::CommandBuffers &oneShotCommandBuffers)
    -> VkBindings::Result;

void copyBuffer(CommandBufferContext &CBctx, VkBindings::Buffer srcBuffer,
                VkBindings::Buffer destBuffer, VkBindings::DeviceSize size,
                VkBindings::DeviceSize srcOffset = 0, VkBindings::DeviceSize dstOffset = 0);

void copyBufferToImage(CommandBufferContext &CBctx, VkBindings::Buffer buffer,
                       VkBindings::Image image, VkBindings::Extent2D extent);
void copyImageToBuffer(CommandBufferContext &CBctx, VkBindings::Image image,
                       VkBindings::Buffer buffer, uint32_t width, uint32_t height);

[[nodiscard]] auto createInitilisedBuffer(VkBindings::PhysicalDevice physicalDevice,
                                          VkBindings::Device device, CommandBufferContext &CBctx,
                                          VkBindings::DeviceSize size, uint8_t *data,
                                          VkBindings::BufferUsageFlagBits type)
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
                     VkBindings::Result>;

[[nodiscard]] auto initiliseBuffer(VkBindings::PhysicalDevice physicalDevice,
                                   VkBindings::Device device, CommandBufferContext &CBctx,
                                   VkBindings::Buffer buffer, VkBindings::DeviceSize offset,
                                   VkBindings::DeviceSize size, const uint8_t *data)
    -> std::expected<void, VkBindings::Result>;

[[nodiscard]] auto createInitilisedBuffers(VkBindings::PhysicalDevice physicalDevice,
                                           VkBindings::Device device, CommandBufferContext &CBctx,
                                           size_t count, VkBindings::DeviceSize size, uint8_t *data,
                                           VkBindings::BufferUsageFlags type)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueBuffer>,
                                std::vector<VkBindings::UniqueDeviceMemory>>,
                     VkBindings::Result>;

auto getAlignedOffset(VkBindings::DeviceSize offset, VkBindings::DeviceSize alignment)
    -> VkBindings::DeviceSize;

void transitionImageLayout(CommandBufferContext &CBctx, VkBindings::Image image,
                           VkBindings::Format format, VkBindings::ImageLayout &oldLayout,
                           VkBindings::ImageLayout newLayout);

[[nodiscard]] auto createTextureImage(
    CommandBufferContext &CBctx, VkBindings::Device device,
    VkBindings::PhysicalDevice physicalDevice,
    std::function<std::tuple<std::pair<uint32_t, uint32_t>, std::span<const unsigned char>>(
        const std::string &)> textureGetter,
    const std::string &imageName)
    -> std::expected<std::tuple<std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory>,
                                VkBindings::ImageLayout>,
                     VkBindings::Result>;

[[nodiscard]] auto cleanupAquireSemaphore(VkBindings::Queue queue, VkBindings::Semaphore sem)
    -> VkBindings::Result;

}; // namespace VkUtils
