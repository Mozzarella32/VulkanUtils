#pragma once

#include "VkBindings/Objects.hpp"
#include "VkBindings/Reflection/HandleToObjectType.hpp"
#include "VkBindings/Reflection/IsPool.hpp"
#include "VkBindings/Reflection/IsUnique.hpp"
#include "VkBindings/Structs.hpp"

#include "Errorhandling.hpp"

#include <utility>

namespace VkUtils {
template <VkBindings::Concepts::IsObject Obj>
auto nameObject(VkBindings::Device device, Obj obj, const std::string &name) -> void {
    if (name == "")
        return;
    VkBindings::DebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo;
    debugUtilsObjectNameInfo.objectName = name;
    debugUtilsObjectNameInfo.objectHandle = std::bit_cast<
        decltype(std::declval<VkBindings::DebugUtilsObjectNameInfoEXT>().objectHandle)>(
        obj.getHandle());
    debugUtilsObjectNameInfo.objectType = VkBindings::Reflections::HandleToObjectType<Obj>();
    std::ignore = succeeded(device.setDebugUtilsObjectNameEXT(debugUtilsObjectNameInfo))
                      .transform_error(printFailedFunction("VkBindings::nameObject"));
}
template <VkBindings::Concepts::IsUnique Unique>
auto nameObject(VkBindings::Device device, const Unique &unique, const std::string &name) -> void {
    nameObject(device, unique.getObject(), name);
}

template <VkBindings::Concepts::IsPool Pool>
auto nameObject(VkBindings::Device device, Pool pool, const std::string &name) -> void {
    for (size_t i = 0; i < pool.size(); i++) {
        nameObject(device, pool[i], name + " " + std::to_string(i));
    }
}
} // namespace VkUtils
