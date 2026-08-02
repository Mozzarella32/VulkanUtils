#pragma once

#include "Functions.hpp"
#include "NameObject.hpp"

#include <VkBindings/Enums.hpp>
#include <VkBindings/Objects.hpp>

namespace VkUtils {

class StaticMesh {
  private:
    VkBindings::Buffer buffer;
    VkBindings::DeviceMemory bufferMemory;

    uint32_t vertexCount = 0;
    VkBindings::DeviceSize indexOffset = 0;
    uint32_t indexCount = 0;

    VkBindings::IndexType indexType = VkBindings::IndexType::eUint16;

  public:
    template <typename VT, typename IT>
    [[nodiscard]] std::expected<void, VkBindings::Result>
    Init(VkBindings::PhysicalDevice &physicalDevice, VkBindings::Device &device,
         CommandBufferContext &CBctx, const std::vector<VT> &vertexData,
         const std::vector<IT> &indexData, const std::string &name = "") {
        VkBindings::DeviceSize vertexBufferSize = sizeof(VT) * vertexData.size();
        VkBindings::DeviceSize indexBufferSize = sizeof(IT) * indexData.size();

        auto props = physicalDevice.getProperties();
        VkBindings::DeviceSize minAlignment = props.limits.minStorageBufferOffsetAlignment;

        vertexCount = static_cast<uint32_t>(vertexData.size());

        indexOffset = getAlignedOffset(vertexBufferSize, minAlignment);
        indexCount = static_cast<uint32_t>(indexData.size());
        indexType = IT::getIndexType();

        VkBindings::DeviceSize totalSize = indexOffset + indexBufferSize;

        return createBuffer(physicalDevice, device, totalSize,
                            VkBindings::BufferUsageFlagBits::eVertexBuffer |
                                VkBindings::BufferUsageFlagBits::eIndexBuffer |
                                VkBindings::BufferUsageFlagBits::eTransferDst,
                            VkBindings::MemoryPropertyFlagBits::eDeviceLocal)
            .and_then([&](auto &&tuple) {
                std::tie(buffer, bufferMemory) = std::move(tuple);
                nameObject(device, buffer, name);
                nameObject(device, bufferMemory, name);
                return initiliseBuffer(physicalDevice, device, CBctx, buffer, 0, vertexBufferSize,
                                       (uint8_t *)vertexData.data());
            })
            .and_then([&]() {
                return initiliseBuffer(physicalDevice, device, CBctx, buffer, indexOffset,
                                       indexBufferSize, (uint8_t *)indexData.data());
            });
    }

    template <typename VT>
    [[nodiscard]] std::expected<void, VkBindings::Result>
    Init(VkBindings::PhysicalDevice &physicalDevice, VkBindings::Device &device,
         CommandBufferContext &CBctx, const std::vector<VT> &vertexData,
         const std::string &name = "") {
        VkBindings::DeviceSize vertexBufferSize = sizeof(VT) * vertexData.size();

        vertexCount = static_cast<uint32_t>(vertexData.size());

        indexOffset = 0;
        indexCount = 0;
        indexType = VkBindings::IndexType::eUint16;

        return createInitilisedBuffer(physicalDevice, device, CBctx, vertexBufferSize,
                                      (uint8_t *)vertexData.data(),
                                      VkBindings::BufferUsageFlagBits::eVertexBuffer)
            .transform([&](auto &&tuple) {
                std::tie(buffer, bufferMemory) = std::move(tuple);
                nameObject(device, buffer, name);
                nameObject(device, bufferMemory, name);
            });
    }

    void draw(VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount = 1,
              uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
    void drawIndexed(VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount = 1,
                     uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                     uint32_t firstInstance = 0) const;
};

} // namespace VkUtils
