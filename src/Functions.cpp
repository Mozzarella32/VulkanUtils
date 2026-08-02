#include "Functions.hpp"
#include "VkBindings/Constants.hpp"
#include "VkBindings/EnumToString.hpp"
#include "VkBindings/Enums.hpp"
#include "VkBindings/Structs.hpp"

#include "NameObject.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <expected>
#include <tuple>
#include <utility>

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
                std::string propLayerName(prop.layerName);
                return std::string(layerName) == propLayerName;
            });
        if (found == availableLayers.end()) {
            return false;
        }
    }
    return true;
}

// returns unsupported extensions
auto checkDeviceExtensionSupport(VkBindings::PhysicalDevice queryDevice,
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

auto QueueFamilyIndices::isComplete() -> bool {
    return graphicsFamily.has_value() && presentFamily.has_value();
}

auto findQueueFamilies(VkBindings::PhysicalDevice queryDevice, VkBindings::SurfaceKHR surface)
    -> QueueFamilyIndices {
    QueueFamilyIndices queueIndices;

    auto queueFamilies = queryDevice.getQueueFamilyProperties();

    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        const auto &queueFamily = queueFamilies[i];

        if ((queueFamily.queueFlags & VkBindings::QueueFlagBits::eGraphics) &&
            (queueFamily.queueFlags & VkBindings::QueueFlagBits::eCompute)) {
            queueIndices.graphicsFamily = i;
        }

        if (queryDevice.getSurfaceSupportKHR(i, surface)) {
            queueIndices.presentFamily = i;
        }

        if (queueIndices.isComplete()) {
            break;
        }
    }

    return queueIndices;
}

auto querySwapChainSupport(VkBindings::PhysicalDevice queryDevice, VkBindings::SurfaceKHR surface)
    -> std::expected<SwapChainSupportDetails, VkBindings::Result> {
    SwapChainSupportDetails details;
    return queryDevice.getSurfaceCapabilitiesKHR(surface)
        .and_then([&](auto capabilities) -> auto {
            details.capabilities = capabilities;
            return queryDevice.getSurfaceFormatsKHR(surface);
        })
        .and_then([&](auto &&formats) -> auto {
            details.formats = std::move(formats);
            return queryDevice.getSurfacePresentModesKHR(surface);
        })
        .transform([&](auto &&presentModes) -> auto {
            details.presentModes = presentModes;
            return details;
        });
}

auto createShaderStages(
    VkBindings::Device device,
    std::function<std::span<const uint32_t>(const std::string &)> spirVGetter,
    const std::vector<std::pair<std::string, VkBindings::ShaderStageFlagBits>> &shaders)
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

auto findSupportedFormat(VkBindings::PhysicalDevice physicalDevice,
                         const std::vector<VkBindings::Format> &candiates,
                         VkBindings::ImageTiling tiling, VkBindings::FormatFeatureFlagBits features)
    -> VkBindings::Format {
    for (VkBindings::Format format : candiates) {
        auto props = physicalDevice.getFormatProperties(format);
        if (tiling == VkBindings::ImageTiling::eLinear &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VkBindings::ImageTiling::eOptimal &&
                   (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

auto createImageView(VkBindings::Device device, VkBindings::Image image, VkBindings::Format format,
                     VkBindings::ImageAspectFlags aspectFlags)
    -> std::expected<VkBindings::UniqueImageView, VkBindings::Result> {
    VkBindings::ImageViewCreateInfo viewInfo;
    viewInfo.image = image;
    viewInfo.viewType = VkBindings::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange = {.aspectMask = aspectFlags,
                                 .baseMipLevel = 0,
                                 .levelCount = 1,
                                 .baseArrayLayer = 0,
                                 .layerCount = 1};

    return device.createImageView(viewInfo);
}

auto createImage(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
                 VkBindings::Extent2D extent, VkBindings::Format format,
                 VkBindings::ImageTiling tiling, VkBindings::ImageUsageFlags usage,
                 VkBindings::MemoryPropertyFlags properties)
    -> std::expected<std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory>,
                     VkBindings::Result> {

    VkBindings::ImageCreateInfo imageInfo;
    imageInfo.imageType = VkBindings::ImageType::e2D;
    imageInfo.extent = VkBindings::Extent3D(extent.width, extent.height, 1);
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VkBindings::ImageLayout::eUndefined;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VkBindings::SharingMode::eExclusive;
    imageInfo.samples = VkBindings::SampleCountFlagBits::e1;

    VkBindings::UniqueImage image;
    VkBindings::UniqueDeviceMemory memory;

    return device.createImage(imageInfo)
        .and_then([&](auto &&resImage) -> auto {
            image = std::move(resImage);
            auto memRequirements = device.getImageMemoryRequirements(image);

            VkBindings::MemoryAllocateInfo allocInfo;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex =
                findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
            return device.allocateMemory(allocInfo);
        })
        .and_then([&](auto &&mem) -> auto {
            memory = std::move(mem);
            return succeeded(device.bindImageMemory(image, memory, 0));
        })
        .transform([&]() -> auto { return std::make_tuple(std::move(image), std::move(memory)); });
}

auto findMemoryType(VkBindings::PhysicalDevice physicalDevice, uint32_t typeFilter,
                    VkBindings::MemoryPropertyFlags properties) -> uint32_t {
    auto memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

auto hasStencilComponent(VkBindings::Format format) -> bool {
    return format == VkBindings::Format::eD32SfloatS8Uint ||
           format == VkBindings::Format::eD24UnormS8Uint;
}

auto createBuffer(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
                  VkBindings::DeviceSize size, VkBindings::BufferUsageFlags usage,
                  VkBindings::MemoryPropertyFlags properties)
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
                     VkBindings::Result> {
    VkBindings::BufferCreateInfo bufferInfo;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VkBindings::SharingMode::eExclusive;

    VkBindings::UniqueBuffer buffer;
    VkBindings::UniqueDeviceMemory memory;
    auto placeholder =
        device.createBuffer(bufferInfo)
            .and_then([&](auto &&resBuffer) -> auto {
                buffer = std::move(resBuffer);

                auto memRequirements = device.getBufferMemoryRequirements(buffer);

                VkBindings::MemoryAllocateInfo allocInfo;
                ;
                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex =
                    findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

                return device.allocateMemory(allocInfo);
            })
            .and_then([&](auto &&resMemory) -> auto {
                memory = std::move(resMemory);
                return succeeded(device.bindBufferMemory(buffer, memory, 0));
            })
            .transform(
                [&]() -> auto { return std::make_tuple(std::move(buffer), std::move(memory)); });
    return placeholder;
}

auto createInitilisedBuffer(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
                            CommandBufferContext &CBctx, VkBindings::DeviceSize size, uint8_t *data,
                            VkBindings::BufferUsageFlagBits type)
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
                     VkBindings::Result> {
    VkBindings::UniqueBuffer buffer;
    VkBindings::UniqueDeviceMemory bufferMemory;
    return createBuffer(physicalDevice, device, size,
                        VkBindings::BufferUsageFlagBits::eTransferDst | type,
                        VkBindings::MemoryPropertyFlagBits::eDeviceLocal)
        .and_then([&](auto &&tuple) -> auto {
            std::tie(buffer, bufferMemory) = std::move(tuple);
            return initiliseBuffer(physicalDevice, device, CBctx, buffer, 0, size, data);
        })
        .and_then([&]() -> std::expected<
                            std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
                            VkBindings::Result> {
            return std::make_tuple(std::move(buffer), std::move(bufferMemory));
        });
}

auto initiliseBuffer(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
                     CommandBufferContext &CBctx, VkBindings::UniqueBuffer &buffer,
                     VkBindings::DeviceSize offset, VkBindings::DeviceSize size,
                     const uint8_t *data) -> std::expected<void, VkBindings::Result> {
    CommandBufferContextAdopted<VkBindings::UniqueBuffer> stagingBuffer{CBctx};
    CommandBufferContextAdopted<VkBindings::UniqueDeviceMemory> stagingBufferMemory{CBctx};

    return createBuffer(physicalDevice, device, size, VkBindings::BufferUsageFlagBits::eTransferSrc,
                        VkBindings::MemoryPropertyFlagBits::eHostVisible |
                            VkBindings::MemoryPropertyFlagBits::eHostCoherent)
        .and_then([&](auto &&tuple) -> auto {
            std::tie(stagingBuffer.get(), stagingBufferMemory.get()) = std::move(tuple);
            return device.mapMemory(stagingBufferMemory.get(), 0, size);
        })
        .transform([&](void *mapped_data) -> void {
            memcpy(mapped_data, data, size);
            device.unmapMemory(stagingBufferMemory.get());
            copyBuffer(CBctx, stagingBuffer.get(), buffer, size, 0, offset);
        });
}

auto createInitilisedBuffers(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
                             CommandBufferContext &CBctx, size_t count, VkBindings::DeviceSize size,
                             uint8_t *data, VkBindings::BufferUsageFlags type)
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
            memcpy(mapped_data, data, size);
            auto res = createBuffer(physicalDevice, device, size,
                                    VkBindings::BufferUsageFlagBits::eTransferDst | type,
                                    VkBindings::MemoryPropertyFlagBits::eDeviceLocal)
                           .transform([&](auto &&tuple) -> auto {
                               std::tie(buffers[i], buffersMemory[i]) = std::move(tuple);
                               copyBuffer(CBctx, stagingBuffer.get(), buffers[i], size);
                           });
            if (!res)
                return std::unexpected(res.error());
        }
        device.unmapMemory(stagingBufferMemory.get());
        return std::make_tuple(std::move(buffers), std::move(buffersMemory));
    };

    return createBuffer(physicalDevice, device, size, VkBindings::BufferUsageFlagBits::eTransferSrc,
                        VkBindings::MemoryPropertyFlagBits::eHostVisible |
                            VkBindings::MemoryPropertyFlagBits::eHostCoherent)
        .and_then([&](auto &&tuple) -> auto {
            std::tie(stagingBuffer.get(), stagingBufferMemory.get()) = std::move(tuple);
            return device.mapMemory(stagingBufferMemory.get(), 0, size);
        })
        .and_then(copyToTheBuffers);
}

auto getAlignedOffset(VkBindings::DeviceSize offset, VkBindings::DeviceSize alignment)
    -> VkBindings::DeviceSize {
    return (offset + alignment - 1) & ~(alignment - 1);
}

// Has size 1
auto beginSingleTimeCommands(VkBindings::Device &device, VkBindings::CommandPool commandPool)
    -> std::expected<VkBindings::CommandBuffers, VkBindings::Result> {
    VkBindings::CommandBufferAllocateInfo allocInfo;
    allocInfo.level = VkBindings::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkBindings::CommandBuffers commandBuffers;
    return device.allocateCommandBuffers(allocInfo)
        .and_then([&](auto &&commandBuffersRes) -> auto {
            commandBuffers = std::move(commandBuffersRes);
            VkBindings::CommandBufferBeginInfo beginInfo;
            beginInfo.flags = VkBindings::CommandBufferUsageFlagBits::eOneTimeSubmit;

            return succeeded(commandBuffers[0].begin(beginInfo));
        })
        .transform([&]() -> auto {
            // nameObject(device, commandBuffers, "signleTime");
            return std::move(commandBuffers);
        });
}

auto endSingleTimeCommands(VkBindings::Queue &graphicsQueue,
                           VkBindings::CommandBuffers &oneShotCommandBuffers)
    -> VkBindings::Result {
    auto commandBuffer = oneShotCommandBuffers[0];
    return succeeded(commandBuffer.end())
        .and_then([&]() -> auto {
            std::vector<VkBindings::impl_Struct::AssignableHandle<VkBindings::CommandBuffer>>
                submitInfoCommandBuffers;
            submitInfoCommandBuffers.emplace_back(commandBuffer.getHandle());
            VkBindings::SubmitInfo submitInfo;
            submitInfo.commandBuffers() = submitInfoCommandBuffers;
            return succeeded(graphicsQueue.submit({submitInfo}));
        })
        .transform([&]() -> auto { return graphicsQueue.waitIdle(); })
        .error_or(VkBindings::Result::eSuccess);
}

void copyBuffer(CommandBufferContext &CBctx, VkBindings::Buffer srcBuffer,
                VkBindings::Buffer destBuffer, VkBindings::DeviceSize size,
                VkBindings::DeviceSize srcOffset, VkBindings::DeviceSize dstOffset) {

    VkBindings::BufferCopy copyRegion;
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;

    CBctx.getBuffer().copyBuffer(srcBuffer, destBuffer, copyRegion);
}

void copyBufferToImage(CommandBufferContext &CBctx, VkBindings::Buffer buffer,
                       VkBindings::Image image, VkBindings::Extent2D extent) {
    VkBindings::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VkBindings::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {};
    region.imageExtent = {.width = extent.width, .height = extent.height, .depth = 1};

    CBctx.getBuffer().copyBufferToImage(buffer, image, VkBindings::ImageLayout::eTransferDstOptimal,
                                        region);
}

void copyImageToBuffer(CommandBufferContext &CBctx, VkBindings::Image image,
                       VkBindings::Buffer buffer, uint32_t width, uint32_t height) {
    VkBindings::BufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VkBindings::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {};
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    CBctx.getBuffer().copyBufferToImage(buffer, image, VkBindings::ImageLayout::eTransferDstOptimal,
                                        region);
}

void transitionImageLayout(CommandBufferContext &CBctx, VkBindings::Image image,
                           VkBindings::Format format, VkBindings::ImageLayout &oldLayout,
                           VkBindings::ImageLayout newLayout) {

    using enum VkBindings::ImageLayout;
    using enum VkBindings::PipelineStageFlagBits;

    using Access = VkBindings::AccessFlagBits;

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

    VkBindings::PipelineStageFlagBits sourceStage;
    VkBindings::PipelineStageFlagBits destinationStage;

    if (oldLayout == eUndefined && newLayout == eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = Access::eTransferWrite;
        sourceStage = eTopOfPipe;
        destinationStage = eTransfer;
    } else if (oldLayout == eTransferDstOptimal && newLayout == eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = Access::eTransferWrite;
        barrier.dstAccessMask = Access::eShaderRead;
        sourceStage = eTransfer;
        destinationStage = eFragmentShader;
    } else if (oldLayout == eUndefined && newLayout == eColorAttachmentOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = Access::eColorAttachmentWrite;
        sourceStage = eColorAttachmentOutput;
        destinationStage = eColorAttachmentOutput;
    } else if (oldLayout == eUndefined && (newLayout == eDepthAttachmentOptimal ||
                                           newLayout == eDepthStencilAttachmentOptimal)) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask =
            Access::eDepthStencilAttachmentWrite | Access::eDepthStencilAttachmentRead;
        sourceStage = eTopOfPipe;
        destinationStage = eEarlyFragmentTests;
    } else if (oldLayout == eColorAttachmentOptimal && newLayout == ePresentSrcKHR) {
        barrier.srcAccessMask = Access::eColorAttachmentWrite;
        barrier.dstAccessMask = {};
        sourceStage = eColorAttachmentOutput;
        destinationStage = eBottomOfPipe;
    } else if ((oldLayout == eDepthAttachmentOptimal || oldLayout == eStencilAttachmentOptimal) &&
               newLayout == eShaderReadOnlyOptimal) {
        barrier.srcAccessMask =
            Access::eDepthStencilAttachmentWrite | Access::eDepthStencilAttachmentRead;
        barrier.dstAccessMask = Access::eShaderRead;
        sourceStage = eLateFragmentTests;
        destinationStage = eFragmentShader;
    } else {
        throw std::invalid_argument(
            "unsupported layout transition: " + VkBindings::Reflections::enumToString(oldLayout) +
            " -> " + VkBindings::Reflections::enumToString(newLayout));
    }

    VkBindings::ImageAspectFlags aspectMask;
    if (format == VkBindings::Format::eD32SfloatS8Uint ||
        format == VkBindings::Format::eD24UnormS8Uint) {
        aspectMask =
            VkBindings::ImageAspectFlagBits::eDepth | VkBindings::ImageAspectFlagBits::eStencil;
    } else if (format == VkBindings::Format::eD32Sfloat ||
               format == VkBindings::Format::eD16Unorm) {
        aspectMask = VkBindings::ImageAspectFlagBits::eDepth;
    } else {
        aspectMask = VkBindings::ImageAspectFlagBits::eColor;
    }

    barrier.subresourceRange.aspectMask = aspectMask;

    CBctx.getBuffer().pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, {barrier});
    oldLayout = newLayout;
}

[[nodiscard]] auto createTextureImage(
    CommandBufferContext &CBctx, VkBindings::Device device,
    VkBindings::PhysicalDevice physicalDevice,
    std::function<std::tuple<std::pair<uint32_t, uint32_t>, std::span<const unsigned char>>(
        const std::string &)> textureGetter,
    const std::string &imageName)
    -> std::expected<std::tuple<std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory>,
                                VkBindings::ImageLayout>,
                     VkBindings::Result> {
    VkUtils::CommandBufferContextAdopted<VkBindings::UniqueBuffer> stagingBuffer{CBctx};
    VkUtils::CommandBufferContextAdopted<VkBindings::UniqueDeviceMemory> stagingBufferMemory{CBctx};
    VkBindings::ImageLayout layout = VkBindings::ImageLayout::eUndefined;

    const auto &[extent, pixels] = textureGetter(imageName);

    return VkUtils::createBuffer(physicalDevice, device, pixels.size(),
                                 VkBindings::BufferUsageFlagBits::eTransferSrc,
                                 VkBindings::MemoryPropertyFlagBits::eHostVisible |
                                     VkBindings::MemoryPropertyFlagBits::eHostCoherent)
        .and_then([&](auto &&tuple) -> auto {
            std::tie(stagingBuffer.get(), stagingBufferMemory.get()) = std::move(tuple);
            return device.mapMemory(stagingBufferMemory.get(), 0, pixels.size());
        })
        .and_then([&](void *data) -> auto {
            memcpy(data, pixels.data(), pixels.size());
            device.unmapMemory(stagingBufferMemory.get());
            return VkUtils::createImage(
                physicalDevice, device, {.width = extent.first, .height = extent.second},
                VkBindings::Format::eR8G8B8A8Srgb, VkBindings::ImageTiling::eOptimal,
                VkBindings::ImageUsageFlagBits::eTransferDst |
                    VkBindings::ImageUsageFlagBits::eSampled,
                VkBindings::MemoryPropertyFlagBits::eDeviceLocal);
        })
        .transform([&](auto &&tuple) -> auto {
            auto &[image, _] = tuple;
            VkUtils::transitionImageLayout(CBctx, image, VkBindings::Format::eR8G8B8A8Srgb, layout,
                                           VkBindings::ImageLayout::eTransferDstOptimal);
            VkUtils::copyBufferToImage(CBctx, stagingBuffer.get(), image,
                                       {extent.first, extent.second});
            VkUtils::transitionImageLayout(CBctx, image, VkBindings::Format::eR8G8B8A8Srgb, layout,
                                           VkBindings::ImageLayout::eShaderReadOnlyOptimal);
            return std::make_tuple(std::move(tuple), layout);
        });
}

}; // namespace VkUtils
