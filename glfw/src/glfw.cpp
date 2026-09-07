// Has to be for glfw.hpp
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define GLFW_INCLUDE_VULKAN 1
// NOLINTEND(cppcoreguidelines-macro-usage)
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "GlfwUtils/glfw.hpp"

#include <VkBindings/Concepts.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/Loader.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Structs.hpp>

#include <VkUtils/Errorhandling.hpp>

#include <expected>
#include <utility>

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
namespace GlfwUtils {
auto createWindowSurface(const VkBindings::Instance &instance, GLFWwindow *window,
                         const VkBindings::AllocationCallbacks *allocator)
    -> std::expected<VkBindings::UniqueSurfaceKHR, VkBindings::Result> {
    VkBindings::SurfaceKHR rawSurface;

    static_assert(VkBindings::Concepts::ABIIsHandle<VkBindings::SurfaceKHR>);
    return VkUtils::succeeded(static_cast<VkBindings::Result>(glfwCreateWindowSurface(
                                  reinterpret_cast<VkInstance>(instance.getHandle()), window,
                                  reinterpret_cast<const VkAllocationCallbacks *>(allocator),
                                  reinterpret_cast<VkSurfaceKHR *>(&rawSurface))))
        .transform([&]() { return instance.adoptForignSurfaceKHR(std::move(rawSurface)); });
}

auto initVulkanLoader() {
    glfwInitVulkanLoader(
        reinterpret_cast<PFN_vkGetInstanceProcAddr>(VkBindings::Loader::GetGetInstanceProcAddr()));
}
}; // namespace GlfwUtils
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
