#pragma once

#include <VkBindings/Enums.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/ObjectsForward.hpp>

#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

namespace VkUtils {

struct CommandBufferContext {
  private:
    VkBindings::Device device;
    VkBindings::CommandPool pool;

    VkBindings::Queue submitQueue;
    VkBindings::CommandBuffers buffers;
    VkBindings::CommandBuffer buffer;

    bool is_externaly_controlled;

    struct DeleterBase {
        DeleterBase() = default;
        DeleterBase(const DeleterBase &) = default;
        DeleterBase(DeleterBase &&) = default;
        auto operator=(const DeleterBase &) -> DeleterBase & = default;
        auto operator=(DeleterBase &&) -> DeleterBase & = default;

        virtual ~DeleterBase() = default;
        virtual void destroy() noexcept = 0;
    };

    template <typename T> struct Deleter final : DeleterBase {
      private:
        T *ptr;

      public:
        explicit Deleter(T *ptr) : ptr(ptr) {}
        ~Deleter() override { delete ptr; }

        Deleter(const Deleter &) = default;
        Deleter(Deleter &&) = default;
        auto operator=(const Deleter &) -> Deleter & = default;
        auto operator=(Deleter &&) -> Deleter & = default;

        void destroy() noexcept override {
            delete ptr;
            ptr = nullptr;
        }
    };

    // Store ownership of adopted objects.
    std::vector<std::unique_ptr<DeleterBase>> lifetimecontainer;

  public:
    CommandBufferContext(VkBindings::Device device, VkBindings::CommandPool pool,
                         VkBindings::Queue submitQueue);
    CommandBufferContext(VkBindings::CommandBuffer buffer);
    CommandBufferContext(const CommandBufferContext &) noexcept = delete;
    CommandBufferContext(CommandBufferContext &&other) noexcept;

    auto operator=(const CommandBufferContext &) noexcept -> CommandBufferContext & = delete;
    auto operator=(CommandBufferContext &&other) noexcept -> CommandBufferContext &;

    [[nodiscard]] auto init() -> VkBindings::Result;
    auto getBuffer() -> VkBindings::CommandBuffer;

    template <typename T> void adopt(T &&value) {
#ifdef MY_VK_IMPL_PRINT_MEM_OPS
        MY_VK_PRINT_ADDR_SIMPLE(std::cout, ts.handle);
        std::cout << " adopted by ";
        MY_VK_PRINT_ADDR_SIMPLE(std::cout, buffer.handle);
        std::cout << "\n";
#endif
        lifetimecontainer.emplace_back(std::make_unique<Deleter<std::decay_t<T>>>(
            new std::decay_t<T>(std::forward<T>(value))));
    }

    [[nodiscard]] auto flush() -> VkBindings::Result;

    ~CommandBufferContext();
};

template <typename T> class CommandBufferContextAdopted {
    std::reference_wrapper<CommandBufferContext> CBctx;
    T t;

  public:
    CommandBufferContextAdopted(CommandBufferContext &CBctx) : CBctx(CBctx) {}
    ~CommandBufferContextAdopted() {
        if (t) {
            CBctx.get().adopt(std::move(t));
        } else {
            std::cerr << "Adoption failed, was VK_BINDINGS_NULL_HANDLE" << "\n";
        }
    }
    CommandBufferContextAdopted(const CommandBufferContextAdopted &) = delete;
    CommandBufferContextAdopted(CommandBufferContextAdopted &&) = delete;

    auto operator=(const CommandBufferContextAdopted &) -> CommandBufferContextAdopted & = delete;
    auto operator=(CommandBufferContextAdopted &&) -> CommandBufferContextAdopted & = delete;

    operator T &() { return t; }
    auto get() -> T & { return t; }
};

} // namespace VkUtils
