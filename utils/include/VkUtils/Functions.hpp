#pragma once

#include "CommandBufferContext.hpp"

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
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace VkUtils {
auto checkValidationLayerSupport(const std::vector<const char *> &validationLayers) -> bool;

// returnes set of unsupported extensions
auto checkDeviceExtensionSupport(const VkBindings::PhysicalDevice &queryDevice,
                                 const std::vector<const char *> &requiredExtensions)
    -> std::set<std::string>;

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

[[nodiscard]] auto
createImage(const VkBindings::PhysicalDevice &physicalDevice, const VkBindings::Device &device,
            VkBindings::Extent2D extent, VkBindings::Format format, VkBindings::ImageTiling tiling,
            VkBindings::ImageUsageFlags usage, VkBindings::MemoryPropertyFlags properties)
    -> std::expected<std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory>,
                     VkBindings::Result>;

auto findMemoryType(const VkBindings::PhysicalDevice &physicalDevice, uint32_t typeFilter,
                    VkBindings::MemoryPropertyFlags properties) -> uint32_t;

auto hasStencilComponent(VkBindings::Format format) -> bool;

[[nodiscard]] auto createBuffer(const VkBindings::PhysicalDevice &physicalDevice,
                                const VkBindings::Device &device, VkBindings::DeviceSize size,
                                VkBindings::BufferUsageFlags usage,
                                VkBindings::MemoryPropertyFlags properties)
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
                     VkBindings::Result>;

[[nodiscard]] auto beginSingleTimeCommands(const VkBindings::Device &device,
                                           const VkBindings::CommandPool &commandPool)
    -> std::expected<VkBindings::CommandBuffers, VkBindings::Result>;

[[nodiscard]] auto endSingleTimeCommands(const VkBindings::Queue &graphicsQueue,
                                         const VkBindings::CommandBuffers &oneShotCommandBuffers)
    -> VkBindings::Result;

void copyBufferToImage(CommandBufferContext &commandBufferContext, const VkBindings::Buffer &buffer,
                       const VkBindings::Image &image, VkBindings::Extent2D extent);
void copyImageToBuffer(CommandBufferContext &commandBufferContext, const VkBindings::Image &image,
                       const VkBindings::Buffer &buffer, const VkBindings::Extent3D &imageExtend);

[[nodiscard]] auto
createInitilisedBuffer(const VkBindings::PhysicalDevice &physicalDevice,
                       const VkBindings::Device &device, CommandBufferContext &commandBufferContext,
                       std::span<const std::byte> data, VkBindings::BufferUsageBits type)
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
                     VkBindings::Result>;

[[nodiscard]] auto
initiliseBuffer(const VkBindings::PhysicalDevice &physicalDevice, const VkBindings::Device &device,
                CommandBufferContext &commandBufferContext, const VkBindings::Buffer &buffer,
                VkBindings::DeviceSize offset, std::span<const std::byte> data)
    -> std::expected<void, VkBindings::Result>;

[[nodiscard]] auto createInitilisedBuffers(const VkBindings::PhysicalDevice &physicalDevice,
                                           const VkBindings::Device &device,
                                           CommandBufferContext &commandBufferContext, size_t count,
                                           std::span<const std::byte> data,
                                           VkBindings::BufferUsageFlags type)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueBuffer>,
                                std::vector<VkBindings::UniqueDeviceMemory>>,
                     VkBindings::Result>;

auto getAlignedOffset(VkBindings::DeviceSize offset, VkBindings::DeviceSize alignment)
    -> VkBindings::DeviceSize;

void transitionImageLayout(CommandBufferContext &commandBufferContext,
                           const VkBindings::Image &image, VkBindings::Format format,
                           VkBindings::ImageLayout &oldLayout, VkBindings::ImageLayout newLayout);

[[nodiscard]] auto createTextureImage(
    CommandBufferContext &commandBufferContext, const VkBindings::Device &device,
    const VkBindings::PhysicalDevice &physicalDevice,
    const std::function<std::tuple<std::pair<uint32_t, uint32_t>, std::span<const std::byte>>(
        const std::string &)> &textureGetter,
    const std::string &imageName)
    -> std::expected<std::tuple<std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory>,
                                VkBindings::ImageLayout>,
                     VkBindings::Result>;

[[nodiscard]] auto cleanupAquireSemaphore(const VkBindings::Queue &queue,
                                          const VkBindings::Semaphore &sem) -> VkBindings::Result;

}; // namespace VkUtils
