#pragma once

#include "CommandBufferContext.hpp"

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/ObjectsForward.hpp>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

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

    auto implInit(const VkBindings::PhysicalDevice &physicalDevice,
                  const VkBindings::Device &device, CommandBufferContext &CBctx,
                  std::span<const std::byte> vertexData, std::span<const std::byte> indexData,
                  std::string_view name) -> std::expected<void, VkBindings::Result>;

    auto implInit(const VkBindings::PhysicalDevice &physicalDevice,
                  const VkBindings::Device &device, CommandBufferContext &CBctx,
                  const std::span<const std::byte> &vertexData, std::string_view name)
        -> std::expected<void, VkBindings::Result>;

  public:
    template <typename VT, typename IT>
        requires requires {
            { IT::getIndexType() } -> std::same_as<VkBindings::IndexType>;
        }
    [[nodiscard]] auto init(const VkBindings::PhysicalDevice &physicalDevice,
                            const VkBindings::Device &device, CommandBufferContext &CBctx,
                            std::span<VT> vertexData, std::span<IT> indexData,
                            std::string_view name = "") -> std::expected<void, VkBindings::Result> {
        vertexCount = static_cast<uint32_t>(vertexData.size());
        indexCount = static_cast<uint32_t>(indexData.size());
        indexType = IT::getIndexType();
        return implInit(physicalDevice, device, CBctx, std::as_bytes(vertexData),
                        std::as_bytes(indexData), name);
    }

    template <typename VT>
    [[nodiscard]] auto init(const VkBindings::PhysicalDevice &physicalDevice,
                            const VkBindings::Device &device, CommandBufferContext &CBctx,
                            std::span<VT> vertexData, std::string_view name = "")
        -> std::expected<void, VkBindings::Result> {
        vertexCount = static_cast<uint32_t>(vertexData.size());
        indexCount = 0;
        return implInit(physicalDevice, device, CBctx, std::as_bytes(vertexData), name);
    }

    void draw(const VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount = 1,
              uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
    void drawIndexed(const VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount = 1,
                     uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                     uint32_t firstInstance = 0) const;
};
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

} // namespace VkUtils
