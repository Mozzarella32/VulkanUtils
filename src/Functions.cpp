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

bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers) {
    auto availableLayersRes =
        VkBindings::Instance::enumerateInstanceLayerProperties().transform_error(
            printFailedFunction("enumerateInstanceLayerProperties"));
    if (!availableLayersRes)
        return false;

    const auto &availableLayers = availableLayersRes.value();

    for (const char *layerName : validationLayers) {
        auto found = std::find_if(availableLayers.begin(), availableLayers.end(),
                                  [layerName](const VkBindings::LayerProperties &prop) {
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
std::set<std::string>
checkDeviceExtensionSupport(VkBindings::PhysicalDevice queryDevice,
                            const std::vector<const char *> &requiredExtensions) {

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

bool QueueFamilyIndices::isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
}

QueueFamilyIndices findQueueFamilies(VkBindings::PhysicalDevice queryDevice,
                                     VkBindings::SurfaceKHR surface) {
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

std::expected<SwapChainSupportDetails, VkBindings::Result>
querySwapChainSupport(VkBindings::PhysicalDevice queryDevice, VkBindings::SurfaceKHR surface) {
    SwapChainSupportDetails details;
    return queryDevice.getSurfaceCapabilitiesKHR(surface)
        .and_then([&](auto capabilities) {
            details.capabilities = capabilities;
            return queryDevice.getSurfaceFormatsKHR(surface);
        })
        .and_then([&](auto &&formats) {
            details.formats = std::move(formats);
            return queryDevice.getSurfacePresentModesKHR(surface);
        })
        .transform([&](auto &&presentModes) {
            details.presentModes = presentModes;
            return details;
        });
}

std::expected<std::tuple<std::vector<VkBindings::UniqueShaderModule>,
                         std::vector<VkBindings::PipelineShaderStageCreateInfo>>,
              VkBindings::Result>
createShaderStages(
    VkBindings::Device device,
    std::function<std::span<const uint32_t>(const std::string &)> spirVGetter,
    const std::vector<std::pair<std::string, VkBindings::ShaderStageFlagBits>> &shaders) {

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
        // device.nameObject(shaderModules.back(), name + " shader");
        VkBindings::PipelineShaderStageCreateInfo shaderStageInfo;
        shaderStageInfo.stage = type;
        shaderStageInfo.module = shaderModules.back();
        shaderStageInfo.name = "main";

        shaderStages.push_back(shaderStageInfo);
    }
    return std::make_tuple(std::move(shaderModules), std::move(shaderStages));
}

VkBindings::Format findSupportedFormat(VkBindings::PhysicalDevice physicalDevice,
                                       const std::vector<VkBindings::Format> &candiates,
                                       VkBindings::ImageTiling tiling,
                                       VkBindings::FormatFeatureFlagBits features) {
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

std::expected<VkBindings::UniqueImageView, VkBindings::Result>
createImageView(VkBindings::Device device, VkBindings::Image image, VkBindings::Format format,
                VkBindings::ImageAspectFlags aspectFlags) {
    VkBindings::ImageViewCreateInfo viewInfo;
    viewInfo.image = image;
    viewInfo.viewType = VkBindings::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange = VkBindings::ImageSubresourceRange{aspectFlags, 0, 1, 0, 1};

    return device.createImageView(viewInfo);
}

std::expected<std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory>,
              VkBindings::Result>
createImage(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
            VkBindings::Extent2D extent, VkBindings::Format format, VkBindings::ImageTiling tiling,
            VkBindings::ImageUsageFlags usage, VkBindings::MemoryPropertyFlags properties) {

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
        .and_then([&](auto &&resImage) {
            image = std::move(resImage);
            auto memRequirements = device.getImageMemoryRequirements(image);

            VkBindings::MemoryAllocateInfo allocInfo;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex =
                findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
            return device.allocateMemory(allocInfo);
        })
        .and_then([&](auto &&mem) {
            memory = std::move(mem);
            return succeeded(device.bindImageMemory(image, memory, 0));
        })
        .transform([&]() { return std::make_tuple(std::move(image), std::move(memory)); });
}

uint32_t findMemoryType(VkBindings::PhysicalDevice physicalDevice, uint32_t typeFilter,
                        VkBindings::MemoryPropertyFlags properties) {
    auto memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

bool hasStencilComponent(VkBindings::Format format) {
    return format == VkBindings::Format::eD32SfloatS8Uint ||
           format == VkBindings::Format::eD24UnormS8Uint;
}

std::expected<std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
              VkBindings::Result>
createBuffer(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
             VkBindings::DeviceSize size, VkBindings::BufferUsageFlags usage,
             VkBindings::MemoryPropertyFlags properties) {
    VkBindings::BufferCreateInfo bufferInfo;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VkBindings::SharingMode::eExclusive;

    VkBindings::UniqueBuffer buffer;
    VkBindings::UniqueDeviceMemory memory;
    return device.createBuffer(bufferInfo)
        .and_then([&](auto &&resBuffer) {
            buffer = std::move(resBuffer);

            auto memRequirements = device.getBufferMemoryRequirements(buffer);

            VkBindings::MemoryAllocateInfo allocInfo;
            ;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex =
                findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

            return device.allocateMemory(allocInfo);
        })
        .and_then([&](auto &&resMemory) {
            memory = std::move(resMemory);
            return succeeded(device.bindBufferMemory(buffer, memory, 0));
        })
        .transform([&]() { return std::make_tuple(std::move(buffer), std::move(memory)); });
}

std::expected<std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
              VkBindings::Result>
createInitilisedBuffer(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
                       CommandBufferContext &CBctx, VkBindings::DeviceSize size, uint8_t *data,
                       VkBindings::BufferUsageFlagBits type) {
    VkBindings::UniqueBuffer buffer;
    VkBindings::UniqueDeviceMemory bufferMemory;
    return createBuffer(physicalDevice, device, size,
                        VkBindings::BufferUsageFlagBits::eTransferDst | type,
                        VkBindings::MemoryPropertyFlagBits::eDeviceLocal)
        .and_then([&](auto &&tuple) {
            std::tie(buffer, bufferMemory) = std::move(tuple);
            return initiliseBuffer(physicalDevice, device, CBctx, buffer, 0, size, data);
        })
        .and_then([&]() -> std::expected<
                            std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory>,
                            VkBindings::Result> {
            return std::make_tuple(std::move(buffer), std::move(bufferMemory));
        });
}

std::expected<void, VkBindings::Result>
initiliseBuffer(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
                CommandBufferContext &CBctx, VkBindings::UniqueBuffer &buffer,
                VkBindings::DeviceSize offset, VkBindings::DeviceSize size, const uint8_t *data) {
    CommandBufferContextAdopted<VkBindings::UniqueBuffer> stagingBuffer{CBctx};
    CommandBufferContextAdopted<VkBindings::UniqueDeviceMemory> stagingBufferMemory{CBctx};

    return createBuffer(physicalDevice, device, size, VkBindings::BufferUsageFlagBits::eTransferSrc,
                        VkBindings::MemoryPropertyFlagBits::eHostVisible |
                            VkBindings::MemoryPropertyFlagBits::eHostCoherent)
        .and_then([&](auto &&tuple) {
            std::tie(stagingBuffer.get(), stagingBufferMemory.get()) = std::move(tuple);
            return device.mapMemory(stagingBufferMemory.get(), 0, size);
        })
        .transform([&](void *mapped_data) {
            memcpy(mapped_data, data, size);
            device.unmapMemory(stagingBufferMemory.get());
            copyBuffer(CBctx, stagingBuffer.get(), buffer, size, 0, offset);
        });
}

std::expected<
    std::tuple<std::vector<VkBindings::UniqueBuffer>, std::vector<VkBindings::UniqueDeviceMemory>>,
    VkBindings::Result>
createInitilisedBuffers(VkBindings::PhysicalDevice physicalDevice, VkBindings::Device device,
                        CommandBufferContext &CBctx, size_t count, VkBindings::DeviceSize size,
                        uint8_t *data, VkBindings::BufferUsageFlags type) {
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
                           .transform([&](auto &&tuple) {
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
        .and_then([&](auto &&tuple) {
            std::tie(stagingBuffer.get(), stagingBufferMemory.get()) = std::move(tuple);
            return device.mapMemory(stagingBufferMemory.get(), 0, size);
        })
        .and_then(copyToTheBuffers);
}

VkBindings::DeviceSize getAlignedOffset(VkBindings::DeviceSize offset,
                                        VkBindings::DeviceSize alignment) {
    return (offset + alignment - 1) & ~(alignment - 1);
}

// Has size 1
std::expected<VkBindings::CommandBuffers, VkBindings::Result>
beginSingleTimeCommands(VkBindings::Device &device, VkBindings::CommandPool commandPool) {
    VkBindings::CommandBufferAllocateInfo allocInfo;
    allocInfo.level = VkBindings::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkBindings::CommandBuffers commandBuffers;
    return device.allocateCommandBuffers(allocInfo)
        .and_then([&](auto &&commandBuffersRes) {
            commandBuffers = std::move(commandBuffersRes);
            VkBindings::CommandBufferBeginInfo beginInfo;
            beginInfo.flags = VkBindings::CommandBufferUsageFlagBits::eOneTimeSubmit;

            return succeeded(commandBuffers[0].begin(beginInfo));
        })
        .transform([&]() {
            // nameObject(device, commandBuffers, "signleTime");
            return std::move(commandBuffers);
        });
}

VkBindings::Result endSingleTimeCommands(VkBindings::Queue &graphicsQueue,
                                         VkBindings::CommandBuffers &oneShotCommandBuffers) {
    auto commandBuffer = oneShotCommandBuffers[0];
    return succeeded(commandBuffer.end())
        .and_then([&]() {
            std::vector<VkBindings::impl_Struct::AssignableHandle<VkBindings::CommandBuffer>>
                submitInfoCommandBuffers;
            submitInfoCommandBuffers.emplace_back(commandBuffer.getHandle());
            VkBindings::SubmitInfo submitInfo;
            submitInfo.commandBuffers() = submitInfoCommandBuffers;
            return succeeded(graphicsQueue.submit({submitInfo}));
        })
        .transform([&]() { return graphicsQueue.waitIdle(); })
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
    region.imageExtent = {extent.width, extent.height, 1};

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

    if (oldLayout == VkBindings::ImageLayout::eUndefined &&
        newLayout == VkBindings::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = VkBindings::AccessFlagBits::eTransferWrite;
        sourceStage = VkBindings::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = VkBindings::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == VkBindings::ImageLayout::eTransferDstOptimal &&
               newLayout == VkBindings::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = VkBindings::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = VkBindings::AccessFlagBits::eShaderRead;
        sourceStage = VkBindings::PipelineStageFlagBits::eTransfer;
        destinationStage = VkBindings::PipelineStageFlagBits::eFragmentShader;
    } else if (oldLayout == VkBindings::ImageLayout::eUndefined &&
               newLayout == VkBindings::ImageLayout::eColorAttachmentOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = VkBindings::AccessFlagBits::eColorAttachmentWrite;
        sourceStage = VkBindings::PipelineStageFlagBits::eColorAttachmentOutput;
        destinationStage = VkBindings::PipelineStageFlagBits::eColorAttachmentOutput;
    } else if (oldLayout == VkBindings::ImageLayout::eUndefined &&
               (newLayout == VkBindings::ImageLayout::eDepthAttachmentOptimal ||
                newLayout == VkBindings::ImageLayout::eDepthStencilAttachmentOptimal)) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = VkBindings::AccessFlagBits::eDepthStencilAttachmentWrite |
                                VkBindings::AccessFlagBits::eDepthStencilAttachmentRead;
        sourceStage = VkBindings::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = VkBindings::PipelineStageFlagBits::eEarlyFragmentTests;
    } else if (oldLayout == VkBindings::ImageLayout::eColorAttachmentOptimal &&
               newLayout == VkBindings::ImageLayout::ePresentSrcKHR) {
        barrier.srcAccessMask = VkBindings::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = {};
        sourceStage = VkBindings::PipelineStageFlagBits::eColorAttachmentOutput;
        destinationStage = VkBindings::PipelineStageFlagBits::eBottomOfPipe;
    } else if ((oldLayout == VkBindings::ImageLayout::eDepthAttachmentOptimal ||
                oldLayout == VkBindings::ImageLayout::eStencilAttachmentOptimal) &&
               newLayout == VkBindings::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = VkBindings::AccessFlagBits::eDepthStencilAttachmentWrite |
                                VkBindings::AccessFlagBits::eDepthStencilAttachmentRead;
        barrier.dstAccessMask = VkBindings::AccessFlagBits::eShaderRead;
        sourceStage = VkBindings::PipelineStageFlagBits::eLateFragmentTests;
        destinationStage = VkBindings::PipelineStageFlagBits::eFragmentShader;
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

[[nodiscard]] std::expected<
    std::tuple<std::tuple<VkBindings::UniqueImage, VkBindings::UniqueDeviceMemory>,
               VkBindings::ImageLayout>,
    VkBindings::Result>
createTextureImage(
    CommandBufferContext &CBctx, VkBindings::Device device,
    VkBindings::PhysicalDevice physicalDevice,
    std::function<std::tuple<std::pair<uint32_t, uint32_t>, std::span<const unsigned char>>(
        const std::string &)> textureGetter,
    const std::string &imageName) {
    VkUtils::CommandBufferContextAdopted<VkBindings::UniqueBuffer> stagingBuffer{CBctx};
    VkUtils::CommandBufferContextAdopted<VkBindings::UniqueDeviceMemory> stagingBufferMemory{CBctx};
    VkBindings::ImageLayout layout = VkBindings::ImageLayout::eUndefined;

    const auto &[extent, pixels] = textureGetter(imageName);

    return VkUtils::createBuffer(physicalDevice, device, pixels.size(),
                                 VkBindings::BufferUsageFlagBits::eTransferSrc,
                                 VkBindings::MemoryPropertyFlagBits::eHostVisible |
                                     VkBindings::MemoryPropertyFlagBits::eHostCoherent)
        .and_then([&](auto &&tuple) {
            std::tie(stagingBuffer.get(), stagingBufferMemory.get()) = std::move(tuple);
            return device.mapMemory(stagingBufferMemory.get(), 0, pixels.size());
        })
        .and_then([&](void *data) {
            memcpy(data, pixels.data(), pixels.size());
            device.unmapMemory(stagingBufferMemory.get());
            return VkUtils::createImage(physicalDevice, device, {extent.first, extent.second},
                                        VkBindings::Format::eR8G8B8A8Srgb,
                                        VkBindings::ImageTiling::eOptimal,
                                        VkBindings::ImageUsageFlagBits::eTransferDst |
                                            VkBindings::ImageUsageFlagBits::eSampled,
                                        VkBindings::MemoryPropertyFlagBits::eDeviceLocal);
        })
        .transform([&](auto &&tuple) {
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
