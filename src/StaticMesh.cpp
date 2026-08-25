#include "StaticMesh.hpp"
#include "CommandBufferContext.hpp"
#include "Functions.hpp"
#include "NameObject.hpp"

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/StackContainer.hpp>

#include <cassert>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace VkUtils {

auto StaticMesh::implInit(const VkBindings::PhysicalDevice &physicalDevice,
                          const VkBindings::Device &device, CommandBufferContext &CBctx,
                          std::span<const uint8_t> vertexData, std::span<const uint8_t> indexData,
                          VkBindings::IndexType indexType, const std::string &name)
    -> std::expected<void, VkBindings::Result> {

    auto props = physicalDevice.getProperties2();
    const VkBindings::DeviceSize minAlignment =
        props.properties.limits.minStorageBufferOffsetAlignment;

    vertexCount = static_cast<uint32_t>(vertexData.size());

    indexOffset = getAlignedOffset(vertexData.size(), minAlignment);
    indexCount = static_cast<uint32_t>(indexData.size());
    this->indexType = indexType;

    const VkBindings::DeviceSize totalSize = indexOffset + indexData.size();

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
                return initiliseBuffer(physicalDevice, device, CBctx, buffer, 0, vertexData);
            })
        .and_then([&]() -> auto {
            return initiliseBuffer(physicalDevice, device, CBctx, buffer, indexOffset, indexData);
        });
}
auto StaticMesh::implInit(const VkBindings::PhysicalDevice &physicalDevice,
                          const VkBindings::Device &device, CommandBufferContext &CBctx,
                          const std::span<const uint8_t> &vertexData, const std::string &name)
    -> std::expected<void, VkBindings::Result> {
    vertexCount = static_cast<uint32_t>(vertexData.size());

    indexOffset = 0;
    indexCount = 0;
    indexType = VkBindings::IndexType::Uint16;

    return createInitilisedBuffer(physicalDevice, device, CBctx, vertexData,
                                  VkBindings::BufferUsageBits::VertexBuffer)
        .transform(
            [&](std::tuple<VkBindings::UniqueBuffer, VkBindings::UniqueDeviceMemory> &&tuple) {
                std::tie(buffer, bufferMemory) = std::move(tuple);
                nameObject(device, buffer, name);
                nameObject(device, bufferMemory, name);
            });
}
void StaticMesh::draw(const VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount,
                      uint32_t firstVertex, uint32_t firstInstance) const {
    assert(indexCount == 0);
    auto buffers = VkBindings::stackContainer(buffer);
    commandBuffer.bindVertexBuffers(0, buffers, {0});
    commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}
void StaticMesh::drawIndexed(const VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount,
                             uint32_t firstIndex, int32_t vertexOffset,
                             uint32_t firstInstance) const {
    assert(indexCount != 0);
    auto buffers = VkBindings::stackContainer(buffer);
    commandBuffer.bindVertexBuffers(0, buffers, {0});
    commandBuffer.bindIndexBuffer(buffer, indexOffset, indexType);
    commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

namespace {
struct Index32 {
    uint32_t i;
    static auto getIndexType() -> VkBindings::IndexType { return VkBindings::IndexType::Uint32; }
};

// TEST that the concepts gets it right
static_assert(requires(StaticMesh &mesh, VkBindings::PhysicalDevice &physicalDevice,
                       VkBindings::Device &device, CommandBufferContext &context) {
    mesh.init(physicalDevice, device, context, std::vector<int32_t>{1, 2, 3, 4},
              std::vector<Index32>{{1}, {2}, {3}, {4}});
});

} // namespace
} // namespace VkUtils
