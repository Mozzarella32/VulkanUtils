#pragma once

#include <VkBindings/Enums.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/StructsForward.hpp>

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

    auto operator=(CommandBufferContext &&other) -> CommandBufferContext &;

    [[nodiscard]] auto init() -> VkBindings::Result;
    auto getBuffer() -> VkBindings::CommandBuffer;

    template <typename Ts> void adopt(Ts &&ts) {
#ifdef MY_VK_IMPL_PRINT_MEM_OPS
        MY_VK_PRINT_ADDR_SIMPLE(std::cout, ts.handle);
        std::cout << " adopted by ";
        MY_VK_PRINT_ADDR_SIMPLE(std::cout, buffer.handle);
        std::cout << "\n";
#endif
        [&] -> auto {
            using T = std::decay_t<Ts>;
            lifetimecontainer.push_back(AnyPtr(
                new T(std::forward<Ts>(ts)), [](void *p) -> void { delete static_cast<T *>(p); }));
        }();
    }
    [[nodiscard]] auto flush() -> VkBindings::Result;

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
    auto get() -> T & { return t; }
};

} // namespace VkUtils
