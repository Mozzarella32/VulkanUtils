#pragma once

#include "CommandBufferContext.hpp"

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/StructsForward.hpp>

#include <VmaBindings/Vma.hpp>
#include <VmaBindings/VmaForward.hpp>

#include <cstddef>
#include <expected>
#include <optional>
#include <span>
#include <string_view>

namespace VkUtils {
struct StreamingBuffer {
  private:
    struct BufferWithAllocation {
        VkBindings::UniqueBuffer buffer;
        VmaBindings::UniqueAllocation allocation;
    };

    BufferWithAllocation destination;
    std::optional<BufferWithAllocation> staging;
    size_t size;

  public:
    static auto create(const VmaBindings::Allocator &allocator,
                       VkBindings::BufferCreateInfo bufferCreateInfo, size_t size,
                       std::string_view name = "")
        -> std::expected<StreamingBuffer, VkBindings::Result>;

    auto upload(std::span<const std::span<const std::byte>> datas, VkBindings::DeviceSize offset,
                VkUtils::CommandBufferContext &commandBufferContext) -> VkBindings::Result;

    auto upload(std::span<const std::byte> data, VkBindings::DeviceSize offset,
                VkUtils::CommandBufferContext &commandBufferContext) -> VkBindings::Result;

    [[nodiscard]] auto getBuffer() const -> VkBindings::Buffer;

    operator VkBindings::Buffer() const;
};
} // namespace VkUtils
