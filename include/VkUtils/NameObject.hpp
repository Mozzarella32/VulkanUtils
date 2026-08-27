#pragma once

#include "Errorhandling.hpp"

#include <VkBindings/Objects.hpp>
#include <VkBindings/Reflection/IsObject.hpp>
#include <VkBindings/Reflection/IsPool.hpp>
#include <VkBindings/Reflection/IsUnique.hpp>
#include <VkBindings/Reflection/ObjectToObjectType.hpp>
#include <VkBindings/Structs.hpp>

#include <format>
#include <ranges>
#include <string>
#include <utility>

namespace VkUtils {
template <VkBindings::Concepts::IsObject Obj>
auto nameObject(const VkBindings::Device &device, Obj obj, const std::string &name) -> void {
    if (name.empty())
        return;
    VkBindings::DebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo;
    debugUtilsObjectNameInfo.objectName = name;
    debugUtilsObjectNameInfo.objectHandle = std::bit_cast<
        decltype(std::declval<VkBindings::DebugUtilsObjectNameInfoEXT>().objectHandle)>(
        obj.getHandle());
    debugUtilsObjectNameInfo.objectType = VkBindings::Reflections::ObjectToObjectType<Obj>();
    unwrap(succeeded(device.setDebugUtilsObjectNameEXT(debugUtilsObjectNameInfo)),
           "VkBindings::nameObject");
}
template <VkBindings::Concepts::IsUnique Unique>
auto nameObject(const VkBindings::Device &device, const Unique &unique, const std::string &name)
    -> void {
    nameObject(device, unique.getObject(), name);
}

template <VkBindings::Concepts::IsPool Pool>
auto nameObject(const VkBindings::Device &device, const Pool &pool, const std::string &name)
    -> void {
    for (const auto &[i, item] : pool | std::views::enumerate) {
        nameObject(device, item, std::format("{}[{}]", name, i));
    }
}

template <typename T>
auto nameObjects(const VkBindings::Device &device, const T &objects, const std::string &name) {
    for (const auto &[i, object] : objects | std::views::enumerate) {
        nameObject(device, object, std::format("{}[{}]", name, i));
    }
}
} // namespace VkUtils
