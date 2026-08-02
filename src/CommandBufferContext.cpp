#include "CommandBufferContext.hpp"
#include "Functions.hpp"
#include "VkBindings/Enums.hpp"
#include "VkBindings/Objects.hpp"
#include "VkBindings/ObjectsForward.hpp"

#include <utility>

namespace VkUtils {

CommandBufferContext::CommandBufferContext(VkBindings::Device device, VkBindings::CommandPool pool,
                                           VkBindings::Queue submitQueue)
    : device(device), pool(pool), submitQueue(submitQueue), is_externaly_controlled(false) {}

CommandBufferContext::CommandBufferContext(VkBindings::CommandBuffer buffer)
    : buffer(buffer), is_externaly_controlled(true) {}

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
CommandBufferContext &CommandBufferContext::operator=(CommandBufferContext &&other) {
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
VkBindings::Result CommandBufferContext::init() {
    assert(!buffer && "A unsubmittet buffer already exists");
    return beginSingleTimeCommands(device, pool)
        .transform([&](auto &&buffersRes) {
            buffers = std::move(buffersRes);
            buffer = buffers[0];
        })
        .error_or(VkBindings::Result::eSuccess);
}
VkBindings::CommandBuffer CommandBufferContext::getBuffer() {
    assert(buffer && "The buffer has not been started");
    return buffer;
}

VkBindings::Result CommandBufferContext::flush() {

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
