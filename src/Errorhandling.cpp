#include "Errorhandling.hpp"

#include "VkBindings/EnumToString.hpp"

#include <iostream>
#include <sstream>

namespace VkUtils {

auto printFailedFunction(const std::string &func)
    -> std::function<VkBindings::Result(VkBindings::Result)> {
    return [func](VkBindings::Result res) -> VkBindings::Result {
        std::cerr << func << " failed with: " << VkBindings::Reflections::enumToString(res) << "\n";
        return res;
    };
}
auto throwFailed(const std::string &func, const std::source_location location)
    -> std::function<VkBindings::Result(VkBindings::Result)> {
    return [func, location](VkBindings::Result res) -> VkBindings::Result {
        std::stringstream str;
        str << std::string("in ") << location.file_name() << ": " << location.function_name()
            << ": " << std::to_string(location.column()) << ": " << func
            << ":\nfailed with: " << VkBindings::Reflections::enumToString(res) << "\n";

        throw std::runtime_error(str.str());
    };
}

void unwrap(std::expected<void, VkBindings::Result> &&expected, const std::string &func,
            const std::source_location location) {
    std::move(expected).transform_error(throwFailed(func, location)).value();
}
auto succeeded(VkBindings::Result res) -> std::expected<void, VkBindings::Result> {
    if (res != VkBindings::Result::eSuccess) {
        return std::unexpected(res);
    }
    return {};
}

} // namespace VkUtils
