//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Structures and functions for working with a snapshot.
//=============================================================================

#ifndef RMV_BACKEND_RMT_DATA_SNAPSHOT_H_
#define RMV_BACKEND_RMT_DATA_SNAPSHOT_H_

#include "rmt_types.h"
#include "rmt_resource_list.h"
#include "rmt_page_table.h"
#include "rmt_virtual_allocation_list.h"
#include "rmt_physical_allocation_list.h"
#include "rmt_configuration.h"
#include "rmt_process_map.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

typedef struct RmtDataSet         RmtDataSet;
typedef struct RmtResourceHistory RmtResourceHistory;
typedef struct RmtSnapshotPoint   RmtSnapshotPoint;

/// An enumeration of all segment status flags.
typedef enum RmtSegmentStatusFlags
{
    kRmtSegmentStatusFlagVram       = (1 << 0),
    kRmtSegmentStatusFlagHost       = (1 << 1),
    kRmtSegmentStatusFlagCpuCached  = (1 << 2),
    kRmtSegmentStatusFlagCpuVisible = (1 << 3),
    kRmtSegmentStatusFlagGpuCached  = (1 << 4),
    kRmtSegmentStatusFlagGpuVisible = (1 << 5)
} RmtSegmentStatusFlags;

/// An enumeration of all subscription
typedef enum RmtSegmentSubscriptionStatus
{
    kRmtSegmentSubscriptionStatusOverLimit    = 0,  ///< The segment is over-subscribed.
    kRmtSegmentSubscriptionStatusUnderLimit   = 1,  ///< The segment is under the advised limit.
    kRmtSegmentSubscriptionStatusCloseToLimit = 2,  ///< The segment is close to the limit.
} RmtSegmentSubscriptionStatus;

/// A structure encapsulating the status of a heap.
typedef struct RmtSegmentStatus
{
    RmtHeapType heap_type;                                 ///< The type of the heap.
    uint32_t    flags;                                     ///< The flags for the segment status.
    uint64_t    total_physical_size;                       ///< The total size (in bytes) of physical memory.
    uint64_t    total_virtual_memory_requested;            ///< The total size (in bytes) of virtual memory that was requested from this segment.
    uint64_t    total_bound_virtual_memory;                ///< The total size (in bytes) of virtual memory that was requested from this segment and then bound.
    uint64_t    total_physical_mapped_by_process;          ///< The total size (in bytes) of the physical memory mapped by the target process.
    uint64_t    total_physical_mapped_by_other_processes;  ///< The total size (in bytes) of the physical memory mapped by other processes.
    uint64_t    peak_bandwidth_in_bytes_per_second;        ///< The peak bandwidth (in bytes per second) that the RAM in the segment is capable of.
    uint64_t    allocation_count;                          ///< The number of allocations owned by this heap.
    uint64_t    mean_allocation_size;                      ///< The mean allcation size (in bytes) of all allocations in this segment.
    uint64_t    max_allocation_size;                       ///< The max allocation size (in bytes) of all allocations in this segment.
    uint64_t    min_allocation_size;                       ///< The min allocation size (in bytes) of all allocations in this segment.
    uint64_t    resource_count;                            ///< The number of resources owned by this heap.
    uint64_t    committed_size;                            ///< The amount of committed memory in bytes.

    uint64_t physical_bytes_per_resource_usage[kRmtResourceUsageTypeCount];  ///< The amount of physical memory (in bytes) of each resource usage type.
} RmtSegmentStatus;

/// Get the status of a specific segment.
///
/// @param [in] segment_status                  A pointer to a <c><i>RmtSegmentStatus</i></c> structure.
///
/// @returns
/// The segment subscription status value.
RmtSegmentSubscriptionStatus RmtSegmentStatusGetOversubscribed(const RmtSegmentStatus* segment_status);

/// A structure encapsulating a single snapshot at a specific point in time.
typedef struct RmtDataSnapshot
{
    char              name[RMT_MAXIMUM_NAME_LENGTH];  ///< The name of the snapshot.
    uint64_t          timestamp;                      ///< The timestamp at the point where the snapshot was taken.
    RmtDataSet*       data_set;        ///< A pointer to a <c><i>RmtDataSet</i></c> structure from which the <c><i>RmtDataSnapshot</i></c> was generated.
    RmtSnapshotPoint* snapshot_point;  ///< The snapshot point the snapshot was generated from.

    // Summary data for the snapshot.
    RmtGpuAddress minimum_virtual_address;                 ///< The minimum virtual address that has been encountered in this snapshot.
    RmtGpuAddress maximum_virtual_address;                 ///< The maximum virtual address that has been encountered in this snapshot.
    uint64_t      minimum_allocation_timestamp;            ///< The minimum timestamp seen for allocations.
    uint64_t      maximum_allocation_timestamp;            ///< The maximum timestamp seen for allocations.
    uint64_t      minimum_resource_size_in_bytes;          ///< The minimum resource size (in bytes) in this snapshot.
    uint64_t      maximum_resource_size_in_bytes;          ///< The maximum resource size (in bytes) in this snapshot.
    uint64_t      maximum_unbound_resource_size_in_bytes;  ///< The maximum unbound resource size (in bytes) in this snapshot.
    uint64_t      maximum_physical_memory_in_bytes;        ///< The maximum amount of physical memory (in bytes).

    RmtVirtualAllocationList virtual_allocation_list;  ///< A list of all virtual allocations.
    RmtResourceList          resource_list;            ///< A list of all resources.
    RmtPageTable             page_table;               ///< The page table at the point the snapshot was taken.
    RmtProcessMap            process_map;              ///< A map of processes seen.

    void* virtual_allocation_buffer;  ///< A pointer to the buffer allocated for the virtual allocation list.
    void* resource_list_buffer;       ///< A pointer to the buffer allocated for the resource list.

    RmtMemoryRegion* region_stack_buffer;
    int32_t          region_stack_count;

} RmtDataSnapshot;

/// Destroy a snapshot.
///
/// @param [in]  snapshot                       The snapshot to destroy.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>snapshot</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorMalformedData                      The operation failed due to the <c><i>snapshot</i></c> data set being <c><i>NULL</i></c>.
RmtErrorCode RmtDataSnapshotDestroy(RmtDataSnapshot* snapshot);

/// Debugging function.
RmtErrorCode RmtSnapshotDumpStateToConsole(const RmtDataSnapshot* snapshot);

/// Generate a resource history for a specific resource from a snapshot.
///
/// @param [in]  snapshot                       The snapshot containing the resource.
/// @param [in]  resource                       The resource to retrieve the history from.
/// @param [out] out_resource_history           Pointer to an <c><i>RmtResourceHistory</i></c> structure to fill.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>snapshot</i></c> , <c><i>resource</i></c> or
///                                             <c><i>out_resource_history</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtDataSnapshotGenerateResourceHistory(RmtDataSnapshot* snapshot, const RmtResource* resource, RmtResourceHistory* out_resource_history);

/// Get the largest resource size (in bytes) seen in a snapshot.
///
/// @param [in]  snapshot                           The snapshot to retrieve the largest resource size from.
///
/// @returns
/// The largest resource size seen in a snapshot.
uint64_t RmtDataSnapshotGetLargestResourceSize(const RmtDataSnapshot* snapshot);

/// Get the largest unbound resource size (in bytes) seen in a snapshot.
///
/// @param [in]  snapshot                           The snapshot to retrieve the largest resource size from.
///
/// @returns
/// The largest unbound resource size seen in a snapshot.
uint64_t RmtDataSnapshotGetLargestUnboundResourceSize(const RmtDataSnapshot* snapshot);

/// Get the smallest resource size (in bytes) seen in a snapshot.
///
/// @param [in]  snapshot                           The snapshot to retrieve the smallest resource size from.
///
/// @returns
/// The smallest resource size seen in a snapshot.
uint64_t RmtDataSnapshotGetSmallestResourceSize(const RmtDataSnapshot* snapshot);

/// Get the segment status for a specific heap type.
///
/// @param [in]  snapshot                           The snapshot to retrieve the status from.
/// @param [in]  heap_type                          The type of heap to get the status for.
/// @param [out] out_segment_status                 A pointer to a <c><i>RmtSegmentStatus</i></c> structure to fill.
///
/// @retval
/// kRmtOk                          The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer         The operation failed because <c><i>snapshot</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtDataSnapshotGetSegmentStatus(const RmtDataSnapshot* snapshot, RmtHeapType heap_type, RmtSegmentStatus* out_segment_status);

/// Get the segment that an address is in.
///
/// @param [in] snapshot                            The snapshot to retrieve the status from.
/// @param [in] gpu_address                         The address to calculate the segment for.
///
/// @returns
/// The heap type where the physical address resides.
RmtHeapType RmtDataSnapshotGetSegmentForAddress(const RmtDataSnapshot* snapshot, RmtGpuAddress gpu_address);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_DATA_SNAPSHOT_H_
