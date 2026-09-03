#pragma once

#include "VkBindings/Enums.hpp"

#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Reflection/IsObject.hpp>
#include <VkBindings/Reflection/IsPool.hpp>
#include <VkBindings/Reflection/IsUnique.hpp>
#include <VkBindings/Reflection/ObjectToObjectType.hpp>

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace VkUtils {

namespace impl {
// delay heavy includes trough type erasure
auto nameObject(const VkBindings::Device &device, uint64_t objHandle,
                VkBindings::ObjectType objType, std::string_view name) -> void;
auto nameObject(const VkBindings::Device &device, uint64_t objHandle,
                VkBindings::ObjectType objType, std::string_view name, size_t idx) -> void;

// pull idx trough
template <VkBindings::Concepts::IsObject Obj>
auto nameObject(const VkBindings::Device &device, const Obj &obj, std::string_view name, size_t idx)
    -> void {
    nameObject(device, std::bit_cast<uint64_t>(obj.getHandle()),
               VkBindings::Reflections::ObjectToObjectType<Obj>(), name, idx);
}
template <VkBindings::Concepts::IsUnique Unique>
auto nameObject(const VkBindings::Device &device, const Unique &unique, std::string_view name,
                size_t idx) -> void {
    nameObject(device, unique.getObject(), name, idx);
}
} // namespace impl

template <VkBindings::Concepts::IsObject Obj>
auto nameObject(const VkBindings::Device &device, const Obj &obj, std::string_view name) -> void {
    impl::nameObject(device, std::bit_cast<uint64_t>(obj.getHandle()),
                     VkBindings::Reflections::ObjectToObjectType<Obj>(), name);
}
template <VkBindings::Concepts::IsUnique Unique>
auto nameObject(const VkBindings::Device &device, const Unique &unique, std::string_view name)
    -> void {
    nameObject(device, unique.getObject(), name);
}

template <VkBindings::Concepts::IsPool Pool>
auto nameObject(const VkBindings::Device &device, const Pool &pool, std::string_view name) -> void {
    for (size_t i = 0; i < pool.size(); i++) {
        impl::nameObject(device, pool.at(i), name, i);
    }
}

template <typename T>
auto nameObjects(const VkBindings::Device &device, const T &objects, std::string_view name) {
    for (size_t i = 0; i < objects.size(); i++) {
        impl::nameObject(device, objects.at(i), name, i);
    }
}
} // namespace VkUtils
