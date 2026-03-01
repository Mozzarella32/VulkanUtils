#pragma once

#include "VulkanObjects.hpp"

#include <expected>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <tuple>

namespace VkUtils {

[[nodiscard]] inline auto printFailedFunction(const std::string &func) {
    return [func] [[nodiscard]] (VkResult res) {
        std::cerr << func << " failed with: " << VkBindings::impl::VkResultToString(res)
                  << "\n";
        return res;
    };
}

[[nodiscard]] inline auto
throwFailed(const std::string &func,
            const std::source_location location = std::source_location::current()) {
    return [func, location](VkResult res) -> VkResult {
        std::stringstream str;
        str << std::string("in ") << location.file_name() << ": " << location.function_name()
            << ": " << std::to_string(location.column()) << ": " << func
            << ":\nfailed with: " << VkBindings::impl::VkResultToString(res) << "\n";

        throw std::runtime_error(str.str());
    };
}

template <typename T>
T unwrap(std::expected<T, VkResult> &&expected, const std::string &func,
         const std::source_location location = std::source_location::current()) {
    auto e = std::move(expected).transform_error(throwFailed(func, location));
    T tmp = std::move(e).value();
    return tmp;
}

inline void unwrap(std::expected<void, VkResult> &&expected, const std::string &func,
                   const std::source_location location = std::source_location::current()) {
    std::move(expected).transform_error(throwFailed(func, location)).value();
}

bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers);

// returnes set of unsupported extensions
std::set<std::string>
checkDeviceExtensionSupport(const VkBindings::HandleVkPhysicalDevice &queryDevice,
                            const std::vector<const char *> &requiredExtensions);

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete();
};

QueueFamilyIndices findQueueFamilies(const VkBindings::HandleVkPhysicalDevice &queryDevice,
                                     VkBindings::UniqueVkSurfaceKHR &surface);

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities = {};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

[[nodiscard]] std::expected<SwapChainSupportDetails, VkResult>
querySwapChainSupport(const VkBindings::HandleVkPhysicalDevice &queryDevice,
                      VkBindings::UniqueVkSurfaceKHR &surface);

[[nodiscard]] std::expected<std::tuple<std::vector<VkBindings::UniqueVkShaderModule>,
                                       std::vector<VkPipelineShaderStageCreateInfo>>,
                            VkResult>
createShaderStages(VkBindings::UniqueVkDevice &device,
                   std::function<std::span<const uint32_t>(const std::string &)> spirVGetter,
                   const std::vector<std::pair<std::string, VkShaderStageFlagBits>> &shaders);

VkFormat findSupportedFormat(const std::vector<VkFormat> &candiates, VkImageTiling tiling,
                             VkFormatFeatureFlagBits features);

VkFormat findSupportedFormat(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                             const std::vector<VkFormat> &candiates, VkImageTiling tiling,
                             VkFormatFeatureFlagBits features);

[[nodiscard]] std::expected<VkBindings::UniqueVkImageView, VkResult>
createImageView(VkBindings::UniqueVkDevice &device, VkImage image, VkFormat format,
                VkImageAspectFlags aspectFlags);

[[nodiscard]] std::expected<
    std::tuple<VkBindings::UniqueVkImage, VkBindings::UniqueVkDeviceMemory>, VkResult>
createImage(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
            VkBindings::UniqueVkDevice &device, VkExtent2D extend, VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

uint32_t findMemoryType(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                        uint32_t typeFilter, VkMemoryPropertyFlags properties);

bool hasStencilComponent(VkFormat format);

[[nodiscard]] std::expected<
    std::tuple<VkBindings::UniqueVkBuffer, VkBindings::UniqueVkDeviceMemory>, VkResult>
createBuffer(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
             VkBindings::UniqueVkDevice &device, VkDeviceSize size, VkBufferUsageFlags usage,
             VkMemoryPropertyFlags properties);

[[nodiscard]] std::expected<VkBindings::UniqueVkCommandBuffers, VkResult>
beginSingleTimeCommands(VkBindings::UniqueVkDevice &device,
                        VkBindings::UniqueVkCommandPool &commandPool);

[[nodiscard]] std::expected<void, VkResult>
endSingleTimeCommands(VkBindings::HandleVkQueue &graphicsQueue,
                      VkBindings::UniqueVkCommandBuffers &oneShotCommandBuffers);

struct CommandBufferContext {
  private:
    std::optional<std::reference_wrapper<VkBindings::UniqueVkDevice>> device;
    std::optional<std::reference_wrapper<VkBindings::UniqueVkCommandPool>> pool;
    VkBindings::HandleVkQueue submitQueue = VK_NULL_HANDLE;
    VkBindings::UniqueVkCommandBuffers buffers;
    VkBindings::HandleVkCommandBuffer buffer = VK_NULL_HANDLE;

    bool is_externaly_controlled;

    using AnyPtr = std::unique_ptr<void, void (*)(void *)>;
    std::vector<AnyPtr> lifetimecontainer;

  public:
    CommandBufferContext(VkBindings::UniqueVkDevice &device,
                         VkBindings::UniqueVkCommandPool &pool,
                         VkBindings::HandleVkQueue submitQueue);
    CommandBufferContext(VkBindings::HandleVkCommandBuffer buffer);
    CommandBufferContext(CommandBufferContext &&other);

    CommandBufferContext &operator=(CommandBufferContext &&other);

    [[nodiscard]] std::expected<void, VkResult> init();
    VkBindings::HandleVkCommandBuffer getBuffer();

    template <typename Ts> void adopt(Ts &&ts) {
#ifdef MY_VK_IMPL_PRINT_MEM_OPS
        MY_VK_PRINT_ADDR_SIMPLE(std::cout, ts.handle);
        std::cout << " adopted by ";
        MY_VK_PRINT_ADDR_SIMPLE(std::cout, buffer.handle);
        std::cout << "\n";
#endif
        [&] {
            using T = std::decay_t<Ts>;
            lifetimecontainer.push_back(
                AnyPtr(new T(std::forward<Ts>(ts)), [](void *p) { delete static_cast<T *>(p); }));
        }();
    }
    [[nodiscard]] std::expected<void, VkResult> flush();

    ~CommandBufferContext();
};

template <typename T> class CommandBufferContextAdopted {
    CommandBufferContext &CBctx;
    T t;

  public:
    CommandBufferContextAdopted(CommandBufferContext &CBctx) : CBctx(CBctx) {}
    ~CommandBufferContextAdopted() {
        if (t) {
            CBctx.adopt(std::move(t));
        } else {
            std::cerr << "Adoption failed, was VK_NULL_HANDLE" << "\n";
        }
    }
    T &operator()() { return t; }
    T &get() { return t; }
};

void copyBuffer(CommandBufferContext &CBctx, VkBuffer srcBuffer, VkBuffer destBuffer,
                VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);

void copyBufferToImage(CommandBufferContext &CBctx, VkBuffer buffer, VkImage image, uint32_t width,
                       uint32_t height);
void copyImageToBuffer(CommandBufferContext &CBctx, VkImage image, VkBuffer buffer, uint32_t width,
                       uint32_t height);

[[nodiscard]] std::expected<
    std::tuple<VkBindings::UniqueVkBuffer, VkBindings::UniqueVkDeviceMemory>, VkResult>
createInitilisedBuffer(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                       VkBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
                       VkDeviceSize size, uint8_t *data, VkBufferUsageFlagBits type);

[[nodiscard]] std::expected<void, VkResult>
initiliseBuffer(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                VkBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
                VkBindings::UniqueVkBuffer &buffer, VkDeviceSize offset, VkDeviceSize size,
                const uint8_t *data);

[[nodiscard]] std::expected<std::tuple<std::vector<VkBindings::UniqueVkBuffer>,
                                       std::vector<VkBindings::UniqueVkDeviceMemory>>,
                            VkResult>
createInitilisedBuffers(const VkBindings::HandleVkPhysicalDevice &physicalDevice,
                        VkBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
                        size_t count, VkDeviceSize size, uint8_t *data, VkBufferUsageFlags type);

VkDeviceSize getAlignedOffset(VkDeviceSize offset, VkDeviceSize alignment);

void transitionImageLayout(CommandBufferContext &CBctx, VkBindings::UniqueVkImage &image,
                           VkFormat format, VkImageLayout newLayout);

[[nodiscard]] std::expected<
    std::tuple<VkBindings::UniqueVkImage, VkBindings::UniqueVkDeviceMemory>, VkResult>
createTextureImage(
    CommandBufferContext &CBctx, VkBindings::UniqueVkDevice &device,
    VkBindings::HandleVkPhysicalDevice physicalDevice,
    std::function<std::tuple<VkExtent2D, std::span<const unsigned char>>(const std::string &)>
        textureGetter,
    const std::string &imageName);

class PipelineVertexBindingDescriptorBuilder {
  private:
    uint32_t nextBinding = 0;
    uint32_t currentBinding = 0;
    uint32_t currentLocation = 0;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

  public:
    void addBinding(VkVertexInputBindingDescription bindingDescription);
    void addAttribute(VkVertexInputAttributeDescription attributeDescription);
    template <typename T>
        requires requires(PipelineVertexBindingDescriptorBuilder desc) {
            T::addBinding(desc, std::declval<VkVertexInputRate>());
        }
    inline void addVertex(VkVertexInputRate inputRate) {
        T::addBinding(*this, inputRate);
    }

    [[nodiscard]] VkPipelineVertexInputStateCreateInfo getVertexInputInfo();

    void print() const;
};

class DescriptorSetLayoutBuilder {
  private:
    uint32_t currentBinding = 0;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<VkSampler> immutableSamplers;

  public:
    void addImmutableImageSampler(VkShaderStageFlags stageFlags,
                                  VkBindings::UniqueVkSampler &sampler);
    void addDescriptor(VkDescriptorSetLayoutBinding binding);
    void addDescriptorArray(VkDescriptorSetLayoutBinding binding, uint32_t count);
    [[nodiscard]] std::expected<VkBindings::UniqueVkDescriptorSetLayout, VkResult>
    build(VkBindings::UniqueVkDevice &device);
    [[nodiscard]] std::expected<VkBindings::UniqueVkDescriptorSetLayout, VkResult>
    buildReset(VkBindings::UniqueVkDevice &device);
};

class StaticMesh {
  private:
    VkBindings::UniqueVkBuffer buffer;
    VkBindings::UniqueVkDeviceMemory bufferMemory;

    uint32_t vertexCount = 0;
    VkDeviceSize indexOffset = 0;
    uint32_t indexCount = 0;

    VkIndexType indexType = VK_INDEX_TYPE_UINT16;

  public:
    template <typename VT, typename IT>
    [[nodiscard]] std::expected<void, VkResult>
    Init(VkBindings::HandleVkPhysicalDevice physicalDevice,
         VkBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
         const std::vector<VT> &vertexData, const std::vector<IT> &indexData,
         const std::string &name = "") {
        VkDeviceSize vertexBufferSize = sizeof(VT) * vertexData.size();
        VkDeviceSize indexBufferSize = sizeof(IT) * indexData.size();

        auto props = physicalDevice.getProperties();
        VkDeviceSize minAlignment = props.limits.minStorageBufferOffsetAlignment;

        vertexCount = static_cast<uint32_t>(vertexData.size());

        indexOffset = getAlignedOffset(vertexBufferSize, minAlignment);
        indexCount = static_cast<uint32_t>(indexData.size());
        indexType = IT::getIndexType();

        VkDeviceSize totalSize = indexOffset + indexBufferSize;

        return createBuffer(physicalDevice, device, totalSize,
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            .and_then([&](auto &&tuple) {
                std::tie(buffer, bufferMemory) = std::move(tuple);
                if (name != "") {
                    device.nameObject(buffer, name);
                    device.nameObject(bufferMemory, name);
                }
                return initiliseBuffer(physicalDevice, device, CBctx, buffer, 0, vertexBufferSize,
                                       (uint8_t *)vertexData.data());
            })
            .and_then([&]() {
                return initiliseBuffer(physicalDevice, device, CBctx, buffer, indexOffset,
                                       indexBufferSize, (uint8_t *)indexData.data());
            });
    }

    template <typename VT>
    [[nodiscard]] std::expected<void, VkResult>
    Init(VkBindings::HandleVkPhysicalDevice physicalDevice,
         VkBindings::UniqueVkDevice &device, CommandBufferContext &CBctx,
         const std::vector<VT> &vertexData, const std::string &name = "") {
        VkDeviceSize vertexBufferSize = sizeof(VT) * vertexData.size();

        vertexCount = static_cast<uint32_t>(vertexData.size());

        indexOffset = 0;
        indexCount = 0;
        indexType = VK_INDEX_TYPE_UINT16;

        return createInitilisedBuffer(physicalDevice, device, CBctx, vertexBufferSize,
                                      (uint8_t *)vertexData.data(),
                                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            .transform([&](auto &&tuple) {
                std::tie(buffer, bufferMemory) = std::move(tuple);
                if (name != "") {
                    device.nameObject(buffer, name);
                    device.nameObject(bufferMemory, name);
                }
            });
    }

    void draw(VkBindings::HandleVkCommandBuffer commandBuffer, uint32_t instanceCount = 1,
              uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
    void drawIndexed(VkBindings::HandleVkCommandBuffer commandBuffer,
                     uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                     uint32_t firstInstance = 0) const;
};

}; // namespace VkUtils
