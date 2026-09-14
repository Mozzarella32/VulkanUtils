#pragma once

#include "CommandBufferContext.hpp"

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/StructsForward.hpp>

#include <VmaBindings/Vma.hpp>
#include <VmaBindings/VmaForward.hpp>

#include <cstddef>
#include <optional>
#include <span>

namespace VkUtils {
struct StreamingBuffer {
  private:
    struct BufferWithAllocation {
        VkBindings::UniqueBuffer buffer;
        VmaBindings::UniqueAllocation allocation;
    };

    BufferWithAllocation buffer;
    std::optional<BufferWithAllocation> staging;
    size_t size;

  public:
    auto init(const VmaBindings::Allocator &allocator,
              VkBindings::BufferCreateInfo bufferCreateInfo, size_t size) -> VkBindings::Result;

    auto upload(std::span<const std::span<const std::byte>> datas, VkBindings::DeviceSize offset,
                VkUtils::CommandBufferContext &commandBufferContext) -> VkBindings::Result;

    auto upload(std::span<const std::byte> data, VkBindings::DeviceSize offset,
                VkUtils::CommandBufferContext &commandBufferContext) -> VkBindings::Result;

    [[nodiscard]] auto getBuffer() const -> VkBindings::Buffer;
};
} // namespace VkBindings
