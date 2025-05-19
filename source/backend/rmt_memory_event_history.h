//=============================================================================
// Copyright (c) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Memory Event History API.
//=============================================================================

#ifndef RMV_MEMORY_EVENT_HISTORY_H_
#define RMV_MEMORY_EVENT_HISTORY_H_

#include <math.h>

#include "rmt_data_set.h"
#include "rmt_resource_history.h"
#include "rmt_resource_list.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

#define RMT_MEMORY_EVENT_HISTORY_API_MAJOR_VERSION 1  ///< Major version number.
#define RMT_MEMORY_EVENT_HISTORY_API_MINOR_VERSION 0  ///< Minor version number.
#define RMT_MEMORY_EVENT_HISTORY_API_PATCH_NUMBER 0   ///< Patch number.

typedef size_t RmtMemoryEventHistoryEventIndex;  ///< Type used for indexing of events in history.
typedef void*  RmtMemoryEventHistoryHandle;      ///< Type used to track instances of event history.

static const RmtMemoryEventHistoryEventIndex kRmtMemoryEventHistoryInvalidEventIndex = UINT64_MAX;

/// An enumeration of the different filtering types that can be used to generate the event history.
typedef enum RmtMemoryEventHistoryFilterType
{
    kRmtMemoryEventHistoryFilterTypeUnknown,                            ///< Unitialized filter type.
    kRmtMemoryEventHistoryFilterTypeResourceHistoryForId,               ///< Filter resource events for a specified resource identifier.
    kRmtMemoryEventHistoryFilterTypeFullAllcationHistory,               ///< Filter all virtual allocation related events for a virtual memory address.
    kRmtMemoryEventHistoryFilterTypeBasicAllocationHistory,             ///< Filter virtual allocation and free events for a specified virtual memory address.
    kRmtMemoryEventHistoryFilterTypeResourceCreationHistoryForAddress,  ///< Filter resource create events for a specified virtual address.
    kRmtMemoryEventHistoryFilterTypeAllResources                        ///< Filter resource events for all resources.
} RmtMemoryEventHistoryFilterType;

/// A structure that describes the parmeters that were used to generate the event history.
typedef struct RmtMemoryEventHistoryUsageParameters
{
    RmtMemoryEventHistoryFilterType filter_type;          ///< The filtering type to used.
    RmtResourceIdentifier           resource_identifier;  ///< The resource ID used when filtering history on resource related events.
    RmtGpuAddress                   virtual_address;      ///< The virtual address used when filtering history on virtual memory related events.
    bool hide_duplicate_page_table_events;  ///< A flag that indicates duplicate consecutive page table update events should not be included in the history.
    bool
        include_resources_in_all_allocations;  ///< A flag that indicates all resources bound to matching virtual allocations should be included in the history.
} RmtMemoryEventHistoryUsageParameters;

/// Physical memory mapping properties for event history (Note: this structure is used by multiple event types).
typedef struct RmtMemoryEventHistoryCommonPhysicalMappingEventInfo
{
    RmtGpuAddress          virtual_address;   ///< The virtual address of the allocation being mapped.
    RmtGpuAddress          physical_address;  ///< The physical address of the allocation being mapped.
    uint64_t               size_in_pages;     ///< The size of the mapping in pages.
    RmtPageSize            page_size;         ///< The page size for the mapping.
    RmtPageTableUpdateType update_type;       ///< The type of the page table update.
    RmtPageTableController controller;        ///< The type of system controlling page table updates.
} RmtMemoryEventHistoryCommonPhysicalMappingEventInfo;

/// Physical memory mapped to local properties for event history.
typedef RmtMemoryEventHistoryCommonPhysicalMappingEventInfo RmtMemoryEventHistoryPhysicalMapToLocalEventInfo;

/// Physical memory mapped to host properties for event history.
typedef RmtMemoryEventHistoryCommonPhysicalMappingEventInfo RmtMemoryEventHistoryPhysicalMapToHostEventInfo;

/// Physical memory unmapped properties for event history.
typedef RmtMemoryEventHistoryCommonPhysicalMappingEventInfo RmtMemoryEventHistoryPhysicalUnmapEventInfo;

/// Resource bind properties for event history.
typedef struct RmtMemoryEventHistoryResourceBindEventInfo
{
    RmtResourceIdentifier resource_identifier;                         ///< The resource ID associated with this event.
    RmtGpuAddress         virtual_address;                             ///< The virtual address that the resource is being bound to.
    RmtGpuAddress         resource_bound_allocation;                   ///< The virtual address of the allocation that this resource is bound to.
    uint64_t              size_in_bytes;                               ///< The size of the resource in bytes.
    bool                  is_system_memory;                            ///< A boolean value indicates if the bind is in system memory.
    RmtHeapType           heap_preferences[RMT_NUM_HEAP_PREFERENCES];  ///< An ordered list of heap preferences for the allocation.
} RmtMemoryEventHistoryResourceBindEventInfo;

/// Resource create properties for event history.
typedef struct RmtMemoryEventHistoryResourceCreateEventInfo
{
    RmtResourceIdentifier resource_identifier;  ///< The resource ID associated with this event.
    const char*           name;                 ///< The name of the resource (Note: the memory is owned by the history instance).
    RmtOwnerType          owner_type;           ///< The part of the software stack creating this resource.
    RmtCommitType         commit_type;          ///< The type of commitment reqeuired for this resource.
    RmtResourceType       resource_type;        ///< The resource type.
    RmtResourceUsageType  resource_usage_type;  ///< The resource usage type.
    bool                  is_implicit;          ///< A flag that indicates, if true, that the resource has been marked as implicitly created.
    union
    {
        RmtResourceDescriptionImage    image;      ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeImage</i></c>.
        RmtResourceDescriptionBuffer   buffer;     ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeBuffer</i></c>.
        RmtResourceDescriptionGpuEvent gpu_event;  ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeGpuEvent</i></c>.
        RmtResourceDescriptionBorderColorPalette
            border_color_palette;                              ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeBorderColorPalette</i></c>.
        RmtResourceDescriptionPerfExperiment perf_experiment;  ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypePerfExperiment</i></c>.
        RmtResourceDescriptionQueryHeap      query_heap;       ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeQueryHeap</i></c>.
        RmtResourceDescriptionPipeline       pipeline;         ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypePipeline</i></c>.
        RmtResourceDescriptionVideoDecoder   video_decoder;    ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeVideoDecoder</i></c>.
        RmtResourceDescriptionVideoEncoder   video_encoder;    ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeVideoEncoder</i></c>.
        RmtResourceDescriptionHeap           heap;             ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeHeap</i></c>.
        RmtResourceDescriptionDescriptorHeap descriptor_heap;  ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeDescriptorHeap</i></c>.
        RmtResourceDescriptionDescriptorPool descriptor_pool;  ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeDescriptorPool</i></c>.
        RmtResourceDescriptionCommandAllocator
                                           command_allocator;  ///< Valid when <c><i>resourceType</i></c> is <c><i>RMT_RESOURCE_TYPE_COMMAND_ALLOCATOR</i></c>.
        RmtResourceDescriptionMiscInternal misc_internal;      ///< Valid when <c><i>resourceType</i></c> is <c><i>RMT_RESOURCE_TYPE_MISC_INTERNAL</i></c>.
    };

} RmtMemoryEventHistoryResourceCreateEventInfo;

/// Resource destroyed properties for event history.
typedef struct RmtMemoryEventHistoryResourceDestroyEventInfo
{
    RmtResourceIdentifier resource_identifier;  ///< The resource ID associated with this event.
} RmtMemoryEventHistoryResourceDestroyEventInfo;

/// Virtual memory allocation properties for event history.
typedef struct RmtMemoryEventHistoryVirtualMemoryAllocationEventInfo
{
    RmtGpuAddress virtual_address;                       ///< The virtual address that was allocated.
    uint64_t      size_in_bytes;                         ///< The size (in bytes) of the allocation.
    RmtOwnerType  owner_type;                            ///< The owner of the allocation.
    RmtHeapType   preference[RMT_NUM_HEAP_PREFERENCES];  ///< An ordered list of heap preferences for the allocation.
    bool          is_external;                           ///< If true, indicates externally owned allocation opened by target application.  False, otherwise.
} RmtMemoryEventHistoryVirtualMemoryAllocationEventInfo;

/// Virtual memory freed properties for event history.
typedef struct RmtMemoryEventHistoryVirtualMemoryFreeEventInfo
{
    RmtGpuAddress virtual_address;  ///< The virtual or physical address being freed.
} RmtMemoryEventHistoryVirtualMemoryFreeEventInfo;

/// Virtual memory mapped properties for event history (Note: this structure is used by multiple event types).
typedef struct RmtMemoryEventHistoryCommonVirtualMemoryMappingEventInfo
{
    RmtGpuAddress virtual_address;  ///< The virtual address that was mapped or unmapped for CPU access.
} RmtMemoryEventHistoryCommonVirtualMemoryMappingEventInfo;

/// Virtual memory mapped properties for event history.
typedef RmtMemoryEventHistoryCommonVirtualMemoryMappingEventInfo RmtMemoryEventHistoryVirtualMemoryMapEventInfo;

/// Virtual memory unmapped properties for event history.
typedef RmtMemoryEventHistoryCommonVirtualMemoryMappingEventInfo RmtMemoryEventHistoryVirtualMemoryUnmapEventInfo;

/// Swap virtual memory properies for event history (Note: this structure is used by multiple event types).
typedef struct RmtMemoryEventHistoryCommonVirtualMemorySwappingEventInfo
{
    RmtGpuAddress          virtual_address;            ///< The virtual address of the memory where the residency or eviction update was requested.
    RmtQueue               queue;                      ///< The queue where the reference was added or removed.
    size_t                 resource_count;             ///< The number of resources affected by this event.
    RmtResourceIdentifier* resource_identifier_array;  ///< The list of resource Ids for the resources affected by this event.
} RmtMemoryEventHistoryCommonVirtualMemorySwappingEventInfo;

/// Virtual memory evicted properties for event history.
typedef RmtMemoryEventHistoryCommonVirtualMemorySwappingEventInfo RmtMemoryEventHistoryVirtualMemoryEvictEventInfo;

/// Virtual memory made resident properties for event history.
typedef RmtMemoryEventHistoryCommonVirtualMemorySwappingEventInfo RmtMemoryEventHistoryVirtualMemoryResidentEventInfo;

/// Locate an event in history with a matching event type.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index_start               The index of an event in the history to begin looking for a matching event type.
/// @param [in]  event_type_to_match             The event type to search for.
/// @param [out] out_event_index                 A pointer to the index that receives the matching event type or <c><i>kRmtMemoryEventHistoryInvalidEventIndex</i></c> if a match is not found.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index_start/i></c>.
RmtErrorCode RmtMemoryEventHistoryFindNextEventIndex(const RmtMemoryEventHistoryHandle     history_handle,
                                                     const RmtMemoryEventHistoryEventIndex event_index_start,
                                                     const RmtResourceHistoryEventType     event_type_to_match,
                                                     RmtMemoryEventHistoryEventIndex*      out_event_index);

/// Release memory resources associated with an instance of history.
///
/// @param [in]  out_history_handle              A pointer to an instance handle for a previously generated history of events.
///                                              The handle is set to null after history has been deleted.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>out_history_handle</i></c>.
RmtErrorCode RmtMemoryEventHistoryFreeHistory(RmtMemoryEventHistoryHandle* out_history_handle);

/// Retrieve the version information for the event history API
///
/// @param [out] out_major_version               A pointer to the variable that receives the major version number of the API.
/// @param [out] out_minor_version               A pointer to the variable that receives the minor version number of the API.
/// @param [out] out_patch_version               A pointer to the variable that receives the patch number of the API.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      One or more of the parameters are invalid pointers.
RmtErrorCode RmtMemoryEventHistoryGetApiVersion(int* out_major_version, int* out_minor_version, int* out_patch_version);

/// Retrieve the number of events in a history instance.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [out] out_event_count                 A pointer to the variable that receives the number of events in the history.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
RmtErrorCode RmtMemoryEventHistoryGetEventCount(const RmtMemoryEventHistoryHandle history_handle, size_t* out_event_count);

/// Retrieve the timestamp for an event in a history instance.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve the timestamp for.
/// @param [out] out_event_timestamp             A pointer to the variable that receives the event timestamp (in CPU clock ticks).
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
RmtErrorCode RmtMemoryEventHistoryGetEventTimestamp(const RmtMemoryEventHistoryHandle     history_handle,
                                                    const RmtMemoryEventHistoryEventIndex event_index,
                                                    uint64_t*                             out_event_timestamp);

/// Retrieve the event type for an event in a history instance.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve the type for.
/// @param [out] out_event_type                  A pointer to the variable that receives the event type.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
RmtErrorCode RmtMemoryEventHistoryGetEventType(const RmtMemoryEventHistoryHandle     history_handle,
                                               const RmtMemoryEventHistoryEventIndex event_index,
                                               RmtResourceHistoryEventType*          out_event_type);

/// Retrieves a structure containing the parameters used for generating a history of events.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [out  out_parameters                  A pointer to the structure containing the parameters used when generating the history.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
RmtErrorCode RmtMemoryEventHistoryGetUsageParameters(const RmtMemoryEventHistoryHandle            history_handle,
                                                     const RmtMemoryEventHistoryUsageParameters** out_parameters);

/// Retrieve information for a Physical Memory Map to Host event.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve information from.
/// @param [out] out_event_info                  Receives a pointer to a <c><i>RmtMemoryEventHistoryPhysicalMapToHostEventInfo</i></c> structure containing the information for the event.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
/// @retval
/// kRmtMalformedData                            The event type for the requested event information at this index doesn't match.
RmtErrorCode RmtMemoryEventHistoryGetPhysicalMapToHostEventInfo(const RmtMemoryEventHistoryHandle                       history_handle,
                                                                const RmtMemoryEventHistoryEventIndex                   event_index,
                                                                const RmtMemoryEventHistoryPhysicalMapToHostEventInfo** out_event_info);

/// Retrieve information for a Physical Memory Map to Local event.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve information from.
/// @param [out] out_event_info                  Receives a pointer to a <c><i>RmtMemoryEventHistoryPhysicalMapToLocalEventInfo</i></c> structure containing the information for the event.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
/// @retval
/// kRmtMalformedData                            The event type for the requested event information at this index doesn't match.
RmtErrorCode RmtMemoryEventHistoryGetPhysicalMemoryMapToLocalEventInfo(const RmtMemoryEventHistoryHandle                        history_handle,
                                                                       const RmtMemoryEventHistoryEventIndex                    event_index,
                                                                       const RmtMemoryEventHistoryPhysicalMapToLocalEventInfo** out_event_info);

/// Retrieve information for a Physical Memory Unmap event.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve information from.
/// @param [out] out_event_info                  Receives a pointer to a <c><i>RmtMemoryEventHistoryPhysicalUnmapEventInfo</i></c> structure containing the information for the event.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
/// @retval
/// kRmtMalformedData                            The event type for the requested event information at this index doesn't match.
RmtErrorCode RmtMemoryEventHistoryGetPhysicalUnmapEventInfo(const RmtMemoryEventHistoryHandle                   history_handle,
                                                            const RmtMemoryEventHistoryEventIndex               event_index,
                                                            const RmtMemoryEventHistoryPhysicalUnmapEventInfo** out_event_info);

/// Retrieve information for a Resource Bind event.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve information from.
/// @param [out] out_event_info                  Receives a pointer to a <c><i>RmtMemoryEventHistoryResourceBindEventInfo</i></c> structure containing the information for the event.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
/// @retval
/// kRmtMalformedData                            The event type for the requested event information at this index doesn't match.
RmtErrorCode RmtMemoryEventHistoryGetResourceBindEventInfo(const RmtMemoryEventHistoryHandle                  history_handle,
                                                           const RmtMemoryEventHistoryEventIndex              event_index,
                                                           const RmtMemoryEventHistoryResourceBindEventInfo** out_event_info);

/// Retrieve information for a Resource Create event.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve information from.
/// @param [out] out_event_info                  Receives a pointer to a <c><i>RmtMemoryEventHistoryResourceCreateEventInfo</i></c> structure containing the information for the event.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
/// @retval
/// kRmtMalformedData                            The event type for the requested event information at this index doesn't match.
RmtErrorCode RmtMemoryEventHistoryGetResourceCreateEventInfo(const RmtMemoryEventHistoryHandle                    history_handle,
                                                             const RmtMemoryEventHistoryEventIndex                event_index,
                                                             const RmtMemoryEventHistoryResourceCreateEventInfo** out_event_info);

/// Retrieve information for a Resource Destroy event.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve information from.
/// @param [out] out_event_info                  Receives a pointer to a <c><i>RmtMemoryEventHistoryResourceDestroyEventInfo</i></c> structure containing the information for the event.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
/// @retval
/// kRmtMalformedData                            The event type for the requested event information at this index doesn't match.
RmtErrorCode RmtMemoryEventHistoryGetResourceDestroyEventInfo(const RmtMemoryEventHistoryHandle                     history_handle,
                                                              const RmtMemoryEventHistoryEventIndex                 event_index,
                                                              const RmtMemoryEventHistoryResourceDestroyEventInfo** out_event_info);

/// Retrieve information for a Virtual Memory Allocation event.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve information from.
/// @param [out] out_event_info                  Receives a pointer to a <c><i>RmtMemoryEventHistoryVirtualMemoryAllocationEventInfo</i></c> structure containing the information for the event.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
/// @retval
/// kRmtMalformedData                            The event type for the requested event information at this index doesn't match.
RmtErrorCode RmtMemoryEventHistoryGetVirtualMemoryAllocationEventInfo(const RmtMemoryEventHistoryHandle                             history_handle,
                                                                      const RmtMemoryEventHistoryEventIndex                         event_index,
                                                                      const RmtMemoryEventHistoryVirtualMemoryAllocationEventInfo** out_event_info);

/// Retrieve information for a Virtual Memory Free event.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve information from.
/// @param [out] out_event_info                  Receives a pointer to a <c><i>RmtMemoryEventHistoryEventVirtualMemoryFreeEventInfo</i></c> structure containing the information for the event.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
/// @retval
/// kRmtMalformedData                            The event type for the requested event information at this index doesn't match.
RmtErrorCode RmtMemoryEventHistoryGetVirtualMemoryFreeEventInfo(const RmtMemoryEventHistoryHandle                       history_handle,
                                                                const RmtMemoryEventHistoryEventIndex                   event_index,
                                                                const RmtMemoryEventHistoryVirtualMemoryFreeEventInfo** out_event_info);

/// Retrieve information for a Virtual Memory Evict event.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve information from.
/// @param [out] out_event_info                  Receives a pointer to a <c><i>RmtMemoryEventHistoryVirtualMemoryEvictEventInfo</i></c> structure containing the information for the event.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
/// @retval
/// kRmtMalformedData                            The event type for the requested event information at this index doesn't match.
RmtErrorCode RmtMemoryEventHistoryGetVirtualMemoryEvictEventInfo(const RmtMemoryEventHistoryHandle                        history_handle,
                                                                 const RmtMemoryEventHistoryEventIndex                    event_index,
                                                                 const RmtMemoryEventHistoryVirtualMemoryEvictEventInfo** out_event_info);

/// Retrieve information for a Virtual Memory Make Resident event.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve information from.
/// @param [out] out_event_info                  Receives a pointer to a <c><i>RmtMemoryEventHistoryVirtualMemoryResidentEventInfo</i></c> structure containing the information for the event.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
/// @retval
/// kRmtMalformedData                            The event type for the requested event information at this index doesn't match.
RmtErrorCode RmtMemoryEventHistoryGetVirtualMemoryMakeResidentEventInfo(const RmtMemoryEventHistoryHandle                           history_handle,
                                                                        const RmtMemoryEventHistoryEventIndex                       event_index,
                                                                        const RmtMemoryEventHistoryVirtualMemoryResidentEventInfo** out_event_info);

/// Retrieve information for a Virtual Memory Map event.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve information from.
/// @param [out] out_event_info                  Receives a pointer to a <c><i>RmtMemoryEventHistoryVirtualMemoryMapEventInfo</i></c> structure containing the information for the event.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
/// @retval
/// kRmtMalformedData                            The event type for the requested event information at this index doesn't match.
RmtErrorCode RmtMemoryEventHistoryGetVirtualMemoryMapEventInfo(const RmtMemoryEventHistoryHandle                      history_handle,
                                                               const RmtMemoryEventHistoryEventIndex                  event_index,
                                                               const RmtMemoryEventHistoryVirtualMemoryMapEventInfo** out_event_info);

/// Retrieve information for a Virtual Memory Unmap event.
///
/// @param [in]  history_handle                  The instance handle for a previously generated history of events.
/// @param [in]  event_index                     The index of the event to retrieve information from.
/// @param [out] out_event_info                  Receives a pointer to a <c><i>RmtMemoryEventHistoryVirtualMemoryUnmapEventInfo</i></c> structure containing the information for the event.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      Invalid <c><i>history_handle</i></c>.
/// @retval
/// kRmtErrorIndexOutOfRange                     Invalid <c><i>event_index/i></c>.
/// @retval
/// kRmtMalformedData                            The event type for the requested event information at this index doesn't match.
RmtErrorCode RmtMemoryEventHistoryGetVirtualMemoryUnmapEventInfo(const RmtMemoryEventHistoryHandle                        history_handle,
                                                                 const RmtMemoryEventHistoryEventIndex                    event_index,
                                                                 const RmtMemoryEventHistoryVirtualMemoryUnmapEventInfo** out_event_info);

/// @brief Generate history of events associated with a resource identifier.
///
/// When the event history object instantiated by this function is no longer needed,
/// the user should delete it by calling the RmtMemoryEventHistoryFreeHistory() API function.
///
/// @param [in]  data_set                             A pointer to the data set containing the events.
/// @param [in]  resource_id                          A resource identifier related to the history of events to be retrieved.
/// @param [in]  hide_duplicate_page_table_events     A flag that indicates duplicate consecutive page table update events should not be included in the history.
/// @param [out] out_history_handle                   A pointer that receives the instance handle for the generated history of events.
///
/// @retval
/// kRmtOk                                            The operation completed successfully.
/// @retval
/// kRmtErrorNoResourceFound                          The resource identifier could not be found.
/// @retval
/// kRmtErrorInvalidPointer                           The operation failed because the <c><i>data_set</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtMemoryEventHistoryGenerateResourceHistoryForId(RmtDataSet*                  data_set,
                                                               const RmtResourceIdentifier  resource_identifier,
                                                               bool                         hide_duplicate_page_table_events,
                                                               RmtMemoryEventHistoryHandle* out_history_handle);

/// @brief Generate history of resource creation events bound to a virtual address.
///
/// When the event history object instantiated by this function is no longer needed,
/// the user should delete it by calling the RmtMemoryEventHistoryFreeHistory() API function.
///
/// @param [in]  data_set                             A pointer to the data set containing the events.
/// @param [in]  virtual_address                      An address contained within a virtual memory allocation block for the resource events to be included in the history.
/// @param [in]  include_resources_in_all_allocations A flag that indicates all resources bound to virtual allocations that overlap with <c><i>virtual_address</i></c> should be included in the history.
/// @param [out] out_history_handle                   A pointer that receives the instance handle for the generated history of events.
///
/// @retval
/// kRmtOk                                            The operation completed successfully.
/// @retval
/// kRmtErrorNoResourceFound                          The resource identifier could not be found.
/// @retval
/// kRmtErrorInvalidPointer                           The operation failed because the <c><i>data_set</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtMemoryEventHistoryGenerateResourceCreateHistoryForAddress(RmtDataSet*                  data_set,
                                                                          const RmtGpuAddress          virtual_address,
                                                                          const bool                   hide_duplicate_page_table_events,
                                                                          RmtMemoryEventHistoryHandle* out_history_handle);

/// @brief Generate history of events associated with a virtual memory address.
///
/// Note that Make Resident and Evict events are only included if the virtual allocation overlaps
/// with the specified virtual_address parameter.  In order to obtain all Make Resident and
/// Evict events for resources that have been rebound to other virtual allocations, use
/// the RmtMemoryEventHistoryGenerateResourceHistoryForId() History API call with the resource ID
/// of interest specified after making the call to RmtMemoryEventHistoryGenerateFullAllocationHistory().
///
/// When the event history object instantiated by this function is no longer needed,
/// the user should delete it by calling the RmtMemoryEventHistoryFreeHistory() API function.
///
/// @param [in]  data_set                             A pointer to the dataset containing the events.
/// @param [in]  virtual_address                      An address contained within a virtual memory allocation block for which the event history is to be retrieved.
/// @param [in]  hide_duplicate_page_table_events     A flag that indicates duplicate consecutive page table update events should not be included in the history.
/// @param [in]  include_resources_in_all_allocations A flag that indicates all resources bound to virtual allocations that overlap with <c><i>virtual_address</i></c> should be included in the history.
/// @param [out] out_history_handle                   A pointer that receives the instance handle for the generated history of events.
///
/// @retval
/// kRmtOk                                            The operation completed successfully.
/// @retval
/// kRmtErrorNoAllocationFound                        An allocation containing the virtual address couldn't be found.
/// @retval
/// kRmtErrorInvalidPointer                           The operation failed because the <c><i>data_set</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtMemoryEventHistoryGenerateFullAllocationHistory(RmtDataSet*                  data_set,
                                                                const RmtGpuAddress          virtual_address,
                                                                bool                         hide_duplicate_page_table_events,
                                                                bool                         include_resources_in_all_allocations,
                                                                RmtMemoryEventHistoryHandle* out_history_handle);

/// @brief Retrieve history of events for virtual memory when it is allocated and freed.
///
/// When the event history object instantiated by this function is no longer needed,
/// the user should delete it by calling the RmtMemoryEventHistoryFreeHistory() API function.
///
/// @param [in]  data_set                        A pointer to the memory trace data containing the virtual memory allocations.
/// @param [in]  virtual_address                 The virtual address to search for in the list of virtual allocations.
/// @param [out] out_history_handle              A pointer that receives the instance handle for the generated history of events.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorNoAllocationFound                   An allocation containing the virtual address couldn't be found.
/// @retval
/// kRmtErrorInvalidPointer                      The operation failed because the <c><i>data_set</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtMemoryEventHistoryGenerateBasicAllocationHistory(RmtDataSet*                  data_set,
                                                                 const RmtGpuAddress          virtual_address,
                                                                 RmtMemoryEventHistoryHandle* out_history_handle);

/// Retrieve history of events for all resources.
///
/// @param [in]  data_set                        A pointer to the memory trace data containing the virtual memory allocations.
/// @param [out] out_history_handle              A pointer that receives the instance handle for the generated history of events.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      The operation failed because the <c><i>data_set</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtMemoryEventHistoryGenerateHistoryForAllResources(RmtDataSet* data_set, RmtMemoryEventHistoryHandle* out_history_handle);

/// @brief Set or clear a global flag that controls whether implicit resources are filtered from generated history instances.
///
/// @param [in]  enable_filtering                A flag that determines whether implicit resources should be filtered from history.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
RmtErrorCode RmtMemoryEventHistorySetImplicitResourceFiltering(const bool enable_filtering);

/// @brief Get the global flag value that controls whether implicit resources are filtered from generated history instances.
///
/// @param [out]  out_enable_filtering           The current setting for the flag that determines whether implicit resources should be filtered from history.
///
/// @retval
/// kRmtOk                                       The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                      The operation failed because the <c><i>out_enable_filtering</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtMemoryEventHistoryGetImplicitResourceFiltering(bool* out_enable_filtering);
#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus

#endif  // #ifndef RMV_MEMORY_EVENT_HISTORY_H_
