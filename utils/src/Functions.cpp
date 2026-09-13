#include "Functions.hpp"

#include "CommandBufferContext.hpp"
#include "Errorhandling.hpp"
#include "NameObject.hpp"

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Bits.hpp>
#include <VkBindings/Constants.hpp>
#include <VkBindings/EnumToString.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/Flags.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/StackContainer.hpp>
#include <VkBindings/Structs.hpp>

#include <VmaBindings/Vma.hpp>
#include <VmaBindings/VmaForward.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <functional>
#include <initializer_list>
#include <numeric>
#include <ranges>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace VkUtils {

auto checkValidationLayerSupport(std::span<const char *const> validationLayers) -> bool {
    auto availableLayersRes = VkBindings::enumerateInstanceLayerProperties().transform_error(
        printFailedFunction("enumerateInstanceLayerProperties"));
    if (!availableLayersRes)
        return false;

    const auto &availableLayers = availableLayersRes.value();

    for (const std::string_view layerName : validationLayers) {
        auto found = std::ranges::find_if(
            availableLayers, [layerName](const VkBindings::LayerProperties &prop) -> bool {
                return std::string(layerName) == std::string(prop.layerName);
            });
        if (found == availableLayers.end()) {
            return false;
        }
    }
    return true;
}

// returns unsupported extensions
auto checkDeviceExtensionSupport(const VkBindings::PhysicalDevice &queryDevice,
                                 std::span<const char *const> requiredExtensions)
    -> std::set<std::string_view> {

    std::set<std::string_view> unsupportedExtensions(requiredExtensions.begin(),
                                                     requiredExtensions.end());

    auto availableExtensionsRes = queryDevice.enumerateDeviceExtensionProperties().transform_error(
        printFailedFunction("enumerateExtensionProperties"));
    if (!availableExtensionsRes)
        return unsupportedExtensions;

    for (const auto &extension : availableExtensionsRes.value()) {
        unsupportedExtensions.erase(std::string_view{std::string{extension.extensionName}});
    }
    return unsupportedExtensions;
}

auto findQueueFamilies(const VkBindings::PhysicalDevice &queryDevice,
                       const VkBindings::SurfaceKHR &surface) -> QueueFamilyIndices {
    QueueFamilyIndices queueIndices;

    auto queueFamilies = queryDevice.getQueueFamilyProperties2();

    for (const auto &[index, queueFamily] : queueFamilies | std::views::enumerate) {
        const auto i = static_cast<uint32_t>(index);
        if ((queueFamily.queueFamilyProperties.queueFlags & VkBindings::QueueBits::Graphics)) {
            queueIndices.graphicsFamily = i;
        }

        if ((queueFamily.queueFamilyProperties.queueFlags & VkBindings::QueueBits::Compute)) {
            queueIndices.computeFamily = i;
        }

        if (queryDevice.getSurfaceSupportKHR(i, surface)) {
            queueIndices.presentFamily = i;
        }

        if (QueueFamilyIndices::isComplete(queueIndices)) {
            break;
        }
    }

    return queueIndices;
}

auto querySwapChainSupport(const VkBindings::PhysicalDevice &queryDevice,
                           const VkBindings::SurfaceKHR &surface)
    -> std::expected<SwapChainSupportDetails, VkBindings::Result> {
    VkBindings::PhysicalDeviceSurfaceInfo2KHR physicalDeviceSurfaceInfo;
    physicalDeviceSurfaceInfo.surface = surface;
    SwapChainSupportDetails details;
    return queryDevice.getSurfaceCapabilities2KHR(physicalDeviceSurfaceInfo)
        .and_then([&](VkBindings::SurfaceCapabilities2KHR capabilities) {
            details.capabilities = capabilities;
            return queryDevice.getSurfaceFormats2KHR(physicalDeviceSurfaceInfo);
        })
        .and_then([&](std::vector<VkBindings::SurfaceFormat2KHR> &&formats) {
            details.formats = std::move(formats);
            return queryDevice.getSurfacePresentModesKHR(surface);
        })
        .transform([&](std::vector<VkBindings::PresentModeKHR> &&presentModes) {
            details.presentModes = std::move(presentModes);
            return details;
        });
}

auto createShaderStages(
    const VkBindings::Device &device,
    const std::function<std::span<const uint32_t>(std::string_view)> &spirVGetter,
    std::span<const std::pair<std::string_view, VkBindings::ShaderStageBits>> shaders)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueShaderModule>,
                                std::vector<VkBindings::PipelineShaderStageCreateInfo>>,
                     VkBindings::Result> {

    std::vector<VkBindings::PipelineShaderStageCreateInfo> shaderStages;
    std::vector<VkBindings::UniqueShaderModule> shaderModules;
    for (const auto &[name, type] : shaders) {
        auto code = spirVGetter(name);
        auto shaderModuleRes =
            device.createShaderModule({.codeSize = code.size() * 4, .pCode = code.data()})
                .transform_error(printFailedFunction("createShaderModule"));
        if (!shaderModuleRes)
            return std::unexpected(shaderModuleRes.error());

        shaderModules.emplace_back(std::move(shaderModuleRes).value());
        nameObject(device, shaderModules.back(), name);

        shaderStages.push_back({.stage = type, .module = shaderModules.back(), .name = "main"});
    }
    return std::make_tuple(std::move(shaderModules), std::move(shaderStages));
}

auto createShaderStages(
    const VkBindings::Device &device,
    const std::function<std::span<const uint32_t>(std::string_view)> &spirVGetter,
    std::initializer_list<const std::pair<std::string_view, VkBindings::ShaderStageBits>> shaders)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueShaderModule>,
                                std::vector<VkBindings::PipelineShaderStageCreateInfo>>,
                     VkBindings::Result> {
    return createShaderStages(device, spirVGetter, std::span{shaders});
}

auto findSupportedFormat(const VkBindings::PhysicalDevice &physicalDevice,
                         const std::vector<VkBindings::Format> &candiates,
                         VkBindings::ImageTiling tiling, VkBindings::FormatFeatureBits features)
    -> VkBindings::Format {
    for (const VkBindings::Format &format : candiates) {
        auto props = physicalDevice.getFormatProperties2(format);
        if ((tiling == VkBindings::ImageTiling::Linear &&
             (props.formatProperties.linearTilingFeatures & features) == features) ||
            (tiling == VkBindings::ImageTiling::Optimal &&
             (props.formatProperties.optimalTilingFeatures & features) == features)) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

auto createImageView(const VkBindings::Device &device, const VkBindings::Image &image,
                     VkBindings::Format format, VkBindings::ImageAspectFlags aspectFlags)
    -> std::expected<VkBindings::UniqueImageView, VkBindings::Result> {
    return device.createImageView({.image = image,
                                   .viewType = VkBindings::ImageViewType::v2D,
                                   .format = format,
                                   .subresourceRange = {.aspectMask = aspectFlags,
                                                        .baseMipLevel = 0,
                                                        .levelCount = 1,
                                                        .baseArrayLayer = 0,
                                                        .layerCount = 1}});
}

auto hasStencilComponent(VkBindings::Format format) -> bool {
    return format == VkBindings::Format::D32SfloatS8Uint ||
           format == VkBindings::Format::D24UnormS8Uint;
}

auto createBufferSingleUpload(const VmaBindings::Allocator &allocator,
                              VkBindings::BufferCreateInfo bufferCreateInfo,
                              std::span<const std::span<const std::byte>> datas,
                              CommandBufferContext &commandBufferContext)
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation>,
                     VkBindings::Result> {

    bufferCreateInfo.usage |= VkBindings::BufferUsageBits::TransferDst;
    bufferCreateInfo.size =
        std::accumulate(datas.begin(), datas.end(), std::size_t{0},
                        [](std::size_t sum, auto bytes) { return sum + bytes.size(); });
    ;

    VkBindings::UniqueBuffer buffer;
    VmaBindings::UniqueAllocation allocation;

    return allocator
        .createBuffer(bufferCreateInfo, {.usage = VmaBindings::MemoryUsage::AutoPreferDevice})
        .and_then([&](std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation,
                                 VmaBindings::AllocationInfo> &&tuple) {
            std::tie(buffer, allocation, std::ignore) = std::move(tuple);
            return VkUtils::succeeded(
                bufferUploadViaStaging(allocator, buffer, 0, datas, commandBufferContext));
        })
        .transform([&]() { return std::make_tuple(std::move(buffer), std::move(allocation)); });
}
auto createBufferSingleUpload(const VmaBindings::Allocator &allocator,
                              VkBindings::BufferCreateInfo bufferCreateInfo,
                              std::span<const std::byte> data,
                              CommandBufferContext &commandBufferContext)
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation>,
                     VkBindings::Result> {
    return createBufferSingleUpload(allocator, bufferCreateInfo, std::array{data},
                                    commandBufferContext);
}

auto bufferUploadViaStaging(const VmaBindings::Allocator &allocator,
                            const VkBindings::Buffer &buffer, VkBindings::DeviceSize offset,
                            std::span<const std::span<const std::byte>> datas,
                            CommandBufferContext &commandBufferContext) -> VkBindings::Result {

    CommandBufferContextAdopted<VkBindings::UniqueBuffer> stagingBuffer{commandBufferContext};
    CommandBufferContextAdopted<VmaBindings::UniqueAllocation> stagingBufferAllocation{
        commandBufferContext};

    VkBindings::BufferCreateInfo stagingBufferCreateInfo;
    stagingBufferCreateInfo.size = datas.size();
    stagingBufferCreateInfo.usage = VkBindings::BufferUsageBits::TransferSrc;

    VmaBindings::AllocationCreateInfo stagingBufferAllocationCreateInfo;
    stagingBufferAllocationCreateInfo.usage = VmaBindings::MemoryUsage::Auto;
    stagingBufferAllocationCreateInfo.flags =
        VmaBindings::AllocationCreateBits::HostAccessSequentialWrite;

    return allocator.createBuffer(stagingBufferCreateInfo, stagingBufferAllocationCreateInfo)
        .and_then([&](std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation,
                                 VmaBindings::AllocationInfo> &&tuple) {
            std::tie(stagingBuffer.get(), stagingBufferAllocation.get(), std::ignore) =
                std::move(tuple);
            return VkUtils::succeeded(allocator.copyMemoryToAllocation(
                datas.data(), stagingBufferAllocation, offset, datas.size()));
        })
        .transform([&]() {
            commandBufferContext->copyBuffer(
                stagingBuffer, buffer,
                VkBindings::BufferCopy{.srcOffset = 0, .dstOffset = offset, .size = datas.size()});
        })
        .error_or(VkBindings::Result::Success);
}
auto bufferUploadViaStaging(const VmaBindings::Allocator &allocator,
                            const VkBindings::Buffer &buffer, VkBindings::DeviceSize offset,
                            std::span<const std::byte> data,
                            CommandBufferContext &commandBufferContext) -> VkBindings::Result {
    return bufferUploadViaStaging(allocator, buffer, offset, std::array{data},
                                  commandBufferContext);
}

auto createBuffersSingleUpload(const VmaBindings::Allocator &allocator,
                               VkBindings::BufferCreateInfo bufferCreateInfo,
                               std::span<const std::span<const std::byte>> datas, size_t count,
                               CommandBufferContext &commandBufferContext)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueBuffer>,
                                std::vector<VmaBindings::UniqueAllocation>>,
                     VkBindings::Result> {
    CommandBufferContextAdopted<VkBindings::UniqueBuffer> stagingBuffer{commandBufferContext};
    CommandBufferContextAdopted<VmaBindings::UniqueAllocation> stagingBufferAllocation{
        commandBufferContext};
    size_t totalSize =
        std::accumulate(datas.begin(), datas.end(), std::size_t{0},
                        [](std::size_t sum, auto bytes) { return sum + bytes.size(); });
    return allocator
        .createBuffer({.size = totalSize, .usage = VkBindings::BufferUsageBits::TransferSrc},
                      {.flags = VmaBindings::AllocationCreateBits::HostAccessSequentialWrite,
                       .usage = VmaBindings::MemoryUsage::Auto})
        .and_then([&](std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation,
                                 VmaBindings::AllocationInfo> &&tuple) {
            std::tie(stagingBuffer.get(), stagingBufferAllocation.get(), std::ignore) =
                std::move(tuple);
            size_t offset = 0;
            for (auto data : datas) {
                auto res = VkUtils::succeeded(allocator.copyMemoryToAllocation(
                    data.data(), stagingBufferAllocation, offset, data.size()));
                offset += data.size();
                if (!res)
                    return res;
            }
            return VkUtils::succeeded(VkBindings::Result::Success);
        })
        .and_then([&]() -> std::expected<std::tuple<std::vector<VkBindings::UniqueBuffer>,
                                                    std::vector<VmaBindings::UniqueAllocation>>,
                                         VkBindings::Result> {
            std::vector<VkBindings::UniqueBuffer> buffers(count);
            std::vector<VmaBindings::UniqueAllocation> bufferAllocations(count);

            bufferCreateInfo.usage |= VkBindings::BufferUsageBits::TransferDst;
            bufferCreateInfo.size = totalSize;

            for (size_t i = 0; i < count; i++) {
                auto res =
                    allocator
                        .createBuffer(bufferCreateInfo,
                                      {.usage = VmaBindings::MemoryUsage::AutoPreferDevice})
                        .transform(
                            [&](std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation,
                                           VmaBindings::AllocationInfo> &&tuple) {
                                std::tie(buffers.at(i), bufferAllocations.at(i), std::ignore) =
                                    std::move(tuple);
                                commandBufferContext->copyBuffer(
                                    stagingBuffer, buffers.at(i),
                                    VkBindings::BufferCopy{.size = totalSize});
                            });
                if (!res) {
                    return std::unexpected(res.error()); // repackage error into new expected
                }
            }

            return std::make_tuple(std::move(buffers), std::move(bufferAllocations));
        });
}
auto createBuffersSingleUpload(const VmaBindings::Allocator &allocator,
                               VkBindings::BufferCreateInfo bufferCreateInfo,
                               std::span<const std::byte> data, size_t count,
                               CommandBufferContext &commandBufferContext)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueBuffer>,
                                std::vector<VmaBindings::UniqueAllocation>>,
                     VkBindings::Result> {
    return createBuffersSingleUpload(allocator, bufferCreateInfo, std::array{data}, count,
                                     commandBufferContext);
}

auto getAlignedOffset(VkBindings::DeviceSize offset, VkBindings::DeviceSize alignment)
    -> VkBindings::DeviceSize {
    return (offset + alignment - 1) & ~(alignment - 1);
}

// Has size 1
auto beginSingleTimeCommands(const VkBindings::Device &device,
                             const VkBindings::CommandPool &commandPool)
    -> std::expected<VkBindings::CommandBuffers, VkBindings::Result> {

    VkBindings::CommandBuffers commandBuffers;
    return device
        .allocateCommandBuffers({.commandPool = commandPool,
                                 .level = VkBindings::CommandBufferLevel::Primary,
                                 .commandBufferCount = 1})
        .and_then([&](VkBindings::CommandBuffers &&commandBuffersRes) {
            commandBuffers = std::move(commandBuffersRes);
            return succeeded(commandBuffers.at(0).begin(
                {.flags = VkBindings::CommandBufferUsageBits::OneTimeSubmit}));
        })
        .transform([&]() {
            nameObject(device, commandBuffers, "signleTime");
            return std::move(commandBuffers);
        });
}

auto endSingleTimeCommands(const VkBindings::Queue &graphicsQueue,
                           const VkBindings::CommandBuffers &oneShotCommandBuffers)
    -> VkBindings::Result {
    auto commandBuffer = oneShotCommandBuffers.at(0);
    return succeeded(commandBuffer.end())
        .and_then([&]() {
            auto submitInfoCommandBuffers =
                VkBindings::stackContainer(commandBuffer) |
                std::views::transform([](const auto &commandBuffer) {
                    return VkBindings::CommandBufferSubmitInfo{.commandBuffer = commandBuffer};
                }) |
                std::ranges::to<std::vector>();
            VkBindings::SubmitInfo2 submitInfo;
            submitInfo.commandBufferInfos() = submitInfoCommandBuffers;
            return succeeded(graphicsQueue.submit2({submitInfo}));
        })
        .transform([&]() { return graphicsQueue.waitIdle(); })
        .error_or(VkBindings::Result::Success);
}

void transitionImageLayout(CommandBufferContext &commandBufferContext,
                           const VkBindings::Image &image, VkBindings::Format format,
                           VkBindings::ImageLayout &oldLayout, VkBindings::ImageLayout newLayout) {

    using enum VkBindings::ImageLayout;
    using enum VkBindings::PipelineStageBits2;

    using Access = VkBindings::AccessBits2;

    VkBindings::ImageMemoryBarrier2 barrier{
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VkBindings::Constants::QueueFamilyIgnored,
        .dstQueueFamilyIndex = VkBindings::Constants::QueueFamilyIgnored,
        .image = image,
        .subresourceRange = {
            .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}};

    if (oldLayout == Undefined && newLayout == TransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = Access::TransferWrite;
        barrier.srcStageMask = TopOfPipe;
        barrier.dstStageMask = AllTransfer;
    } else if (oldLayout == TransferDstOptimal && newLayout == ShaderReadOnlyOptimal) {
        barrier.srcAccessMask = Access::TransferWrite;
        barrier.dstAccessMask = Access::ShaderRead;
        barrier.srcStageMask = AllTransfer;
        barrier.dstStageMask = FragmentShader;
    } else if (oldLayout == Undefined && newLayout == ColorAttachmentOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = Access::ColorAttachmentWrite;
        barrier.srcStageMask = ColorAttachmentOutput;
        barrier.dstStageMask = ColorAttachmentOutput;
    } else if (oldLayout == Undefined && (newLayout == DepthAttachmentOptimal ||
                                          newLayout == DepthStencilAttachmentOptimal)) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask =
            Access::DepthStencilAttachmentWrite | Access::DepthStencilAttachmentRead;
        barrier.srcStageMask = TopOfPipe;
        barrier.dstStageMask = EarlyFragmentTests;
    } else if (oldLayout == ColorAttachmentOptimal && newLayout == PresentSrcKHR) {
        barrier.srcAccessMask = Access::ColorAttachmentWrite;
        barrier.dstAccessMask = {};
        barrier.srcStageMask = ColorAttachmentOutput;
        barrier.dstStageMask = BottomOfPipe;
    } else if ((oldLayout == DepthAttachmentOptimal || oldLayout == StencilAttachmentOptimal) &&
               newLayout == ShaderReadOnlyOptimal) {
        barrier.srcAccessMask =
            Access::DepthStencilAttachmentWrite | Access::DepthStencilAttachmentRead;
        barrier.dstAccessMask = Access::ShaderRead;
        barrier.srcStageMask = LateFragmentTests;
        barrier.dstStageMask = FragmentShader;
    } else {
        throw std::invalid_argument("unsupported layout transition: " +
                                    std::string(VkBindings::Reflections::enumToString(oldLayout)) +
                                    " -> " +
                                    std::string(VkBindings::Reflections::enumToString(newLayout)));
    }

    if (format == VkBindings::Format::D32SfloatS8Uint ||
        format == VkBindings::Format::D24UnormS8Uint) {
        barrier.subresourceRange.aspectMask =
            VkBindings::ImageAspectBits::Depth | VkBindings::ImageAspectBits::Stencil;
    } else if (format == VkBindings::Format::D32Sfloat || format == VkBindings::Format::D16Unorm) {
        barrier.subresourceRange.aspectMask = VkBindings::ImageAspectBits::Depth;
    } else {
        barrier.subresourceRange.aspectMask = VkBindings::ImageAspectBits::Color;
    }

    VkBindings::DependencyInfo dependencyInfo;
    dependencyInfo.imageMemoryBarriers() = barrier;

    commandBufferContext->pipelineBarrier2(dependencyInfo);
    oldLayout = newLayout;
}

[[nodiscard]] auto createTextureImage(
    const VmaBindings::Allocator &allocator, CommandBufferContext &commandBufferContext,
    const std::function<std::tuple<std::pair<uint32_t, uint32_t>, std::span<const std::byte>>(
        std::string_view)> &textureGetter,
    std::string_view imageName)
    -> std::expected<
        std::tuple<VkBindings::UniqueImage, VmaBindings::UniqueAllocation, VkBindings::ImageLayout>,
        VkBindings::Result> {
    VkUtils::CommandBufferContextAdopted<VkBindings::UniqueBuffer> stagingBuffer{
        commandBufferContext};
    VkUtils::CommandBufferContextAdopted<VmaBindings::UniqueAllocation> stagingBufferAllocation{
        commandBufferContext};
    VkBindings::ImageLayout layout = VkBindings::ImageLayout::Undefined;

    const auto &[extent, pixels] = textureGetter(imageName);

    return allocator
        .createBuffer({.size = pixels.size(), .usage = VkBindings::BufferUsageBits::TransferSrc},
                      {.flags = VmaBindings::AllocationCreateBits::HostAccessSequentialWrite,
                       .usage = VmaBindings::MemoryUsage::Auto})
        .and_then([&](std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation,
                                 VmaBindings::AllocationInfo> &&tuple) {
            std::tie(stagingBuffer.get(), stagingBufferAllocation.get(), std::ignore) =
                std::move(tuple);
            return VkUtils::succeeded(allocator.copyMemoryToAllocation(
                pixels.data(), stagingBufferAllocation, 0, pixels.size()));
        })
        .and_then([&]() {
            return allocator.createImage(
                {
                    .format = VkBindings::Format::R8G8B8A8Srgb,
                    .extent = VkBindings::Extent3D{.width = extent.first, .height = extent.second},
                    .tiling = VkBindings::ImageTiling::Optimal,
                    .usage = VkBindings::ImageUsageBits::TransferDst |
                             VkBindings::ImageUsageBits::Sampled,
                },
                {.usage = VmaBindings::MemoryUsage::Auto});
        })
        .transform([&](std::tuple<VkBindings::UniqueImage, VmaBindings::UniqueAllocation,
                                  VmaBindings::AllocationInfo> &&tuple) {
            auto &&[image, imageAllocation, _] = std::move(tuple);
            VkUtils::transitionImageLayout(commandBufferContext, image,
                                           VkBindings::Format::R8G8B8A8Srgb, layout,
                                           VkBindings::ImageLayout::TransferDstOptimal);
            commandBufferContext->copyBufferToImage(
                stagingBuffer, image, layout,
                VkBindings::BufferImageCopy{
                    .imageSubresource = {.aspectMask = VkBindings::ImageAspectBits::Color,
                                         .layerCount = 1},
                    .imageExtent = {.width = extent.first, .height = extent.second, .depth = 1}});
            VkUtils::transitionImageLayout(commandBufferContext, image,
                                           VkBindings::Format::R8G8B8A8Srgb, layout,
                                           VkBindings::ImageLayout::ShaderReadOnlyOptimal);
            return std::make_tuple(std::move(image), std::move(imageAllocation), layout);
        });
}

[[nodiscard]] auto cleanupAquireSemaphore(const VkBindings::Queue &queue,
                                          const VkBindings::Semaphore &sem) -> VkBindings::Result {
    const VkBindings::SemaphoreSubmitInfo waitSemaphoreInfo{
        .semaphore = sem,
        .stageMask = VkBindings::PipelineStageBits2::BottomOfPipe,
    };
    VkBindings::SubmitInfo2 submitInfo;
    submitInfo.waitSemaphoreInfos() = waitSemaphoreInfo;
    return queue.submit2(submitInfo);
}
auto QueueFamilyIndices::isComplete(const QueueFamilyIndices &indices) -> bool {
    return indices.graphicsFamily.has_value() && indices.presentFamily.has_value() &&
           indices.computeFamily.has_value();
}
}; // namespace VkUtils
