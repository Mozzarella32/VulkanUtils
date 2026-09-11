#pragma once
#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Constants.hpp>
#include <VkBindings/Defines.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/Flags.hpp>
#include <VkBindings/Handles.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Reflection/BitsToFlag.hpp>
#include <VkBindings/Reflection/FlagToBits.hpp>
#include <VkBindings/Reflection/HandleToObject.hpp>
#include <VkBindings/Reflection/IsBits.hpp>
#include <VkBindings/Reflection/IsEnum.hpp>
#include <VkBindings/Reflection/IsFlag.hpp>
#include <VkBindings/Reflection/IsObject.hpp>
#include <VkBindings/Reflection/IsUnique.hpp>
#include <VkBindings/Reflection/ObjectToHandle.hpp>
#include <VkBindings/Structs.hpp>
#include <VkBindings/private/Creator.hpp>
#include <VkBindings/private/FlagsInterface.hpp>
#include <VkBindings/private/FunctionTables.hpp>
#include <VkBindings/private/Loader.hpp>
#include <VkBindings/private/ObjectTemplatesIntreface.hpp>
#include <VkBindings/private/StructTemplates/ArrayProxyInterface.hpp>
#include <VkBindings/private/StructTemplates/AssignableHandleInterface.hpp>
#include <VkBindings/private/StructTemplates/InOutString.hpp>
#include <VkBindings/private/StructTemplates/VecViewInterface.hpp>
#include <VkBindings/private/vk_platform.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

namespace VmaBindings {

#define VMA_VERSION (VK_MAKE_VERSION(3, 4, 0))

// NOLINTBEGIN(performance-enum-size)
enum class AllocatorCreateBits : std::int32_t {
    ExternallySynchronized = 0x00000001,
    DedicatedAllocationKHR = 0x00000002,
    BindMemory2KHR = 0x00000004,
    MemoryBudgetEXT = 0x00000008,
    DeviceCoherentMemoryAMD = 0x00000010,
    BufferDeviceAddress = 0x00000020,
    MemoryPriorityEXT = 0x00000040,
    Maintenance4KHR = 0x00000080,
    Maintenance5KHR = 0x00000100,
    ExternalMemoryWin32KHR = 0x00000200,
    AllBits = 0x7FFFFFFF
};
using AllocatorCreateFlags = VkBindings::impl_Enum::Flags<AllocatorCreateBits>;

enum class MemoryUsage : std::int32_t {
    Unknown = 0,
    GpuOnly = 1,
    CpuOnly = 2,
    CpuToGpu = 3,
    GpuToCpu = 4,
    CpuCopy = 5,
    GpuLazilyAllocated = 6,
    Auto = 7,
    AutoPreferDevice = 8,
    AutoPreferHost = 9,
    AllBits = 0x7FFFFFFF
};

enum class AllocationCreateBits : std::int32_t {
    DedicatedMemory = 0x00000001,
    NeverAllocate = 0x00000002,
    Mapped = 0x00000004,
    UserDataCopyString = 0x00000020,
    UpperAddress = 0x00000040,
    DontBind = 0x00000080,
    WithinBudget = 0x00000100,
    CanAlias = 0x00000200,
    HostAccessSequentialWrite = 0x00000400,
    HostAccessRandom = 0x00000800,
    HostAccessAllowTransferInstead = 0x00001000,
    StrategyMinMemory = 0x00010000,
    StrategyMinTime = 0x00020000,
    StrategyMinOffset = 0x00040000,
    StrategyBestFit = StrategyMinMemory,
    StrategyFirstFit = StrategyMinTime,
    StrategyMask = StrategyMinMemory | StrategyMinTime | StrategyMinOffset,
    AllBits = 0x7FFFFFFF
};
using AllocationCreateFlags = VkBindings::impl_Enum::Flags<AllocationCreateBits>;

enum class PoolCreateBits : std::int32_t {
    IgnoreBufferImageGranularity = 0x00000002,
    LinearAlgorithm = 0x00000004,
    AlgorithmMask = LinearAlgorithm,
    AllBits = 0x7FFFFFFF
};
using PoolCreateFlags = VkBindings::impl_Enum::Flags<PoolCreateBits>;

enum class DefragmentationBits : std::int32_t {
    AlgorithmFast = 0x1,
    AlgorithmBalanced = 0x2,
    AlgorithmFull = 0x4,
    AlgorithmExtensive = 0x8,
    AlgorithmMask = AlgorithmFast | AlgorithmBalanced | AlgorithmFull | AlgorithmExtensive,
    AllBits = 0x7FFFFFFF
};
using DefragmentationFlags = VkBindings::impl_Enum::Flags<DefragmentationBits>;

enum class DefragmentationMoveOperation : std::int32_t {
    Copy = 0,
    Ignore = 1,
    Destroy = 2,
};

enum class VirtualBlockCreateBits : std::int32_t {
    LinearAlgorithm = 0x00000001,
    AlgorithmMask = LinearAlgorithm,
    AllBits = 0x7FFFFFFF
};
using VirtualBlockCreateFlags = VkBindings::impl_Enum::Flags<VirtualBlockCreateBits>;

enum class VirtualAllocationCreateBits : std::int32_t {
    UpperAddress = static_cast<std::int32_t>(AllocationCreateBits::UpperAddress),
    StrategyMinMemory = static_cast<std::int32_t>(AllocationCreateBits::StrategyMinMemory),
    StrategyMinTime = static_cast<std::int32_t>(AllocationCreateBits::StrategyMinTime),
    StrategyMinOffset = static_cast<std::int32_t>(AllocationCreateBits::StrategyMinOffset),
    StrategyMask = static_cast<std::int32_t>(AllocationCreateBits::StrategyMask),
    AllBits = 0x7FFFFFFF
};
using VirtualAllocationCreateFlags = VkBindings::impl_Enum::Flags<VirtualAllocationCreateBits>;
// NOLINTEND(performance-enum-size)

} // namespace VmaBindings
namespace VkBindings::Reflections::Reflections_impl {
template <> struct IsBits<VmaBindings::AllocatorCreateBits> : std::true_type {};
template <> struct IsBits<VmaBindings::AllocationCreateBits> : std::true_type {};
template <> struct IsBits<VmaBindings::PoolCreateBits> : std::true_type {};
template <> struct IsBits<VmaBindings::DefragmentationBits> : std::true_type {};
template <> struct IsBits<VmaBindings::VirtualBlockCreateBits> : std::true_type {};
template <> struct IsBits<VmaBindings::VirtualAllocationCreateBits> : std::true_type {};

template <> struct IsFlag<VmaBindings::AllocatorCreateFlags> : std::true_type {};
template <> struct IsFlag<VmaBindings::AllocationCreateFlags> : std::true_type {};
template <> struct IsFlag<VmaBindings::PoolCreateFlags> : std::true_type {};
template <> struct IsFlag<VmaBindings::DefragmentationFlags> : std::true_type {};
template <> struct IsFlag<VmaBindings::VirtualBlockCreateFlags> : std::true_type {};
template <> struct IsFlag<VmaBindings::VirtualAllocationCreateFlags> : std::true_type {};

template <> struct BitsToFlag<VmaBindings::AllocatorCreateBits> {
    using t = VmaBindings::AllocatorCreateFlags;
};
template <> struct BitsToFlag<VmaBindings::AllocationCreateBits> {
    using t = VmaBindings::AllocationCreateFlags;
};
template <> struct BitsToFlag<VmaBindings::PoolCreateBits> {
    using t = VmaBindings::PoolCreateFlags;
};
template <> struct BitsToFlag<VmaBindings::DefragmentationBits> {
    using t = VmaBindings::DefragmentationFlags;
};
template <> struct BitsToFlag<VmaBindings::VirtualBlockCreateBits> {
    using t = VmaBindings::VirtualBlockCreateFlags;
};
template <> struct BitsToFlag<VmaBindings::VirtualAllocationCreateBits> {
    using t = VmaBindings::VirtualAllocationCreateFlags;
};

template <> struct FlagToBits<VmaBindings::AllocatorCreateFlags> {
    using t = VmaBindings::AllocatorCreateBits;
};
template <> struct FlagToBits<VmaBindings::AllocationCreateFlags> {
    using t = VmaBindings::AllocationCreateBits;
};
template <> struct FlagToBits<VmaBindings::PoolCreateFlags> {
    using t = VmaBindings::PoolCreateBits;
};
template <> struct FlagToBits<VmaBindings::DefragmentationFlags> {
    using t = VmaBindings::DefragmentationBits;
};
template <> struct FlagToBits<VmaBindings::VirtualBlockCreateFlags> {
    using t = VmaBindings::VirtualBlockCreateBits;
};
template <> struct FlagToBits<VmaBindings::VirtualAllocationCreateFlags> {
    using t = VmaBindings::VirtualAllocationCreateBits;
};

template <> struct IsEnum<VmaBindings::MemoryUsage> : std::true_type {};
template <> struct IsEnum<VmaBindings::DefragmentationMoveOperation> : std::true_type {};
} // namespace VkBindings::Reflections::Reflections_impl
namespace VmaBindings {

namespace impl_Objects {
template <typename BaseObject> struct Unique : public BaseObject {
    using object_type = BaseObject;

  protected:
    Unique(object_type &&obj) noexcept;

    friend VkBindings::impl_Objects::Creator;

  public:
    Unique();

    Unique(const Unique &other) noexcept = delete;
    Unique(Unique &&other) noexcept;

    auto operator=(const Unique &other) noexcept -> Unique & = delete;
    auto operator=(Unique &&other) noexcept -> Unique &;

    ~Unique() noexcept;

    void cleanup() noexcept;

    operator const object_type &() const noexcept;
    [[nodiscard]] auto getObject() const noexcept -> const object_type &;
};

template <typename Handle_T>
using Object = VkBindings::impl_Objects::ObjectWithoutFunctions<Handle_T>;

template <typename Handle_T, typename Owner_Handle_T> struct ObjectOwner {
    using handle_type = Handle_T;
    using owner_handle_type = Owner_Handle_T;

  private:
    handle_type handle = VK_BINDINGS_NULL_HANDLE;
    owner_handle_type ownerHandle = VK_BINDINGS_NULL_HANDLE;

  protected:
    [[nodiscard]] auto getOwnerHandle() const -> const owner_handle_type;

  public:
    [[nodiscard]] auto getHandle() const -> const handle_type &;

  protected:
    friend VkBindings::impl_Objects::Creator;

    ObjectOwner(const handle_type &handle, const owner_handle_type &ownerHandle);

  public:
    ObjectOwner();

    ObjectOwner(const ObjectOwner &other) noexcept;
    ObjectOwner(ObjectOwner &&other) noexcept;

    auto operator=(const ObjectOwner &other) noexcept -> ObjectOwner & = default;
    auto operator=(ObjectOwner &&other) noexcept -> ObjectOwner &;

    ~ObjectOwner() noexcept;

    operator handle_type() const noexcept;
    explicit operator bool() const noexcept;
};
} // namespace impl_Objects

namespace Handle {
VK_BINDINGS_DEFINE_HANDLE(Allocator)
VK_BINDINGS_DEFINE_HANDLE(Pool)
VK_BINDINGS_DEFINE_HANDLE(Allocation)
VK_BINDINGS_DEFINE_HANDLE(DefragmentationContext)
VK_BINDINGS_DEFINE_NON_DISPATCHABLE_HANDLE(VirtualAllocation)
VK_BINDINGS_DEFINE_HANDLE(VirtualBlock)
} // namespace Handle

struct Allocator;
using UniqueAllocator = impl_Objects::Unique<Allocator>;
struct Pool;
using UniquePool = impl_Objects::Unique<Pool>;
struct Allocation;
using UniqueAllocation = impl_Objects::Unique<Allocation>;
struct DefragmentationContext;
struct VirtualAllocation;
using UniqueVirtualAllocation = impl_Objects::Unique<VirtualAllocation>;
struct VirtualBlock;
using UniqueVirtualBlock = impl_Objects::Unique<VirtualBlock>;

} // namespace VmaBindings

namespace VkBindings::Reflections::Reflections_impl {
template <> struct IsObject<VmaBindings::Allocator> : std::true_type {};
template <> struct IsObject<VmaBindings::Pool> : std::true_type {};
template <> struct IsObject<VmaBindings::Allocation> : std::true_type {};
template <> struct IsObject<VmaBindings::DefragmentationContext> : std::true_type {};
template <> struct IsObject<VmaBindings::VirtualAllocation> : std::true_type {};
template <> struct IsObject<VmaBindings::VirtualBlock> : std::true_type {};

template <> struct IsUnique<VmaBindings::UniqueAllocator> : std::true_type {};
template <> struct IsUnique<VmaBindings::UniquePool> : std::true_type {};
template <> struct IsUnique<VmaBindings::UniqueAllocation> : std::true_type {};
template <> struct IsUnique<VmaBindings::UniqueVirtualAllocation> : std::true_type {};
template <> struct IsUnique<VmaBindings::UniqueVirtualBlock> : std::true_type {};

template <> struct HandleToObject<VmaBindings::Handle::Allocator> {
    using t = VmaBindings::Allocator;
};
template <> struct HandleToObject<VmaBindings::Handle::Pool> {
    using t = VmaBindings::Pool;
};
template <> struct HandleToObject<VmaBindings::Handle::Allocation> {
    using t = VmaBindings::Allocation;
};
template <> struct HandleToObject<VmaBindings::Handle::DefragmentationContext> {
    using t = VmaBindings::DefragmentationContext;
};
template <> struct HandleToObject<VmaBindings::Handle::VirtualAllocation> {
    using t = VmaBindings::VirtualAllocation;
};
template <> struct HandleToObject<VmaBindings::Handle::VirtualBlock> {
    using t = VmaBindings::VirtualBlock;
};

template <> struct ObjectToHandle<VmaBindings::Allocator> {
    using t = VmaBindings::Handle::Allocator;
};
template <> struct ObjectToHandle<VmaBindings::Pool> {
    using t = VmaBindings::Handle::Pool;
};
template <> struct ObjectToHandle<VmaBindings::Allocation> {
    using t = VmaBindings::Handle::Allocation;
};
template <> struct ObjectToHandle<VmaBindings::DefragmentationContext> {
    using t = VmaBindings::Handle::DefragmentationContext;
};
template <> struct ObjectToHandle<VmaBindings::VirtualAllocation> {
    using t = VmaBindings::Handle::VirtualAllocation;
};
template <> struct ObjectToHandle<VmaBindings::VirtualBlock> {
    using t = VmaBindings::Handle::VirtualBlock;
};

} // namespace VkBindings::Reflections::Reflections_impl

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
    VkBindings::impl_Struct::AssignableHandle<VkBindings::Instance> instance;
    VkBindings::impl_Struct::AssignableHandle<VkBindings::PhysicalDevice> physicalDevice;
    VkBindings::impl_Struct::AssignableHandle<VkBindings::Device> device;
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
    VkBindings::impl_Struct::AssignableHandle<VkBindings::DeviceMemory> deviceMemory;
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

struct DefragmentationMove {
    DefragmentationMoveOperation operation = {};
    VkBindings::impl_Struct::AssignableHandle<Allocation> srcAllocation;
    VkBindings::impl_Struct::AssignableHandle<Allocation> dstTmpAllocation;
};

struct DefragmentationPassMoveInfo {
    uint32_t moveCount = 0;
    DefragmentationMove *pMoves = nullptr;
    auto moves() -> VkBindings::impl_Struct::VecView<uint32_t, DefragmentationMove>;
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

class UniqueMemoryPages {
    Allocator *allocator{};
    std::vector<Allocation> allocations;
    std::vector<AllocationInfo> infos;

  public:
    UniqueMemoryPages(Allocator *allocator, std::vector<Allocation> &&allocations,
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

    auto getPhysicalDeviceProperties() -> const VkBindings::PhysicalDeviceProperties &;

    auto getMemoryProperties() -> const VkBindings::PhysicalDeviceMemoryProperties &;

    auto getMemoryTypeProperties(uint32_t memoryTypeIndex) -> VkBindings::MemoryPropertyFlags;

    void setCurrentFrameIndex(uint32_t frameIndex);

    auto calculateStatistics() -> TotalStatistics;

    auto getHeapBudgets() -> Budget;

    auto findMemoryTypeIndex(uint32_t memoryTypeBits,
                             const AllocationCreateInfo &allocationCreateInfo)
        -> std::expected<uint32_t, VkBindings::Result>;

    auto findMemoryTypeIndexForBufferInfo(const VkBindings::BufferCreateInfo &bufferCreateInfo,
                                          const AllocationCreateInfo &allocationCreateInfo)
        -> std::expected<uint32_t, VkBindings::Result>;

    auto findMemoryTypeIndexForImageInfo(const VkBindings::ImageCreateInfo &imageCreateInfo,
                                         const AllocationCreateInfo &allocationCreateInfo)
        -> std::expected<uint32_t, VkBindings::Result>;

    auto createPool(const PoolCreateInfo &createInfo)
        -> std::expected<UniquePool, VkBindings::Result>;

    auto allocateMemory(const VkBindings::MemoryRequirements &memoryRequirements,
                        const AllocationCreateInfo &createInfo)
        -> std::expected<std::tuple<UniqueAllocation, AllocationInfo>, VkBindings::Result>;

    auto allocateDedicatedMemory(const VkBindings::MemoryRequirements &memoryRequirements,
                                 const AllocationCreateInfo &createInfo, void *pMemoryAllocateNext)
        -> std::expected<std::tuple<UniqueAllocation, AllocationInfo>, VkBindings::Result>;

    auto allocateMemoryPages(const VkBindings::MemoryRequirements &memoryRequirements,
                             const AllocationCreateInfo &createInfo, size_t allocationCount)
        -> std::expected<UniqueMemoryPages, VkBindings::Result>;

    auto allocateMemoryForBuffer(const VkBindings::Buffer &buffer,
                                 const AllocationCreateInfo &createInfo)
        -> std::expected<std::tuple<UniqueAllocation, AllocationInfo>, VkBindings::Result>;

    auto allocateMemoryForImage(const VkBindings::Image &image,
                                const AllocationCreateInfo &createInfo)
        -> std::expected<std::tuple<UniqueAllocation, AllocationInfo>, VkBindings::Result>;

    auto flushAllocation(const Allocation &allocation, VkBindings::DeviceSize offset,
                         VkBindings::DeviceSize size) -> VkBindings::Result;

    auto invalidateAllocation(const Allocation &allocation, VkBindings::DeviceSize offset,
                              VkBindings::DeviceSize size) -> VkBindings::Result;

    auto
    flushAllocations(const VkBindings::impl_Struct::ArrayProxy<Allocation> &allocations,
                     const VkBindings::impl_Struct::ArrayProxy<VkBindings::DeviceSize> &offsets,
                     const VkBindings::impl_Struct::ArrayProxy<VkBindings::DeviceSize> &sizes)
        -> VkBindings::Result;

    auto invalidateAllocations(
        const VkBindings::impl_Struct::ArrayProxy<Allocation> &allocations,
        const VkBindings::impl_Struct::ArrayProxy<VkBindings::DeviceSize> &offsets,
        const VkBindings::impl_Struct::ArrayProxy<VkBindings::DeviceSize> &sizes)
        -> VkBindings::Result;

    auto copyMemoryToAllocation(const void *pSrcHostPointer, const Allocation &dstAllocation,
                                VkBindings::DeviceSize dstAllocationLocalOffset,
                                VkBindings::DeviceSize size) -> VkBindings::Result;

    auto copyAllocationToMemory(const Allocation &srcAllocation,
                                VkBindings::DeviceSize srcAllocationLocalOffset,
                                void *pDstHostPointer, VkBindings::DeviceSize size)
        -> VkBindings::Result;

    auto checkCorruption(uint32_t memoryTypeBits) -> VkBindings::Result;

    auto createBuffer(const VkBindings::BufferCreateInfo &bufferCreateInfo,
                      const AllocationCreateInfo &allocationCreateInfo)
        -> std::expected<std::tuple<VkBindings::UniqueBuffer, UniqueAllocation, AllocationInfo>,
                         VkBindings::Result>;

    auto createBufferWithAlignment(const VkBindings::BufferCreateInfo &bufferCreateInfo,
                                   const AllocationCreateInfo &allocationCreateInfo,
                                   VkBindings::DeviceSize minAlignment)
        -> std::expected<std::tuple<VkBindings::UniqueBuffer, UniqueAllocation, AllocationInfo>,
                         VkBindings::Result>;

    auto createDedicatedBuffer(const VkBindings::BufferCreateInfo &bufferCreateInfo,
                               const AllocationCreateInfo &allocationCreateInfo,
                               void *pMemoryAllocateNext)
        -> std::expected<std::tuple<VkBindings::UniqueBuffer, UniqueAllocation, AllocationInfo>,
                         VkBindings::Result>;

    auto createAliasingBuffer(const Allocation &allocation,
                              const VkBindings::BufferCreateInfo &bufferCreateInfo)
        -> std::expected<VkBindings::UniqueBuffer, VkBindings::Result>;

    auto createAliasingBuffer2(const Allocation &allocation,
                               VkBindings::DeviceSize allocationLocalOffset,
                               const VkBindings::BufferCreateInfo &bufferCreateInfo)
        -> std::expected<VkBindings::UniqueBuffer, VkBindings::Result>;

    auto createImage(const VkBindings::ImageCreateInfo &imageCreateInfo,
                     const AllocationCreateInfo &allocationCreateInfo)
        -> std::expected<std::tuple<VkBindings::UniqueImage, UniqueAllocation, AllocationInfo>,
                         VkBindings::Result>;

    auto createDedicatedImage(const VkBindings::ImageCreateInfo &imageCreateInfo,
                              const AllocationCreateInfo &allocationCreateInfo,
                              void *pMemoryAllocateNext)
        -> std::expected<std::tuple<VkBindings::UniqueImage, UniqueAllocation, AllocationInfo>,
                         VkBindings::Result>;

    auto createAliasingImage(const Allocation &allocation,
                             const VkBindings::ImageCreateInfo &imageCreateInfo)
        -> std::expected<VkBindings::UniqueImage, VkBindings::Result>;

    auto createAliasingImage2(const Allocation &allocation,
                              VkBindings::DeviceSize allocationLocalOffset,
                              const VkBindings::ImageCreateInfo &imageCreateInfo)
        -> std::expected<VkBindings::UniqueImage, VkBindings::Result>;

    auto buildStatsString(VkBindings::Bool32 detailedMap) -> std::string;

    auto beginDefragmentation(const DefragmentationInfo &info)
        -> std::expected<DefragmentationContext, VkBindings::Result>;

    auto endDefragmentation(const DefragmentationContext &context) -> void;

    auto endDefragmentationGetStats(const DefragmentationContext &context) -> DefragmentationStats;
};

struct DefragmentationContext
    : public impl_Objects::ObjectOwner<Handle::DefragmentationContext, Handle::Allocator> {
    using ObjectOwner::ObjectOwner;
    DefragmentationContext() = default;

    auto beginPass() -> std::expected<DefragmentationPassMoveInfo, VkBindings::Result>;

    auto endPass(DefragmentationPassMoveInfo &passInfo) -> VkBindings::Result;
};

struct Pool : public impl_Objects::ObjectOwner<Handle::Pool, Handle::Allocator> {
    using ObjectOwner::ObjectOwner;
    Pool() = default;

    auto getStatistics() -> Statistics;

    auto calculateStatistics() -> DetailedStatistics;

    auto checkCorruption() -> VkBindings::Result;

    auto getName() -> std::string_view;

    void setName(VkBindings::impl_Struct::InOutString name);
};

struct VirtualAllocation
    : public impl_Objects::ObjectOwner<Handle::VirtualAllocation, Handle::VirtualBlock> {
    using ObjectOwner::ObjectOwner;
    VirtualAllocation() = default;

    auto getAllocationInfo() -> VirtualAllocationInfo;

    void setUserData(void *pUserData);
};

struct VirtualBlock : public impl_Objects::Object<Handle::VirtualBlock> {
    using impl_Objects::Object<Handle::VirtualBlock>::Object;
    VirtualBlock() = default;

    auto isEmpty() -> VkBindings::Bool32;

    auto virtualAllocate(const VirtualAllocationCreateInfo &createInfo)
        -> std::expected<std::tuple<UniqueVirtualAllocation, VkBindings::DeviceSize>,
                         VkBindings::Result>;

    void clear();

    auto getStatistics() -> Statistics;

    auto calculateStatistics() -> DetailedStatistics;

    auto buildStatsString(VkBindings::Bool32 detailedMap) -> std::string;
};

// Device is needed to snatch the dispatcher from somewhere, as the AllocatorCreateInfo does only
// contain the handle
auto createAllocator(const VkBindings::Device &device, const AllocatorCreateInfo &createInfo)
    -> std::expected<UniqueAllocator, VkBindings::Result>;

auto createVirtualBlock(const VirtualBlockCreateInfo &createInfo)
    -> std::expected<UniqueVirtualBlock, VkBindings::Result>;
} // namespace VmaBindings
