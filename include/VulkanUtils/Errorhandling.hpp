#pragma once

#include <expected>
#include <functional>
#include <source_location>
#include <string>

#include "VkBindings/Enums.hpp"

namespace VkUtils {
[[nodiscard]] auto printFailedFunction(const std::string &func)
    -> std::function<VkBindings::Result(VkBindings::Result)>;

[[nodiscard]] auto
throwFailed(const std::string &func,
            const std::source_location location = std::source_location::current())
    -> std::function<VkBindings::Result(VkBindings::Result)>;

template <typename T>
T unwrap(std::expected<T, VkBindings::Result> &&expected, const std::string &func,
         const std::source_location location = std::source_location::current()) {
    auto e = std::move(expected).transform_error(throwFailed(func, location));
    T tmp = std::move(e).value();
    return tmp;
}

void unwrap(std::expected<void, VkBindings::Result> &&expected, const std::string &func,
            const std::source_location location = std::source_location::current());

auto succeeded(VkBindings::Result res) -> std::expected<void, VkBindings::Result>;
} // namespace VkUtils
