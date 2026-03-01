#include "Functions.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <expected>
#include <tuple>
#include <utility>

namespace VkUtils {

bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers) {
    auto availableLayersRes =
        VkBindings::UniqueVkInstance::enumerateInstanceLayerProperties().transform_error(
            printFailedFunction("enumerateInstanceLayerProperties"));
    if (!availableLayersRes)
        return false;

    const auto &availableLayers = availableLayersRes.value();

    for (const char *layerName : validationLayers) {
        auto found = std::find_if(availableLayers.begin(), availableLayers.end(),
                                  [layerName](const VkLayerProperties &prop) {
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
checkDeviceExtensionSupport(const VkBindings::HandleVkPhysicalDevice &queryDevice,
                            const std::vector<const char *> &requiredExtensions) {

    std::set<std::string> unsupportedExtensions(requiredExtensions.begin(),
                                                requiredExtensions.end());

    auto availableExtensionsRes = queryDevice.enumerateExtensionProperties().transform_error(
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

QueueFamilyIndices findQueueFamilies(const VkBindings::HandleVkPhysicalDevice &queryDevice,
                                     VkBindings::UniqueVkSurfaceKHR &surface) {
    QueueFamilyIndices queueIndices;

    auto queueFamilies = queryDevice.getQueueFamilyProperties();

    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        const auto &queueFamily = queueFamilies[i];

        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
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

std::expected<SwapChainSupportDetails, VkResult>
querySwapChainSupport(const VkBindings::HandleVkPhysicalDevice &queryDevice,
                      VkBindings::UniqueVkSurfaceKHR &surface) {
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
        .and_then([&](auto &&presentModes) -> std::expected<SwapChainSupportDetails, VkResult> {
            details.presentModes = presentModes;
            return details;
        });
}

std::expected<std::tuple<std::vector<VkBindings::UniqueVkShaderModule>,
                         std::vector<VkPipelineShaderStageCreateInfo>>,
              VkResult>
createShaderStages(VkBindings::UniqueVkDevice &device,
                   std::function<std::span<const uint32_t>(const std::string &)> spirVGetter,
                   const std::vector<std::pair<std::string, VkShaderStageFlagBits>> &shaders) {

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    std::vector<VkBindings::UniqueVkShaderModule> shaderModules;
    for (const auto &[name, type] : shaders) {
        auto shaderModuleCreateInfo = VkBindings::Init<VkShaderModuleCreateInfo>();
        auto code = spirVGetter(name);
        shaderModuleCreateInfo.codeSize = code.size() * 4;
        shaderModuleCreateInfo.pCode = code.data();
        auto shaderModuleRes = device.createShaderModule(&shaderModuleCreateInfo)
                                   .transform_error(printFailedFunction("createShaderModule"));
        if (!shaderModuleRes)
            return std::unexpected(shaderModuleRes.error());
        shaderModules.emplace_back(std::move(shaderModuleRes).value());
        device.nameObject(shaderModules.back(), name + " shader");
        auto shaderStageInfo = VkBindings::Init<VkPipelineShaderStageCreateInfo>();
        shaderStageInfo.stage = type;
        shaderStageInfo.module = shaderModules.back();
        shaderStageInfo.pName = "main";

        shaderStages.push_back(shaderStageInfo);
    }
    return std::make_tuple(std::move(shaderModules), std::move(shaderStages));
}

VkFormat findSupportedFormat(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                             const std::vector<VkFormat> &candiates, VkImageTiling tiling,
                             VkFormatFeatureFlagBits features) {
    for (VkFormat format : candiates) {
        auto props = physicalDevice.getFormatProperties(format);
        if (tiling == VK_IMAGE_TILING_LINEAR &&
            static_cast<VkFormatFeatureFlagBits>(props.linearTilingFeatures & features) ==
                features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   static_cast<VkFormatFeatureFlagBits>(props.optimalTilingFeatures & features) ==
                       features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

std::expected<VkBindings::UniqueVkImageView, VkResult>
createImageView(VkBindings::UniqueVkDevice &device, VkImage image, VkFormat format,
                VkImageAspectFlags aspectFlags) {
    auto viewInfo = VkBindings::Init<VkImageViewCreateInfo>();
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange = VkImageSubresourceRange{aspectFlags, 0, 1, 0, 1};

    return device.createImageView(&viewInfo);
}

std::expected<std::tuple<VkBindings::UniqueVkImage, VkBindings::UniqueVkDeviceMemory>, VkResult>
createImage(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
            VkBindings::UniqueVkDevice &device, VkExtent2D extent, VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {

    auto imageInfo = VkBindings::Init<VkImageCreateInfo>();
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = VkExtent3D(extent.width, extent.height, 1);
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VkBindings::UniqueVkImage image;
    VkBindings::UniqueVkDeviceMemory memory;

    return device.createImage(&imageInfo)
        .and_then([&](auto &&resImage) {
            image = std::move(resImage);
            auto memRequirements = device.getImageMemoryRequirements(image);

            auto allocInfo = VkBindings::Init<VkMemoryAllocateInfo>();
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex =
                findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
            return device.allocateMemory(&allocInfo);
        })
        .and_then([&](auto &&mem) {
            memory = std::move(mem);
            return device.bindImageMemory(image, memory, 0);
        })
        .and_then(
            [&]() -> std::expected<
                      std::tuple<VkBindings::UniqueVkImage, VkBindings::UniqueVkDeviceMemory>,
                      VkResult> { return std::make_tuple(std::move(image), std::move(memory)); });
}

uint32_t findMemoryType(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                        uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

std::expected<std::tuple<VkBindings::UniqueVkBuffer, VkBindings::UniqueVkDeviceMemory>, VkResult>
createBuffer(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
             VkBindings::UniqueVkDevice &device, VkDeviceSize size, VkBufferUsageFlags usage,
             VkMemoryPropertyFlags properties) {
    auto bufferInfo = VkBindings::Init<VkBufferCreateInfo>();
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBindings::UniqueVkBuffer buffer;
    VkBindings::UniqueVkDeviceMemory memory;
    return device.createBuffer(&bufferInfo)
        .and_then([&](auto &&resBuffer) {
            buffer = std::move(resBuffer);

            auto memRequirements = device.getBufferMemoryRequirements(buffer);

            auto allocInfo = VkBindings::Init<VkMemoryAllocateInfo>();
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex =
                findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

            return device.allocateMemory(&allocInfo);
        })
        .and_then([&](auto &&resMemory) {
            memory = std::move(resMemory);
            return device.bindBufferMemory(buffer, memory, 0);
        })
        .and_then(
            [&]() -> std::expected<
                      std::tuple<VkBindings::UniqueVkBuffer, VkBindings::UniqueVkDeviceMemory>,
                      VkResult> { return std::make_tuple(std::move(buffer), std::move(memory)); });
}

std::expected<std::tuple<VkBindings::UniqueVkBuffer, VkBindings::UniqueVkDeviceMemory>, VkResult>
createInitilisedBuffer(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                       VkBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
                       VkDeviceSize size, uint8_t *data, VkBufferUsageFlagBits type) {
    VkBindings::UniqueVkBuffer buffer;
    VkBindings::UniqueVkDeviceMemory bufferMemory;
    return createBuffer(physicalDevice, device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | type,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        .and_then([&](auto &&tuple) {
            std::tie(buffer, bufferMemory) = std::move(tuple);
            return initiliseBuffer(physicalDevice, device, CBctx, buffer, 0, size, data);
        })
        .and_then(
            [&]() -> std::expected<
                      std::tuple<VkBindings::UniqueVkBuffer, VkBindings::UniqueVkDeviceMemory>,
                      VkResult> {
                return std::make_tuple(std::move(buffer), std::move(bufferMemory));
            });
}

std::expected<void, VkResult>
initiliseBuffer(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                VkBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
                VkBindings::UniqueVkBuffer &buffer, VkDeviceSize offset, VkDeviceSize size,
                const uint8_t *data) {
    CommandBufferContextAdopted<VkBindings::UniqueVkBuffer> stagingBuffer{CBctx};
    CommandBufferContextAdopted<VkBindings::UniqueVkDeviceMemory> stagingBufferMemory{CBctx};

    return createBuffer(physicalDevice, device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
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

std::expected<std::tuple<std::vector<VkBindings::UniqueVkBuffer>,
                         std::vector<VkBindings::UniqueVkDeviceMemory>>,
              VkResult>
createInitilisedBuffers(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                        VkBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
                        size_t count, VkDeviceSize size, uint8_t *data, VkBufferUsageFlags type) {

    CommandBufferContextAdopted<VkBindings::UniqueVkBuffer> stagingBuffer{CBctx};
    CommandBufferContextAdopted<VkBindings::UniqueVkDeviceMemory> stagingBufferMemory{CBctx};

    auto copyToTheBuffers = [&](void *mapped_data)
        -> std::expected<std::tuple<std::vector<VkBindings::UniqueVkBuffer>,
                                    std::vector<VkBindings::UniqueVkDeviceMemory>>,
                         VkResult> {
        std::vector<VkBindings::UniqueVkBuffer> buffers(count);
        std::vector<VkBindings::UniqueVkDeviceMemory> buffersMemory(count);

        for (size_t i = 0; i < count; i++) {
            memcpy(mapped_data, data, size);
            auto res =
                createBuffer(physicalDevice, device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | type,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
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

    return createBuffer(physicalDevice, device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        .and_then([&](auto &&tuple) {
            std::tie(stagingBuffer.get(), stagingBufferMemory.get()) = std::move(tuple);
            return device.mapMemory(stagingBufferMemory.get(), 0, size);
        })
        .and_then(copyToTheBuffers);
}

VkDeviceSize getAlignedOffset(VkDeviceSize offset, VkDeviceSize alignment) {
    return (offset + alignment - 1) & ~(alignment - 1);
}

// Has size 1
std::expected<VkBindings::UniqueVkCommandBuffers, VkResult>
beginSingleTimeCommands(VkBindings::UniqueVkDevice &device,
                        VkBindings::UniqueVkCommandPool &commandPool) {
    auto allocInfo = VkBindings::Init<VkCommandBufferAllocateInfo>();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkBindings::UniqueVkCommandBuffers commandBuffers;
    return device.allocateCommandBuffers(&allocInfo)
        .and_then([&](auto &&commandBuffersRes) {
            commandBuffers = std::move(commandBuffersRes);
            auto beginInfo = VkBindings::Init<VkCommandBufferBeginInfo>();
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            return commandBuffers[0].begin(&beginInfo);
        })
        .and_then([&]() -> std::expected<VkBindings::UniqueVkCommandBuffers, VkResult> {
            device.nameObject(commandBuffers, "singleTime");
            return std::move(commandBuffers);
        });
}

std::expected<void, VkResult>
endSingleTimeCommands(VkBindings::HandleVkQueue &graphicsQueue,
                      VkBindings::UniqueVkCommandBuffers &oneShotCommandBuffers) {
    auto commandBuffer = oneShotCommandBuffers[0];
    return commandBuffer.end()
        .and_then([&]() {
            auto submitInfo = VkBindings::Init<VkSubmitInfo>();
            VkCommandBuffer buffer = commandBuffer;
            submitInfo.pCommandBuffers = &buffer;
            submitInfo.commandBufferCount = 1;

            return graphicsQueue.submit(&submitInfo);
        })
        .and_then([&]() { return graphicsQueue.waitIdle(); });
}

void copyBuffer(CommandBufferContext &CBctx, VkBuffer srcBuffer, VkBuffer destBuffer,
                VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset) {

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;

    CBctx.getBuffer().copyBuffer(srcBuffer, destBuffer, &copyRegion);
}

void copyBufferToImage(CommandBufferContext &CBctx, VkBuffer buffer, VkImage image,
                       VkExtent2D extent) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {};
    region.imageExtent = VkExtent3D{extent.width, extent.height, 1};

    CBctx.getBuffer().copyBufferToImage(buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        &region);
}

void copyImageToBuffer(CommandBufferContext &CBctx, VkImage image, VkBuffer buffer, uint32_t width,
                       uint32_t height) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {};
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    CBctx.getBuffer().copyBufferToImage(buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        &region);
}

void transitionImageLayout(CommandBufferContext &CBctx, VkBindings::UniqueVkImage &image,
                           VkFormat format, VkImageLayout newLayout) {

    VkImageLayout oldLayout = image.layout;

    VkImageMemoryBarrier barrier = VkBindings::Init<VkImageMemoryBarrier>();
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlagBits sourceStage;
    VkPipelineStageFlagBits destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    } else if ((oldLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition: " + std::to_string(oldLayout) +
                                    " -> " + std::to_string(newLayout));
    }

    VkImageAspectFlags aspectMask;
    if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    } else if (format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D16_UNORM) {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.aspectMask = aspectMask;

    CBctx.getBuffer().pipelineBarrier(sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1,
                                      &barrier);
    image.layout = newLayout;
}

std::expected<std::tuple<VkBindings::UniqueVkImage, VkBindings::UniqueVkDeviceMemory>, VkResult>
createTextureImage(CommandBufferContext &CBctx, VkBindings::UniqueVkDevice &device,
                   VkBindings::HandleVkPhysicalDevice physicalDevice,
                   std::function<std::tuple<std::pair<uint32_t, uint32_t>,
                                            std::span<const unsigned char>>(const std::string &)>
                       textureGetter,
                   const std::string &imageName) {
    VkUtils::CommandBufferContextAdopted<VkBindings::UniqueVkBuffer> stagingBuffer{CBctx};
    VkUtils::CommandBufferContextAdopted<VkBindings::UniqueVkDeviceMemory> stagingBufferMemory{
        CBctx};

    const auto &[extent, pixels] = textureGetter(imageName);

    return VkUtils::createBuffer(
               physicalDevice, device, pixels.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        .and_then([&](auto &&tuple) {
            std::tie(stagingBuffer.get(), stagingBufferMemory.get()) = std::move(tuple);
            return device.mapMemory(stagingBufferMemory.get(), 0, pixels.size());
        })
        .and_then([&](void *data) {
            memcpy(data, pixels.data(), pixels.size());
            device.unmapMemory(stagingBufferMemory.get());
            return VkUtils::createImage(physicalDevice, device, {extent.first, extent.second},
                                        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                            VK_IMAGE_USAGE_SAMPLED_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        })
        .and_then([&](auto &&tuple) -> std::expected<std::tuple<VkBindings::UniqueVkImage,
                                                                VkBindings::UniqueVkDeviceMemory>,
                                                     VkResult> {
            auto &[image, _] = tuple;
            VkUtils::transitionImageLayout(CBctx, image, VK_FORMAT_R8G8B8A8_SRGB,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            VkUtils::copyBufferToImage(CBctx, stagingBuffer.get(), image,
                                       {extent.first, extent.second});
            VkUtils::transitionImageLayout(CBctx, image, VK_FORMAT_R8G8B8A8_SRGB,
                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            return tuple;
        });
}

}; // namespace VkUtils
