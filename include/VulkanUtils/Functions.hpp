#pragma once

#include "CommandBufferContext.hpp"

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Structs.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <set>
#include <span>
#include <string>
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

    static auto isComplete(const QueueFamilyIndices &indices) -> bool;
};

auto findQueueFamilies(const VkBindings::PhysicalDevice &queryDevice,
                       const VkBindings::SurfaceKHR &surface) -> QueueFamilyIndices;

struct SwapChainSupportDetails {
    VkBindings::SurfaceCapabilitiesKHR capabilities = {};
    std::vector<VkBindings::SurfaceFormatKHR> formats;
    std::vector<VkBindings::PresentModeKHR> presentModes;
};

[[nodiscard]] auto querySwapChainSupport(const VkBindings::PhysicalDevice &queryDevice,
                                         const VkBindings::SurfaceKHR &surface)
    -> std::expected<SwapChainSupportDetails, VkBindings::Result>;

[[nodiscard]] auto
createShaderStages(const VkBindings::Device &device,
                   const std::function<std::span<const uint32_t>(const std::string &)> &spirVGetter,
                   const std::vector<std::pair<std::string, VkBindings::ShaderStageBits>> &shaders)
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

void copyBufferToImage(CommandBufferContext &CBctx, const VkBindings::Buffer &buffer,
                       const VkBindings::Image &image, VkBindings::Extent2D extent);
void copyImageToBuffer(CommandBufferContext &CBctx, const VkBindings::Image &image,
                       const VkBindings::Buffer &buffer, const VkBindings::Extent3D &imageExtend);

[[nodiscard]] auto createInitilisedBuffer(const VkBindings::PhysicalDevice &physicalDevice,
                                          const VkBindings::Device &device,
                                          CommandBufferContext &CBctx, std::span<uint8_t> data,
                                          VkBindings::BufferUsageBits type)
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
                     VkBindings::Result>;

[[nodiscard]] auto initiliseBuffer(const VkBindings::PhysicalDevice &physicalDevice,
                                   const VkBindings::Device &device, CommandBufferContext &CBctx,
                                   const VkBindings::Buffer &buffer, VkBindings::DeviceSize offset,
                                   std::span<uint8_t> data)
    -> std::expected<void, VkBindings::Result>;

[[nodiscard]] auto
createInitilisedBuffers(const VkBindings::PhysicalDevice &physicalDevice,
                        const VkBindings::Device &device, CommandBufferContext &CBctx, size_t count,
                        std::span<uint8_t> data, VkBindings::BufferUsageFlags type)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueBuffer>,
                                std::vector<VkBindings::UniqueDeviceMemory>>,
                     VkBindings::Result>;

auto getAlignedOffset(VkBindings::DeviceSize offset, VkBindings::DeviceSize alignment)
    -> VkBindings::DeviceSize;

void transitionImageLayout(CommandBufferContext &CBctx, const VkBindings::Image &image,
                           VkBindings::Format format, VkBindings::ImageLayout &oldLayout,
                           VkBindings::ImageLayout newLayout);

[[nodiscard]] auto createTextureImage(
    CommandBufferContext &CBctx, const VkBindings::Device &device,
    const VkBindings::PhysicalDevice &physicalDevice,
    const std::function<std::tuple<std::pair<uint32_t, uint32_t>, std::span<const unsigned char>>(
        const std::string &)> &textureGetter,
    const std::string &imageName)
    -> std::expected<std::tuple<std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory>,
                                VkBindings::ImageLayout>,
                     VkBindings::Result>;

[[nodiscard]] auto cleanupAquireSemaphore(const VkBindings::Queue &queue,
                                          const VkBindings::Semaphore &sem) -> VkBindings::Result;

}; // namespace VkUtils
