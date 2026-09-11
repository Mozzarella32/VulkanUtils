#include "VmaBindings/Vma.hpp"

#include <VkBindings/Loader.hpp>
#include <VkBindings/private/FunctionTables.hpp>
#include <vulkan/vulkan_core.h>

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
// NOLINTEND(cppcoreguidelines-macro-usage)

#include "vk_mem_alloc.h"

#include <VkBindings/BaseTypes.hpp>
#include <VkBindings/Defines.hpp>
#include <VkBindings/Enums.hpp>
#include <VkBindings/Flags.hpp>
#include <VkBindings/Handles.hpp>
#include <VkBindings/Objects.hpp>
#include <VkBindings/ObjectsForward.hpp>
#include <VkBindings/Structs.hpp>
#include <VkBindings/private/Creator.hpp>
#include <VkBindings/private/FlagsInterface.hpp>
#include <VkBindings/private/ObjectTemplatesIntreface.hpp>
#include <VkBindings/private/StructTemplates/ArrayProxyInterface.hpp>
#include <VkBindings/private/StructTemplates/AssignableHandleInterface.hpp>
#include <VkBindings/private/StructTemplates/InOutString.hpp>
#include <VkBindings/private/StructTemplates/VecViewInterface.hpp>

// NOLINTBEGIN(misc-include-cleaner)
// Needed for getting implmenetations
#include <VkBindings/private/Flags.hpp>
#include <VkBindings/private/ObjectTemplates.hpp>
#include <VkBindings/private/StructTemplates/ArrayProxy.hpp>
#include <VkBindings/private/StructTemplates/AssignableHandle.hpp>
#include <VkBindings/private/StructTemplates/VecView.hpp>

#include <compare>
// NOLINTEND(misc-include-cleaner)

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace VkBindings {
template struct VkBindings::impl_Enum::Flags<VmaBindings::AllocatorCreateBits>;
template auto operator| <VmaBindings::AllocatorCreateBits>(VmaBindings::AllocatorCreateBits,
                                                           VmaBindings::AllocatorCreateBits)
    -> VmaBindings::AllocatorCreateFlags;
template auto operator& <VmaBindings::AllocatorCreateBits>(VmaBindings::AllocatorCreateBits,
                                                           VmaBindings::AllocatorCreateBits)
    -> VmaBindings::AllocatorCreateFlags;
template auto operator^ <VmaBindings::AllocatorCreateBits>(VmaBindings::AllocatorCreateBits,
                                                           VmaBindings::AllocatorCreateBits)
    -> VmaBindings::AllocatorCreateFlags;
template auto operator| <VmaBindings::AllocatorCreateBits>(VmaBindings::AllocatorCreateBits,
                                                           VmaBindings::AllocatorCreateFlags)
    -> VmaBindings::AllocatorCreateFlags;
template auto operator& <VmaBindings::AllocatorCreateBits>(VmaBindings::AllocatorCreateBits,
                                                           VmaBindings::AllocatorCreateFlags)
    -> VmaBindings::AllocatorCreateFlags;
template auto operator^ <VmaBindings::AllocatorCreateBits>(VmaBindings::AllocatorCreateBits,
                                                           VmaBindings::AllocatorCreateFlags)
    -> VmaBindings::AllocatorCreateFlags;
template auto operator| <VmaBindings::AllocatorCreateBits>(VmaBindings::AllocatorCreateFlags,
                                                           VmaBindings::AllocatorCreateBits)
    -> VmaBindings::AllocatorCreateFlags;
template auto operator& <VmaBindings::AllocatorCreateBits>(VmaBindings::AllocatorCreateFlags,
                                                           VmaBindings::AllocatorCreateBits)
    -> VmaBindings::AllocatorCreateFlags;
template auto operator^ <VmaBindings::AllocatorCreateBits>(VmaBindings::AllocatorCreateFlags,
                                                           VmaBindings::AllocatorCreateBits)
    -> VmaBindings::AllocatorCreateFlags;

template struct VkBindings::impl_Enum::Flags<VmaBindings::AllocationCreateBits>;
template auto operator| <VmaBindings::AllocationCreateBits>(VmaBindings::AllocationCreateBits,
                                                            VmaBindings::AllocationCreateBits)
    -> VmaBindings::AllocationCreateFlags;
template auto operator& <VmaBindings::AllocationCreateBits>(VmaBindings::AllocationCreateBits,
                                                            VmaBindings::AllocationCreateBits)
    -> VmaBindings::AllocationCreateFlags;
template auto operator^ <VmaBindings::AllocationCreateBits>(VmaBindings::AllocationCreateBits,
                                                            VmaBindings::AllocationCreateBits)
    -> VmaBindings::AllocationCreateFlags;
template auto operator| <VmaBindings::AllocationCreateBits>(VmaBindings::AllocationCreateBits,
                                                            VmaBindings::AllocationCreateFlags)
    -> VmaBindings::AllocationCreateFlags;
template auto operator& <VmaBindings::AllocationCreateBits>(VmaBindings::AllocationCreateBits,
                                                            VmaBindings::AllocationCreateFlags)
    -> VmaBindings::AllocationCreateFlags;
template auto operator^ <VmaBindings::AllocationCreateBits>(VmaBindings::AllocationCreateBits,
                                                            VmaBindings::AllocationCreateFlags)
    -> VmaBindings::AllocationCreateFlags;
template auto operator| <VmaBindings::AllocationCreateBits>(VmaBindings::AllocationCreateFlags,
                                                            VmaBindings::AllocationCreateBits)
    -> VmaBindings::AllocationCreateFlags;
template auto operator& <VmaBindings::AllocationCreateBits>(VmaBindings::AllocationCreateFlags,
                                                            VmaBindings::AllocationCreateBits)
    -> VmaBindings::AllocationCreateFlags;
template auto operator^ <VmaBindings::AllocationCreateBits>(VmaBindings::AllocationCreateFlags,
                                                            VmaBindings::AllocationCreateBits)
    -> VmaBindings::AllocationCreateFlags;

template struct VkBindings::impl_Enum::Flags<VmaBindings::PoolCreateBits>;
template auto operator|
    <VmaBindings::PoolCreateBits>(VmaBindings::PoolCreateBits, VmaBindings::PoolCreateBits)
        -> VmaBindings::PoolCreateFlags;
template auto operator&
    <VmaBindings::PoolCreateBits>(VmaBindings::PoolCreateBits, VmaBindings::PoolCreateBits)
        -> VmaBindings::PoolCreateFlags;
template auto operator^
    <VmaBindings::PoolCreateBits>(VmaBindings::PoolCreateBits, VmaBindings::PoolCreateBits)
        -> VmaBindings::PoolCreateFlags;
template auto operator|
    <VmaBindings::PoolCreateBits>(VmaBindings::PoolCreateBits, VmaBindings::PoolCreateFlags)
        -> VmaBindings::PoolCreateFlags;
template auto operator&
    <VmaBindings::PoolCreateBits>(VmaBindings::PoolCreateBits, VmaBindings::PoolCreateFlags)
        -> VmaBindings::PoolCreateFlags;
template auto operator^
    <VmaBindings::PoolCreateBits>(VmaBindings::PoolCreateBits, VmaBindings::PoolCreateFlags)
        -> VmaBindings::PoolCreateFlags;
template auto operator|
    <VmaBindings::PoolCreateBits>(VmaBindings::PoolCreateFlags, VmaBindings::PoolCreateBits)
        -> VmaBindings::PoolCreateFlags;
template auto operator&
    <VmaBindings::PoolCreateBits>(VmaBindings::PoolCreateFlags, VmaBindings::PoolCreateBits)
        -> VmaBindings::PoolCreateFlags;
template auto operator^
    <VmaBindings::PoolCreateBits>(VmaBindings::PoolCreateFlags, VmaBindings::PoolCreateBits)
        -> VmaBindings::PoolCreateFlags;

template struct VkBindings::impl_Enum::Flags<VmaBindings::DefragmentationBits>;
template auto operator| <VmaBindings::DefragmentationBits>(VmaBindings::DefragmentationBits,
                                                           VmaBindings::DefragmentationBits)
    -> VmaBindings::DefragmentationFlags;
template auto operator& <VmaBindings::DefragmentationBits>(VmaBindings::DefragmentationBits,
                                                           VmaBindings::DefragmentationBits)
    -> VmaBindings::DefragmentationFlags;
template auto operator^ <VmaBindings::DefragmentationBits>(VmaBindings::DefragmentationBits,
                                                           VmaBindings::DefragmentationBits)
    -> VmaBindings::DefragmentationFlags;
template auto operator| <VmaBindings::DefragmentationBits>(VmaBindings::DefragmentationBits,
                                                           VmaBindings::DefragmentationFlags)
    -> VmaBindings::DefragmentationFlags;
template auto operator& <VmaBindings::DefragmentationBits>(VmaBindings::DefragmentationBits,
                                                           VmaBindings::DefragmentationFlags)
    -> VmaBindings::DefragmentationFlags;
template auto operator^ <VmaBindings::DefragmentationBits>(VmaBindings::DefragmentationBits,
                                                           VmaBindings::DefragmentationFlags)
    -> VmaBindings::DefragmentationFlags;
template auto operator| <VmaBindings::DefragmentationBits>(VmaBindings::DefragmentationFlags,
                                                           VmaBindings::DefragmentationBits)
    -> VmaBindings::DefragmentationFlags;
template auto operator& <VmaBindings::DefragmentationBits>(VmaBindings::DefragmentationFlags,
                                                           VmaBindings::DefragmentationBits)
    -> VmaBindings::DefragmentationFlags;
template auto operator^ <VmaBindings::DefragmentationBits>(VmaBindings::DefragmentationFlags,
                                                           VmaBindings::DefragmentationBits)
    -> VmaBindings::DefragmentationFlags;

template struct VkBindings::impl_Enum::Flags<VmaBindings::VirtualBlockCreateBits>;
template auto operator| <VmaBindings::VirtualBlockCreateBits>(VmaBindings::VirtualBlockCreateBits,
                                                              VmaBindings::VirtualBlockCreateBits)
    -> VmaBindings::VirtualBlockCreateFlags;
template auto operator& <VmaBindings::VirtualBlockCreateBits>(VmaBindings::VirtualBlockCreateBits,
                                                              VmaBindings::VirtualBlockCreateBits)
    -> VmaBindings::VirtualBlockCreateFlags;
template auto operator^ <VmaBindings::VirtualBlockCreateBits>(VmaBindings::VirtualBlockCreateBits,
                                                              VmaBindings::VirtualBlockCreateBits)
    -> VmaBindings::VirtualBlockCreateFlags;
template auto operator| <VmaBindings::VirtualBlockCreateBits>(VmaBindings::VirtualBlockCreateBits,
                                                              VmaBindings::VirtualBlockCreateFlags)
    -> VmaBindings::VirtualBlockCreateFlags;
template auto operator& <VmaBindings::VirtualBlockCreateBits>(VmaBindings::VirtualBlockCreateBits,
                                                              VmaBindings::VirtualBlockCreateFlags)
    -> VmaBindings::VirtualBlockCreateFlags;
template auto operator^ <VmaBindings::VirtualBlockCreateBits>(VmaBindings::VirtualBlockCreateBits,
                                                              VmaBindings::VirtualBlockCreateFlags)
    -> VmaBindings::VirtualBlockCreateFlags;
template auto operator| <VmaBindings::VirtualBlockCreateBits>(VmaBindings::VirtualBlockCreateFlags,
                                                              VmaBindings::VirtualBlockCreateBits)
    -> VmaBindings::VirtualBlockCreateFlags;
template auto operator& <VmaBindings::VirtualBlockCreateBits>(VmaBindings::VirtualBlockCreateFlags,
                                                              VmaBindings::VirtualBlockCreateBits)
    -> VmaBindings::VirtualBlockCreateFlags;
template auto operator^ <VmaBindings::VirtualBlockCreateBits>(VmaBindings::VirtualBlockCreateFlags,
                                                              VmaBindings::VirtualBlockCreateBits)
    -> VmaBindings::VirtualBlockCreateFlags;

template struct VkBindings::impl_Enum::Flags<VmaBindings::VirtualAllocationCreateBits>;
template auto operator|
    <VmaBindings::VirtualAllocationCreateBits>(VmaBindings::VirtualAllocationCreateBits,
                                               VmaBindings::VirtualAllocationCreateBits)
        -> VmaBindings::VirtualAllocationCreateFlags;
template auto operator&
    <VmaBindings::VirtualAllocationCreateBits>(VmaBindings::VirtualAllocationCreateBits,
                                               VmaBindings::VirtualAllocationCreateBits)
        -> VmaBindings::VirtualAllocationCreateFlags;
template auto operator^
    <VmaBindings::VirtualAllocationCreateBits>(VmaBindings::VirtualAllocationCreateBits,
                                               VmaBindings::VirtualAllocationCreateBits)
        -> VmaBindings::VirtualAllocationCreateFlags;
template auto operator|
    <VmaBindings::VirtualAllocationCreateBits>(VmaBindings::VirtualAllocationCreateBits,
                                               VmaBindings::VirtualAllocationCreateFlags)
        -> VmaBindings::VirtualAllocationCreateFlags;
template auto operator&
    <VmaBindings::VirtualAllocationCreateBits>(VmaBindings::VirtualAllocationCreateBits,
                                               VmaBindings::VirtualAllocationCreateFlags)
        -> VmaBindings::VirtualAllocationCreateFlags;
template auto operator^
    <VmaBindings::VirtualAllocationCreateBits>(VmaBindings::VirtualAllocationCreateBits,
                                               VmaBindings::VirtualAllocationCreateFlags)
        -> VmaBindings::VirtualAllocationCreateFlags;
template auto operator|
    <VmaBindings::VirtualAllocationCreateBits>(VmaBindings::VirtualAllocationCreateFlags,
                                               VmaBindings::VirtualAllocationCreateBits)
        -> VmaBindings::VirtualAllocationCreateFlags;
template auto operator&
    <VmaBindings::VirtualAllocationCreateBits>(VmaBindings::VirtualAllocationCreateFlags,
                                               VmaBindings::VirtualAllocationCreateBits)
        -> VmaBindings::VirtualAllocationCreateFlags;
template auto operator^
    <VmaBindings::VirtualAllocationCreateBits>(VmaBindings::VirtualAllocationCreateFlags,
                                               VmaBindings::VirtualAllocationCreateBits)
        -> VmaBindings::VirtualAllocationCreateFlags;

} // namespace VkBindings
namespace VmaBindings::impl_Objects {
template <typename BaseObject> Unique<BaseObject>::Unique() = default;

template <typename BaseObject>
Unique<BaseObject>::Unique(object_type &&obj) noexcept : BaseObject(std::move(obj)) {}

template <typename BaseObject>
Unique<BaseObject>::Unique(Unique &&other) noexcept : BaseObject(std::move(other)) {}

template <typename BaseObject>
auto Unique<BaseObject>::operator=(Unique &&other) noexcept -> Unique<BaseObject> & {
    if (this != &other) {
        cleanup();
        BaseObject::operator=(std::move(other));
    }
    return *this;
}

template <typename BaseObject> Unique<BaseObject>::~Unique() noexcept { cleanup(); }

template <typename BaseObject> Unique<BaseObject>::operator const object_type &() const noexcept {
    return static_cast<const object_type &>(*this);
}

template <typename BaseObject>
auto Unique<BaseObject>::getObject() const noexcept -> const object_type & {
    return static_cast<const object_type &>(*this);
}
template <typename Handle_T, typename Owner_Handle_T>
auto ObjectOwner<Handle_T, Owner_Handle_T>::getHandle() const -> const handle_type & {
    return handle;
}

template <typename Handle_T, typename Owner_Handle_T>
ObjectOwner<Handle_T, Owner_Handle_T>::ObjectOwner(const handle_type &handle,
                                                   const owner_handle_type &ownerHandle)
    : handle(handle), ownerHandle(ownerHandle) {}

template <typename Handle_T, typename Owner_Handle_T>
ObjectOwner<Handle_T, Owner_Handle_T>::ObjectOwner() = default;

template <typename Handle_T, typename Owner_Handle_T>
ObjectOwner<Handle_T, Owner_Handle_T>::ObjectOwner(const ObjectOwner &other) noexcept
    : handle(other.handle), ownerHandle(other.ownerHandle) {}

template <typename Handle_T, typename Owner_Handle_T>
ObjectOwner<Handle_T, Owner_Handle_T>::ObjectOwner(ObjectOwner &&other) noexcept
    : handle(std::exchange(other.handle, VK_BINDINGS_NULL_HANDLE)),
      ownerHandle(std::exchange(other.ownerHandle, VK_BINDINGS_NULL_HANDLE)) {}

template <typename Handle_T, typename Owner_Handle_T>
auto ObjectOwner<Handle_T, Owner_Handle_T>::operator=(ObjectOwner &&other) noexcept
    -> ObjectOwner<Handle_T, Owner_Handle_T> & {
    handle = std::exchange(other.handle, VK_BINDINGS_NULL_HANDLE);
    ownerHandle = std::exchange(other.ownerHandle, nullptr);
    return *this;
}

template <typename Handle_T, typename Owner_Handle_T>
ObjectOwner<Handle_T, Owner_Handle_T>::~ObjectOwner() noexcept = default;

template <typename Handle_T, typename Owner_Handle_T>
ObjectOwner<Handle_T, Owner_Handle_T>::operator handle_type() const noexcept {
    return handle;
}

template <typename Handle_T, typename Owner_Handle_T>
ObjectOwner<Handle_T, Owner_Handle_T>::operator bool() const noexcept {
    return handle != VK_BINDINGS_NULL_HANDLE;
}

} // namespace VmaBindings::impl_Objects

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
namespace VmaBindings::impl_Objects {
template <> void impl_Objects::Unique<Allocator>::cleanup() noexcept {
    if (getHandle() == VK_BINDINGS_NULL_HANDLE) {
        return;
    }
    vmaDestroyAllocator(reinterpret_cast<VmaAllocator>(getHandle()));
    *static_cast<object_type *>(this) = {};
};
template <> void impl_Objects::Unique<Pool>::cleanup() noexcept {
    if (getHandle() == VK_BINDINGS_NULL_HANDLE) {
        return;
    }
    vmaDestroyPool(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                   reinterpret_cast<VmaPool>(getHandle()));
    *static_cast<object_type *>(this) = {};
};
template <> void impl_Objects::Unique<Allocation>::cleanup() noexcept {
    if (getHandle() == VK_BINDINGS_NULL_HANDLE) {
        return;
    }
    vmaFreeMemory(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                  reinterpret_cast<VmaAllocation>(getHandle()));
    *static_cast<object_type *>(this) = {};
};
template <> void impl_Objects::Unique<VirtualAllocation>::cleanup() noexcept {
    if (getHandle() == VK_BINDINGS_NULL_HANDLE) {
        return;
    }
    vmaVirtualFree(reinterpret_cast<VmaVirtualBlock>(getOwnerHandle()),
                   reinterpret_cast<VmaVirtualAllocation>(getHandle()));
    *static_cast<object_type *>(this) = {};
};
template <> void impl_Objects::Unique<VirtualBlock>::cleanup() noexcept {
    if (getHandle() == VK_BINDINGS_NULL_HANDLE) {
        return;
    }
    vmaDestroyVirtualBlock(reinterpret_cast<VmaVirtualBlock>(getHandle()));
    *static_cast<object_type *>(this) = {};
};
} // namespace VmaBindings::impl_Objects
namespace VkBindings::impl_Struct {
template struct VkBindings::impl_Struct::AssignableHandle<VmaBindings::Allocation>;
template struct VkBindings::impl_Struct::AssignableHandle<VmaBindings::Pool>;
template struct VkBindings::impl_Struct::VecView<uint32_t, VmaBindings::DefragmentationMove>;
template struct VkBindings::impl_Struct::ArrayProxy<VmaBindings::Allocation>;
template struct VkBindings::impl_Struct::ArrayProxy<VkBindings::DeviceSize>;
} // namespace VkBindings::impl_Struct
namespace VkBindings::impl_Objects {
template struct VkBindings::impl_Objects::ObjectWithoutFunctions<VmaBindings::Handle::Allocator>;
template struct VkBindings::impl_Objects::ObjectWithoutFunctions<
    VmaBindings::Handle::VirtualAllocation>;
template struct VkBindings::impl_Objects::ObjectWithoutFunctions<VmaBindings::Handle::VirtualBlock>;
} // namespace VkBindings::impl_Objects
namespace VmaBindings::impl_Objects {
template struct impl_Objects::Unique<Allocator>;
template struct impl_Objects::ObjectOwner<Handle::Pool, Handle::Allocator>;
template struct impl_Objects::Unique<Pool>;
template struct impl_Objects::ObjectOwner<Handle::Allocation, Handle::Allocator>;
template struct impl_Objects::Unique<Allocation>;
template struct impl_Objects::ObjectOwner<Handle::DefragmentationContext, Handle::Allocator>;
template struct impl_Objects::Unique<VirtualAllocation>;
template struct impl_Objects::Unique<VirtualBlock>;
} // namespace VmaBindings::impl_Objects
namespace VmaBindings {
auto Allocator::getAllocatorInfo() const -> AllocatorInfo {
    AllocatorInfo allocatorInfo;
    vmaGetAllocatorInfo(reinterpret_cast<VmaAllocator>(getHandle()),
                        reinterpret_cast<VmaAllocatorInfo *>(&allocatorInfo));
    return allocatorInfo;
}

auto Allocator::getPhysicalDeviceProperties() const
    -> const VkBindings::PhysicalDeviceProperties & {
    const VkPhysicalDeviceProperties *properties = nullptr;
    vmaGetPhysicalDeviceProperties(reinterpret_cast<VmaAllocator>(getHandle()), &properties);
    return *reinterpret_cast<const VkBindings::PhysicalDeviceProperties *>(properties);
}

auto Allocator::getMemoryProperties() const -> const VkBindings::PhysicalDeviceMemoryProperties & {
    const VkPhysicalDeviceMemoryProperties *properties = nullptr;
    vmaGetMemoryProperties(reinterpret_cast<VmaAllocator>(getHandle()), &properties);
    return *reinterpret_cast<const VkBindings::PhysicalDeviceMemoryProperties *>(properties);
}

auto Allocator::getMemoryTypeProperties(uint32_t memoryTypeIndex) const
    -> VkBindings::MemoryPropertyFlags {
    VkBindings::MemoryPropertyFlags flags;
    vmaGetMemoryTypeProperties(reinterpret_cast<VmaAllocator>(getHandle()), memoryTypeIndex,
                               reinterpret_cast<VkMemoryPropertyFlags *>(&flags));
    return flags;
}

void Allocator::setCurrentFrameIndex(uint32_t frameIndex) const {
    vmaSetCurrentFrameIndex(reinterpret_cast<VmaAllocator>(getHandle()), frameIndex);
}

auto Allocator::calculateStatistics() const -> TotalStatistics {
    TotalStatistics statistics{};
    vmaCalculateStatistics(reinterpret_cast<VmaAllocator>(getHandle()),
                           reinterpret_cast<VmaTotalStatistics *>(&statistics));
    return statistics;
}

auto Allocator::getHeapBudgets() const -> Budget {
    Budget budget{};
    vmaGetHeapBudgets(reinterpret_cast<VmaAllocator>(getHandle()),
                      reinterpret_cast<VmaBudget *>(&budget));
    return budget;
}

auto Allocator::findMemoryTypeIndex(uint32_t memoryTypeBits,
                                    const AllocationCreateInfo &allocationCreateInfo) const
    -> std::expected<uint32_t, VkBindings::Result> {
    uint32_t memoryTypeIndex = 0;
    if (auto res = static_cast<VkBindings::Result>(vmaFindMemoryTypeIndex(
            reinterpret_cast<VmaAllocator>(getHandle()), memoryTypeBits,
            reinterpret_cast<const VmaAllocationCreateInfo *>(&allocationCreateInfo),
            &memoryTypeIndex));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return memoryTypeIndex;
}

auto Allocator::findMemoryTypeIndexForBufferInfo(
    const VkBindings::BufferCreateInfo &bufferCreateInfo,
    const AllocationCreateInfo &allocationCreateInfo) const
    -> std::expected<uint32_t, VkBindings::Result> {
    uint32_t memoryTypeIndex = 0;
    if (auto res = static_cast<VkBindings::Result>(vmaFindMemoryTypeIndexForBufferInfo(
            reinterpret_cast<VmaAllocator>(getHandle()),
            reinterpret_cast<const VkBufferCreateInfo *>(&bufferCreateInfo),
            reinterpret_cast<const VmaAllocationCreateInfo *>(&allocationCreateInfo),
            &memoryTypeIndex));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return memoryTypeIndex;
}

auto Allocator::findMemoryTypeIndexForImageInfo(const VkBindings::ImageCreateInfo &imageCreateInfo,
                                                const AllocationCreateInfo &allocationCreateInfo)
    const -> std::expected<uint32_t, VkBindings::Result> {
    uint32_t memoryTypeIndex = 0;
    if (auto res = static_cast<VkBindings::Result>(vmaFindMemoryTypeIndexForImageInfo(
            reinterpret_cast<VmaAllocator>(getHandle()),
            reinterpret_cast<const VkImageCreateInfo *>(&imageCreateInfo),
            reinterpret_cast<const VmaAllocationCreateInfo *>(&allocationCreateInfo),
            &memoryTypeIndex));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return memoryTypeIndex;
}

auto Allocator::createPool(const PoolCreateInfo &createInfo) const
    -> std::expected<UniquePool, VkBindings::Result> {
    Handle::Pool handlePool = VK_BINDINGS_NULL_HANDLE;
    if (auto res = static_cast<VkBindings::Result>(
            vmaCreatePool(reinterpret_cast<VmaAllocator>(getHandle()),
                          reinterpret_cast<const VmaPoolCreateInfo *>(&createInfo),
                          reinterpret_cast<VmaPool *>(&handlePool)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return VkBindings::impl_Objects::Creator::create<UniquePool>(
        VkBindings::impl_Objects::Creator::create<Pool>(handlePool, getHandle()));
}

auto Allocator::allocateMemory(const VkBindings::MemoryRequirements &memoryRequirements,
                               const AllocationCreateInfo &createInfo) const
    -> std::expected<std::tuple<UniqueAllocation, AllocationInfo>, VkBindings::Result> {
    Handle::Allocation allocationHandle = VK_BINDINGS_NULL_HANDLE;
    AllocationInfo allocationInfo;
    if (auto res = static_cast<VkBindings::Result>(
            vmaAllocateMemory(reinterpret_cast<VmaAllocator>(getHandle()),
                              reinterpret_cast<const VkMemoryRequirements *>(&memoryRequirements),
                              reinterpret_cast<const VmaAllocationCreateInfo *>(&createInfo),
                              reinterpret_cast<VmaAllocation *>(&allocationHandle),
                              reinterpret_cast<VmaAllocationInfo *>(&allocationInfo)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return std::make_tuple(
        VkBindings::impl_Objects::Creator::create<UniqueAllocation>(
            VkBindings::impl_Objects::Creator::create<Allocation>(allocationHandle, getHandle())),
        allocationInfo);
}

auto Allocator::allocateDedicatedMemory(const VkBindings::MemoryRequirements &memoryRequirements,
                                        const AllocationCreateInfo &createInfo,
                                        void *pMemoryAllocateNext) const
    -> std::expected<std::tuple<UniqueAllocation, AllocationInfo>, VkBindings::Result> {
    Handle::Allocation allocationHandle = VK_BINDINGS_NULL_HANDLE;
    AllocationInfo allocationInfo;
    if (auto res = static_cast<VkBindings::Result>(vmaAllocateDedicatedMemory(
            reinterpret_cast<VmaAllocator>(getHandle()),
            reinterpret_cast<const VkMemoryRequirements *>(&memoryRequirements),
            reinterpret_cast<const VmaAllocationCreateInfo *>(&createInfo), pMemoryAllocateNext,
            reinterpret_cast<VmaAllocation *>(&allocationHandle),
            reinterpret_cast<VmaAllocationInfo *>(&allocationInfo)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return std::make_tuple(
        VkBindings::impl_Objects::Creator::create<UniqueAllocation>(
            VkBindings::impl_Objects::Creator::create<Allocation>(allocationHandle, getHandle())),
        allocationInfo);
}

auto Allocator::allocateMemoryPages(const VkBindings::MemoryRequirements &memoryRequirements,
                                    const AllocationCreateInfo &createInfo,
                                    size_t allocationCount) const
    -> std::expected<UniqueMemoryPages, VkBindings::Result> {
    std::vector<Handle::Allocation> handles(allocationCount);
    std::vector<AllocationInfo> infos(allocationCount);
    if (auto res = static_cast<VkBindings::Result>(vmaAllocateMemoryPages(
            reinterpret_cast<VmaAllocator>(getHandle()),
            reinterpret_cast<const VkMemoryRequirements *>(&memoryRequirements),
            reinterpret_cast<const VmaAllocationCreateInfo *>(&createInfo), allocationCount,
            reinterpret_cast<VmaAllocation *>(handles.data()),
            reinterpret_cast<VmaAllocationInfo *>(infos.data())));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    std::vector<Allocation> allocations(allocationCount);
    for (const auto &handle : handles) {
        allocations.emplace_back(
            VkBindings::impl_Objects::Creator::create<Allocation>(handle, getHandle()));
    }
    return UniqueMemoryPages(this, std::move(allocations), std::move(infos));
}

auto Allocator::allocateMemoryForBuffer(const VkBindings::Buffer &buffer,
                                        const AllocationCreateInfo &createInfo) const
    -> std::expected<std::tuple<UniqueAllocation, AllocationInfo>, VkBindings::Result> {
    Handle::Allocation allocationHandle = VK_BINDINGS_NULL_HANDLE;
    AllocationInfo allocationInfo;
    if (auto res = static_cast<VkBindings::Result>(vmaAllocateMemoryForBuffer(
            reinterpret_cast<VmaAllocator>(getHandle()),
            reinterpret_cast<VkBuffer>(buffer.getHandle()),
            reinterpret_cast<const VmaAllocationCreateInfo *>(&createInfo),
            reinterpret_cast<VmaAllocation *>(&allocationHandle),
            reinterpret_cast<VmaAllocationInfo *>(&allocationInfo)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return std::make_tuple(
        VkBindings::impl_Objects::Creator::create<UniqueAllocation>(
            VkBindings::impl_Objects::Creator::create<Allocation>(allocationHandle, getHandle())),
        allocationInfo);
}

auto Allocator::allocateMemoryForImage(const VkBindings::Image &image,
                                       const AllocationCreateInfo &createInfo) const
    -> std::expected<std::tuple<UniqueAllocation, AllocationInfo>, VkBindings::Result> {
    Handle::Allocation allocationHandle = VK_BINDINGS_NULL_HANDLE;
    AllocationInfo allocationInfo;
    if (auto res = static_cast<VkBindings::Result>(vmaAllocateMemoryForImage(
            reinterpret_cast<VmaAllocator>(getHandle()),
            reinterpret_cast<VkImage>(image.getHandle()),
            reinterpret_cast<const VmaAllocationCreateInfo *>(&createInfo),
            reinterpret_cast<VmaAllocation *>(&allocationHandle),
            reinterpret_cast<VmaAllocationInfo *>(&allocationInfo)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return std::make_tuple(
        VkBindings::impl_Objects::Creator::create<UniqueAllocation>(
            VkBindings::impl_Objects::Creator::create<Allocation>(allocationHandle, getHandle())),
        allocationInfo);
}
// void vmaFreeMemory(VmaAllocator allocator, VmaAllocation allocation);

// void vmaFreeMemoryPages(VmaAllocator allocator, size_t allocationCount,
//                         const VmaAllocation *pAllocations);

auto Allocator::flushAllocation(const Allocation &allocation, VkBindings::DeviceSize offset,
                                VkBindings::DeviceSize size) const -> VkBindings::Result {
    return static_cast<VkBindings::Result>(
        vmaFlushAllocation(reinterpret_cast<VmaAllocator>(getHandle()),
                           reinterpret_cast<VmaAllocation>(allocation.getHandle()), offset, size));
}

auto Allocator::invalidateAllocation(const Allocation &allocation, VkBindings::DeviceSize offset,
                                     VkBindings::DeviceSize size) const -> VkBindings::Result {
    return static_cast<VkBindings::Result>(vmaInvalidateAllocation(
        reinterpret_cast<VmaAllocator>(getHandle()),
        reinterpret_cast<VmaAllocation>(allocation.getHandle()), offset, size));
}

auto Allocator::flushAllocations(
    const VkBindings::impl_Struct::ArrayProxy<Allocation> &allocations,
    const VkBindings::impl_Struct::ArrayProxy<VkBindings::DeviceSize> &offsets,
    const VkBindings::impl_Struct::ArrayProxy<VkBindings::DeviceSize> &sizes) const
    -> VkBindings::Result {
    assert(allocations.size() == offsets.size() && allocations.size() == sizes.size());

    return static_cast<VkBindings::Result>(
        vmaFlushAllocations(reinterpret_cast<VmaAllocator>(getHandle()), allocations.size(),
                            reinterpret_cast<VmaAllocation const *>(allocations.data()),
                            reinterpret_cast<const VkDeviceSize *>(offsets.data()),
                            reinterpret_cast<const VkDeviceSize *>(sizes.data())));
}

auto Allocator::invalidateAllocations(
    const VkBindings::impl_Struct::ArrayProxy<Allocation> &allocations,
    const VkBindings::impl_Struct::ArrayProxy<VkBindings::DeviceSize> &offsets,
    const VkBindings::impl_Struct::ArrayProxy<VkBindings::DeviceSize> &sizes) const
    -> VkBindings::Result {
    assert(allocations.size() == offsets.size() && allocations.size() == sizes.size());

    return static_cast<VkBindings::Result>(
        vmaInvalidateAllocations(reinterpret_cast<VmaAllocator>(getHandle()), allocations.size(),
                                 reinterpret_cast<VmaAllocation const *>(allocations.data()),
                                 reinterpret_cast<const VkDeviceSize *>(offsets.data()),
                                 reinterpret_cast<const VkDeviceSize *>(sizes.data())));
}

auto Allocator::copyMemoryToAllocation(const void *pSrcHostPointer, const Allocation &dstAllocation,
                                       VkBindings::DeviceSize dstAllocationLocalOffset,
                                       VkBindings::DeviceSize size) const -> VkBindings::Result {
    return static_cast<VkBindings::Result>(
        vmaCopyMemoryToAllocation(reinterpret_cast<VmaAllocator>(getHandle()), pSrcHostPointer,
                                  reinterpret_cast<VmaAllocation>(dstAllocation.getHandle()),
                                  dstAllocationLocalOffset, size));
}

auto Allocator::copyAllocationToMemory(const Allocation &srcAllocation,
                                       VkBindings::DeviceSize srcAllocationLocalOffset,
                                       void *pDstHostPointer, VkBindings::DeviceSize size) const
    -> VkBindings::Result {
    return static_cast<VkBindings::Result>(
        vmaCopyAllocationToMemory(reinterpret_cast<VmaAllocator>(getHandle()),
                                  reinterpret_cast<VmaAllocation>(srcAllocation.getHandle()),
                                  srcAllocationLocalOffset, pDstHostPointer, size));
}

auto Allocator::checkCorruption(uint32_t memoryTypeBits) const -> VkBindings::Result {
    return static_cast<VkBindings::Result>(
        vmaCheckCorruption(reinterpret_cast<VmaAllocator>(getHandle()), memoryTypeBits));
}

auto Allocator::createBuffer(const VkBindings::BufferCreateInfo &bufferCreateInfo,
                             const AllocationCreateInfo &allocationCreateInfo) const
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, UniqueAllocation, AllocationInfo>,
                     VkBindings::Result> {
    VkBindings::Handle::Buffer handleBuffer = VK_BINDINGS_NULL_HANDLE;
    Handle::Allocation handleAllocation = VK_BINDINGS_NULL_HANDLE;
    AllocationInfo info;
    if (auto res = static_cast<VkBindings::Result>(vmaCreateBuffer(
            reinterpret_cast<VmaAllocator>(getHandle()),
            reinterpret_cast<const VkBufferCreateInfo *>(&bufferCreateInfo),
            reinterpret_cast<const VmaAllocationCreateInfo *>(&allocationCreateInfo),
            reinterpret_cast<VkBuffer *>(&handleBuffer),
            reinterpret_cast<VmaAllocation *>(&handleAllocation),
            reinterpret_cast<VmaAllocationInfo *>(&info)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return std::make_tuple(
        VkBindings::impl_Objects::Creator::create<VkBindings::UniqueBuffer>(
            VkBindings::impl_Objects::Creator::create<VkBindings::Buffer>(handleBuffer),
            getAllocatorInfo().device, *dispatcher, nullptr),
        VkBindings::impl_Objects::Creator::create<UniqueAllocation>(
            VkBindings::impl_Objects::Creator::create<Allocation>(handleAllocation, getHandle())),
        info);
}

auto Allocator::createBufferWithAlignment(const VkBindings::BufferCreateInfo &bufferCreateInfo,
                                          const AllocationCreateInfo &allocationCreateInfo,
                                          VkBindings::DeviceSize minAlignment) const
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, UniqueAllocation, AllocationInfo>,
                     VkBindings::Result> {
    VkBindings::Handle::Buffer handleBuffer = VK_BINDINGS_NULL_HANDLE;
    Handle::Allocation handleAllocation = VK_BINDINGS_NULL_HANDLE;
    AllocationInfo info;
    if (auto res = static_cast<VkBindings::Result>(vmaCreateBufferWithAlignment(
            reinterpret_cast<VmaAllocator>(getHandle()),
            reinterpret_cast<const VkBufferCreateInfo *>(&bufferCreateInfo),
            reinterpret_cast<const VmaAllocationCreateInfo *>(&allocationCreateInfo), minAlignment,
            reinterpret_cast<VkBuffer *>(&handleBuffer),
            reinterpret_cast<VmaAllocation *>(&handleAllocation),
            reinterpret_cast<VmaAllocationInfo *>(&info)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return std::make_tuple(
        VkBindings::impl_Objects::Creator::create<VkBindings::UniqueBuffer>(
            VkBindings::impl_Objects::Creator::create<VkBindings::Buffer>(handleBuffer),
            getAllocatorInfo().device, *dispatcher, nullptr),
        VkBindings::impl_Objects::Creator::create<UniqueAllocation>(
            VkBindings::impl_Objects::Creator::create<Allocation>(handleAllocation, getHandle())),
        info);
}

auto Allocator::createDedicatedBuffer(const VkBindings::BufferCreateInfo &bufferCreateInfo,
                                      const AllocationCreateInfo &allocationCreateInfo,
                                      void *pMemoryAllocateNext) const
    -> std::expected<std::tuple<VkBindings::UniqueBuffer, UniqueAllocation, AllocationInfo>,
                     VkBindings::Result> {
    VkBindings::Handle::Buffer handleBuffer = VK_BINDINGS_NULL_HANDLE;
    Handle::Allocation handleAllocation = VK_BINDINGS_NULL_HANDLE;
    AllocationInfo info;
    if (auto res = static_cast<VkBindings::Result>(vmaCreateDedicatedBuffer(
            reinterpret_cast<VmaAllocator>(getHandle()),
            reinterpret_cast<const VkBufferCreateInfo *>(&bufferCreateInfo),
            reinterpret_cast<const VmaAllocationCreateInfo *>(&allocationCreateInfo),
            pMemoryAllocateNext, reinterpret_cast<VkBuffer *>(&handleBuffer),
            reinterpret_cast<VmaAllocation *>(&handleAllocation),
            reinterpret_cast<VmaAllocationInfo *>(&info)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return std::make_tuple(
        VkBindings::impl_Objects::Creator::create<VkBindings::UniqueBuffer>(
            VkBindings::impl_Objects::Creator::create<VkBindings::Buffer>(handleBuffer),
            getAllocatorInfo().device, *dispatcher, nullptr),
        VkBindings::impl_Objects::Creator::create<UniqueAllocation>(
            VkBindings::impl_Objects::Creator::create<Allocation>(handleAllocation, getHandle())),
        info);
}

auto Allocator::createAliasingBuffer(const Allocation &allocation,
                                     const VkBindings::BufferCreateInfo &bufferCreateInfo) const
    -> std::expected<VkBindings::UniqueBuffer, VkBindings::Result> {
    VkBindings::Handle::Buffer handleBuffer = VK_BINDINGS_NULL_HANDLE;
    if (auto res = static_cast<VkBindings::Result>(
            vmaCreateAliasingBuffer(reinterpret_cast<VmaAllocator>(getHandle()),
                                    reinterpret_cast<VmaAllocation>(allocation.getHandle()),
                                    reinterpret_cast<const VkBufferCreateInfo *>(&bufferCreateInfo),
                                    reinterpret_cast<VkBuffer *>(&handleBuffer)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return VkBindings::impl_Objects::Creator::create<VkBindings::UniqueBuffer>(
        VkBindings::impl_Objects::Creator::create<VkBindings::Buffer>(handleBuffer),
        getAllocatorInfo().device, *dispatcher, nullptr);
}

auto Allocator::createAliasingBuffer2(const Allocation &allocation,
                                      VkBindings::DeviceSize allocationLocalOffset,
                                      const VkBindings::BufferCreateInfo &bufferCreateInfo) const
    -> std::expected<VkBindings::UniqueBuffer, VkBindings::Result> {
    VkBindings::Handle::Buffer handleBuffer = VK_BINDINGS_NULL_HANDLE;
    if (auto res = static_cast<VkBindings::Result>(vmaCreateAliasingBuffer2(
            reinterpret_cast<VmaAllocator>(getHandle()),
            reinterpret_cast<VmaAllocation>(allocation.getHandle()), allocationLocalOffset,
            reinterpret_cast<const VkBufferCreateInfo *>(&bufferCreateInfo),
            reinterpret_cast<VkBuffer *>(&handleBuffer)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return VkBindings::impl_Objects::Creator::create<VkBindings::UniqueBuffer>(
        VkBindings::impl_Objects::Creator::create<VkBindings::Buffer>(handleBuffer),
        getAllocatorInfo().device, *dispatcher, nullptr);
}

auto Allocator::createImage(const VkBindings::ImageCreateInfo &imageCreateInfo,
                            const AllocationCreateInfo &allocationCreateInfo) const
    -> std::expected<std::tuple<VkBindings::UniqueImage, UniqueAllocation, AllocationInfo>,
                     VkBindings::Result> {
    VkBindings::Handle::Image handleImage = VK_BINDINGS_NULL_HANDLE;
    Handle::Allocation handleAllocation = VK_BINDINGS_NULL_HANDLE;
    AllocationInfo info;
    if (auto res = static_cast<VkBindings::Result>(
            vmaCreateImage(reinterpret_cast<VmaAllocator>(getHandle()),
                           reinterpret_cast<const VkImageCreateInfo *>(&imageCreateInfo),
                           reinterpret_cast<const VmaAllocationCreateInfo *>(&allocationCreateInfo),
                           reinterpret_cast<VkImage *>(&handleImage),
                           reinterpret_cast<VmaAllocation *>(&handleAllocation),
                           reinterpret_cast<VmaAllocationInfo *>(&info)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return std::make_tuple(
        VkBindings::impl_Objects::Creator::create<VkBindings::UniqueImage>(
            VkBindings::impl_Objects::Creator::create<VkBindings::Image>(handleImage),
            getAllocatorInfo().device, *dispatcher, nullptr),
        VkBindings::impl_Objects::Creator::create<UniqueAllocation>(
            VkBindings::impl_Objects::Creator::create<Allocation>(handleAllocation, getHandle())),
        info);
}

auto Allocator::createDedicatedImage(const VkBindings::ImageCreateInfo &imageCreateInfo,
                                     const AllocationCreateInfo &allocationCreateInfo,
                                     void *pMemoryAllocateNext) const
    -> std::expected<std::tuple<VkBindings::UniqueImage, UniqueAllocation, AllocationInfo>,
                     VkBindings::Result> {
    VkBindings::Handle::Image handleImage = VK_BINDINGS_NULL_HANDLE;
    Handle::Allocation handleAllocation = VK_BINDINGS_NULL_HANDLE;
    AllocationInfo info;
    if (auto res = static_cast<VkBindings::Result>(vmaCreateDedicatedImage(
            reinterpret_cast<VmaAllocator>(getHandle()),
            reinterpret_cast<const VkImageCreateInfo *>(&imageCreateInfo),
            reinterpret_cast<const VmaAllocationCreateInfo *>(&allocationCreateInfo),
            pMemoryAllocateNext, reinterpret_cast<VkImage *>(&handleImage),
            reinterpret_cast<VmaAllocation *>(&handleAllocation),
            reinterpret_cast<VmaAllocationInfo *>(&info)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return std::make_tuple(
        VkBindings::impl_Objects::Creator::create<VkBindings::UniqueImage>(
            VkBindings::impl_Objects::Creator::create<VkBindings::Image>(handleImage),
            getAllocatorInfo().device, *dispatcher, nullptr),
        VkBindings::impl_Objects::Creator::create<UniqueAllocation>(
            VkBindings::impl_Objects::Creator::create<Allocation>(handleAllocation, getHandle())),
        info);
}

auto Allocator::createAliasingImage(const Allocation &allocation,
                                    const VkBindings::ImageCreateInfo &imageCreateInfo) const
    -> std::expected<VkBindings::UniqueImage, VkBindings::Result> {
    VkBindings::Handle::Image handleImage = VK_BINDINGS_NULL_HANDLE;
    if (auto res = static_cast<VkBindings::Result>(
            vmaCreateAliasingImage(reinterpret_cast<VmaAllocator>(getHandle()),
                                   reinterpret_cast<VmaAllocation>(allocation.getHandle()),
                                   reinterpret_cast<const VkImageCreateInfo *>(&imageCreateInfo),
                                   reinterpret_cast<VkImage *>(&handleImage)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return VkBindings::impl_Objects::Creator::create<VkBindings::UniqueImage>(
        VkBindings::impl_Objects::Creator::create<VkBindings::Image>(handleImage),
        getAllocatorInfo().device, *dispatcher, nullptr);
}

auto Allocator::createAliasingImage2(const Allocation &allocation,
                                     VkBindings::DeviceSize allocationLocalOffset,
                                     const VkBindings::ImageCreateInfo &imageCreateInfo) const
    -> std::expected<VkBindings::UniqueImage, VkBindings::Result> {
    VkBindings::Handle::Image handleImage = VK_BINDINGS_NULL_HANDLE;
    if (auto res = static_cast<VkBindings::Result>(vmaCreateAliasingImage2(
            reinterpret_cast<VmaAllocator>(getHandle()),
            reinterpret_cast<VmaAllocation>(allocation.getHandle()), allocationLocalOffset,
            reinterpret_cast<const VkImageCreateInfo *>(&imageCreateInfo),
            reinterpret_cast<VkImage *>(&handleImage)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return VkBindings::impl_Objects::Creator::create<VkBindings::UniqueImage>(
        VkBindings::impl_Objects::Creator::create<VkBindings::Image>(handleImage),
        getAllocatorInfo().device, *dispatcher, nullptr);
}

auto Allocator::buildStatsString(VkBindings::Bool32 detailedMap) const -> std::string {
    char *rawString = nullptr;

    vmaBuildStatsString(reinterpret_cast<VmaAllocator>(getHandle()), &rawString,
                        static_cast<VkBool32>(detailedMap));

    const auto deleter = [allocator =
                              reinterpret_cast<VmaAllocator>(getHandle())](char *string) noexcept {
        if (string != nullptr) {
            vmaFreeStatsString(allocator, string);
        }
    };

    const std::unique_ptr<char, decltype(deleter)> stringGuard{rawString, deleter};

    return stringGuard ? std::string{stringGuard.get()} : std::string{};
}

auto Allocator::beginDefragmentation(const DefragmentationInfo &info) const
    -> std::expected<DefragmentationContext, VkBindings::Result> {
    Handle::DefragmentationContext defragmentationContextHandle = VK_BINDINGS_NULL_HANDLE;
    if (auto res = static_cast<VkBindings::Result>(vmaBeginDefragmentation(
            reinterpret_cast<VmaAllocator>(getHandle()),
            reinterpret_cast<const VmaDefragmentationInfo *>(&info),
            reinterpret_cast<VmaDefragmentationContext *>(&defragmentationContextHandle)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return VkBindings::impl_Objects::Creator::create<DefragmentationContext>(
        defragmentationContextHandle, getHandle());
}

void Allocator::endDefragmentation(const DefragmentationContext &context) const {
    vmaEndDefragmentation(reinterpret_cast<VmaAllocator>(getHandle()),
                          reinterpret_cast<VmaDefragmentationContext>(context.getHandle()),
                          nullptr);
}

auto Allocator::endDefragmentationGetStats(const DefragmentationContext &context) const
    -> DefragmentationStats {
    DefragmentationStats stats{};
    vmaEndDefragmentation(reinterpret_cast<VmaAllocator>(getHandle()),
                          reinterpret_cast<VmaDefragmentationContext>(context.getHandle()),
                          reinterpret_cast<VmaDefragmentationStats *>(&stats));
    return stats;
}

auto DefragmentationContext::beginPass() const
    -> std::expected<DefragmentationPassMoveInfo, VkBindings::Result> {
    DefragmentationPassMoveInfo passInfo;
    if (auto res = static_cast<VkBindings::Result>(vmaBeginDefragmentationPass(
            reinterpret_cast<VmaAllocator>(getOwnerHandle()),
            reinterpret_cast<VmaDefragmentationContext>(getHandle()),
            reinterpret_cast<VmaDefragmentationPassMoveInfo *>(&passInfo)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return passInfo;
}

auto DefragmentationContext::endPass(DefragmentationPassMoveInfo &passInfo) const
    -> VkBindings::Result {
    return static_cast<VkBindings::Result>(
        vmaEndDefragmentationPass(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                                  reinterpret_cast<VmaDefragmentationContext>(getHandle()),
                                  reinterpret_cast<VmaDefragmentationPassMoveInfo *>(&passInfo)));
}

auto Pool::getStatistics() const -> Statistics {
    Statistics stats{};
    vmaGetPoolStatistics(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                         reinterpret_cast<VmaPool>(getHandle()),
                         reinterpret_cast<VmaStatistics *>(&stats));
    return stats;
}

auto Pool::calculateStatistics() const -> DetailedStatistics {
    DetailedStatistics stats{};
    vmaCalculatePoolStatistics(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                               reinterpret_cast<VmaPool>(getHandle()),
                               reinterpret_cast<VmaDetailedStatistics *>(&stats));
    return stats;
}

auto Pool::checkCorruption() const -> VkBindings::Result {
    return static_cast<VkBindings::Result>(vmaCheckPoolCorruption(
        reinterpret_cast<VmaAllocator>(getOwnerHandle()), reinterpret_cast<VmaPool>(getHandle())));
}

auto Pool::getName() const -> std::string_view {
    const char *name = nullptr;
    vmaGetPoolName(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                   reinterpret_cast<VmaPool>(getHandle()), &name);
    return name;
}

void Pool::setName(VkBindings::impl_Struct::InOutString name) const {
    vmaSetPoolName(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                   reinterpret_cast<VmaPool>(getHandle()), name.to_c_str());
}

auto Allocation::getAllocationInfo() -> AllocationInfo {
    AllocationInfo info;
    vmaGetAllocationInfo(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                         reinterpret_cast<VmaAllocation>(getHandle()),
                         reinterpret_cast<VmaAllocationInfo *>(&info));
    return info;
}

auto Allocation::getAllocationInfo2() -> AllocationInfo2 {
    AllocationInfo2 info;
    vmaGetAllocationInfo2(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                          reinterpret_cast<VmaAllocation>(getHandle()),
                          reinterpret_cast<VmaAllocationInfo2 *>(&info));
    return info;
}

void Allocation::setUserData(void *pUserData) {
    vmaSetAllocationUserData(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                             reinterpret_cast<VmaAllocation>(getHandle()), pUserData);
}

void Allocation::setName(VkBindings::impl_Struct::InOutString name) {
    vmaSetAllocationName(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                         reinterpret_cast<VmaAllocation>(getHandle()), name.to_c_str());
}

auto Allocation::getMemoryProperties() -> VkBindings::MemoryPropertyFlags {
    VkBindings::MemoryPropertyFlags flags;
    vmaGetAllocationMemoryProperties(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                                     reinterpret_cast<VmaAllocation>(getHandle()),
                                     reinterpret_cast<VkMemoryPropertyFlags *>(&flags));
    return flags;
}

#if VK_USE_PLATFORM_WIN32_KHR
static_assert(false, "Not implemented");
auto Allocation::GetMemoryWin32Handle(HANDLE hTargetProcess)
    -> std::expected<HANDLE, VkBindings::Result>;

auto Allocation::GetMemoryWin32Handle2(VkBindings::ExternalMemoryHandleTypeBits handleType,
                                       HANDLE hTargetProcess)
    -> std::expected<HANDLE, VkBindings::Result>;
#endif // VMA_EXTERNAL_MEMORY_WIN32

auto Allocation::mapMemory() -> std::expected<void *, VkBindings::Result> {
    void *pData = nullptr;
    if (auto res = static_cast<VkBindings::Result>(
            vmaMapMemory(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                         reinterpret_cast<VmaAllocation>(getHandle()), &pData));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return pData;
}

void Allocation::unmapMemory() {
    vmaUnmapMemory(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                   reinterpret_cast<VmaAllocation>(getHandle()));
}

auto Allocation::bindBufferMemory(const VkBindings::Buffer &buffer) -> VkBindings::Result {
    return static_cast<VkBindings::Result>(
        vmaBindBufferMemory(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                            reinterpret_cast<VmaAllocation>(getHandle()),
                            reinterpret_cast<VkBuffer>(buffer.getHandle())));
}

auto Allocation::bindBufferMemory2(VkBindings::DeviceSize allocationLocalOffset,
                                   const VkBindings::Buffer &buffer, const void *pNext)
    -> VkBindings::Result {
    return static_cast<VkBindings::Result>(
        vmaBindBufferMemory2(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                             reinterpret_cast<VmaAllocation>(getHandle()), allocationLocalOffset,
                             reinterpret_cast<VkBuffer>(buffer.getHandle()), pNext));
}

auto Allocation::bindImageMemory(const VkBindings::Image &image) -> VkBindings::Result {
    return static_cast<VkBindings::Result>(
        vmaBindImageMemory(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                           reinterpret_cast<VmaAllocation>(getHandle()),
                           reinterpret_cast<VkImage>(image.getHandle())));
}
auto Allocation::bindImageMemory2(VkBindings::DeviceSize allocationLocalOffset,
                                  const VkBindings::Image &image, const void *pNext)
    -> VkBindings::Result {
    return static_cast<VkBindings::Result>(
        vmaBindImageMemory2(reinterpret_cast<VmaAllocator>(getOwnerHandle()),
                            reinterpret_cast<VmaAllocation>(getHandle()), allocationLocalOffset,
                            reinterpret_cast<VkImage>(image.getHandle()), pNext));
}

auto VirtualAllocation::getAllocationInfo() const -> VirtualAllocationInfo {
    VirtualAllocationInfo info = {};
    vmaGetVirtualAllocationInfo(reinterpret_cast<VmaVirtualBlock>(getOwnerHandle()),
                                reinterpret_cast<VmaVirtualAllocation>(getHandle()),
                                reinterpret_cast<VmaVirtualAllocationInfo *>(&info));
    return info;
}

void VirtualAllocation::setUserData(void *pUserData) const {
    vmaSetVirtualAllocationUserData(reinterpret_cast<VmaVirtualBlock>(getOwnerHandle()),
                                    reinterpret_cast<VmaVirtualAllocation>(getHandle()), pUserData);
}

auto VirtualBlock::isEmpty() const -> VkBindings::Bool32 {
    return static_cast<VkBindings::Bool32>(
        vmaIsVirtualBlockEmpty(reinterpret_cast<VmaVirtualBlock>(getHandle())));
}

auto VirtualBlock::virtualAllocate(const VirtualAllocationCreateInfo &createInfo) const
    -> std::expected<std::tuple<UniqueVirtualAllocation, VkBindings::DeviceSize>,
                     VkBindings::Result> {
    Handle::VirtualAllocation virtualAllocationHandle = VK_BINDINGS_NULL_HANDLE;
    VkBindings::DeviceSize offset = 0;
    if (auto res = static_cast<VkBindings::Result>(vmaVirtualAllocate(
            reinterpret_cast<VmaVirtualBlock>(getHandle()),
            reinterpret_cast<const VmaVirtualAllocationCreateInfo *>(&createInfo),
            reinterpret_cast<VmaVirtualAllocation *>(&virtualAllocationHandle),
            reinterpret_cast<VkDeviceSize *>(&offset)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return std::make_tuple(VkBindings::impl_Objects::Creator::create<UniqueVirtualAllocation>(
                               VkBindings::impl_Objects::Creator::create<VirtualAllocation>(
                                   virtualAllocationHandle, getHandle())),
                           offset);
}

void VirtualBlock::clear() const {
    vmaClearVirtualBlock(reinterpret_cast<VmaVirtualBlock>(getHandle()));
}

auto VirtualBlock::getStatistics() const -> Statistics {
    Statistics stats{};
    vmaGetVirtualBlockStatistics(reinterpret_cast<VmaVirtualBlock>(getHandle()),
                                 reinterpret_cast<VmaStatistics *>(&stats));
    return stats;
}

auto VirtualBlock::calculateStatistics() const -> DetailedStatistics {
    DetailedStatistics stats{};
    vmaCalculateVirtualBlockStatistics(reinterpret_cast<VmaVirtualBlock>(getHandle()),
                                       reinterpret_cast<VmaDetailedStatistics *>(&stats));
    return stats;
}

auto VirtualBlock::buildStatsString(VkBindings::Bool32 detailedMap) const -> std::string {
    char *rawString = nullptr;

    vmaBuildVirtualBlockStatsString(reinterpret_cast<VmaVirtualBlock>(getHandle()), &rawString,
                                    static_cast<VkBool32>(detailedMap));

    const auto deleter =
        [virtualBlock = reinterpret_cast<VmaVirtualBlock>(getHandle())](char *string) noexcept {
            if (string != nullptr) {
                vmaFreeVirtualBlockStatsString(virtualBlock, string);
            }
        };

    const std::unique_ptr<char, decltype(deleter)> stringGuard{rawString, deleter};

    return stringGuard ? std::string{stringGuard.get()} : std::string{};
}

auto createAllocator(const VkBindings::Device &device, const AllocatorCreateInfo &createInfo)
    -> std::expected<UniqueAllocator, VkBindings::Result> {
    Handle::Allocator allocatorHandle = VK_BINDINGS_NULL_HANDLE;

    const auto &dispatcher = VkBindings::impl_Objects::Creator::getDispatcher(device);

    VulkanFunctions functions{
        .getInstanceProcAddr = VkBindings::Loader::GetGetInstanceProcAddr(),
        .getDeviceProcAddr = dispatcher.instanceTable.getDeviceProcAddr,
        .getPhysicalDeviceProperties = dispatcher.instanceTable.getPhysicalDeviceProperties,
        .getPhysicalDeviceMemoryProperties =
            dispatcher.instanceTable.getPhysicalDeviceMemoryProperties,
        .allocateMemory = dispatcher.deviceTable.allocateMemory,
        .freeMemory = dispatcher.deviceTable.freeMemory,
        .mapMemory = dispatcher.deviceTable.mapMemory,
        .unmapMemory = dispatcher.deviceTable.unmapMemory,
        .flushMappedMemoryRanges = dispatcher.deviceTable.flushMappedMemoryRanges,
        .invalidateMappedMemoryRanges = dispatcher.deviceTable.invalidateMappedMemoryRanges,
        .bindBufferMemory = dispatcher.deviceTable.bindBufferMemory,
        .bindImageMemory = dispatcher.deviceTable.bindImageMemory,
        .getBufferMemoryRequirements = dispatcher.deviceTable.getBufferMemoryRequirements,
        .getImageMemoryRequirements = dispatcher.deviceTable.getImageMemoryRequirements,
        .createBuffer = dispatcher.deviceTable.createBuffer,
        .destroyBuffer = dispatcher.deviceTable.destroyBuffer,
        .createImage = dispatcher.deviceTable.createImage,
        .destroyImage = dispatcher.deviceTable.destroyImage,
        .cmdCopyBuffer = dispatcher.deviceTable.cmdCopyBuffer,
        .getBufferMemoryRequirements2 = dispatcher.deviceTable.getBufferMemoryRequirements2,
        .getImageMemoryRequirements2 = dispatcher.deviceTable.getImageMemoryRequirements2,
        .bindBufferMemory2 = dispatcher.deviceTable.bindBufferMemory2,
        .bindImageMemory2 = dispatcher.deviceTable.bindImageMemory2,
        .getPhysicalDeviceMemoryProperties2 =
            dispatcher.instanceTable.getPhysicalDeviceMemoryProperties2,
        .getDeviceBufferMemoryRequirements =
            dispatcher.deviceTable.getDeviceBufferMemoryRequirements,
        .getDeviceImageMemoryRequirements = dispatcher.deviceTable.getDeviceImageMemoryRequirements,
#if VK_USE_PLATFORM_WIN32_KHR
        .getMemoryWin32HandleKHR = dispatcher.deviceTable.getMemoryWin32HandleKHR,
#else
        .getMemoryWin32HandleKHR = nullptr,
#endif

        .getPhysicalDeviceProperties2 = dispatcher.instanceTable.getPhysicalDeviceProperties2,
    };

    const VmaAllocatorCreateInfo allocatorCreateInfo{
        .flags = *reinterpret_cast<const VmaAllocatorCreateFlags *>(&createInfo.flags),
        .physicalDevice = reinterpret_cast<VkPhysicalDevice>(createInfo.physicalDevice.getHandle()),
        .device = reinterpret_cast<VkDevice>(device.getHandle()),
        .preferredLargeHeapBlockSize = createInfo.preferredLargeHeapBlockSize,
        .pAllocationCallbacks =
            reinterpret_cast<const VkAllocationCallbacks *>(createInfo.pAllocationCallbacks),
        .pDeviceMemoryCallbacks =
            reinterpret_cast<const VmaDeviceMemoryCallbacks *>(createInfo.pDeviceMemoryCallbacks),
        .pHeapSizeLimit = reinterpret_cast<const VkDeviceSize *>(createInfo.pHeapSizeLimit),
        .pVulkanFunctions = reinterpret_cast<const VmaVulkanFunctions *>(&functions),
        .instance = reinterpret_cast<VkInstance>(createInfo.instance.getHandle()),
        .vulkanApiVersion = createInfo.vulkanApiVersion,
        .pTypeExternalMemoryHandleTypes =
            reinterpret_cast<const VkExternalMemoryHandleTypeFlagsKHR *>(
                createInfo.pTypeExternalMemoryHandleTypes),
    };
    if (auto res = static_cast<VkBindings::Result>(vmaCreateAllocator(
            &allocatorCreateInfo, reinterpret_cast<VmaAllocator *>(&allocatorHandle)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return VkBindings::impl_Objects::Creator::create<UniqueAllocator>(
        VkBindings::impl_Objects::Creator::create<Allocator>(
            allocatorHandle, &VkBindings::impl_Objects::Creator::getDispatcher(device)));
}

auto createVirtualBlock(const VirtualBlockCreateInfo &createInfo)
    -> std::expected<UniqueVirtualBlock, VkBindings::Result> {
    Handle::VirtualBlock virtualBlockHandle = VK_BINDINGS_NULL_HANDLE;
    if (auto res = static_cast<VkBindings::Result>(
            vmaCreateVirtualBlock(reinterpret_cast<const VmaVirtualBlockCreateInfo *>(&createInfo),
                                  reinterpret_cast<VmaVirtualBlock *>(&virtualBlockHandle)));
        res != VkBindings::Result::Success) {
        return std::unexpected(res);
    }
    return VkBindings::impl_Objects::Creator::create<UniqueVirtualBlock>(
        VkBindings::impl_Objects::Creator::create<VirtualBlock>(virtualBlockHandle));
}

UniqueMemoryPages::UniqueMemoryPages(const Allocator *allocator,
                                     std::vector<Allocation> &&allocations,
                                     std::vector<AllocationInfo> &&infos)
    : allocator(allocator), allocations(std::move(allocations)), infos(std::move(infos)) {}

UniqueMemoryPages::~UniqueMemoryPages() { free(); }

UniqueMemoryPages::UniqueMemoryPages(UniqueMemoryPages &&other) noexcept
    : allocator(std::exchange(other.allocator, nullptr)), allocations(std::move(other.allocations)),
      infos(std::move(other.infos)) {}
auto UniqueMemoryPages::operator=(UniqueMemoryPages &&other) noexcept -> UniqueMemoryPages & {
    if (this != &other) {
        free();

        allocator = std::exchange(other.allocator, nullptr);
        allocations = std::move(other.allocations);
        infos = std::move(other.infos);
    }

    return *this;
}

auto UniqueMemoryPages::getAllocations() const noexcept -> const std::vector<Allocation> & {
    return allocations;
}

auto UniqueMemoryPages::getInfos() const noexcept -> const std::vector<AllocationInfo> & {
    return infos;
}

void UniqueMemoryPages::free() noexcept {
    if (allocator == nullptr) {
        return;
    }

    vmaFreeMemoryPages(reinterpret_cast<VmaAllocator>(allocator->getHandle()), allocations.size(),
                       reinterpret_cast<VmaAllocation const *>(allocations.data()));

    allocations.clear();
    infos.clear();
    allocator = nullptr;
}
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
} // namespace VmaBindings
