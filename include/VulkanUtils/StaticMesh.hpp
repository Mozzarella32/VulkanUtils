#pragma once

#include "CommandBufferContext.hpp"

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/ObjectsForward.hpp>

#include <concepts>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
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

    auto Init(const VkBindings::PhysicalDevice &physicalDevice, const VkBindings::Device &device,
              CommandBufferContext &CBctx, std::span<const uint8_t> vertexData,
              std::span<const uint8_t> indexData, VkBindings::IndexType indexType,
              const std::string &name) -> std::expected<void, VkBindings::Result>;

    auto Init(const VkBindings::PhysicalDevice &physicalDevice, const VkBindings::Device &device,
              CommandBufferContext &CBctx, const std::span<const uint8_t> &vertexData,
              const std::string &name = "") -> std::expected<void, VkBindings::Result>;

  public:
    template <typename VT, typename IT>
        requires requires {
            { IT::getIndexType() } -> std::same_as<VkBindings::IndexType>;
        }
    [[nodiscard]] auto Init(const VkBindings::PhysicalDevice &physicalDevice,
                            const VkBindings::Device &device, CommandBufferContext &CBctx,
                            const std::vector<VT> &vertexData, const std::vector<IT> &indexData,
                            const std::string &name = "")
        -> std::expected<void, VkBindings::Result> {
        return Init(physicalDevice, device, CBctx, vertexData, indexData, name);
    }

    template <typename VT>
    [[nodiscard]] auto Init(const VkBindings::PhysicalDevice &physicalDevice,
                            const VkBindings::Device &device, CommandBufferContext &CBctx,
                            const std::vector<VT> &vertexData, const std::string &name = "")
        -> std::expected<void, VkBindings::Result> {
        return Init(physicalDevice, device, CBctx, vertexData, name);
    }

    void draw(const VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount = 1,
              uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
    void drawIndexed(const VkBindings::CommandBuffer &commandBuffer, uint32_t instanceCount = 1,
                     uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                     uint32_t firstInstance = 0) const;
};
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

} // namespace VkUtils
