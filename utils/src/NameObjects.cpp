#include "NameObject.hpp"

#include <VmaBindings/Vma.hpp>
#include <VmaBindings/VmaForward.hpp>

#include "Errorhandling.hpp"

#include <VkBindings/Enums.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/Structs.hpp>

#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <utility>

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
    std::ignore = succeeded(device.setDebugUtilsObjectNameEXT(debugUtilsObjectNameInfo))
                      .transform_error(printFailedFunction("VkUtils::nameObject"));
}

auto nameObject(const VkBindings::Device &device, uint64_t objHandle,
                VkBindings::ObjectType objType, std::string_view name, size_t idx) -> void {
    nameObject(device, objHandle, objType, std::format("{}[{}]", name, idx));
}
} // namespace VkUtils::impl

namespace VkUtils {
auto nameObject(const VmaBindings::Allocation &allocation, std::string_view name) -> void {
    allocation.setName(std::string(name));
}

auto nameObject(const VmaBindings::Pool &pool, std::string_view name) -> void {
    pool.setName(std::string(name));
}
auto nameObject(const VmaBindings::UniquePool &pool, std::string_view name) -> void {
    nameObject(pool.getObject(), name);
}

auto nameObject(const VmaBindings::UniqueAllocation &allocation, std::string_view name) -> void {
    nameObject(allocation.getObject(), name);
}
} // namespace VkUtils

namespace VkUtils::impl {
auto nameObject(const VmaBindings::Allocation &allocation, std::string_view name, size_t idx)
    -> void {
    allocation.setName(std::format("{}[{}]", name, idx));
}

auto nameObject(const VmaBindings::Pool &pool, std::string_view name, size_t idx) -> void {
    pool.setName(std::format("{}[{}]", name, idx));
}
} // namespace VkUtils::impl
