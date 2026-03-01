#pragma once

#include <VulkanObjects.hpp>

#include <memory>
#include <optional>

namespace VkUtils {

struct CommandBufferContext {
  private:
    std::optional<std::reference_wrapper<VkBindings::UniqueVkDevice>> device;
    std::optional<std::reference_wrapper<VkBindings::UniqueVkCommandPool>> pool;
    VkBindings::HandleVkQueue submitQueue = VK_NULL_HANDLE;
    VkBindings::UniqueVkCommandBuffers buffers;
    VkBindings::HandleVkCommandBuffer buffer = VK_NULL_HANDLE;

    bool is_externaly_controlled;

    using AnyPtr = std::unique_ptr<void, void (*)(void *)>;
    std::vector<AnyPtr> lifetimecontainer;

  public:
    CommandBufferContext(VkBindings::UniqueVkDevice &device, VkBindings::UniqueVkCommandPool &pool,
                         VkBindings::HandleVkQueue submitQueue);
    CommandBufferContext(VkBindings::HandleVkCommandBuffer buffer);
    CommandBufferContext(CommandBufferContext &&other);

    CommandBufferContext &operator=(CommandBufferContext &&other);

    [[nodiscard]] std::expected<void, VkResult> init();
    VkBindings::HandleVkCommandBuffer getBuffer();

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
    [[nodiscard]] std::expected<void, VkResult> flush();

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
            std::cerr << "Adoption failed, was VK_NULL_HANDLE" << "\n";
        }
    }
    T &operator()() { return t; }
    T &get() { return t; }
};

} // namespace VkUtils
