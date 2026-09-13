#pragma once

#include "CommandBufferContext.hpp"

#include <VmaBindings/Vma.hpp>
#include <VmaBindings/VmaForward.hpp>

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/ObjectsForward.hpp>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace VkUtils {

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
class StaticMesh {
  private:
    VkBindings::UniqueBuffer buffer;
    VmaBindings::UniqueAllocation bufferAllocation;

    uint32_t vertexCount = 0;
    VkBindings::DeviceSize indexOffset = 0;
    uint32_t indexCount = 0;

    VkBindings::IndexType indexType = VkBindings::IndexType::Uint16;

    auto implInit(const VmaBindings::Allocator &allocator,
                  CommandBufferContext &commandBufferContext, std::span<const std::byte> vertexData,
                  std::span<const std::byte> indexData, std::string_view name)
        -> VkBindings::Result;

    auto implInit(const VmaBindings::Allocator &allocator,
                  CommandBufferContext &commandBufferContext,
                  const std::span<const std::byte> &vertexData, std::string_view name)
        -> VkBindings::Result;

  public:
    template <typename VT, typename IT>
        requires requires {
            { IT::getIndexType() } -> std::same_as<VkBindings::IndexType>;
        }
    [[nodiscard]] auto init(const VmaBindings::Allocator &allocator,
                            CommandBufferContext &commandBufferContext, std::span<VT> vertexData,
                            std::span<IT> indexData, std::string_view name = "")
        -> VkBindings::Result {
        vertexCount = static_cast<uint32_t>(vertexData.size());
        indexCount = static_cast<uint32_t>(indexData.size());
        indexType = IT::getIndexType();
        return implInit(allocator, commandBufferContext, std::as_bytes(vertexData),
                        std::as_bytes(indexData), name);
    }

    template <typename VT>
    [[nodiscard]] auto init(const VmaBindings::Allocator &allocator,
                            CommandBufferContext &commandBufferContext, std::span<VT> vertexData,
                            std::string_view name = "") -> VkBindings::Result {
        vertexCount = static_cast<uint32_t>(vertexData.size());
        indexCount = 0;
        return implInit(allocator, commandBufferContext, std::as_bytes(vertexData), name);
    }

    void draw(const VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount = 1,
              uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
    void drawIndexed(const VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount = 1,
                     uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                     uint32_t firstInstance = 0) const;
};
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

} // namespace VkUtils
