#pragma once

#include "CommandBufferContext.hpp"

#include <VmaBindings/VmaForward.hpp>

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Bits.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/Flags.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Structs.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <initializer_list>
#include <optional>
#include <set>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace VkUtils {
auto checkValidationLayerSupport(std::span<const char *const> validationLayers) -> bool;

// returnes set of unsupported extensions
auto checkDeviceExtensionSupport(const VkBindings::PhysicalDevice &queryDevice,
                                 std::span<const char *const> requiredExtensions)
    -> std::set<std::string_view>;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;

    static auto isComplete(const QueueFamilyIndices &indices) -> bool;
};

auto findQueueFamilies(const VkBindings::PhysicalDevice &queryDevice,
                       const VkBindings::SurfaceKHR &surface) -> QueueFamilyIndices;

struct SwapChainSupportDetails {
    VkBindings::SurfaceCapabilities2KHR capabilities = {};
    std::vector<VkBindings::SurfaceFormat2KHR> formats;
    std::vector<VkBindings::PresentModeKHR> presentModes;
};

[[nodiscard]] auto querySwapChainSupport(const VkBindings::PhysicalDevice &queryDevice,
                                         const VkBindings::SurfaceKHR &surface)
    -> std::expected<SwapChainSupportDetails, VkBindings::Result>;

[[nodiscard]] auto createShaderStages(
    const VkBindings::Device &device,
    const std::function<std::span<const uint32_t>(std::string_view)> &spirVGetter,
    std::span<const std::pair<std::string_view, VkBindings::ShaderStageBits>> shaders)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueShaderModule>,
                                std::vector<VkBindings::PipelineShaderStageCreateInfo>>,
                     VkBindings::Result>;

[[nodiscard]] auto createShaderStages(
    const VkBindings::Device &device,
    const std::function<std::span<const uint32_t>(std::string_view)> &spirVGetter,
    std::initializer_list<const std::pair<std::string_view, VkBindings::ShaderStageBits>> shaders)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueShaderModule>,
                                std::vector<VkBindings::PipelineShaderStageCreateInfo>>,
                     VkBindings::Result>;

auto findSupportedFormat(const std::vector<VkBindings::Format> &candiates,
                         VkBindings::ImageTiling tiling, VkBindings::FormatFeatureBits features)
    -> VkBindings::Format;

auto findSupportedFormat(const VkBindings::PhysicalDevice &physicalDevice,
                         const std::vector<VkBindings::Format> &candiates,
                         VkBindings::ImageTiling tiling, VkBindings::FormatFeatureBits features)
    -> VkBindings::Format;

[[nodiscard]] auto createImageView(const VkBindings::Device &device, const VkBindings::Image &image,
                                   VkBindings::Format format,
                                   VkBindings::ImageAspectFlags aspectFlags)
    -> std::expected<VkBindings::UniqueImageView, VkBindings::Result>;

auto hasStencilComponent(VkBindings::Format format) -> bool;

[[nodiscard]] auto beginSingleTimeCommands(const VkBindings::Device &device,
                                           const VkBindings::CommandPool &commandPool)
    -> std::expected<VkBindings::CommandBuffers, VkBindings::Result>;

[[nodiscard]] auto endSingleTimeCommands(const VkBindings::Queue &graphicsQueue,
                                         const VkBindings::CommandBuffers &oneShotCommandBuffers)
    -> VkBindings::Result;

[[nodiscard]] auto bufferUploadViaStaging(const VmaBindings::Allocator &allocator,
                                          const VkBindings::Buffer &buffer,
                                          VkBindings::DeviceSize offset,
                                          std::span<const std::span<const std::byte>> datas,
                                          CommandBufferContext &commandBufferContext)
    -> VkBindings::Result;
[[nodiscard]] auto
bufferUploadViaStaging(const VmaBindings::Allocator &allocator, const VkBindings::Buffer &buffer,
                       VkBindings::DeviceSize offset, std::span<const std::byte> data,
                       CommandBufferContext &commandBufferContext) -> VkBindings::Result;

auto createBufferSingleUpload(const VmaBindings::Allocator &allocator,
                              VkBindings::BufferCreateInfo bufferCreateInfo,
                              std::span<const std::span<const std::byte>> datas,
                              CommandBufferContext &commandBufferContext)
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation>,
                     VkBindings::Result>;
auto createBufferSingleUpload(const VmaBindings::Allocator &allocator,
                              VkBindings::BufferCreateInfo bufferCreateInfo,
                              std::span<const std::byte> data,
                              CommandBufferContext &commandBufferContext)
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation>,
                     VkBindings::Result>;

auto createBuffersSingleUpload(const VmaBindings::Allocator &allocator,
                               VkBindings::BufferCreateInfo bufferCreateInfo,
                               std::span<const std::span<const std::byte>> datas, size_t count,
                               CommandBufferContext &commandBufferContext)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueBuffer>,
                                std::vector<VmaBindings::UniqueAllocation>>,
                     VkBindings::Result>;
auto createBuffersSingleUpload(const VmaBindings::Allocator &allocator,
                               VkBindings::BufferCreateInfo bufferCreateInfo,
                               std::span<const std::byte> data, size_t count,
                               CommandBufferContext &commandBufferContext)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueBuffer>,
                                std::vector<VmaBindings::UniqueAllocation>>,
                     VkBindings::Result>;

auto getAlignedOffset(VkBindings::DeviceSize offset, VkBindings::DeviceSize alignment)
    -> VkBindings::DeviceSize;

void transitionImageLayout(CommandBufferContext &commandBufferContext,
                           const VkBindings::Image &image, VkBindings::Format format,
                           VkBindings::ImageLayout &oldLayout, VkBindings::ImageLayout newLayout);

[[nodiscard]] auto createTextureImage(
    const VmaBindings::Allocator &allocator, CommandBufferContext &commandBufferContext,
    const std::function<std::tuple<std::pair<uint32_t, uint32_t>, std::span<const std::byte>>(
        std::string_view)> &textureGetter,
    std::string_view imageName)
    -> std::expected<
        std::tuple<VkBindings::UniqueImage, VmaBindings::UniqueAllocation, VkBindings::ImageLayout>,
        VkBindings::Result>;

[[nodiscard]] auto cleanupAquireSemaphore(const VkBindings::Queue &queue,
                                          const VkBindings::Semaphore &sem) -> VkBindings::Result;

}; // namespace VkUtils
