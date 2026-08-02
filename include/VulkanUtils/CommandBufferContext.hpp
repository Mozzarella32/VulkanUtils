#pragma once

#include <VkBindings/Enums.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/StructsForward.hpp>

#include <expected>
#include <iostream>
#include <memory>
#include <type_traits>

namespace VkUtils {

struct CommandBufferContext {
  private:
    VkBindings::Device device;
    VkBindings::CommandPool pool;

    VkBindings::Queue submitQueue;
    VkBindings::CommandBuffers buffers;
    VkBindings::CommandBuffer buffer;

    bool is_externaly_controlled;

    using AnyPtr = std::unique_ptr<void, void (*)(void *)>;
    std::vector<AnyPtr> lifetimecontainer;

  public:
    CommandBufferContext(VkBindings::Device device, VkBindings::CommandPool pool,
                         VkBindings::Queue submitQueue);
    CommandBufferContext(VkBindings::CommandBuffer buffer);
    CommandBufferContext(CommandBufferContext &&other);

    CommandBufferContext &operator=(CommandBufferContext &&other);

    [[nodiscard]] std::expected<void, VkBindings::Result> init();
    VkBindings::CommandBuffer getBuffer();

    template <typename Ts> void adopt(Ts &&ts) {
#ifdef MY_VK_IMPL_PRINT_MEM_OPS
        MY_VK_PRINT_ADDR_SIMPLE(std::cout, ts.handle);
        std::cout << " adopted by ";
        MY_VK_PRINT_ADDR_SIMPLE(std::cout, buffer.handle);
        std::cout << "\n";
#endif
        [&] {
            using T = std::decay_t<Ts>;
            lifetimecontainer.push_back(
                AnyPtr(new T(std::forward<Ts>(ts)), [](void *p) { delete static_cast<T *>(p); }));
        }();
    }
    [[nodiscard]] VkBindings::Result flush();

    ~CommandBufferContext();
};

template <typename T> class CommandBufferContextAdopted {
    CommandBufferContext &CBctx;
    T t;

  public:
    CommandBufferContextAdopted(CommandBufferContext &CBctx) : CBctx(CBctx) {}
    ~CommandBufferContextAdopted() {
        if (t) {
            CBctx.adopt(std::move(t));
        } else {
            std::cerr << "Adoption failed, was VK_BINDINGS_NULL_HANDLE" << "\n";
        }
    }
    operator T() { return t; }
    T &get() { return t; }
};

} // namespace VkUtils
