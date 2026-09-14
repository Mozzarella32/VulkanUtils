#include "VkUtils/StreamingBuffer.hpp"

#include "CommandBufferContext.hpp"
#include "Errorhandling.hpp"
#include "NameObject.hpp"

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
#include <format>
#include <numeric>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>

namespace VkUtils {
auto StreamingBuffer::create(const VmaBindings::Allocator &allocator,
                             VkBindings::BufferCreateInfo bufferCreateInfo, size_t size,
                             std::string_view name)
    -> std::expected<StreamingBuffer, VkBindings::Result> {
    StreamingBuffer streamingBuffer;
    streamingBuffer.size = size;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage |= VkBindings::BufferUsageBits::TransferDst;
    const auto device = allocator.getDevice();
    return allocator
        .createBuffer(bufferCreateInfo,
                      {.flags = VmaBindings::AllocationCreateBits::HostAccessSequentialWrite |
                                VmaBindings::AllocationCreateBits::HostAccessAllowTransferInstead |
                                VmaBindings::AllocationCreateBits::Mapped,
                       .usage = VmaBindings::MemoryUsage::Auto})
        .and_then([&](std::tuple<VkBindings::UniqueBuffer, VmaBindings::UniqueAllocation,
                                 VmaBindings::AllocationInfo> &&tuple)
                      -> std::expected<void, VkBindings::Result> {
            std::tie(streamingBuffer.destination.buffer, streamingBuffer.destination.allocation,
                     std::ignore) = std::move(tuple);
            nameObject(device, streamingBuffer.destination.buffer,
                       std::format("{}StreamingDestination", name));
            nameObject(streamingBuffer.destination.allocation,
                       std::format("{}StreamingDestinationAllocation", name));
            if (streamingBuffer.destination.allocation.getMemoryProperties() &
                VkBindings::MemoryPropertyBits::HostVisible) {
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
                    streamingBuffer.staging = BufferWithAllocation{};
                    std::tie(streamingBuffer.staging.value().buffer,
                             streamingBuffer.staging.value().allocation, std::ignore) =
                        std::move(tuple);
                    nameObject(device, streamingBuffer.staging.value().buffer,
                               std::format("{}StreamingStagingBuffer", name));
                    nameObject(streamingBuffer.staging.value().allocation,
                               std::format("{}StreamingStagingAllocation", name));
                    return;
                });
        })
        .transform([&]() { return std::move(streamingBuffer); });
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
        staging.has_value() ? staging.value().allocation : destination.allocation;

    const auto allocator = destination.allocation.getAllocator();

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
            staging.value().buffer, destination.buffer,
            VkBindings::BufferCopy{.srcOffset = offset, .dstOffset = offset, .size = totalSize});
    }
    return VkBindings::Result::Success;
}

auto StreamingBuffer::upload(std::span<const std::byte> data, VkBindings::DeviceSize offset,
                             VkUtils::CommandBufferContext &commandBufferContext)
    -> VkBindings::Result {
    return upload(std::array{data}, offset, commandBufferContext);
}

auto StreamingBuffer::getBuffer() const -> VkBindings::Buffer { return destination.buffer; }
} // namespace VkUtils
