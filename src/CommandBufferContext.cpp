#include "CommandBufferContext.hpp"
#include "Functions.hpp"
#include "VkBindings/Enums.hpp"
#include "VkBindings/Objects.hpp"
#include "VkBindings/ObjectsForward.hpp"

#include <utility>

namespace VkUtils {

CommandBufferContext::CommandBufferContext(VkBindings::Device device, VkBindings::CommandPool pool,
                                           VkBindings::Queue submitQueue)
    : device(std::move(device)), pool(std::move(pool)), submitQueue(std::move(submitQueue)),
      is_externaly_controlled(false) {}

CommandBufferContext::CommandBufferContext(VkBindings::CommandBuffer buffer)
    : buffer(std::move(buffer)), is_externaly_controlled(true) {}

CommandBufferContext::CommandBufferContext(CommandBufferContext &&other) {
    lifetimecontainer = std::move(other.lifetimecontainer);
    is_externaly_controlled = std::exchange(other.is_externaly_controlled, true);
    device = std::exchange(other.device, {});
    pool = std::exchange(other.pool, {});
    submitQueue = std::exchange(other.submitQueue, VkBindings::Queue{});
    if (other.buffers) {
        buffers = std::move(other.buffers);
    } else {
        buffers.cleanup();
    }
    buffer = std::exchange(other.buffer, VkBindings::CommandBuffer{});
};
auto CommandBufferContext::operator=(CommandBufferContext &&other) -> CommandBufferContext & {
    assert(((is_externaly_controlled || !buffers) && lifetimecontainer.empty()) &&
           "The CommandBufferContext to move to had a unflushed CommandBuffer\n");
    lifetimecontainer = std::move(other.lifetimecontainer);
    is_externaly_controlled = std::exchange(other.is_externaly_controlled, true);
    device = std::exchange(other.device, {});
    pool = std::exchange(other.pool, {});
    submitQueue = std::exchange(other.submitQueue, VkBindings::Queue{});
    if (other.buffers) {
        buffers = std::move(other.buffers);
    } else {
        buffers.cleanup();
    }
    buffer = std::exchange(other.buffer, VkBindings::CommandBuffer{});
    return *this;
}
auto CommandBufferContext::init() -> VkBindings::Result {
    assert(!buffer && "A unsubmittet buffer already exists");
    return beginSingleTimeCommands(device, pool)
        .transform([&](auto &&buffersRes) -> void {
            buffers = std::move(buffersRes);
            buffer = buffers[0];
        })
        .error_or(VkBindings::Result::eSuccess);
}
auto CommandBufferContext::getBuffer() -> VkBindings::CommandBuffer {
    assert(buffer && "The buffer has not been started");
    return buffer;
}

auto CommandBufferContext::flush() -> VkBindings::Result {

    if (!is_externaly_controlled) {
        if (buffers) {
            auto endRes = endSingleTimeCommands(submitQueue, buffers);
            buffer = VkBindings::CommandBuffer{};
            buffers.cleanup();
            lifetimecontainer.clear();
            return endRes;
        }
    }
    lifetimecontainer.clear();
    return {};
}

CommandBufferContext::~CommandBufferContext() {
    assert(((is_externaly_controlled || !buffers) && lifetimecontainer.empty()) &&
           "The CommandBufferContext has a unflushed CommandBuffer on deletion\n");
}

} // namespace VkUtils
