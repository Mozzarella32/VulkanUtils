#pragma once
#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Defines.hpp>
#include <VkBindings/Handles.hpp>
#include <VkBindings/Reflection/BitsToFlag.hpp>
#include <VkBindings/Reflection/FlagToBits.hpp>
#include <VkBindings/Reflection/HandleToObject.hpp>
#include <VkBindings/Reflection/IsBits.hpp>
#include <VkBindings/Reflection/IsEnum.hpp>
#include <VkBindings/Reflection/IsFlag.hpp>
#include <VkBindings/Reflection/IsObject.hpp>
#include <VkBindings/Reflection/IsUnique.hpp>
#include <VkBindings/Reflection/ObjectToHandle.hpp>
#include <VkBindings/private/Creator.hpp>
#include <VkBindings/private/FlagsInterface.hpp>
#include <VkBindings/private/ObjectTemplatesIntreface.hpp>
#include <VkBindings/private/vk_platform.h>

#include <cstdint>
#include <type_traits>

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

struct DeviceMemoryCallbacks;

struct VulkanFunctions;

struct AllocatorCreateInfo;
struct AllocatorInfo;

struct Statistics;
struct DetailedStatistics;
struct TotalStatistics;
struct Budget;

struct AllocationCreateInfo;

struct PoolCreateInfo;

struct AllocationInfo;
struct AllocationInfo2;

namespace PFN {
using CheckDefragmentationBreakFunction = auto(VKAPI_PTR *)(void *) -> VkBindings::Bool32;
} // namespace PFN

struct DefragmentationInfo;
struct DefragmentationMove;
struct DefragmentationPassMoveInfo;
struct DefragmentationStats;

struct VirtualBlockCreateInfo;
struct VirtualAllocationCreateInfo;
struct VirtualAllocationInfo;

} // namespace VmaBindings
