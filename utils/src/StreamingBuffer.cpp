#include "VkUtils/StreamingBuffer.hpp"

#include "CommandBufferContext.hpp"
#include "Errorhandling.hpp"

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Bits.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Structs.hpp>
#include <VkBindings/StructsForward.hpp>

#include <VmaBindings/Vma.hpp>
#include <VmaBindings/VmaForward.hpp>

#include <array>
#include <cassert>
#include <cstddef>
#include <expected>
#include <numeric>
#include <optional>
#include <span>
#include <tuple>
#include <utility>

namespace VkBindings {
auto StreamingBuffer::init(const VmaBindings::Allocator &allocator,
                           VkBindings::BufferCreateInfo bufferCreateInfo, size_t size)
    -> VkBindings::Result {
    this->size = size;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage |= VkBindings::BufferUsageBits::TransferDst;
    return allocator
        .createBuffer(bufferCreateInfo,
                      {.flags = VmaBindings::AllocationCreateBits::HostAccessSequentialWrite |
                                VmaBindings::AllocationCreateBits::HostAccessAllowTransferInstead |
                                VmaBindings::AllocationCreateBits::Mapped,
                       .usage = VmaBindings::MemoryUsage::Auto})
        .and_then([&](std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation,
                                 VmaBindings::AllocationInfo> &&tuple)
                      -> std::expected<void, VkBindings::Result> {
            std::tie(buffer.buffer, buffer.allocation, std::ignore) = std::move(tuple);
            if (buffer.allocation.getMemoryProperties() &
                VkBindings::MemoryPropertyBits::HostVisible) {
                staging.reset();
                return {};
            }
            return allocator
                .createBuffer(
                    {.size = size, .usage = VkBindings::BufferUsageBits::TransferSrc},
                    {.flags = VmaBindings::AllocationCreateBits::HostAccessSequentialWrite |
                              VmaBindings::AllocationCreateBits::Mapped,
                     .usage = VmaBindings::MemoryUsage::Auto})
                .transform([&](std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation,
                                          VmaBindings::AllocationInfo> &&tuple) {
                    staging = BufferWithAllocation{};
                    std::tie(staging.value().buffer, staging.value().allocation, std::ignore) =
                        std::move(tuple);
                    return;
                });
        })
        .error_or(VkBindings::Result::Success);
}

auto StreamingBuffer::upload(std::span<const std::span<const std::byte>> datas,
                             VkBindings::DeviceSize offset,
                             VkUtils::CommandBufferContext &commandBufferContext)
    -> VkBindings::Result {
    const size_t totalSize =
        std::accumulate(datas.begin(), datas.end(), std::size_t{0},
                        [](std::size_t sum, auto bytes) { return sum + bytes.size(); });
    assert(offset + totalSize <= size);

    const VmaBindings::Allocation mapped =
        staging.has_value() ? staging.value().allocation : buffer.allocation;

    const auto allocator = buffer.allocation.getAllocator();

    auto dataOffset = offset;
    for (auto data : datas) {
        if (auto res = VkUtils::succeeded(
                allocator.copyMemoryToAllocation(data.data(), mapped, dataOffset, data.size()));
            !res) {
            return res.error();
        }
        dataOffset += data.size();
    }
    if (staging.has_value()) {
        commandBufferContext->copyBuffer(
            staging.value().buffer, buffer.buffer,
            VkBindings::BufferCopy{.srcOffset = offset, .dstOffset = offset, .size = totalSize});
    }
    return VkBindings::Result::Success;
}

auto StreamingBuffer::upload(std::span<const std::byte> data, VkBindings::DeviceSize offset,
                             VkUtils::CommandBufferContext &commandBufferContext)
    -> VkBindings::Result {
    return upload(std::array{data}, offset, commandBufferContext);
}

auto StreamingBuffer::getBuffer() const -> VkBindings::Buffer { return buffer.buffer; }
} // namespace VkBindings
