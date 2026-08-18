#include <VkBindings/EnumToString.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Structs.hpp>

#include "Errorhandling.hpp"
#include "PiplineCacheManager.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
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
    if (std::filesystem::exists(cache_file)) {
        std::ifstream inFile(cache_file, std::ios::binary);
        size_t size = 0;
        inFile.read(reinterpret_cast<char *>(&size), sizeof(size_t));
        std::vector<uint8_t> data(size);
        inFile.read(reinterpret_cast<char *>(data.data()),
                    static_cast<std::streamsize>(data.size()));
        std::cout << "Read Pipline Cache: " << format_bytes(data.size()) << "\n";

        VkBindings::PipelineCacheCreateInfo createInfo;
        createInfo.initialDataSize = data.size();
        createInfo.pInitialData = data.data();

        // fallback if cache is bad
        auto resPipelineCache = device.createPipelineCache(createInfo);
        if (resPipelineCache) {
            pipelineCache = std::move(resPipelineCache.value());
            return;
        }
        std::cerr << "Cache was bad, falling back to new one: "
                  << VkBindings::Reflections::enumToString(resPipelineCache.error());
    }

    VkBindings::PipelineCacheCreateInfo createInfo;
    createInfo.initialDataSize = 0;
    createInfo.pInitialData = nullptr;
    pipelineCache = unwrap(device.createPipelineCache(createInfo), "createPiplineCache");
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
    std::cout << "Wrote Pipline Cache: " << format_bytes(data.size()) << "\n";
}
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

PipelineCacheManager::~PipelineCacheManager() {
    if (pipelineCache) {
        std::cerr << "Forgot to write back PiplineCacheData!\n";
    }
}
} // namespace VkUtils
