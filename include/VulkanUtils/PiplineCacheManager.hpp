#pragma once

#include <VkBindings/Objects.hpp>
#include <VkBindings/ObjectsForward.hpp>

#include <filesystem>

namespace VkUtils {
struct PipelineCacheManager {
  private:
    VkBindings::UniquePipelineCache pipelineCache;
    std::filesystem::path cache_file;

  public:
    void read(const VkBindings::Device &device, const std::filesystem::path &cache_file);
    void write(const VkBindings::Device &device);

    PipelineCacheManager() = default;
    PipelineCacheManager(const PipelineCacheManager &) = delete;
    PipelineCacheManager(PipelineCacheManager &&) = default;
    auto operator=(const PipelineCacheManager &) -> PipelineCacheManager & = delete;
    auto operator=(PipelineCacheManager &&) -> PipelineCacheManager & = default;

    ~PipelineCacheManager();
};
} // namespace VkUtils
