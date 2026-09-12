#pragma once

#include "VmaForward.hpp"

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Constants.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/Flags.hpp>
#include <VkBindings/Handles.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Structs.hpp>
#include <VkBindings/private/Creator.hpp>
#include <VkBindings/private/FunctionTables.hpp>
#include <VkBindings/private/Loader.hpp>
#include <VkBindings/private/ObjectTemplatesIntreface.hpp>
#include <VkBindings/private/StructTemplates/ArrayProxyInterface.hpp>
#include <VkBindings/private/StructTemplates/AssignableHandleInterface.hpp>
#include <VkBindings/private/StructTemplates/InOutString.hpp>
#include <VkBindings/private/vk_platform.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace VmaBindings {
namespace PFN {
using AllocateDeviceMemoryFunction = auto(VKAPI_PTR *)(Handle::Allocator, uint32_t memoryType,
                                                       VkBindings::Handle::DeviceMemory,
                                                       VkBindings::DeviceSize, void *pUserData)
    -> void;
using FreeDeviceMemoryFunction = auto(VKAPI_PTR *)(Handle::Allocator, uint32_t,
                                                   VkBindings::Handle::DeviceMemory,
                                                   VkBindings::DeviceSize, void *pUserData) -> void;
} // namespace PFN

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct DeviceMemoryCallbacks {
    PFN::AllocateDeviceMemoryFunction pfnAllocate;
    PFN::FreeDeviceMemoryFunction pfnFree;
    void *pUserData;
};

struct VulkanFunctions {
    VkBindings::PFN::GetInstanceProcAddr getInstanceProcAddr;
    VkBindings::PFN::GetDeviceProcAddr getDeviceProcAddr;
    VkBindings::PFN::GetPhysicalDeviceProperties getPhysicalDeviceProperties;
    VkBindings::PFN::GetPhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties;
    VkBindings::PFN::AllocateMemory allocateMemory;
    VkBindings::PFN::FreeMemory freeMemory;
    VkBindings::PFN::MapMemory mapMemory;
    VkBindings::PFN::UnmapMemory unmapMemory;
    VkBindings::PFN::FlushMappedMemoryRanges flushMappedMemoryRanges;
    VkBindings::PFN::InvalidateMappedMemoryRanges invalidateMappedMemoryRanges;
    VkBindings::PFN::BindBufferMemory bindBufferMemory;
    VkBindings::PFN::BindImageMemory bindImageMemory;
    VkBindings::PFN::GetBufferMemoryRequirements getBufferMemoryRequirements;
    VkBindings::PFN::GetImageMemoryRequirements getImageMemoryRequirements;
    VkBindings::PFN::CreateBuffer createBuffer;
    VkBindings::PFN::DestroyBuffer destroyBuffer;
    VkBindings::PFN::CreateImage createImage;
    VkBindings::PFN::DestroyImage destroyImage;
    VkBindings::PFN::CmdCopyBuffer cmdCopyBuffer;
    VkBindings::PFN::GetBufferMemoryRequirements2 getBufferMemoryRequirements2;
    VkBindings::PFN::GetImageMemoryRequirements2 getImageMemoryRequirements2;
    VkBindings::PFN::BindBufferMemory2 bindBufferMemory2;
    VkBindings::PFN::BindImageMemory2 bindImageMemory2;
    VkBindings::PFN::GetPhysicalDeviceMemoryProperties2 getPhysicalDeviceMemoryProperties2;
    VkBindings::PFN::GetDeviceBufferMemoryRequirements getDeviceBufferMemoryRequirements;
    VkBindings::PFN::GetDeviceImageMemoryRequirements getDeviceImageMemoryRequirements;
#if VK_USE_PLATFORM_WIN32_KHR
    VkBindings::PFN::GetMemoryWin32HandleKHR getMemoryWin32HandleKHR;
#else
    void *getMemoryWin32HandleKHR;
#endif
    VkBindings::PFN::GetPhysicalDeviceProperties2 getPhysicalDeviceProperties2;
};

struct AllocatorCreateInfo {
    AllocatorCreateFlags flags = {};
    VkBindings::impl_Struct::AssignableHandle<VkBindings::PhysicalDevice> physicalDevice;
    VkBindings::DeviceSize preferredLargeHeapBlockSize = 0;
    const VkBindings::AllocationCallbacks *pAllocationCallbacks = nullptr;
    const DeviceMemoryCallbacks *pDeviceMemoryCallbacks = nullptr;
    const VkBindings::DeviceSize *pHeapSizeLimit = nullptr;
    VkBindings::impl_Struct::AssignableHandle<VkBindings::Instance> instance;
    uint32_t vulkanApiVersion = 0;
    const VkBindings::ExternalMemoryHandleTypeFlags *pTypeExternalMemoryHandleTypes = nullptr;
};

struct AllocatorInfo {
    VkBindings::Instance instance;
    VkBindings::PhysicalDevice physicalDevice;
    VkBindings::Device device;
};

struct Statistics {
    uint32_t blockCount;
    uint32_t allocationCount;
    VkBindings::DeviceSize blockBytes;
    VkBindings::DeviceSize allocationBytes;
};

struct DetailedStatistics {
    Statistics statistics;
    uint32_t unusedRangeCount;
    VkBindings::DeviceSize allocationSizeMin;
    VkBindings::DeviceSize allocationSizeMax;
    VkBindings::DeviceSize unusedRangeSizeMin;
    VkBindings::DeviceSize unusedRangeSizeMax;
};

struct TotalStatistics {
    std::array<DetailedStatistics, VkBindings::Constants::MaxMemoryTypes> memoryType;
    std::array<DetailedStatistics, VkBindings::Constants::MaxMemoryHeaps> memoryHeap;
    DetailedStatistics total;
};

struct Budget {
    Statistics statistics;
    VkBindings::DeviceSize usage;
    VkBindings::DeviceSize budget;
};

struct AllocationCreateInfo {
    AllocationCreateFlags flags = {};
    MemoryUsage usage = {};
    VkBindings::MemoryPropertyFlags requiredFlags = {};
    VkBindings::MemoryPropertyFlags preferredFlags = {};
    uint32_t memoryTypeBits = 0;
    VkBindings::impl_Struct::AssignableHandle<Pool> pool;
    void *pUserData = nullptr;
    float priority = 0.0F;
    VkBindings::DeviceSize minAlignment = 0;
};

struct PoolCreateInfo {
    uint32_t memoryTypeIndex;
    PoolCreateFlags flags;
    VkBindings::DeviceSize blockSize;
    size_t minBlockCount;
    size_t maxBlockCount;
    float priority;
    VkBindings::DeviceSize minAllocationAlignment;
    void *pMemoryAllocateNext;
};

struct AllocationInfo {
    uint32_t memoryType = 0;
    VkBindings::DeviceMemory deviceMemory;
    VkBindings::DeviceSize offset = 0;
    VkBindings::DeviceSize size = 0;
    void *pMappedData = nullptr;
    void *pUserData = nullptr;
    const char *pName = nullptr;
};

struct AllocationInfo2 {
    AllocationInfo allocationInfo;
    VkBindings::DeviceSize blockSize = 0;
    VkBindings::Bool32 dedicatedMemory = VkBindings::Constants::False;
};

namespace PFN {
using CheckDefragmentationBreakFunction = auto(VKAPI_PTR *)(void *) -> VkBindings::Bool32;
} // namespace PFN

struct DefragmentationInfo {
    DefragmentationFlags flags = {};
    VkBindings::impl_Struct::AssignableHandle<Pool> pool;
    VkBindings::DeviceSize maxBytesPerPass = 0;
    uint32_t maxAllocationsPerPass = 0;
    PFN::CheckDefragmentationBreakFunction pfnBreakCallback = nullptr;
    void *pBreakCallbackUserData = nullptr;
};

struct DefragmentationStats {
    VkBindings::DeviceSize bytesMoved;
    VkBindings::DeviceSize bytesFreed;
    uint32_t allocationsMoved;
    uint32_t deviceMemoryBlocksFreed;
};

struct VirtualBlockCreateInfo {
    VkBindings::DeviceSize size;

    VirtualBlockCreateFlags flags;

    const VkBindings::AllocationCallbacks *pAllocationCallbacks;
};

struct VirtualAllocationCreateInfo {
    VkBindings::DeviceSize size;
    VkBindings::DeviceSize alignment;
    VirtualAllocationCreateFlags flags;
    void *pUserData;
};

struct VirtualAllocationInfo {
    VkBindings::DeviceSize offset;
    VkBindings::DeviceSize size;
    void *pUserData;
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

struct Allocation : public impl_Objects::ObjectOwner<Handle::Allocation, Handle::Allocator> {
    using ObjectOwner::ObjectOwner;
    Allocation() = default;

    auto getAllocationInfo() -> AllocationInfo;

    auto getAllocationInfo2() -> AllocationInfo2;

    void setUserData(void *pUserData);

    void setName(VkBindings::impl_Struct::InOutString name);

    auto getMemoryProperties() -> VkBindings::MemoryPropertyFlags;

#if VK_USE_PLATFORM_WIN32_KHR
    auto GetMemoryWin32Handle(HANDLE hTargetProcess) -> std::expected<HANDLE, VkBindings::Result>;

    auto GetMemoryWin32Handle2(VkBindings::ExternalMemoryHandleTypeBits handleType,
                               HANDLE hTargetProcess) -> std::expected<HANDLE, VkBindings::Result>;
#endif // VMA_EXTERNAL_MEMORY_WIN32

    auto mapMemory() -> std::expected<void *, VkBindings::Result>;

    void unmapMemory();

    auto bindBufferMemory(const VkBindings::Buffer &buffer) -> VkBindings::Result;

    auto bindBufferMemory2(VkBindings::DeviceSize allocationLocalOffset,
                           const VkBindings::Buffer &buffer, const void *pNext)
        -> VkBindings::Result;

    auto bindImageMemory(const VkBindings::Image &image) -> VkBindings::Result;

    auto bindImageMemory2(VkBindings::DeviceSize allocationLocalOffset,
                          const VkBindings::Image &image, const void *pNext) -> VkBindings::Result;
};

struct DefragmentationMove {
    DefragmentationMoveOperation operation = {};
    Allocation srcAllocation;
    Allocation dstTmpAllocation;
};

struct DefragmentationPassMoveInfo {
    std::vector<DefragmentationMove> moves;

  private:
    void *originalMoves = nullptr;
    friend DefragmentationContext;
};

class UniqueMemoryPages {
    const Allocator *allocator{};
    std::vector<Allocation> allocations;
    std::vector<AllocationInfo> infos;

  public:
    UniqueMemoryPages(const Allocator *allocator, std::vector<Allocation> &&allocations,
                      std::vector<AllocationInfo> &&infos);

    ~UniqueMemoryPages();

    UniqueMemoryPages(const UniqueMemoryPages &) = delete;
    auto operator=(const UniqueMemoryPages &) -> UniqueMemoryPages & = delete;

    UniqueMemoryPages(UniqueMemoryPages &&other) noexcept;

    auto operator=(UniqueMemoryPages &&other) noexcept -> UniqueMemoryPages &;

    void free() noexcept;

    [[nodiscard]]
    auto getAllocations() const noexcept -> const std::vector<Allocation> &;

    [[nodiscard]]
    auto getInfos() const noexcept -> const std::vector<AllocationInfo> &;
};

struct Allocator : public impl_Objects::Object<Handle::Allocator> {
    using impl_Objects::Object<Handle::Allocator>::Object;
    Allocator() = default;

  private:
    const VkBindings::impl_Loader::Dispatcher *dispatcher = nullptr;

    friend VkBindings::impl_Objects::Creator;

    Allocator(const handle_type &handle, const VkBindings::impl_Loader::Dispatcher *dispatcher);

  public:
    [[nodiscard]] auto getAllocatorInfo() const -> AllocatorInfo;

    [[nodiscard]] auto getPhysicalDeviceProperties() const
        -> const VkBindings::PhysicalDeviceProperties &;

    [[nodiscard]] auto getMemoryProperties() const
        -> const VkBindings::PhysicalDeviceMemoryProperties &;

    [[nodiscard]] auto getMemoryTypeProperties(uint32_t memoryTypeIndex) const
        -> VkBindings::MemoryPropertyFlags;

    void setCurrentFrameIndex(uint32_t frameIndex) const;

    [[nodiscard]] auto calculateStatistics() const -> TotalStatistics;

    [[nodiscard]] auto getHeapBudgets() const -> Budget;

    [[nodiscard]] auto findMemoryTypeIndex(uint32_t memoryTypeBits,
                                           const AllocationCreateInfo &allocationCreateInfo) const
        -> std::expected<uint32_t, VkBindings::Result>;

    [[nodiscard]] auto
    findMemoryTypeIndexForBufferInfo(const VkBindings::BufferCreateInfo &bufferCreateInfo,
                                     const AllocationCreateInfo &allocationCreateInfo) const
        -> std::expected<uint32_t, VkBindings::Result>;

    [[nodiscard]] auto
    findMemoryTypeIndexForImageInfo(const VkBindings::ImageCreateInfo &imageCreateInfo,
                                    const AllocationCreateInfo &allocationCreateInfo) const
        -> std::expected<uint32_t, VkBindings::Result>;

    [[nodiscard]] auto createPool(const PoolCreateInfo &createInfo) const
        -> std::expected<UniquePool, VkBindings::Result>;

    [[nodiscard]] auto allocateMemory(const VkBindings::MemoryRequirements &memoryRequirements,
                                      const AllocationCreateInfo &createInfo) const
        -> std::expected<std::tuple<UniqueAllocation, AllocationInfo>, VkBindings::Result>;

    [[nodiscard]] auto
    allocateDedicatedMemory(const VkBindings::MemoryRequirements &memoryRequirements,
                            const AllocationCreateInfo &createInfo, void *pMemoryAllocateNext) const
        -> std::expected<std::tuple<UniqueAllocation, AllocationInfo>, VkBindings::Result>;

    [[nodiscard]] auto allocateMemoryPages(const VkBindings::MemoryRequirements &memoryRequirements,
                                           const AllocationCreateInfo &createInfo,
                                           size_t allocationCount) const
        -> std::expected<UniqueMemoryPages, VkBindings::Result>;

    [[nodiscard]] auto allocateMemoryForBuffer(const VkBindings::Buffer &buffer,
                                               const AllocationCreateInfo &createInfo) const
        -> std::expected<std::tuple<UniqueAllocation, AllocationInfo>, VkBindings::Result>;

    [[nodiscard]] auto allocateMemoryForImage(const VkBindings::Image &image,
                                              const AllocationCreateInfo &createInfo) const
        -> std::expected<std::tuple<UniqueAllocation, AllocationInfo>, VkBindings::Result>;

    [[nodiscard]] auto flushAllocation(const Allocation &allocation, VkBindings::DeviceSize offset,
                                       VkBindings::DeviceSize size) const -> VkBindings::Result;

    [[nodiscard]] auto invalidateAllocation(const Allocation &allocation,
                                            VkBindings::DeviceSize offset,
                                            VkBindings::DeviceSize size) const
        -> VkBindings::Result;

    [[nodiscard]] auto
    flushAllocations(const VkBindings::impl_Struct::ArrayProxy<Allocation> &allocations,
                     const VkBindings::impl_Struct::ArrayProxy<VkBindings::DeviceSize> &offsets,
                     const VkBindings::impl_Struct::ArrayProxy<VkBindings::DeviceSize> &sizes) const
        -> VkBindings::Result;

    [[nodiscard]] auto invalidateAllocations(
        const VkBindings::impl_Struct::ArrayProxy<Allocation> &allocations,
        const VkBindings::impl_Struct::ArrayProxy<VkBindings::DeviceSize> &offsets,
        const VkBindings::impl_Struct::ArrayProxy<VkBindings::DeviceSize> &sizes) const
        -> VkBindings::Result;

    [[nodiscard]] auto copyMemoryToAllocation(const void *pSrcHostPointer,
                                              const Allocation &dstAllocation,
                                              VkBindings::DeviceSize dstAllocationLocalOffset,
                                              VkBindings::DeviceSize size) const
        -> VkBindings::Result;

    [[nodiscard]] auto copyAllocationToMemory(const Allocation &srcAllocation,
                                              VkBindings::DeviceSize srcAllocationLocalOffset,
                                              void *pDstHostPointer,
                                              VkBindings::DeviceSize size) const
        -> VkBindings::Result;

    [[nodiscard]] auto checkCorruption(uint32_t memoryTypeBits) const -> VkBindings::Result;

    [[nodiscard]] auto createBuffer(const VkBindings::BufferCreateInfo &bufferCreateInfo,
                                    const AllocationCreateInfo &allocationCreateInfo) const
        -> std::expected<std::tuple<VkBindings::UniqueBuffer, UniqueAllocation, AllocationInfo>,
                         VkBindings::Result>;

    [[nodiscard]] auto
    createBufferWithAlignment(const VkBindings::BufferCreateInfo &bufferCreateInfo,
                              const AllocationCreateInfo &allocationCreateInfo,
                              VkBindings::DeviceSize minAlignment) const
        -> std::expected<std::tuple<VkBindings::UniqueBuffer, UniqueAllocation, AllocationInfo>,
                         VkBindings::Result>;

    [[nodiscard]] auto createDedicatedBuffer(const VkBindings::BufferCreateInfo &bufferCreateInfo,
                                             const AllocationCreateInfo &allocationCreateInfo,
                                             void *pMemoryAllocateNext) const
        -> std::expected<std::tuple<VkBindings::UniqueBuffer, UniqueAllocation, AllocationInfo>,
                         VkBindings::Result>;

    [[nodiscard]] auto
    createAliasingBuffer(const Allocation &allocation,
                         const VkBindings::BufferCreateInfo &bufferCreateInfo) const
        -> std::expected<VkBindings::UniqueBuffer, VkBindings::Result>;

    [[nodiscard]] auto
    createAliasingBuffer2(const Allocation &allocation,
                          VkBindings::DeviceSize allocationLocalOffset,
                          const VkBindings::BufferCreateInfo &bufferCreateInfo) const
        -> std::expected<VkBindings::UniqueBuffer, VkBindings::Result>;

    [[nodiscard]] auto createImage(const VkBindings::ImageCreateInfo &imageCreateInfo,
                                   const AllocationCreateInfo &allocationCreateInfo) const
        -> std::expected<std::tuple<VkBindings::UniqueImage, UniqueAllocation, AllocationInfo>,
                         VkBindings::Result>;

    [[nodiscard]] auto createDedicatedImage(const VkBindings::ImageCreateInfo &imageCreateInfo,
                                            const AllocationCreateInfo &allocationCreateInfo,
                                            void *pMemoryAllocateNext) const
        -> std::expected<std::tuple<VkBindings::UniqueImage, UniqueAllocation, AllocationInfo>,
                         VkBindings::Result>;

    [[nodiscard]] auto createAliasingImage(const Allocation &allocation,
                                           const VkBindings::ImageCreateInfo &imageCreateInfo) const
        -> std::expected<VkBindings::UniqueImage, VkBindings::Result>;

    [[nodiscard]] auto
    createAliasingImage2(const Allocation &allocation, VkBindings::DeviceSize allocationLocalOffset,
                         const VkBindings::ImageCreateInfo &imageCreateInfo) const
        -> std::expected<VkBindings::UniqueImage, VkBindings::Result>;

    [[nodiscard]] auto buildStatsString(VkBindings::Bool32 detailedMap) const -> std::string;

    [[nodiscard]] auto beginDefragmentation(const DefragmentationInfo &info) const
        -> std::expected<DefragmentationContext, VkBindings::Result>;

    void endDefragmentation(const DefragmentationContext &context) const;

    [[nodiscard]] auto endDefragmentationGetStats(const DefragmentationContext &context) const
        -> DefragmentationStats;
};

struct DefragmentationContext
    : public impl_Objects::ObjectOwner<Handle::DefragmentationContext, Handle::Allocator> {
    using ObjectOwner::ObjectOwner;
    DefragmentationContext() = default;

    [[nodiscard]] auto beginPass() const
        -> std::expected<DefragmentationPassMoveInfo, VkBindings::Result>;

    [[nodiscard]] auto endPass(DefragmentationPassMoveInfo &passInfo) const -> VkBindings::Result;
};

struct Pool : public impl_Objects::ObjectOwner<Handle::Pool, Handle::Allocator> {
    using ObjectOwner::ObjectOwner;
    Pool() = default;

    [[nodiscard]] auto getStatistics() const -> Statistics;

    [[nodiscard]] auto calculateStatistics() const -> DetailedStatistics;

    [[nodiscard]] auto checkCorruption() const -> VkBindings::Result;

    [[nodiscard]] auto getName() const -> std::string_view;

    void setName(VkBindings::impl_Struct::InOutString name) const;
};

struct VirtualAllocation
    : public impl_Objects::ObjectOwner<Handle::VirtualAllocation, Handle::VirtualBlock> {
    using ObjectOwner::ObjectOwner;
    VirtualAllocation() = default;

    [[nodiscard]] auto getAllocationInfo() const -> VirtualAllocationInfo;

    void setUserData(void *pUserData) const;
};

struct VirtualBlock : public impl_Objects::Object<Handle::VirtualBlock> {
    using impl_Objects::Object<Handle::VirtualBlock>::Object;
    VirtualBlock() = default;

    [[nodiscard]] auto isEmpty() const -> VkBindings::Bool32;

    [[nodiscard]] auto virtualAllocate(const VirtualAllocationCreateInfo &createInfo) const
        -> std::expected<std::tuple<UniqueVirtualAllocation, VkBindings::DeviceSize>,
                         VkBindings::Result>;

    void clear() const;

    [[nodiscard]] auto getStatistics() const -> Statistics;

    [[nodiscard]] auto calculateStatistics() const -> DetailedStatistics;

    [[nodiscard]] auto buildStatsString(VkBindings::Bool32 detailedMap) const -> std::string;
};

// Device is needed to snatch the dispatcher from somewhere, as the AllocatorCreateInfo does only
// contain the handle
auto createAllocator(const VkBindings::Device &device, const AllocatorCreateInfo &createInfo)
    -> std::expected<UniqueAllocator, VkBindings::Result>;

auto createVirtualBlock(const VirtualBlockCreateInfo &createInfo)
    -> std::expected<UniqueVirtualBlock, VkBindings::Result>;
} // namespace VmaBindings
