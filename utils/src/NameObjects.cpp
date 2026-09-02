#include "NameObject.hpp"

#include "Errorhandling.hpp"

#include "VkBindings/Objects.hpp"

#include <cstdint>
#include <format>
#include <string_view>

namespace VkUtils::impl {
auto nameObject(const VkBindings::Device &device, uint64_t objHandle,
                VkBindings::ObjectType objType, std::string_view name) -> void {
    if (name.empty())
        return;
    std::string nameStr{name};
    VkBindings::DebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo;
    debugUtilsObjectNameInfo.objectName = nameStr;
    debugUtilsObjectNameInfo.objectHandle = objHandle;
    debugUtilsObjectNameInfo.objectType = objType;
    unwrap(succeeded(device.setDebugUtilsObjectNameEXT(debugUtilsObjectNameInfo)),
           "VkUtils::nameObject");
}

auto nameObject(const VkBindings::Device &device, uint64_t objHandle,
                VkBindings::ObjectType objType, std::string_view name, size_t idx) -> void {
    nameObject(device, objHandle, objType, std::format("{}[{}]", name, idx));
}

} // namespace VkUtils::impl
