//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Structures and functions for managing a virtual allocation list.
//=============================================================================

#ifndef RMV_BACKEND_RMT_VIRTUAL_ALLOCATION_LIST_H_
#define RMV_BACKEND_RMT_VIRTUAL_ALLOCATION_LIST_H_

#include "rmt_configuration.h"
#include "rmt_error.h"
#include "rmt_format.h"
#include "rmt_pool.h"
#include "rmt_types.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

typedef struct RmtResource                  RmtResource;
typedef struct RmtResourceList              RmtResourceList;
typedef struct RmtDataSnapshot              RmtDataSnapshot;
typedef struct RmtVirtualAllocationInterval RmtVirtualAllocationInterval;

/// A structure encapsulating a region of memory that is unbound.
typedef struct RmtMemoryRegion
{
    uint64_t offset;  ///< The offset in bytes from the start of the parent <c><i>RmtVirtualAllocation</i></c>.
    uint64_t size;    ///< The size (in bytes) of the unbound memory region.
} RmtMemoryRegion;

/// An enumeration of flags for allocation details.
typedef enum RmtAllocationDetailFlagBits
{
    kRmtAllocationDetailIsCpuMapped         = (1 << 0),  ///< The allocation is currently mapped for CPU access.
    kRmtAllocationDetailHasBeenCpuMapped    = (1 << 1),  ///< The allocation has been mapped for CPU access.
    kRmtAllocationDetailIsMadeResident      = (1 << 2),  ///< The allocation is currently requested to be made resident.
    kRmtAllocationDetailHasBeenMadeResident = (1 << 3),  ///< The allocation has been requested to be made resident.
    kRmtAllocationDetailHasBeenEvicted      = (1 << 4),  ///< The allocation has been requested to be evicted.
    kRmtAllocationDetailIsDead              = (1 << 5),  ///< The allocation has been freed later on and is waiting for deferred compaction.
} RmtAllocationDetailFlagBits;

/// A structure encapsulating extra details about an allocation.
typedef struct RmtVirtualAllocation
{
    uint64_t      base_address;                                ///< The base address of the allocation.
    int32_t       size_in_4kb_page;                            ///< The size of the allocation.
    int32_t       guid;                                        ///< A GUID for the this allocation.
    uint32_t      flags;                                       ///< A set of flags for the allocation.
    uint64_t      timestamp;                                   ///< The timestamp when the allocation was made.
    uint64_t      last_residency_update;                       ///< The timestamp when the last residency update was made.
    uint64_t      last_cpu_map;                                ///< The timestamp when the last CPU map operation occurred.
    uint64_t      last_cpu_un_map;                             ///< The timestamp when the last CPU unmap operation occurred.
    int32_t       add_count;                                   ///< The number of times a residency update add was requested for this allocation.
    int32_t       remove_count;                                ///< The number of times a residency update remove was requeted for this allocation.
    int32_t       map_count;                                   ///< The current number of times the address is CPU mapped.
    int32_t       resource_count;                              ///< The number of resources bound to this allocation.
    int32_t       non_heap_resource_count;                     ///< The number of resources bound to this allocation which are not heaps.
    RmtHeapType   heap_preferences[RMT_NUM_HEAP_PREFERENCES];  ///< The heap preferences in order.
    RmtOwnerType  owner;                                       ///< The owner of the allocation.
    uint32_t      commit_type;                                 ///< A bit field of all commit types used by resources inside this allocation.
    RmtResource** resources;                                   ///< The address of an array of points to <c><i>RmtResource</i></c> structures.
    int32_t       next_resource_index;                         ///< The index of the new resource.
    RmtMemoryRegion*
        unbound_memory_regions;  ///< An array of <c><i>RmtUnboundMemoryRegion</i></c> structures representing the unbound memory inside this virtual allocation.
    int32_t     unbound_memory_region_count;  ///< The number of <c><i>RmtUnboundMemoryRegion</i></c> structures inside <c><i>unboundMemoryRegions</i></c>.
    uint64_t    allocation_identifier;        ///< Uniquely identifies this virtual memory allocation.
    uint64_t    resource_usage_aliased_size[kRmtResourceUsageTypeCount];  ///< Aliased resource usage sizes for resource bound to this allocation.
    const char* name;                                                     ///< Allocation name.

} RmtVirtualAllocation;

/// Get the size (in bytes) of a virtual allocation.
///
/// @param [in] virtual_allocation                  A pointer to a <c><i>RmtVirtualAllocation</i></c> structure.
///
/// @returns
/// The size (in bytes) of the <c><i>virtualAllocation</i></c>.
uint64_t RmtVirtualAllocationGetSizeInBytes(const RmtVirtualAllocation* virtual_allocation);

/// Get the size (in bytes) of the largest resource bound to a virtual allocation.
///
/// @param [in] virtual_allocation                  A pointer to a <c><i>RmtVirtualAllocation</i></c> structure.
///
/// @returns
/// The size (in bytes) of the largest resource bound to <c><i>virtualAllocation</i></c>.
uint64_t RmtVirtualAllocationGetLargestResourceSize(const RmtVirtualAllocation* virtual_allocation);

/// Get the total amount of memory used for resources within a virtual allocation.
///
/// @param [in] snapshot                            A pointer to a <c><i>RmtDataSnapshot</i></c> structure.
/// @param [in] virtual_allocation                  A pointer to a <c><i>RmtVirtualAllocation</i></c> structure.
///
/// @returns
/// The size (in bytes) of the space inside <c><i>virtualAllocation</i></c> bound to resources.
uint64_t RmtVirtualAllocationGetTotalResourceMemoryInBytes(const RmtDataSnapshot* snapshot, const RmtVirtualAllocation* virtual_allocation);

/// Get the amount of memory not used for any resources.
///
/// @param [in] snapshot                            A pointer to a <c><i>RmtDataSnapshot</i></c> structure.
/// @param [in] virtual_allocation                  A pointer to a <c><i>RmtVirtualAllocation</i></c> structure.
///
/// @returns
/// The size (in bytes) of the free space inside <c><i>virtualAllocation</i></c>.
uint64_t RmtVirtualAllocationGetTotalUnboundSpaceInAllocation(const RmtDataSnapshot* snapshot, const RmtVirtualAllocation* virtual_allocation);

/// Get the average resource size inside a virtual allocation.
///
/// @param [in] snapshot                            A pointer to a <c><i>RmtDataSnapshot</i></c> structure.
/// @param [in] virtual_allocation                  A pointer to a <c><i>RmtVirtualAllocation</i></c> structure.
///
/// @returns
/// The average size (in bytes) of a resource inside <c><i>virtualAllocation</i></c>.
uint64_t RmtVirtualAllocationGetAverageResourceSizeInBytes(const RmtDataSnapshot* snapshot, const RmtVirtualAllocation* virtual_allocation);

/// Get the standard deviation for the resources in side a virtual allocation.
///
/// @param [in] snapshot                            A pointer to a <c><i>RmtDataSnapshot</i></c> structure.
/// @param [in] virtual_allocation                  A pointer to a <c><i>RmtVirtualAllocation</i></c> structure.
///
/// @returns
/// The standard deviation (in bytes) for the resources inside <c><i>virtualAllocation</i></c>.
uint64_t RmtVirtualAllocationGetResourceStandardDeviationInBytes(const RmtDataSnapshot* snapshot, const RmtVirtualAllocation* virtual_allocation);

/// Get the fragmentation quotient for a virtual allocation.
///
/// A fragmentation quotient is a score in the range [0..1] which tells you how fragmented a
/// virtual allocation is.
///
/// @param [in] virtual_allocation                  A pointer to a <c><i>RmtVirtualAllocation</i></c> structure.
///
/// @returns
/// The fragmentation quotient for <c><i>virtualAllocation</i></c>.
float RmtVirtualAllocationGetFragmentationQuotient(const RmtVirtualAllocation* virtual_allocation);

/// Get a histogram of bytes backing a virtual allocation.
///
/// @param [in] snapshot                                A pointer to a <c><i>RmtDataSnapshot</i></c> structure.
/// @param [in] virtual_allocation                      A pointer to a <c><i>RmtVirtualAllocation</i></c> structure.
/// @param [out] out_bytes_per_backing_storage_type     A pointer an array of <c><i>uint64_t</i></c> that will contain histogram values (in bytes).
/// @param [out] out_histogram_total                    A pointer to a <c><i>uint64_t</i></c> value where the size will be written.
///
/// @retval
/// kRmtOk                                  The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                 The operation failed because <c><i>snapshot</i></c> or <c><i>outAllocation</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtVirtualAllocationGetBackingStorageHistogram(const RmtDataSnapshot*      snapshot,
                                                            const RmtVirtualAllocation* virtual_allocation,
                                                            uint64_t*                   out_bytes_per_backing_storage_type,
                                                            uint64_t*                   out_histogram_total);

/// A structure encapsulating critical allocation identifier information.
typedef struct RmtVirtualAllocationInterval
{
    RmtGpuAddress                 base_address;       ///< The base address of the allocation.
    int32_t                       size_in_4kb_pages;  ///< The size of the allocation in 4KiB page
    int32_t                       dead;               ///< Set to true if its dead.
    RmtVirtualAllocation*         allocation;         ///< A pointer to a <c><i>RmtVirtualAllocation</i></c> structure containing the resource payload.
    RmtVirtualAllocationInterval* left;               ///< A pointer to a <c><i>RmtVirtualAllocationNode</i></c> structure that is the left child of this node.
    RmtVirtualAllocationInterval* right;              ///< A pointer to a <c><i>RmtVirtualAllocationNode</i></c> structure that is the right child of this node.
} RmtVirtualAllocationInterval;

/// A structure encapsulating a list of allocations.
typedef struct RmtVirtualAllocationList
{
    // data structures for lookups.
    RmtVirtualAllocationInterval* root;                      ///< The root node of the acceleration structure.
    RmtVirtualAllocationInterval* allocation_intervals;      ///< A buffer of allocation intervals.
    RmtPool                       allocation_interval_pool;  ///< A pool allocator for the memory buffer pointed to by <c><i>allocationIntervals</i></c>.

    // storage for allocations.
    RmtVirtualAllocation* allocation_details;                                 ///< A buffer of extra allocation details.
    int32_t               allocation_count;                                   ///< The number of live allocations in the list.
    int32_t               next_allocation_guid;                               ///< The next allocation GUID to assign.
    int32_t               maximum_concurrent_allocations;                     ///< The maximum number of concurrent allocations.
    int32_t               total_allocations;                                  ///< The total number of allocations.
    uint64_t              total_allocated_bytes;                              ///< The total number of bytes allocated.
    uint64_t              allocations_per_preferred_heap[kRmtHeapTypeCount];  ///< The number of bytes for each preferred heap type.
    RmtResource**         resource_connectivity;                              ///< An array of pointers to resources, sorted by the resource's base address.
    int32_t               resource_connectivity_count;  ///< The number of resoure pointers in the buffer pointed to by <c><i>resourceConnectivity</i></c>.
    RmtMemoryRegion*
        unbound_memory_regions;  ///< An array of <c><i>RmtUnboundMemoryRegion</i></c> structures representing all unbound memory regions for all allocations.
    int32_t unbound_memory_region_count;  ///< The number of <c><i>RmtUnboundMemoryRegion</i></c> structures inside <c><i>unboundMemoryRegions</i></c>.

} RmtVirtualAllocationList;

/// Calculate the size of the workling buffer required for a specific number of concurrent allocations.
///
/// @param [in] total_allocations                   The maximum number of concurrent allocations that can be in flight at any one time.
/// @param [in] max_concurrent_resources            The maximum number of concurrent resources that can be in flight at any one time.
///
/// @returns
/// The size of the virtual allocation list buffer that is required to initialize a <c><i>RmtVirtualAllocationList</i></c> structure using <c><i>rmtVirtualAllocationListInitialize</i></c>.
size_t RmtVirtualAllocationListGetBufferSize(int32_t total_allocations, int32_t max_concurrent_resources);

/// Initialize the allocation list.
///
/// @param [in] virtual_allocation_list                 A pointer to a <c><i>RmtVirtualAllocationList</i></c> structure.
/// @param [in] buffer                                  A pointer to a buffer containing the raw resource data.
/// @param [in] buffer_size                             The buffer size, in bytes.
/// @param [in] maximum_concurrent_allocations          The maximum number of allocations that can be in flight at once.
/// @param [in] maximum_concurrent_resources            The maximum number of resources that can be in flight at once.
/// @param [in] total_resources                         The maximum number of resources that can be in flight at once.
///
/// @retval
/// kRmtOk                          The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer         The operation failed because <c><i>virtual_allocation_list</i></c> or <c><i>buffer</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorInvalidSize            The operation failed because <c><i>buffer_size</i></c> is invalid.
RmtErrorCode RmtVirtualAllocationListInitialize(RmtVirtualAllocationList* virtual_allocation_list,
                                                void*                     buffer,
                                                size_t                    buffer_size,
                                                int32_t                   maximum_concurrent_allocations,
                                                int32_t                   maximum_concurrent_resources,
                                                int32_t                   total_allocations);

/// Add an allocation to the list.
///
/// @param [in] virtual_allocation_list                 A pointer to a <c><i>RmtVirtualAllocationList</i></c> structure.
/// @param [in] timestamp                               The timestamp for when the allocation happened.
/// @param [in] address                                 The address of the allocation.
/// @param [in] size_in_4kb_pages                       The size of the allocation.
/// @param [in] preferences                             An array of preferred heaps for the allocation.
/// @param [in] owner                                   The owner of the allocation.
/// @param [in] allocation_identifier                   A value that uniquely identifies the virtual memory allocation.
///
/// @retval
/// kRmtOk                          The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer         The operation failed because <c><i>virtual_allocation_list</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorInvalidSize            The operation failed because <c><i>size_in_4kb_pages</i></c> is invalid.
RmtErrorCode RmtVirtualAllocationListAddAllocation(RmtVirtualAllocationList* virtual_allocation_list,
                                                   uint64_t                  timestamp,
                                                   RmtGpuAddress             address,
                                                   int32_t                   size_in_4kb_pages,
                                                   const RmtHeapType         preferences[4],
                                                   RmtOwnerType              owner,
                                                   const uint64_t            allocation_identifier);

/// Remove an allocation from the list.
///
/// @param [in] virtual_allocation_list                 A pointer to a <c><i>RmtVirtualAllocationList</i></c> structure.
/// @param [in] address                                 The address of the allocation.
///
/// @retval
/// kRmtOk                          The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer         The operation failed because <c><i>virtual_allocation_list</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorNoAllocationFound      The operation failed because the allocation at <c><i>address</i></c> wasn't found.
RmtErrorCode RmtVirtualAllocationListRemoveAllocation(RmtVirtualAllocationList* virtual_allocation_list, RmtGpuAddress address);

/// Add a residency update to a specific address.
///
/// @param [in] virtual_allocation_list                 A pointer to a <c><i>RmtVirtualAllocationList</i></c> structure.
/// @param [in] timestamp                               The timestamp for when the allocation happened.
/// @param [in] address                                 The address of the allocation.
/// @param [in] update_type                             The address of the allocation.
/// @param [in] queue                                   The address of the allocation.
///
/// @retval
/// kRmtOk                          The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer         The operation failed because <c><i>virtual_allocation_list</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorNoAllocationFound      The operation failed because the allocation at <c><i>address</i></c> wasn't found.
RmtErrorCode RmtVirtualAllocationListAddResourceReference(RmtVirtualAllocationList* virtual_allocation_list,
                                                          uint64_t                  timestamp,
                                                          RmtGpuAddress             address,
                                                          RmtResidencyUpdateType    update_type,
                                                          RmtQueue                  queue);

/// Add a CPU map to a specific address.
///
/// @param [in] virtual_allocation_list                 A pointer to a <c><i>RmtVirtualAllocationList</i></c> structure.
/// @param [in] timestamp                               The timestamp for when the allocation happened.
/// @param [in] address                                 The address of the allocation.
///
/// @retval
/// kRmtOk                          The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer         The operation failed because <c><i>virtual_allocation_list</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorNoAllocationFound      The operation failed because the allocation at <c><i>address</i></c> wasn't found.
RmtErrorCode RmtVirtualAllocationListAddCpuMap(RmtVirtualAllocationList* virtual_allocation_list, uint64_t timestamp, RmtGpuAddress address);

/// Add a CPU unmap to a specific address.
///
/// @param [in] virtual_allocation_list                 A pointer to a <c><i>RmtVirtualAllocationList</i></c> structure.
/// @param [in] timestamp                               The timestamp for when the allocation happened.
/// @param [in] address                                 The address of the allocation.
///
/// @retval
/// kRmtOk                          The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer         The operation failed because <c><i>virtual_allocation_list</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorNoAllocationFound      The operation failed because the allocation at <c><i>address</i></c> wasn't found.
RmtErrorCode RmtVirtualAllocationListAddCpuUnmap(RmtVirtualAllocationList* virtual_allocation_list, uint64_t timestamp, RmtGpuAddress address);

/// Perform compaction on the virtual allocation list. This will remove any allocations
/// that are marked as dead and fix up anay resources that point at them.
///
/// @param [in] virtual_allocation_list                 A pointer to a <c><i>RmtVirtualAllocationList</i></c> structure.
/// @param [in] fixup_resources                         Set to true if you want the compaction to attempt to change the bound pointers on resources.
///
/// @retval
/// kRmtOk                                  The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                 The operation failed because <c><i>virtual_allocation_list</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtVirtualAllocationListCompact(RmtVirtualAllocationList* virtual_allocation_list, bool fixup_resources);

/// Find base address of allocation from address.
///
/// @param [in] virtual_allocation_list                 A pointer to a <c><i>RmtVirtualAllocationList</i></c> structure.
/// @param [in] address                                 The address of the allocation.
/// @param [out] out_allocation                         A pointer to a pointer to a <c><i>RmtVirtualAllocation</i></c> structure to receive the allocation.

///
/// @retval
/// kRmtOk                                  The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                 The operation failed because <c><i>virtual_allocation_list</i></c> or <c><i>out_allocation</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtVirtualAllocationListGetAllocationForAddress(const RmtVirtualAllocationList* virtual_allocation_list,
                                                             RmtGpuAddress                   address,
                                                             const RmtVirtualAllocation**    out_allocation);

/// Get the total size (in bytes) of the memory in a virtual allocation list.
///
/// @param [in] virtual_allocation_list                 A pointer to a <c><i>RmtVirtualAllocationList</i></c> structure.
///
/// @returns
/// The size (in bytes) of all virtual allocations contained in <c><i>virtual_allocation_list</i></c>.
uint64_t RmtVirtualAllocationListGetTotalSizeInBytes(const RmtVirtualAllocationList* virtual_allocation_list);

/// Get the size (in bytes) of the memory in a virtual allocation list that is bound to resources.
///
/// @param [in] snapshot                                A pointer to a <c><i>RmtDataSnapshot</i></c> structure.
/// @param [in] virtual_allocation_list                 A pointer to a <c><i>RmtVirtualAllocationList</i></c> structure.
///
/// @returns
/// The size (in bytes) of the memory in a <c><i>virtual_allocation_list</i></c> that is bound to resources.
uint64_t RmtVirtualAllocationListGetBoundTotalSizeInBytes(const RmtDataSnapshot* snapshot, const RmtVirtualAllocationList* virtual_allocation_list);

/// Get the size (in bytes) of the memory in a virtual allocation list that is not bound to resources.
///
/// @param [in] snapshot                                A pointer to a <c><i>RmtDataSnapshot</i></c> structure.
/// @param [in] virtual_allocation_list                 A pointer to a <c><i>RmtVirtualAllocationList</i></c> structure.
///
/// @returns
/// The size (in bytes) of the memory in a <c><i>virtual_allocation_list</i></c> that is not bound to resources.
uint64_t RmtVirtualAllocationListGetUnboundTotalSizeInBytes(const RmtDataSnapshot* snapshot, const RmtVirtualAllocationList* virtual_allocation_list);

/// For each virtual allocation, update the size after aliasing for each bound resource.
///
/// @param [in] allocation_list                        A pointer to a <c><i>RmtVirtualAllocationList</i></c> structure.
/// @param [in] resource_list                          A pointer to a <c><i>RmtResourceList</i></c> structure.
/// @param [in] resource_usage_mask                    A bit mask of all usage types that should be included when calculating alias size.
///
/// @retval
/// kRmtOk                                  The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                 The operation failed because <c><i>virtual_allocation_list</i></c> or <c><i>resource_list</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtVirtualAllocationListUpdateAliasedResourceSizes(const RmtVirtualAllocationList* allocation_list,
                                                                const RmtResourceList*          resource_list,
                                                                const uint64_t                  resource_usage_mask);
#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_VIRTUAL_ALLOCATION_LIST_H_
