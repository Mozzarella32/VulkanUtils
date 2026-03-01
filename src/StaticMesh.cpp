#include "StaticMesh.hpp"

namespace VkUtils {

void StaticMesh::draw(VkBindings::HandleVkCommandBuffer commandBuffer, uint32_t instanceCount,
                      uint32_t firstVertex, uint32_t firstInstance) const {
    assert(indexCount == 0);
    commandBuffer.bindVertexBuffer(0, buffer, 0);
    commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}
void StaticMesh::drawIndexed(VkBindings::HandleVkCommandBuffer commandBuffer,
                             uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                             uint32_t firstInstance) const {
    assert(indexCount != 0);
    commandBuffer.bindVertexBuffer(0, buffer, 0);
    commandBuffer.bindIndexBuffer(buffer, indexOffset, indexType);
    commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

} // namespace VkUtils
