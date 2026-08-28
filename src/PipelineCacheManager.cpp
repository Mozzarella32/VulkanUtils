#include <VkBindings/ObjectsForward.hpp>

#include "Errorhandling.hpp"
#include "PipelineCacheManager.hpp"
#include "VkBindings/Enums.hpp"

#include <array>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace VkUtils {

namespace {
auto format_bytes(size_t bytes) -> std::string {
    std::array units = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    auto size = static_cast<double>(bytes);

    constinit static const double base = 1024.0;

    while (size >= base && unit_index < 4) {
        size /= base;
        ++unit_index;
    }

    return std::format("{:.2f} {}", size, units.at(unit_index));
}
} // namespace

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
void PipelineCacheManager::read(const VkBindings::Device &device,
                                const std::filesystem::path &supplyed_cache_file) {
    cache_file = supplyed_cache_file;
    if (!std::filesystem::exists(cache_file)) {
        pipelineCache = unwrap(device.createPipelineCache({}), "createPiplineCache");
        return;
    }
    std::ifstream inFile(cache_file, std::ios::binary);
    size_t size = 0;
    inFile.read(reinterpret_cast<char *>(&size), sizeof(size_t));
    std::vector<std::byte> data(size);
    inFile.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    std::cout << "Read piplinecache: " << format_bytes(data.size()) << "\n";

    pipelineCache = unwrap(
        device
            .createPipelineCache({
                .initialDataSize = data.size(),
                .pInitialData = data.data(),
            })
            .or_else([&](VkBindings::Result err)
                         -> std::expected<VkBindings::UniquePipelineCache, VkBindings::Result> {
                std::ignore =
                    printFailedFunction("Pipelinecache was bad; falling back to a new one")(err);

                return device.createPipelineCache({});
            }),
        "Failed to create fallback pipelinecache");
}

void PipelineCacheManager::write(const VkBindings::Device &device) {
    if (!pipelineCache)
        return;
    auto data = unwrap(device.getPipelineCacheData(pipelineCache), "getPiplineCacheData");

    size_t size = data.size();
    std::ofstream outFile(cache_file, std::ios::binary);
    outFile.write(reinterpret_cast<char *>(&size), sizeof(size_t));
    outFile.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    pipelineCache.cleanup();
    std::cout << "Wrote piplinecache: " << format_bytes(data.size()) << "\n";
}
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

PipelineCacheManager::~PipelineCacheManager() {
    if (pipelineCache) {
        // cannot throw exception in destructur resenably
        std::cerr << "Forgot to write back PiplinecacheData!\n";
    }
}

PipelineCacheManager::operator VkBindings::PipelineCache() { return pipelineCache; }

auto PipelineCacheManager::get() -> VkBindings::PipelineCache { return pipelineCache; }

} // namespace VkUtils
