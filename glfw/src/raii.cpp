#include "GlfwUtils/raii.hpp"

#include "GlfwUtils/glfw.hpp"

namespace GlfwUtils {
Context::Context() { glfwInit(); }
Context::~Context() { glfwTerminate(); }

namespace impl {
void WindowDestructor::operator()(GLFWwindow *window) const { glfwDestroyWindow(window); }
} // namespace impl
} // namespace GlfwUtils
