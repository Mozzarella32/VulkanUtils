#include "CommandBufferContext.hpp"
#include "Functions.hpp"

namespace VkUtils {

CommandBufferContext::CommandBufferContext(VkBindings::UniqueVkDevice &device,
                                           VkBindings::UniqueVkCommandPool &pool,
                                           VkBindings::HandleVkQueue submitQueue)
    : device(device), pool(pool), submitQueue(submitQueue), is_externaly_controlled(false) {}

CommandBufferContext::CommandBufferContext(VkBindings::HandleVkCommandBuffer buffer)
    : buffer(buffer), is_externaly_controlled(true) {}

CommandBufferContext::CommandBufferContext(CommandBufferContext &&other) {
    lifetimecontainer = std::move(other.lifetimecontainer);
    is_externaly_controlled = std::exchange(other.is_externaly_controlled, true);
    device = std::exchange(other.device, {});
    pool = std::exchange(other.pool, {});
    submitQueue = std::exchange(other.submitQueue, VK_NULL_HANDLE);
    if (other.buffers) {
        buffers = std::move(other.buffers);
    } else {
        buffers.cleanup();
    }
    buffer = std::exchange(other.buffer, VK_NULL_HANDLE);
};
CommandBufferContext &CommandBufferContext::operator=(CommandBufferContext &&other) {
    assert(((is_externaly_controlled || !buffers) && lifetimecontainer.empty()) &&
           "The CommandBufferContext to move to had a unflushed CommandBuffer\n");
    lifetimecontainer = std::move(other.lifetimecontainer);
    is_externaly_controlled = std::exchange(other.is_externaly_controlled, true);
    device = std::exchange(other.device, {});
    pool = std::exchange(other.pool, {});
    submitQueue = std::exchange(other.submitQueue, VK_NULL_HANDLE);
    if (other.buffers) {
        buffers = std::move(other.buffers);
    } else {
        buffers.cleanup();
    }
    buffer = std::exchange(other.buffer, VK_NULL_HANDLE);
    return *this;
}
std::expected<void, VkResult> CommandBufferContext::init() {
    assert(!buffer && "A unsubmittet buffer already exists");
    return beginSingleTimeCommands(device.value(), pool.value()).transform([&](auto &&buffersRes) {
        buffers = std::move(buffersRes);
        buffer = buffers[0];
    });
}
VkBindings::HandleVkCommandBuffer CommandBufferContext::getBuffer() {
    assert(buffer && "The buffer has not been started");
    return buffer;
}

std::expected<void, VkResult> CommandBufferContext::flush() {

    if (!is_externaly_controlled) {
        if (buffers) {
            auto endRes = endSingleTimeCommands(submitQueue, buffers);
            buffer = VkBindings::HandleVkCommandBuffer{};
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
