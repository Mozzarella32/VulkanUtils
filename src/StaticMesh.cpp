#include "StaticMesh.hpp"

#include <VkBindings/ObjectsForward.hpp>

#include <cassert>
#include <cstdint>

namespace VkUtils {

void StaticMesh::draw(const VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount,
                      uint32_t firstVertex, uint32_t firstInstance) const {
    assert(indexCount == 0);
    commandBuffer.bindVertexBuffers(0, {buffer}, {0});
    commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}
void StaticMesh::drawIndexed(const VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount,
                             uint32_t firstIndex, int32_t vertexOffset,
                             uint32_t firstInstance) const {
    assert(indexCount != 0);
    commandBuffer.bindVertexBuffers(0, {buffer}, {0});
    commandBuffer.bindIndexBuffer(buffer, indexOffset, indexType);
    commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

} // namespace VkUtils
