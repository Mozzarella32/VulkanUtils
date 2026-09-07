#pragma once
#include <memory>

extern "C" {
struct GLFWwindow;
}

namespace GlfwUtils {
struct Context {
    Context();
    ~Context();
    Context(const Context &) noexcept = delete;
    Context(Context &&) noexcept = delete;
    auto operator=(const Context &) noexcept -> Context = delete;
    auto operator=(Context &&) noexcept -> Context = delete;
};

namespace impl {
struct WindowDestructor {
    void operator()(GLFWwindow *window) const;
};
} // namespace impl

using UniqueWindow = std::unique_ptr<GLFWwindow, impl::WindowDestructor>;
} // namespace GlfwUtils
