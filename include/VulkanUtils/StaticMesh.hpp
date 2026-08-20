#pragma once

#include "CommandBufferContext.hpp"
#include "Functions.hpp"
#include "NameObject.hpp"

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/ObjectsForward.hpp>

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace VkUtils {

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
class StaticMesh {
  private:
    VkBindings::UniqueBuffer buffer;
    VkBindings::UniqueDeviceMemory bufferMemory;

    uint32_t vertexCount = 0;
    VkBindings::DeviceSize indexOffset = 0;
    uint32_t indexCount = 0;

    VkBindings::IndexType indexType = VkBindings::IndexType::Uint16;

  public:
    template <typename VT, typename IT>
    [[nodiscard]] auto Init(const VkBindings::PhysicalDevice &physicalDevice,
                            const VkBindings::Device &device, CommandBufferContext &CBctx,
                            const std::vector<VT> &vertexData, const std::vector<IT> &indexData,
                            const std::string &name = "")
        -> std::expected<void, VkBindings::Result> {
        VkBindings::DeviceSize vertexBufferSize = sizeof(VT) * vertexData.size();
        VkBindings::DeviceSize indexBufferSize = sizeof(IT) * indexData.size();

        auto props = physicalDevice.getProperties();
        const VkBindings::DeviceSize minAlignment = props.limits.minStorageBufferOffsetAlignment;

        vertexCount = static_cast<uint32_t>(vertexData.size());

        indexOffset = getAlignedOffset(vertexBufferSize, minAlignment);
        indexCount = static_cast<uint32_t>(indexData.size());
        indexType = IT::getIndexType();

        const VkBindings::DeviceSize totalSize = indexOffset + indexBufferSize;

        return createBuffer(physicalDevice, device, totalSize,
                            VkBindings::BufferUsageBits::VertexBuffer |
                                VkBindings::BufferUsageBits::IndexBuffer |
                                VkBindings::BufferUsageBits::TransferDst,
                            VkBindings::MemoryPropertyBits::DeviceLocal)
            .and_then(
                [&](std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory> &&tuple) {
                    std::tie(buffer, bufferMemory) = std::move(tuple);
                    nameObject(device, buffer, name);
                    nameObject(device, bufferMemory, name);
                    return initiliseBuffer(
                        physicalDevice, device, CBctx, buffer, 0,
                        std::span(reinterpret_cast<const uint8_t *>(vertexData.data()),
                                  vertexBufferSize));
                })
            .and_then([&]() -> auto {
                return initiliseBuffer(
                    physicalDevice, device, CBctx, buffer, indexOffset,

                    std::span(reinterpret_cast<const uint8_t *>(indexData.data()),
                              indexBufferSize));
            });
    }

    template <typename VT>
    [[nodiscard]] auto Init(const VkBindings::PhysicalDevice &physicalDevice,
                            const VkBindings::Device &device, CommandBufferContext &CBctx,
                            const std::vector<VT> &vertexData, const std::string &name = "")
        -> std::expected<void, VkBindings::Result> {
        VkBindings::DeviceSize vertexBufferSize = sizeof(VT) * vertexData.size();

        vertexCount = static_cast<uint32_t>(vertexData.size());

        indexOffset = 0;
        indexCount = 0;
        indexType = VkBindings::IndexType::Uint16;

        return createInitilisedBuffer(
                   physicalDevice, device, CBctx,
                   std::span(reinterpret_cast<const uint8_t *>(vertexData.data()),
                             vertexBufferSize),
                   VkBindings::BufferUsageBits::VertexBuffer)
            .transform(
                [&](std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory> &&tuple) {
                    std::tie(buffer, bufferMemory) = std::move(tuple);
                    nameObject(device, buffer, name);
                    nameObject(device, bufferMemory, name);
                });
    }

    void draw(const VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount = 1,
              uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
    void drawIndexed(const VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount = 1,
                     uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                     uint32_t firstInstance = 0) const;
};
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

} // namespace VkUtils
