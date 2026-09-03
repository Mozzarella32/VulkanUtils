#include "NameObject.hpp"

#include "Errorhandling.hpp"

#include <VkBindings/Enums.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/Structs.hpp>

#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>

namespace VkUtils::impl {
auto nameObject(const VkBindings::Device &device, uint64_t objHandle,
                VkBindings::ObjectType objType, std::string_view name) -> void {
    if (name.empty())
        return;
    const std::string nameStr{name};
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
