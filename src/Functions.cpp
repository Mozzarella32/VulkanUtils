#include "Functions.hpp"
#include "CommandBufferContext.hpp"
#include "Errorhandling.hpp"
#include "NameObject.hpp"

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Constants.hpp>
#include <VkBindings/EnumToString.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/StackContainer.hpp>
#include <VkBindings/Structs.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <expected>
#include <format>
#include <functional>
#include <ranges>
#include <set>
#include <span>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace VkUtils {

auto checkValidationLayerSupport(const std::vector<const char *> &validationLayers) -> bool {
    auto availableLayersRes =
        VkBindings::Instance::enumerateInstanceLayerProperties().transform_error(
            printFailedFunction("enumerateInstanceLayerProperties"));
    if (!availableLayersRes)
        return false;

    const auto &availableLayers = availableLayersRes.value();

    for (const char *layerName : validationLayers) {
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
                                 const std::vector<const char *> &requiredExtensions)
    -> std::set<std::string> {

    std::set<std::string> unsupportedExtensions(requiredExtensions.begin(),
                                                requiredExtensions.end());

    auto availableExtensionsRes = queryDevice.enumerateDeviceExtensionProperties().transform_error(
        printFailedFunction("enumerateExtensionProperties"));
    if (!availableExtensionsRes)
        return unsupportedExtensions;

    for (const auto &extension : availableExtensionsRes.value()) {
        unsupportedExtensions.erase(extension.extensionName);
    }
    return unsupportedExtensions;
}

auto findQueueFamilies(const VkBindings::PhysicalDevice &queryDevice,
                       const VkBindings::SurfaceKHR &surface) -> QueueFamilyIndices {
    QueueFamilyIndices queueIndices;

    auto queueFamilies = queryDevice.getQueueFamilyProperties();

    for (const auto &[i, queueFamily] : queueFamilies | std::views::enumerate) {
        if ((queueFamily.queueFlags & VkBindings::QueueBits::Graphics) &&
            (queueFamily.queueFlags & VkBindings::QueueBits::Compute)) {
            queueIndices.graphicsFamily = i;
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
    SwapChainSupportDetails details;
    return queryDevice.getSurfaceCapabilitiesKHR(surface)
        .and_then([&](VkBindings::SurfaceCapabilitiesKHR capabilities) {
            details.capabilities = capabilities;
            return queryDevice.getSurfaceFormatsKHR(surface);
        })
        .and_then([&](std::vector<VkBindings::SurfaceFormatKHR> &&formats) {
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
    const std::function<std::span<const uint32_t>(const std::string &)> &spirVGetter,
    const std::vector<std::pair<std::string, VkBindings::ShaderStageBits>> &shaders)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueShaderModule>,
                                std::vector<VkBindings::PipelineShaderStageCreateInfo>>,
                     VkBindings::Result> {

    std::vector<VkBindings::PipelineShaderStageCreateInfo> shaderStages;
    std::vector<VkBindings::UniqueShaderModule> shaderModules;
    for (const auto &[name, type] : shaders) {
        VkBindings::ShaderModuleCreateInfo shaderModuleCreateInfo;
        auto code = spirVGetter(name);
        shaderModuleCreateInfo.codeSize = code.size() * 4;
        shaderModuleCreateInfo.pCode = code.data();
        auto shaderModuleRes = device.createShaderModule(shaderModuleCreateInfo)
                                   .transform_error(printFailedFunction("createShaderModule"));
        if (!shaderModuleRes)
            return std::unexpected(shaderModuleRes.error());

        shaderModules.emplace_back(std::move(shaderModuleRes).value());
        nameObject(device, shaderModules.back(), name + " shader");
        VkBindings::PipelineShaderStageCreateInfo shaderStageInfo;
        shaderStageInfo.stage = type;
        shaderStageInfo.module = shaderModules.back();
        shaderStageInfo.name = "main";

        shaderStages.push_back(shaderStageInfo);
    }
    return std::make_tuple(std::move(shaderModules), std::move(shaderStages));
}

auto findSupportedFormat(const VkBindings::PhysicalDevice &physicalDevice,
                         const std::vector<VkBindings::Format> &candiates,
                         VkBindings::ImageTiling tiling, VkBindings::FormatFeatureBits features)
    -> VkBindings::Format {
    for (const VkBindings::Format &format : candiates) {
        auto props = physicalDevice.getFormatProperties(format);
        if ((tiling == VkBindings::ImageTiling::Linear &&
             (props.linearTilingFeatures & features) == features) ||
            (tiling == VkBindings::ImageTiling::Optimal &&
             (props.optimalTilingFeatures & features) == features)) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

auto createImageView(const VkBindings::Device &device, const VkBindings::Image &image,
                     VkBindings::Format format, VkBindings::ImageAspectFlags aspectFlags)
    -> std::expected<VkBindings::UniqueImageView, VkBindings::Result> {
    VkBindings::ImageViewCreateInfo viewInfo;
    viewInfo.image = image;
    viewInfo.viewType = VkBindings::ImageViewType::v2D;
    viewInfo.format = format;
    viewInfo.subresourceRange = {.aspectMask = aspectFlags,
                                 .baseMipLevel = 0,
                                 .levelCount = 1,
                                 .baseArrayLayer = 0,
                                 .layerCount = 1};

    return device.createImageView(viewInfo);
}

auto createImage(const VkBindings::PhysicalDevice &physicalDevice, const VkBindings::Device &device,
                 VkBindings::Extent2D extent, VkBindings::Format format,
                 VkBindings::ImageTiling tiling, VkBindings::ImageUsageFlags usage,
                 VkBindings::MemoryPropertyFlags properties)
    -> std::expected<std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory>,
                     VkBindings::Result> {

    VkBindings::ImageCreateInfo imageInfo;
    imageInfo.imageType = VkBindings::ImageType::v2D;
    imageInfo.extent = VkBindings::Extent3D(extent.width, extent.height, 1);
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VkBindings::ImageLayout::Undefined;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VkBindings::SharingMode::Exclusive;
    imageInfo.samples = VkBindings::SampleCountBits::v1;

    VkBindings::UniqueImage image;
    VkBindings::UniqueDeviceMemory memory;

    return device.createImage(imageInfo)
        .and_then([&](VkBindings::UniqueImage &&resImage) {
            image = std::move(resImage);
            auto memRequirements = device.getImageMemoryRequirements(image);

            VkBindings::MemoryAllocateInfo allocInfo;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex =
                findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
            return device.allocateMemory(allocInfo);
        })
        .and_then([&](VkBindings::UniqueDeviceMemory &&mem) {
            memory = std::move(mem);
            return succeeded(device.bindImageMemory(image, memory, 0));
        })
        .transform([&]() { return std::make_tuple(std::move(image), std::move(memory)); });
}

auto findMemoryType(const VkBindings::PhysicalDevice &physicalDevice, uint32_t typeFilter,
                    VkBindings::MemoryPropertyFlags properties) -> uint32_t {
    auto memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (((typeFilter & (1 << i)) != 0U) &&
            (memProperties.memoryTypes.at(i).propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

auto hasStencilComponent(VkBindings::Format format) -> bool {
    return format == VkBindings::Format::D32SfloatS8Uint ||
           format == VkBindings::Format::D24UnormS8Uint;
}

auto createBuffer(const VkBindings::PhysicalDevice &physicalDevice,
                  const VkBindings::Device &device, VkBindings::DeviceSize size,
                  VkBindings::BufferUsageFlags usage, VkBindings::MemoryPropertyFlags properties)
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
                     VkBindings::Result> {
    VkBindings::BufferCreateInfo bufferInfo;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VkBindings::SharingMode::Exclusive;

    VkBindings::UniqueBuffer buffer;
    VkBindings::UniqueDeviceMemory memory;
    return device.createBuffer(bufferInfo)
        .and_then([&](VkBindings::UniqueBuffer &&resBuffer) {
            buffer = std::move(resBuffer);

            auto memRequirements = device.getBufferMemoryRequirements(buffer);

            VkBindings::MemoryAllocateInfo allocInfo;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex =
                findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

            return device.allocateMemory(allocInfo);
        })
        .and_then([&](VkBindings::UniqueDeviceMemory &&resMemory) {
            memory = std::move(resMemory);
            return succeeded(device.bindBufferMemory(buffer, memory, 0));
        })
        .transform([&]() { return std::make_tuple(std::move(buffer), std::move(memory)); });
}

auto createInitilisedBuffer(const VkBindings::PhysicalDevice &physicalDevice,
                            const VkBindings::Device &device, CommandBufferContext &CBctx,
                            std::span<const uint8_t> data, VkBindings::BufferUsageBits type)
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
                     VkBindings::Result> {
    VkBindings::UniqueBuffer buffer;
    VkBindings::UniqueDeviceMemory bufferMemory;
    return createBuffer(physicalDevice, device, data.size(),
                        VkBindings::BufferUsageBits::TransferDst | type,
                        VkBindings::MemoryPropertyBits::DeviceLocal)
        .and_then(
            [&](std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory> &&tuple) {
                std::tie(buffer, bufferMemory) = std::move(tuple);
                return initiliseBuffer(physicalDevice, device, CBctx, buffer, 0, data);
            })
        .transform([&]() { return std::make_tuple(std::move(buffer), std::move(bufferMemory)); });
}

auto initiliseBuffer(const VkBindings::PhysicalDevice &physicalDevice,
                     const VkBindings::Device &device, CommandBufferContext &CBctx,
                     const VkBindings::Buffer &buffer, VkBindings::DeviceSize offset,
                     std::span<const uint8_t> data) -> std::expected<void, VkBindings::Result> {

    CommandBufferContextAdopted<VkBindings::UniqueBuffer> stagingBuffer{CBctx};
    CommandBufferContextAdopted<VkBindings::UniqueDeviceMemory> stagingBufferMemory{CBctx};

    return createBuffer(physicalDevice, device, data.size(),
                        VkBindings::BufferUsageBits::TransferSrc,
                        VkBindings::MemoryPropertyBits::HostVisible |
                            VkBindings::MemoryPropertyBits::HostCoherent)
        .and_then(
            [&](std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory> &&tuple) {
                std::tie(stagingBuffer.get(), stagingBufferMemory.get()) = std::move(tuple);
                return device.mapMemory(stagingBufferMemory, 0, data.size());
            })
        .transform([&](void *mapped_data) {
            memcpy(mapped_data, data.data(), data.size());
            device.unmapMemory(stagingBufferMemory);
            CBctx.getBuffer().copyBuffer(
                stagingBuffer, buffer,
                VkBindings::BufferCopy{.srcOffset = 0, .dstOffset = offset, .size = data.size()});
        });
}

auto createInitilisedBuffers(const VkBindings::PhysicalDevice &physicalDevice,
                             const VkBindings::Device &device, CommandBufferContext &CBctx,
                             size_t count, std::span<const uint8_t> data,
                             VkBindings::BufferUsageFlags type)
    -> std::expected<std::tuple<std::vector<VkBindings::UniqueBuffer>,
                                std::vector<VkBindings::UniqueDeviceMemory>>,
                     VkBindings::Result> {
    CommandBufferContextAdopted<VkBindings::UniqueBuffer> stagingBuffer{CBctx};
    CommandBufferContextAdopted<VkBindings::UniqueDeviceMemory> stagingBufferMemory{CBctx};

    auto copyToTheBuffers = [&](void *mapped_data)
        -> std::expected<std::tuple<std::vector<VkBindings::UniqueBuffer>,
                                    std::vector<VkBindings::UniqueDeviceMemory>>,
                         VkBindings::Result> {
        std::vector<VkBindings::UniqueBuffer> buffers(count);
        std::vector<VkBindings::UniqueDeviceMemory> buffersMemory(count);

        for (size_t i = 0; i < count; i++) {
            memcpy(mapped_data, data.data(), data.size());
            auto res =
                createBuffer(physicalDevice, device, data.size(),
                             VkBindings::BufferUsageBits::TransferDst | type,
                             VkBindings::MemoryPropertyBits::DeviceLocal)
                    .transform([&](std::tuple<VkBindings::UniqueBuffer,
                                              VkBindings::UniqueDeviceMemory> &&tuple) {
                        std::tie(buffers.at(i), buffersMemory.at(i)) = std::move(tuple);
                        CBctx.getBuffer().copyBuffer(stagingBuffer, buffers.at(i),
                                                     VkBindings::BufferCopy{.size = data.size()});
                    });
            if (!res)
                return std::unexpected(res.error());
        }
        device.unmapMemory(stagingBufferMemory);
        return std::make_tuple(std::move(buffers), std::move(buffersMemory));
    };

    return createBuffer(physicalDevice, device, data.size(),
                        VkBindings::BufferUsageBits::TransferSrc,
                        VkBindings::MemoryPropertyBits::HostVisible |
                            VkBindings::MemoryPropertyBits::HostCoherent)
        .and_then(
            [&](std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory> &&tuple) {
                std::tie(stagingBuffer.get(), stagingBufferMemory.get()) = std::move(tuple);
                return device.mapMemory(stagingBufferMemory, 0, data.size());
            })
        .and_then(copyToTheBuffers);
}

auto getAlignedOffset(VkBindings::DeviceSize offset, VkBindings::DeviceSize alignment)
    -> VkBindings::DeviceSize {
    return (offset + alignment - 1) & ~(alignment - 1);
}

// Has size 1
auto beginSingleTimeCommands(const VkBindings::Device &device,
                             const VkBindings::CommandPool &commandPool)
    -> std::expected<VkBindings::CommandBuffers, VkBindings::Result> {
    VkBindings::CommandBufferAllocateInfo allocInfo;
    allocInfo.level = VkBindings::CommandBufferLevel::Primary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkBindings::CommandBuffers commandBuffers;
    return device.allocateCommandBuffers(allocInfo)
        .and_then([&](VkBindings::CommandBuffers &&commandBuffersRes) {
            commandBuffers = std::move(commandBuffersRes);
            VkBindings::CommandBufferBeginInfo beginInfo;
            beginInfo.flags = VkBindings::CommandBufferUsageBits::OneTimeSubmit;
            return succeeded(commandBuffers.at(0).begin(beginInfo));
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
            auto submitInfoCommandBuffers = VkBindings::stackContainer(commandBuffer);
            VkBindings::SubmitInfo submitInfo;
            submitInfo.commandBuffers() = submitInfoCommandBuffers;
            return succeeded(graphicsQueue.submit({submitInfo}));
        })
        .transform([&]() { return graphicsQueue.waitIdle(); })
        .error_or(VkBindings::Result::Success);
}

void copyBufferToImage(CommandBufferContext &CBctx, const VkBindings::Buffer &buffer,
                       const VkBindings::Image &image, VkBindings::Extent2D extent) {
    VkBindings::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VkBindings::ImageAspectBits::Color;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {};
    region.imageExtent = {.width = extent.width, .height = extent.height, .depth = 1};

    CBctx.getBuffer().copyBufferToImage(buffer, image, VkBindings::ImageLayout::TransferDstOptimal,
                                        region);
}

void copyImageToBuffer(CommandBufferContext &CBctx, const VkBindings::Image &image,
                       const VkBindings::Buffer &buffer, const VkBindings::Extent3D &imageExtend) {
    VkBindings::BufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VkBindings::ImageAspectBits::Color;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {};
    region.imageExtent = imageExtend;

    CBctx.getBuffer().copyBufferToImage(buffer, image, VkBindings::ImageLayout::TransferDstOptimal,
                                        region);
}

void transitionImageLayout(CommandBufferContext &CBctx, const VkBindings::Image &image,
                           VkBindings::Format format, VkBindings::ImageLayout &oldLayout,
                           VkBindings::ImageLayout newLayout) {

    using enum VkBindings::ImageLayout;
    using enum VkBindings::PipelineStageBits;

    using Access = VkBindings::AccessBits;

    VkBindings::ImageMemoryBarrier barrier;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VkBindings::Constants::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = VkBindings::Constants::QueueFamilyIgnored;
    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkBindings::PipelineStageBits sourceStage = {};
    VkBindings::PipelineStageBits destinationStage = {};

    if (oldLayout == Undefined && newLayout == TransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = Access::TransferWrite;
        sourceStage = TopOfPipe;
        destinationStage = Transfer;
    } else if (oldLayout == TransferDstOptimal && newLayout == ShaderReadOnlyOptimal) {
        barrier.srcAccessMask = Access::TransferWrite;
        barrier.dstAccessMask = Access::ShaderRead;
        sourceStage = Transfer;
        destinationStage = FragmentShader;
    } else if (oldLayout == Undefined && newLayout == ColorAttachmentOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = Access::ColorAttachmentWrite;
        sourceStage = ColorAttachmentOutput;
        destinationStage = ColorAttachmentOutput;
    } else if (oldLayout == Undefined && (newLayout == DepthAttachmentOptimal ||
                                          newLayout == DepthStencilAttachmentOptimal)) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask =
            Access::DepthStencilAttachmentWrite | Access::DepthStencilAttachmentRead;
        sourceStage = TopOfPipe;
        destinationStage = EarlyFragmentTests;
    } else if (oldLayout == ColorAttachmentOptimal && newLayout == PresentSrcKHR) {
        barrier.srcAccessMask = Access::ColorAttachmentWrite;
        barrier.dstAccessMask = {};
        sourceStage = ColorAttachmentOutput;
        destinationStage = BottomOfPipe;
    } else if ((oldLayout == DepthAttachmentOptimal || oldLayout == StencilAttachmentOptimal) &&
               newLayout == ShaderReadOnlyOptimal) {
        barrier.srcAccessMask =
            Access::DepthStencilAttachmentWrite | Access::DepthStencilAttachmentRead;
        barrier.dstAccessMask = Access::ShaderRead;
        sourceStage = LateFragmentTests;
        destinationStage = FragmentShader;
    } else {
        throw std::invalid_argument(std::format("unsupported layout transition: {} -> {}",
                                                VkBindings::Reflections::enumToString(oldLayout),
                                                VkBindings::Reflections::enumToString(newLayout)));
    }

    VkBindings::ImageAspectFlags aspectMask;
    if (format == VkBindings::Format::D32SfloatS8Uint ||
        format == VkBindings::Format::D24UnormS8Uint) {
        aspectMask = VkBindings::ImageAspectBits::Depth | VkBindings::ImageAspectBits::Stencil;
    } else if (format == VkBindings::Format::D32Sfloat || format == VkBindings::Format::D16Unorm) {
        aspectMask = VkBindings::ImageAspectBits::Depth;
    } else {
        aspectMask = VkBindings::ImageAspectBits::Color;
    }

    barrier.subresourceRange.aspectMask = aspectMask;

    CBctx.getBuffer().pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, {barrier});
    oldLayout = newLayout;
}

[[nodiscard]] auto createTextureImage(
    CommandBufferContext &CBctx, const VkBindings::Device &device,
    const VkBindings::PhysicalDevice &physicalDevice,
    const std::function<std::tuple<std::pair<uint32_t, uint32_t>, std::span<const unsigned char>>(
        const std::string &)> &textureGetter,
    const std::string &imageName)
    -> std::expected<std::tuple<std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory>,
                                VkBindings::ImageLayout>,
                     VkBindings::Result> {
    VkUtils::CommandBufferContextAdopted<VkBindings::UniqueBuffer> stagingBuffer{CBctx};
    VkUtils::CommandBufferContextAdopted<VkBindings::UniqueDeviceMemory> stagingBufferMemory{CBctx};
    VkBindings::ImageLayout layout = VkBindings::ImageLayout::Undefined;

    const auto &[extent, pixels] = textureGetter(imageName);

    return VkUtils::createBuffer(physicalDevice, device, pixels.size(),
                                 VkBindings::BufferUsageBits::TransferSrc,
                                 VkBindings::MemoryPropertyBits::HostVisible |
                                     VkBindings::MemoryPropertyBits::HostCoherent)
        .and_then(
            [&](std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory> &&tuple) {
                std::tie(stagingBuffer.get(), stagingBufferMemory.get()) = std::move(tuple);
                return device.mapMemory(stagingBufferMemory, 0, pixels.size());
            })
        .and_then([&](void *data) {
            memcpy(data, pixels.data(), pixels.size());
            device.unmapMemory(stagingBufferMemory);
            return VkUtils::createImage(
                physicalDevice, device, {.width = extent.first, .height = extent.second},
                VkBindings::Format::R8G8B8A8Srgb, VkBindings::ImageTiling::Optimal,
                VkBindings::ImageUsageBits::TransferDst | VkBindings::ImageUsageBits::Sampled,
                VkBindings::MemoryPropertyBits::DeviceLocal);
        })
        .transform(
            [&](std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory> &&tuple) {
                auto &[image, _] = tuple;
                VkUtils::transitionImageLayout(CBctx, image, VkBindings::Format::R8G8B8A8Srgb,
                                               layout, VkBindings::ImageLayout::TransferDstOptimal);
                VkUtils::copyBufferToImage(CBctx, stagingBuffer, image,
                                           {.width = extent.first, .height = extent.second});
                VkUtils::transitionImageLayout(CBctx, image, VkBindings::Format::R8G8B8A8Srgb,
                                               layout,
                                               VkBindings::ImageLayout::ShaderReadOnlyOptimal);
                return std::make_tuple(std::move(tuple), layout);
            });
}

[[nodiscard]] auto cleanupAquireSemaphore(const VkBindings::Queue &queue,
                                          const VkBindings::Semaphore &sem) -> VkBindings::Result {
    VkBindings::SubmitInfo submitInfo;
    submitInfo.waitSemaphores() = sem;
    auto waitStages = VkBindings::stackContainer(VkBindings::PipelineStageBits::BottomOfPipe);
    submitInfo.pWaitDstStageMask = waitStages.data();
    return queue.submit(submitInfo);
}
auto QueueFamilyIndices::isComplete(const QueueFamilyIndices &indices) -> bool {
    return indices.graphicsFamily.has_value() && indices.presentFamily.has_value();
}
}; // namespace VkUtils
