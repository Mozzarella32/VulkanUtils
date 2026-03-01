#include "VulkanUtils.hpp"

#include "VulkanBindings.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <expected>
#include <tuple>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace VkUtils {

bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers) {
    auto availableLayersRes =
        VulkanBindings::UniqueVkInstance::enumerateInstanceLayerProperties().transform_error(
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
checkDeviceExtensionSupport(const VulkanBindings::HandleVkPhysicalDevice &queryDevice,
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

QueueFamilyIndices findQueueFamilies(const VulkanBindings::HandleVkPhysicalDevice &queryDevice,
                                     VulkanBindings::UniqueVkSurfaceKHR &surface) {
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
querySwapChainSupport(const VulkanBindings::HandleVkPhysicalDevice &queryDevice,
                      VulkanBindings::UniqueVkSurfaceKHR &surface) {
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

std::expected<std::tuple<std::vector<VulkanBindings::UniqueVkShaderModule>,
                         std::vector<VkPipelineShaderStageCreateInfo>>,
              VkResult>
createShaderStages(VulkanBindings::UniqueVkDevice &device,
                   std::function<std::span<const uint32_t>(const std::string &)> spirVGetter,
                   const std::vector<std::pair<std::string, VkShaderStageFlagBits>> &shaders) {

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    std::vector<VulkanBindings::UniqueVkShaderModule> shaderModules;
    for (const auto &[name, type] : shaders) {
        auto shaderModuleCreateInfo = VulkanBindings::Init<VkShaderModuleCreateInfo>();
        auto code = spirVGetter(name);
        shaderModuleCreateInfo.codeSize = code.size() * 4;
        shaderModuleCreateInfo.pCode = code.data();
        auto shaderModuleRes = device.createShaderModule(&shaderModuleCreateInfo)
                                   .transform_error(printFailedFunction("createShaderModule"));
        if (!shaderModuleRes)
            return std::unexpected(shaderModuleRes.error());
        shaderModules.emplace_back(std::move(shaderModuleRes).value());
        device.nameObject(shaderModules.back(), name + " shader");
        auto shaderStageInfo = VulkanBindings::Init<VkPipelineShaderStageCreateInfo>();
        shaderStageInfo.stage = type;
        shaderStageInfo.module = shaderModules.back();
        shaderStageInfo.pName = "main";

        shaderStages.push_back(shaderStageInfo);
    }
    return std::make_tuple(std::move(shaderModules), std::move(shaderStages));
}

VkFormat findSupportedFormat(const VulkanBindings::HandleVkPhysicalDevice &physicalDevice,
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

std::expected<VulkanBindings::UniqueVkImageView, VkResult>
createImageView(VulkanBindings::UniqueVkDevice &device, VkImage image, VkFormat format,
                VkImageAspectFlags aspectFlags) {
    auto viewInfo = VulkanBindings::Init<VkImageViewCreateInfo>();
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange = VkImageSubresourceRange{aspectFlags, 0, 1, 0, 1};

    return device.createImageView(&viewInfo);
}

std::expected<std::tuple<VulkanBindings::UniqueVkImage, VulkanBindings::UniqueVkDeviceMemory>,
              VkResult>
createImage(const VulkanBindings::HandleVkPhysicalDevice &physicalDevice,
            VulkanBindings::UniqueVkDevice &device, VkExtent2D extend, VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {

    auto imageInfo = VulkanBindings::Init<VkImageCreateInfo>();
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = VkExtent3D(extend.width, extend.height, 1);
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VulkanBindings::UniqueVkImage image;
    VulkanBindings::UniqueVkDeviceMemory memory;

    return device.createImage(&imageInfo)
        .and_then([&](auto &&resImage) {
            image = std::move(resImage);
            auto memRequirements = device.getImageMemoryRequirements(image);

            auto allocInfo = VulkanBindings::Init<VkMemoryAllocateInfo>();
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex =
                findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
            return device.allocateMemory(&allocInfo);
        })
        .and_then([&](auto &&mem) {
            memory = std::move(mem);
            return device.bindImageMemory(image, memory, 0);
        })
        .and_then([&]() -> std::expected<std::tuple<VulkanBindings::UniqueVkImage,
                                                    VulkanBindings::UniqueVkDeviceMemory>,
                                         VkResult> {
            return std::make_tuple(std::move(image), std::move(memory));
        });
}

uint32_t findMemoryType(const VulkanBindings::HandleVkPhysicalDevice &physicalDevice,
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

std::expected<std::tuple<VulkanBindings::UniqueVkBuffer, VulkanBindings::UniqueVkDeviceMemory>,
              VkResult>
createBuffer(const VulkanBindings::HandleVkPhysicalDevice &physicalDevice,
             VulkanBindings::UniqueVkDevice &device, VkDeviceSize size, VkBufferUsageFlags usage,
             VkMemoryPropertyFlags properties) {
    auto bufferInfo = VulkanBindings::Init<VkBufferCreateInfo>();
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VulkanBindings::UniqueVkBuffer buffer;
    VulkanBindings::UniqueVkDeviceMemory memory;
    return device.createBuffer(&bufferInfo)
        .and_then([&](auto &&resBuffer) {
            buffer = std::move(resBuffer);

            auto memRequirements = device.getBufferMemoryRequirements(buffer);

            auto allocInfo = VulkanBindings::Init<VkMemoryAllocateInfo>();
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex =
                findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

            return device.allocateMemory(&allocInfo);
        })
        .and_then([&](auto &&resMemory) {
            memory = std::move(resMemory);
            return device.bindBufferMemory(buffer, memory, 0);
        })
        .and_then([&]() -> std::expected<std::tuple<VulkanBindings::UniqueVkBuffer,
                                                    VulkanBindings::UniqueVkDeviceMemory>,
                                         VkResult> {
            return std::make_tuple(std::move(buffer), std::move(memory));
        });
}

std::expected<std::tuple<VulkanBindings::UniqueVkBuffer, VulkanBindings::UniqueVkDeviceMemory>,
              VkResult>
createInitilisedBuffer(const VulkanBindings::HandleVkPhysicalDevice &physicalDevice,
                       VulkanBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
                       VkDeviceSize size, uint8_t *data, VkBufferUsageFlagBits type) {
    VulkanBindings::UniqueVkBuffer buffer;
    VulkanBindings::UniqueVkDeviceMemory bufferMemory;
    return createBuffer(physicalDevice, device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | type,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        .and_then([&](auto &&tuple) {
            std::tie(buffer, bufferMemory) = std::move(tuple);
            return initiliseBuffer(physicalDevice, device, CBctx, buffer, 0, size, data);
        })
        .and_then([&]() -> std::expected<std::tuple<VulkanBindings::UniqueVkBuffer,
                                                    VulkanBindings::UniqueVkDeviceMemory>,
                                         VkResult> {
            return std::make_tuple(std::move(buffer), std::move(bufferMemory));
        });
}

std::expected<void, VkResult>
initiliseBuffer(const VulkanBindings::HandleVkPhysicalDevice &physicalDevice,
                VulkanBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
                VulkanBindings::UniqueVkBuffer &buffer, VkDeviceSize offset, VkDeviceSize size,
                const uint8_t *data) {
    CommandBufferContextAdopted<VulkanBindings::UniqueVkBuffer> stagingBuffer{CBctx};
    CommandBufferContextAdopted<VulkanBindings::UniqueVkDeviceMemory> stagingBufferMemory{CBctx};

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

std::expected<std::tuple<std::vector<VulkanBindings::UniqueVkBuffer>,
                         std::vector<VulkanBindings::UniqueVkDeviceMemory>>,
              VkResult>
createInitilisedBuffers(const VulkanBindings::HandleVkPhysicalDevice &physicalDevice,
                        VulkanBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
                        size_t count, VkDeviceSize size, uint8_t *data, VkBufferUsageFlags type) {

    CommandBufferContextAdopted<VulkanBindings::UniqueVkBuffer> stagingBuffer{CBctx};
    CommandBufferContextAdopted<VulkanBindings::UniqueVkDeviceMemory> stagingBufferMemory{CBctx};

    auto copyToTheBuffers = [&](void *mapped_data)
        -> std::expected<std::tuple<std::vector<VulkanBindings::UniqueVkBuffer>,
                                    std::vector<VulkanBindings::UniqueVkDeviceMemory>>,
                         VkResult> {
        std::vector<VulkanBindings::UniqueVkBuffer> buffers(count);
        std::vector<VulkanBindings::UniqueVkDeviceMemory> buffersMemory(count);

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
std::expected<VulkanBindings::UniqueVkCommandBuffers, VkResult>
beginSingleTimeCommands(VulkanBindings::UniqueVkDevice &device,
                        VulkanBindings::UniqueVkCommandPool &commandPool) {
    auto allocInfo = VulkanBindings::Init<VkCommandBufferAllocateInfo>();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VulkanBindings::UniqueVkCommandBuffers commandBuffers;
    return device.allocateCommandBuffers(&allocInfo)
        .and_then([&](auto &&commandBuffersRes) {
            commandBuffers = std::move(commandBuffersRes);
            auto beginInfo = VulkanBindings::Init<VkCommandBufferBeginInfo>();
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            return commandBuffers[0].begin(&beginInfo);
        })
        .and_then([&]() -> std::expected<VulkanBindings::UniqueVkCommandBuffers, VkResult> {
            device.nameObject(commandBuffers, "singleTime");
            return std::move(commandBuffers);
        });
}

std::expected<void, VkResult>
endSingleTimeCommands(VulkanBindings::HandleVkQueue &graphicsQueue,
                      VulkanBindings::UniqueVkCommandBuffers &oneShotCommandBuffers) {
    auto commandBuffer = oneShotCommandBuffers[0];
    return commandBuffer.end()
        .and_then([&]() {
            auto submitInfo = VulkanBindings::Init<VkSubmitInfo>();
            VkCommandBuffer buffer = commandBuffer;
            submitInfo.pCommandBuffers = &buffer;
            submitInfo.commandBufferCount = 1;

            return graphicsQueue.submit(&submitInfo);
        })
        .and_then([&]() { return graphicsQueue.waitIdle(); });
}

CommandBufferContext::CommandBufferContext(VulkanBindings::UniqueVkDevice &device,
                                           VulkanBindings::UniqueVkCommandPool &pool,
                                           VulkanBindings::HandleVkQueue submitQueue)
    : device(device), pool(pool), submitQueue(submitQueue), is_externaly_controlled(false) {}

CommandBufferContext::CommandBufferContext(VulkanBindings::HandleVkCommandBuffer buffer)
    : buffer(buffer), is_externaly_controlled(true) {}

CommandBufferContext::CommandBufferContext(CommandBufferContext &&other) {
    lifetimecontainer = std::move(other.lifetimecontainer);
    is_externaly_controlled = std::exchange(other.is_externaly_controlled, true);
    device = std::exchange(other.device, {});
    pool = std::exchange(other.pool, {});
    submitQueue = std::exchange(other.submitQueue, VK_NULL_HANDLE);
    if (other.buffers) {
        buffers = std::move(other.buffers);
    } else {
        buffers.cleanup();
    }
    buffer = std::exchange(other.buffer, VK_NULL_HANDLE);
};
CommandBufferContext &CommandBufferContext::operator=(CommandBufferContext &&other) {
    assert(((is_externaly_controlled || !buffers) && lifetimecontainer.empty()) &&
           "The CommandBufferContext to move to had a unflushed CommandBuffer\n");
    lifetimecontainer = std::move(other.lifetimecontainer);
    is_externaly_controlled = std::exchange(other.is_externaly_controlled, true);
    device = std::exchange(other.device, {});
    pool = std::exchange(other.pool, {});
    submitQueue = std::exchange(other.submitQueue, VK_NULL_HANDLE);
    if (other.buffers) {
        buffers = std::move(other.buffers);
    } else {
        buffers.cleanup();
    }
    buffer = std::exchange(other.buffer, VK_NULL_HANDLE);
    return *this;
}
std::expected<void, VkResult> CommandBufferContext::init() {
    assert(!buffer && "A unsubmittet buffer already exists");
    return beginSingleTimeCommands(device.value(), pool.value()).transform([&](auto &&buffersRes) {
        buffers = std::move(buffersRes);
        buffer = buffers[0];
    });
}
VulkanBindings::HandleVkCommandBuffer CommandBufferContext::getBuffer() {
    assert(buffer && "The buffer has not been started");
    return buffer;
}

std::expected<void, VkResult> CommandBufferContext::flush() {

    if (!is_externaly_controlled) {
        if (buffers) {
            auto endRes = endSingleTimeCommands(submitQueue, buffers);
            buffer = VulkanBindings::HandleVkCommandBuffer{};
            buffers.cleanup();
            lifetimecontainer.clear();
            return endRes;
        }
    }
    lifetimecontainer.clear();
    return {};
}

CommandBufferContext::~CommandBufferContext() {
    assert(((is_externaly_controlled || !buffers) && lifetimecontainer.empty()) &&
           "The CommandBufferContext has a unflushed CommandBuffer on deletion\n");
}

void copyBuffer(CommandBufferContext &CBctx, VkBuffer srcBuffer, VkBuffer destBuffer,
                VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset) {

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;

    CBctx.getBuffer().copyBuffer(srcBuffer, destBuffer, &copyRegion);
}

void copyBufferToImage(CommandBufferContext &CBctx, VkBuffer buffer, VkImage image, uint32_t width,
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

void transitionImageLayout(CommandBufferContext &CBctx, VulkanBindings::UniqueVkImage &image,
                           VkFormat format, VkImageLayout newLayout) {

    VkImageLayout oldLayout = image.layout;

    VkImageMemoryBarrier barrier = VulkanBindings::Init<VkImageMemoryBarrier>();
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

// void SaveImage() {
//     copyBufferToImage(CommandBufferContext &CBctx, VkBuffer buffer, VkImage image, uint32_t
//     width, uint32_t height)
// }

std::expected<std::tuple<VulkanBindings::UniqueVkImage, VulkanBindings::UniqueVkDeviceMemory>,
              VkResult>
createTextureImage(
    CommandBufferContext &CBctx, VulkanBindings::UniqueVkDevice &device,
    VulkanBindings::HandleVkPhysicalDevice physicalDevice,
    std::function<std::tuple<VkExtent2D, std::span<const unsigned char>>(const std::string &)>
        textureGetter,
    const std::string &imageName) {
    VkUtils::CommandBufferContextAdopted<VulkanBindings::UniqueVkBuffer> stagingBuffer{CBctx};
    VkUtils::CommandBufferContextAdopted<VulkanBindings::UniqueVkDeviceMemory> stagingBufferMemory{
        CBctx};

    const auto &[extend, pixels] = textureGetter(imageName);

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
            return VkUtils::createImage(
                physicalDevice, device, extend, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        })
        .and_then(
            [&](auto &&tuple) -> std::expected<std::tuple<VulkanBindings::UniqueVkImage,
                                                          VulkanBindings::UniqueVkDeviceMemory>,
                                               VkResult> {
                auto &[image, _] = tuple;
                VkUtils::transitionImageLayout(CBctx, image, VK_FORMAT_R8G8B8A8_SRGB,
                                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                VkUtils::copyBufferToImage(CBctx, stagingBuffer.get(), image, extend.width,
                                           extend.height);
                VkUtils::transitionImageLayout(CBctx, image, VK_FORMAT_R8G8B8A8_SRGB,
                                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                return tuple;
            });
}

void PipelineVertexBindingDescriptorBuilder::addBinding(
    VkVertexInputBindingDescription bindingDescription) {
    currentBinding = nextBinding++;
    bindingDescription.binding = currentBinding;
    bindingDescriptions.emplace_back(bindingDescription);
}

void PipelineVertexBindingDescriptorBuilder::addAttribute(
    VkVertexInputAttributeDescription attributeDescription) {
    attributeDescription.binding = currentBinding;
    attributeDescription.location = currentLocation++;
    attributeDescriptions.emplace_back(attributeDescription);
}

VkPipelineVertexInputStateCreateInfo PipelineVertexBindingDescriptorBuilder::getVertexInputInfo() {
    auto vertexInputInfo = VulkanBindings::Init<VkPipelineVertexInputStateCreateInfo>();
    vertexInputInfo.vertexBindingDescriptionCount =
        static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    return vertexInputInfo;
}

void PipelineVertexBindingDescriptorBuilder::print() const {
    for (size_t b = 0; b < bindingDescriptions.size(); b++) {
        const auto &bind = bindingDescriptions[b];
        std::cout << "Binding: " << bind.binding << " rate: "
                  << (bind.inputRate == VK_VERTEX_INPUT_RATE_VERTEX ? "vertex" : "instance")
                  << " stride: " << bind.stride << "\n";
        for (size_t l = 0; l < attributeDescriptions.size(); l++) {
            const auto &attr = attributeDescriptions[l];
            if (attr.binding != b)
                continue;
            std::cout << "\tlayout(location = " << attr.location << ") ";

            switch (attr.format) {
            // Single component formats
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SNORM:
            case VK_FORMAT_R8_USCALED:
            case VK_FORMAT_R8_SSCALED:
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R16_UNORM:
            case VK_FORMAT_R16_SNORM:
            case VK_FORMAT_R16_USCALED:
            case VK_FORMAT_R16_SSCALED:
            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R64_SFLOAT:
                std::cout << "float";
                break;
            case VK_FORMAT_R8_UINT:
            case VK_FORMAT_R16_UINT:
            case VK_FORMAT_R32_UINT:
            case VK_FORMAT_R64_UINT:
                std::cout << "uint";
                break;
            case VK_FORMAT_R8_SINT:
            case VK_FORMAT_R16_SINT:
            case VK_FORMAT_R32_SINT:
            case VK_FORMAT_R64_SINT:
                std::cout << "int";
                break;
            // Two component formats (vec2)
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SNORM:
            case VK_FORMAT_R8G8_USCALED:
            case VK_FORMAT_R8G8_SSCALED:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R16G16_UNORM:
            case VK_FORMAT_R16G16_SNORM:
            case VK_FORMAT_R16G16_USCALED:
            case VK_FORMAT_R16G16_SSCALED:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_R64G64_SFLOAT:
                std::cout << "vec2";
                break;
            case VK_FORMAT_R8G8_UINT:
            case VK_FORMAT_R16G16_UINT:
            case VK_FORMAT_R32G32_UINT:
            case VK_FORMAT_R64G64_UINT:
                std::cout << "uvec2";
                break;
            case VK_FORMAT_R8G8_SINT:
            case VK_FORMAT_R16G16_SINT:
            case VK_FORMAT_R32G32_SINT:
            case VK_FORMAT_R64G64_SINT:
                std::cout << "ivec2";
                break;
            // Three component formats (vec3)
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SNORM:
            case VK_FORMAT_R8G8B8_USCALED:
            case VK_FORMAT_R8G8B8_SSCALED:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_B8G8R8_UNORM:
            case VK_FORMAT_B8G8R8_SNORM:
            case VK_FORMAT_B8G8R8_USCALED:
            case VK_FORMAT_B8G8R8_SSCALED:
            case VK_FORMAT_B8G8R8_SRGB:
            case VK_FORMAT_R16G16B16_UNORM:
            case VK_FORMAT_R16G16B16_SNORM:
            case VK_FORMAT_R16G16B16_USCALED:
            case VK_FORMAT_R16G16B16_SSCALED:
            case VK_FORMAT_R16G16B16_SFLOAT:
            case VK_FORMAT_R32G32B32_SFLOAT:
            case VK_FORMAT_R64G64B64_SFLOAT:
            case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
                std::cout << "vec3";
                break;
            case VK_FORMAT_R8G8B8_UINT:
            case VK_FORMAT_B8G8R8_UINT:
            case VK_FORMAT_R16G16B16_UINT:
            case VK_FORMAT_R32G32B32_UINT:
            case VK_FORMAT_R64G64B64_UINT:
                std::cout << "uvec3";
                break;
            case VK_FORMAT_R8G8B8_SINT:
            case VK_FORMAT_B8G8R8_SINT:
            case VK_FORMAT_R16G16B16_SINT:
            case VK_FORMAT_R32G32B32_SINT:
            case VK_FORMAT_R64G64B64_SINT:
                std::cout << "ivec3";
                break;
            // Four component formats (vec4)
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SNORM:
            case VK_FORMAT_R8G8B8A8_USCALED:
            case VK_FORMAT_R8G8B8A8_SSCALED:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SNORM:
            case VK_FORMAT_B8G8R8A8_USCALED:
            case VK_FORMAT_B8G8R8A8_SSCALED:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
            case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
            case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
            case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
            case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
            case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
            case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
            case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
            case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
            case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
            case VK_FORMAT_R16G16B16A16_UNORM:
            case VK_FORMAT_R16G16B16A16_SNORM:
            case VK_FORMAT_R16G16B16A16_USCALED:
            case VK_FORMAT_R16G16B16A16_SSCALED:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
            case VK_FORMAT_R64G64B64A64_SFLOAT:
            case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
            case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
            case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
            case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            case VK_FORMAT_A1B5G5R5_UNORM_PACK16:
            case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
            case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
            case VK_FORMAT_R5G6B5_UNORM_PACK16:
            case VK_FORMAT_B5G6R5_UNORM_PACK16:
                std::cout << "vec4";
                break;
            case VK_FORMAT_R8G8B8A8_UINT:
            case VK_FORMAT_B8G8R8A8_UINT:
            case VK_FORMAT_A8B8G8R8_UINT_PACK32:
            case VK_FORMAT_A2R10G10B10_UINT_PACK32:
            case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            case VK_FORMAT_R16G16B16A16_UINT:
            case VK_FORMAT_R32G32B32A32_UINT:
            case VK_FORMAT_R64G64B64A64_UINT:
                std::cout << "uvec4";
                break;
            case VK_FORMAT_R8G8B8A8_SINT:
            case VK_FORMAT_B8G8R8A8_SINT:
            case VK_FORMAT_A8B8G8R8_SINT_PACK32:
            case VK_FORMAT_A2R10G10B10_SINT_PACK32:
            case VK_FORMAT_A2B10G10R10_SINT_PACK32:
            case VK_FORMAT_R16G16B16A16_SINT:
            case VK_FORMAT_R32G32B32A32_SINT:
            case VK_FORMAT_R64G64B64A64_SINT:
                std::cout << "ivec4";
                break;
            // Depth/Stencil formats
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
                std::cout << "float"; // Depth
                break;
            case VK_FORMAT_S8_UINT:
                std::cout << "uint"; // Stencil
                break;
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                std::cout << "vec2"; // Depth + Stencil (typically accessed separately)
                break;
            // Special formats
            case VK_FORMAT_R4G4_UNORM_PACK8:
                std::cout << "vec2";
                break;
            case VK_FORMAT_A8_UNORM:
                std::cout << "float";
                break;
            case VK_FORMAT_R8_BOOL_ARM:
                std::cout << "bool";
                break;
            // Compressed formats (sampled as vec4)
            case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
            case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
            case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
            case VK_FORMAT_BC2_UNORM_BLOCK:
            case VK_FORMAT_BC2_SRGB_BLOCK:
            case VK_FORMAT_BC3_UNORM_BLOCK:
            case VK_FORMAT_BC3_SRGB_BLOCK:
            case VK_FORMAT_BC4_UNORM_BLOCK:
            case VK_FORMAT_BC4_SNORM_BLOCK:
            case VK_FORMAT_BC5_UNORM_BLOCK:
            case VK_FORMAT_BC5_SNORM_BLOCK:
            case VK_FORMAT_BC6H_UFLOAT_BLOCK:
            case VK_FORMAT_BC6H_SFLOAT_BLOCK:
            case VK_FORMAT_BC7_UNORM_BLOCK:
            case VK_FORMAT_BC7_SRGB_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
            case VK_FORMAT_EAC_R11_UNORM_BLOCK:
            case VK_FORMAT_EAC_R11_SNORM_BLOCK:
            case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
            case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
            case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
            case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
            case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
            case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
            case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
            case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
            case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
            case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
            case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
            case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
            case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
            case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
            case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
            case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
            case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
            case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
            case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
            case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
            case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
            case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
            case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
            case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
            case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
            case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
            case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
            case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
            case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
            case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
            case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
            case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
                std::cout << "vec4"; // Compressed formats are sampled as vec4
                break;
            case VK_FORMAT_UNDEFINED:
            default:
                std::cout << "/* UNSUPPORTED FORMAT */";
                break;
            }
            std::cout << " (offset " << attr.offset << ")\n";
        }
    }
}

void DescriptorSetLayoutBuilder::addImmutableImageSampler(
    VkShaderStageFlags stageFlags, VulkanBindings::UniqueVkSampler &sampler) {
    immutableSamplers.emplace_back(sampler);
    assert(immutableSamplers.back() != VK_NULL_HANDLE);
    VkDescriptorSetLayoutBinding binding;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.binding = currentBinding++;
    binding.pImmutableSamplers = (VkSampler *)-1;
    binding.stageFlags = stageFlags;
    binding.descriptorCount = 1;
    bindings.emplace_back(binding);
}
void DescriptorSetLayoutBuilder::addDescriptor(VkDescriptorSetLayoutBinding binding) {
    binding.binding = currentBinding++;
    binding.descriptorCount = 1;
    bindings.emplace_back(binding);
}
void DescriptorSetLayoutBuilder::addDescriptorArray(VkDescriptorSetLayoutBinding binding,
                                                    uint32_t count) {
    binding.binding = currentBinding++;
    binding.descriptorCount = count;
    bindings.emplace_back(binding);
}
std::expected<VulkanBindings::UniqueVkDescriptorSetLayout, VkResult>
DescriptorSetLayoutBuilder::build(VulkanBindings::UniqueVkDevice &device) {
    auto createInfo = VulkanBindings::Init<VkDescriptorSetLayoutCreateInfo>();
    size_t sampler = 0;
    for (auto &binding : bindings) {
        if (binding.pImmutableSamplers == nullptr)
            continue;
        assert(sampler <= immutableSamplers.size());
        binding.pImmutableSamplers = &immutableSamplers[sampler];
        assert(binding.pImmutableSamplers != nullptr);
    }
    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    createInfo.pBindings = bindings.data();
    return device.createDescriptorSetLayout(&createInfo);
}
std::expected<VulkanBindings::UniqueVkDescriptorSetLayout, VkResult>
DescriptorSetLayoutBuilder::buildReset(VulkanBindings::UniqueVkDevice &device) {
    auto layoutRes = build(device);
    bindings.clear();
    immutableSamplers.clear();
    currentBinding = 0;
    return layoutRes;
}
void StaticMesh::draw(VulkanBindings::HandleVkCommandBuffer commandBuffer, uint32_t instanceCount,
                      uint32_t firstVertex, uint32_t firstInstance) const {
    assert(indexCount == 0);
    commandBuffer.bindVertexBuffer(0, buffer, 0);
    commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}
void StaticMesh::drawIndexed(VulkanBindings::HandleVkCommandBuffer commandBuffer,
                             uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                             uint32_t firstInstance) const {
    assert(indexCount != 0);
    commandBuffer.bindVertexBuffer(0, buffer, 0);
    commandBuffer.bindIndexBuffer(buffer, indexOffset, indexType);
    commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
}; // namespace VkUtils
