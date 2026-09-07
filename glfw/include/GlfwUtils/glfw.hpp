#pragma once

#include <VkBindings/Enums.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Structs.hpp>

#include <GLFW/glfw3.h> // IWYU pragma: export

#include <expected>

namespace GlfwUtils {
auto createWindowSurface(const VkBindings::Instance &instance, GLFWwindow *window,
                         const VkBindings::AllocationCallbacks *allocator)
    -> std::expected<VkBindings::UniqueSurfaceKHR, VkBindings::Result>;

auto initVulkanLoader();
}; // namespace GlfwUtils
