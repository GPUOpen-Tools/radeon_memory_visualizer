//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Structures and functions for working with a resource history.
//=============================================================================

#ifndef RMV_BACKEND_RMT_RESOURCE_HISTORY_H_
#define RMV_BACKEND_RMT_RESOURCE_HISTORY_H_

#include "rmt_types.h"
#include "rmt_error.h"
#include "rmt_configuration.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

typedef struct RmtResource          RmtResource;
typedef struct RmtVirtualAllocation RmtVirtualAllocation;

/// An enumeration of all resource history event types.
typedef enum RmtResourceHistoryEventType
{
    kRmtResourceHistoryEventSnapshotTaken             = -1,  ///< A snapshot was taken
    kRmtResourceHistoryEventResourceCreated           = 0,   ///< A resource was created.
    kRmtResourceHistoryEventResourceDestroyed         = 1,   ///< A resource was destroyed.
    kRmtResourceHistoryEventResourceBound             = 2,   ///< A resource was bound to a virtual address range.
    kRmtResourceHistoryEventVirtualMemoryAllocated    = 3,   ///< The virtual memory backing the resource was allocated.
    kRmtResourceHistoryEventVirtualMemoryFree         = 4,   ///< The virtual memory backing the resource was freed.
    kRmtResourceHistoryEventVirtualMemoryMapped       = 5,   ///< The virtual memory backing the resource was CPU mapped.
    kRmtResourceHistoryEventVirtualMemoryUnmapped     = 6,   ///< The virtual memory backing the resource was CPU unmapped.
    kRmtResourceHistoryEventVirtualMemoryMakeResident = 7,   ///< The virtual memory backing the resource was requested to be made resident.
    kRmtResourceHistoryEventVirtualMemoryEvict        = 8,   ///< The virtual memory backing the resource was requested to be evicted.
    kRmtResourceHistoryEventBackingMemoryPaged        = 9,   ///< Some or all of the backing memory was paged from one memory type to another.
    kRmtResourceHistoryEventPhysicalMapToLocal        = 10,  ///< Some or all of the physical memory backing this resource was mapped.
    kRmtResourceHistoryEventPhysicalUnmap             = 11,  ///< Some or all of the physical memory backing this resource was unmapped.
    kRmtResourceHistoryEventPhysicalMapToHost         = 12,  ///< Some or all of the physical memory backing this resource was paged to local.

} RmtResourceHistoryEventType;

/// A structure encapsulating a single event in the resource history.
typedef struct RmtResourceHistoryEvent
{
    uint64_t                    timestamp;           ///< The time at which the event occurred.
    uint64_t                    thread_id;           ///< The CPU thread on which the event occurred.
    uint64_t                    virtual_address;     ///< The virtual address of the event, if applicable.
    uint64_t                    physical_address;    ///< The physical address of the event, if applicable.
    uint64_t                    size_in_bytes;       ///< The size of the event, in bytes, if applicable.
    RmtResourceHistoryEventType event_type;          ///< The type of resource history event that occurred.
    uint32_t                    page_size_in_bytes;  ///< The page size, in bytes, if applicable.
} RmtResourceHistoryEvent;

/// A structure encapsulating an address range.
typedef struct RmtResourceAddressRange
{
    RmtGpuAddress address;        ///< The address of the address range.
    uint64_t      size_in_bytes;  ///< The size of the range in bytes.
} RmtResourceAddressRange;

/// A structure encapsulating the history of a resource.
typedef struct RmtResourceHistory
{
    const RmtResource*          resource;                                     ///< The resource the history pertains to.
    RmtResourceHistoryEvent     events[RMT_MAXIMUM_RESOURCE_HISTORY_EVENTS];  ///< A pointer to an array of <c><i>RmtResourceHistoryEvent</i></c> structures.
    int32_t                     event_count;      ///< The number of <c><i>RmtResourceHistoryEvent</i></c> structures pointed to by <c><i>events</i></c>.
    const RmtVirtualAllocation* base_allocation;  ///< A pointer to a <c><i>RmtVirtualAllocation</i></c> structure that underpins <c><i>resource</i></c>.

} RmtResourceHistory;

/// Add a new event to the resource history.
///
/// @param [in] resource_history                    A pointer to a <c><i>RmtResourceHistory</i></c> structure to add the event to.
/// @param [in] event_type                          The type of event that occurred.
/// @param [in] thread_id                           The CPU thread ID where the event occurred.
/// @param [in] timestamp                           The time at which the event occurred.
/// @param [in] virtual_address                     The virtual address of the event, if applicable.
/// @param [in] physical_address                    The physical address of the event, if applicable.
/// @param [in] size_in_bytes                       The size, in bytes of the event, if applicable.
/// @param [in] page_size_in_bytes                  The page size, in bytes of the event, if applicable.
/// @param [in] compact                             If true, ignore identical sequential events.
///
/// @retval
/// kRmtOk                                          The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                         The operation failed because <c><i>resourceHistory</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtResourceHistoryAddEvent(RmtResourceHistory*         resource_history,
                                        RmtResourceHistoryEventType event_type,
                                        uint64_t                    thread_id,
                                        uint64_t                    timestamp,
                                        uint64_t                    virtual_address,
                                        uint64_t                    physical_address,
                                        uint64_t                    size_in_bytes,
                                        uint64_t                    page_size_in_bytes,
                                        bool                        compact);

/// Destroy the resource history data.
///
/// @param [in] resource_history                    A pointer to a <c><i>RmtResourceHistory</i></c> structure to destroy.
///
/// @retval
/// kRmtOk                                          The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                         The operation failed because <c><i>resourceHistory</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtResourceHistoryDestroy(RmtResourceHistory* resource_history);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_RESOURCE_HISTORY_H_
