#pragma once

#include <expected>
#include <functional>
#include <source_location>
#include <string_view>

#include "VkBindings/Enums.hpp"

namespace VkUtils {
[[nodiscard]] auto printFailedFunction(std::string_view func)
    -> std::function<VkBindings::Result(VkBindings::Result)>;

[[nodiscard]] auto throwFailed(std::string_view func,
                               std::source_location location = std::source_location::current())
    -> std::function<VkBindings::Result(VkBindings::Result)>;

template <typename T>
auto unwrap(std::expected<T, VkBindings::Result> &&expected, std::string_view func,
            const std::source_location location = std::source_location::current()) -> T {
    auto &&error = std::move(expected).transform_error(throwFailed(func, location));
    T tmp = std::move(error).value();
    return tmp;
}

void unwrap(std::expected<void, VkBindings::Result> &&expected, std::string_view func,
            std::source_location location = std::source_location::current());

auto succeeded(VkBindings::Result res) -> std::expected<void, VkBindings::Result>;
} // namespace VkUtils
