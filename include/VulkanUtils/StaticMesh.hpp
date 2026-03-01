#pragma once

#include "Functions.hpp"
#include <VulkanObjects.hpp>

namespace VkUtils {

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
    Init(VkBindings::HandleVkPhysicalDevice physicalDevice, VkBindings::UniqueVkDevice &device,
         CommandBufferContext &CBctx, const std::vector<VT> &vertexData,
         const std::vector<IT> &indexData, const std::string &name = "") {
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
    Init(VkBindings::HandleVkPhysicalDevice physicalDevice, VkBindings::UniqueVkDevice &device,
         CommandBufferContext &CBctx, const std::vector<VT> &vertexData,
         const std::string &name = "") {
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
    void drawIndexed(VkBindings::HandleVkCommandBuffer commandBuffer, uint32_t instanceCount = 1,
                     uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                     uint32_t firstInstance = 0) const;
};

} // namespace VkUtils
