#include "StaticMesh.hpp"

#include "CommandBufferContext.hpp"
#include "Functions.hpp"
#include "NameObject.hpp"

#include <VmaBindings/Vma.hpp>
#include <VmaBindings/VmaForward.hpp>

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Bits.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/StackContainer.hpp>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>

namespace VkUtils {

auto StaticMesh::implInit(const VmaBindings::Allocator &allocator,
                          CommandBufferContext &commandBufferContext,
                          std::span<const std::byte> vertexData,
                          std::span<const std::byte> indexData, std::string_view name)
    -> VkBindings::Result {

    const VkBindings::DeviceSize minAlignment =
        allocator.getPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment;

    indexOffset = getAlignedOffset(vertexData.size(), minAlignment);

    const VkBindings::DeviceSize totalSize = indexOffset + indexData.size();

    return createBufferSingleUpload(allocator,
                                    {.size = totalSize,
                                     .usage = VkBindings::BufferUsageBits::VertexBuffer |
                                              VkBindings::BufferUsageBits::IndexBuffer},
                                    std::array{vertexData, indexData}, commandBufferContext)
        .transform(
            [&](std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation> &&tuple) {
                std::tie(buffer, bufferAllocation) = std::move(tuple);
                nameObject(allocator.getAllocatorInfo().device, buffer, name);
                nameObject(bufferAllocation, name);
            })
        .error_or(VkBindings::Result::Success);
}
auto StaticMesh::implInit(const VmaBindings::Allocator &allocator,
                          CommandBufferContext &commandBufferContext,
                          const std::span<const std::byte> &vertexData, std::string_view name)
    -> VkBindings::Result {
    indexOffset = 0;
    indexCount = 0;
    indexType = VkBindings::IndexType::Uint16;

    return createBufferSingleUpload(
               allocator,
               {.size = vertexData.size(), .usage = VkBindings::BufferUsageBits::VertexBuffer},
               vertexData, commandBufferContext)
        .transform(
            [&](std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation> &&tuple) {
                std::tie(buffer, bufferAllocation) = std::move(tuple);
                nameObject(allocator.getAllocatorInfo().device, buffer, name);
                nameObject(bufferAllocation, name);
            })
        .error_or(VkBindings::Result::Success);
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
static_assert(requires(StaticMesh &mesh, const VmaBindings::Allocator &allocator,
                       CommandBufferContext &context, const std::array<uint32_t, 2> &vert,
                       const std::array<Index32, 2> &index) {
    mesh.init(allocator, context, std::span<const uint32_t>{vert}, std::span<const Index32>{index});
});

} // namespace
} // namespace VkUtils
