//=============================================================================
// Copyright (c) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the Memory Event History API.
//=============================================================================

#include "rmt_memory_event_history.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "rmt_address_helper.h"
#include "rmt_assert.h"
#include "rmt_memory_event_history_impl.h"
#include "rmt_resource_list.h"
#include "rmt_resource_userdata.h"

// A flag used to determine if implicit resources should be included in generated history instances.
static bool hide_implicit_resources = false;

// Memory Event History API Functions

RmtErrorCode RmtMemoryEventHistorySetImplicitResourceFiltering(const bool enable_filtering)
{
    hide_implicit_resources = enable_filtering;
    return kRmtOk;
}

RmtErrorCode RmtMemoryEventHistoryGetImplicitResourceFiltering(bool* out_enable_filtering)
{
    RMT_RETURN_ON_ERROR(out_enable_filtering != nullptr, kRmtErrorInvalidPointer);

    *out_enable_filtering = hide_implicit_resources;

    return kRmtOk;
}

RmtErrorCode RmtMemoryEventHistoryFindNextEventIndex(const RmtMemoryEventHistoryHandle     history_handle,
                                                     const RmtMemoryEventHistoryEventIndex event_index_start,
                                                     const RmtResourceHistoryEventType     event_type_to_match,
                                                     RmtMemoryEventHistoryEventIndex*      out_event_index)
{
    RMT_RETURN_ON_ERROR(out_event_index != nullptr, kRmtErrorInvalidPointer);
    RmtErrorCode result = kRmtErrorInvalidPointer;

    *out_event_index                = kRmtMemoryEventHistoryInvalidEventIndex;
    const EventHistoryImpl* history = EventHistoryImpl::FromHandle(history_handle);

    if (history != nullptr)
    {
        result             = kRmtOk;
        size_t event_count = history->GetEventCount();
        for (RmtMemoryEventHistoryEventIndex index = event_index_start; index < event_count; index++)
        {
            RmtResourceHistoryEventType event_type;
            result = history->GetEventType(index, &event_type);
            if (result != kRmtOk)
            {
                break;
            }

            if (event_type == event_type_to_match)
            {
                *out_event_index = index;
                break;
            }
        }
    }

    return result;
}

RmtErrorCode RmtMemoryEventHistoryFreeHistory(RmtMemoryEventHistoryHandle* out_history_handle)
{
    RmtErrorCode result = kRmtErrorInvalidPointer;

    if (out_history_handle != nullptr)
    {
        const EventHistoryImpl* history = EventHistoryImpl::FromHandle(*out_history_handle);

        if (history != nullptr)
        {
            delete history;
            *out_history_handle = nullptr;
            result              = kRmtOk;
        }
    }

    return result;
}

RmtErrorCode RmtMemoryEventHistoryGetApiVersion(int* out_major_version, int* out_minor_version, int* out_patch_version)
{
    RMT_RETURN_ON_ERROR(out_major_version != nullptr, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_minor_version != nullptr, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_patch_version != nullptr, kRmtErrorInvalidPointer);

    *out_major_version = RMT_MEMORY_EVENT_HISTORY_API_MAJOR_VERSION;
    *out_minor_version = RMT_MEMORY_EVENT_HISTORY_API_MINOR_VERSION;
    *out_patch_version = RMT_MEMORY_EVENT_HISTORY_API_PATCH_NUMBER;

    return kRmtOk;
}

RmtErrorCode RmtMemoryEventHistoryGetEventCount(const RmtMemoryEventHistoryHandle history_handle, size_t* out_event_count)
{
    RmtErrorCode            result  = kRmtErrorInvalidPointer;
    const EventHistoryImpl* history = EventHistoryImpl::FromHandle(history_handle);

    if (history != nullptr)
    {
        *out_event_count = history->GetEventCount();
        result           = kRmtOk;
    }
    else
    {
        *out_event_count = 0;
    }

    return result;
}

RmtErrorCode RmtMemoryEventHistoryGetUsageParameters(const RmtMemoryEventHistoryHandle            history_handle,
                                                     const RmtMemoryEventHistoryUsageParameters** out_parameters)
{
    RmtErrorCode            result  = kRmtErrorInvalidPointer;
    const EventHistoryImpl* history = EventHistoryImpl::FromHandle(history_handle);

    if (history != nullptr)
    {
        *out_parameters = history->GetUsageParameters();
        result          = kRmtOk;
    }

    return result;
}

RmtErrorCode RmtMemoryEventHistoryGetEventTimestamp(const RmtMemoryEventHistoryHandle     history_handle,
                                                    const RmtMemoryEventHistoryEventIndex event_index,
                                                    uint64_t*                             out_event_timestamp)
{
    const EventHistoryImpl* history = EventHistoryImpl::FromHandle(history_handle);
    RMT_RETURN_ON_ERROR(history != nullptr, kRmtErrorInvalidPointer);

    return history->GetEventTimestamp(event_index, out_event_timestamp);
}

RmtErrorCode RmtMemoryEventHistoryGetEventType(const RmtMemoryEventHistoryHandle     history_handle,
                                               const RmtMemoryEventHistoryEventIndex event_index,
                                               RmtResourceHistoryEventType*          out_event_type)
{
    const EventHistoryImpl* history = EventHistoryImpl::FromHandle(history_handle);
    RMT_RETURN_ON_ERROR(history != nullptr, kRmtErrorInvalidPointer);

    return history->GetEventType(event_index, out_event_type);
}

RmtErrorCode RmtMemoryEventHistoryGetPhysicalMapToHostEventInfo(const RmtMemoryEventHistoryHandle                       history_handle,
                                                                const RmtMemoryEventHistoryEventIndex                   event_index,
                                                                const RmtMemoryEventHistoryPhysicalMapToHostEventInfo** out_event_info)
{
    return EventHistoryImpl::GetEventInfo(history_handle, event_index, RmtResourceHistoryEventType::kRmtResourceHistoryEventPhysicalMapToHost, out_event_info);
}

RmtErrorCode RmtMemoryEventHistoryGetPhysicalMemoryMapToLocalEventInfo(const RmtMemoryEventHistoryHandle                        history_handle,
                                                                       const RmtMemoryEventHistoryEventIndex                    event_index,
                                                                       const RmtMemoryEventHistoryPhysicalMapToLocalEventInfo** out_event_info)
{
    return EventHistoryImpl::GetEventInfo(history_handle, event_index, RmtResourceHistoryEventType::kRmtResourceHistoryEventPhysicalMapToLocal, out_event_info);
}

RmtErrorCode RmtMemoryEventHistoryGetPhysicalUnmapEventInfo(const RmtMemoryEventHistoryHandle                   history_handle,
                                                            const RmtMemoryEventHistoryEventIndex               event_index,
                                                            const RmtMemoryEventHistoryPhysicalUnmapEventInfo** out_event_info)
{
    return EventHistoryImpl::GetEventInfo(history_handle, event_index, RmtResourceHistoryEventType::kRmtResourceHistoryEventPhysicalUnmap, out_event_info);
}

RmtErrorCode RmtMemoryEventHistoryGetResourceBindEventInfo(const RmtMemoryEventHistoryHandle                  history_handle,
                                                           const RmtMemoryEventHistoryEventIndex              event_index,
                                                           const RmtMemoryEventHistoryResourceBindEventInfo** out_event_info)
{
    return EventHistoryImpl::GetEventInfo(history_handle, event_index, RmtResourceHistoryEventType::kRmtResourceHistoryEventResourceBound, out_event_info);
}

RmtErrorCode RmtMemoryEventHistoryGetResourceCreateEventInfo(const RmtMemoryEventHistoryHandle                    history_handle,
                                                             const RmtMemoryEventHistoryEventIndex                event_index,
                                                             const RmtMemoryEventHistoryResourceCreateEventInfo** out_event_info)
{
    return EventHistoryImpl::GetEventInfo(history_handle, event_index, RmtResourceHistoryEventType::kRmtResourceHistoryEventResourceCreated, out_event_info);
}

RmtErrorCode RmtMemoryEventHistoryGetResourceDestroyEventInfo(const RmtMemoryEventHistoryHandle                     history_handle,
                                                              const RmtMemoryEventHistoryEventIndex                 event_index,
                                                              const RmtMemoryEventHistoryResourceDestroyEventInfo** out_event_info)
{
    return EventHistoryImpl::GetEventInfo(history_handle, event_index, RmtResourceHistoryEventType::kRmtResourceHistoryEventResourceDestroyed, out_event_info);
}

RmtErrorCode RmtMemoryEventHistoryGetVirtualMemoryAllocationEventInfo(const RmtMemoryEventHistoryHandle                             history_handle,
                                                                      const RmtMemoryEventHistoryEventIndex                         event_index,
                                                                      const RmtMemoryEventHistoryVirtualMemoryAllocationEventInfo** out_event_info)
{
    return EventHistoryImpl::GetEventInfo(
        history_handle, event_index, RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryAllocated, out_event_info);
}

RmtErrorCode RmtMemoryEventHistoryGetVirtualMemoryFreeEventInfo(const RmtMemoryEventHistoryHandle                       history_handle,
                                                                const RmtMemoryEventHistoryEventIndex                   event_index,
                                                                const RmtMemoryEventHistoryVirtualMemoryFreeEventInfo** out_event_info)
{
    return EventHistoryImpl::GetEventInfo(history_handle, event_index, RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryFree, out_event_info);
}

RmtErrorCode RmtMemoryEventHistoryGetVirtualMemoryEvictEventInfo(const RmtMemoryEventHistoryHandle                        history_handle,
                                                                 const RmtMemoryEventHistoryEventIndex                    event_index,
                                                                 const RmtMemoryEventHistoryVirtualMemoryEvictEventInfo** out_event_info)
{
    return EventHistoryImpl::GetEventInfo(history_handle, event_index, RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryEvict, out_event_info);
}

RmtErrorCode RmtMemoryEventHistoryGetVirtualMemoryMakeResidentEventInfo(const RmtMemoryEventHistoryHandle                           history_handle,
                                                                        const RmtMemoryEventHistoryEventIndex                       event_index,
                                                                        const RmtMemoryEventHistoryVirtualMemoryResidentEventInfo** out_event_info)
{
    return EventHistoryImpl::GetEventInfo(
        history_handle, event_index, RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryMakeResident, out_event_info);
}

RmtErrorCode RmtMemoryEventHistoryGetVirtualMemoryMapEventInfo(const RmtMemoryEventHistoryHandle                      history_handle,
                                                               const RmtMemoryEventHistoryEventIndex                  event_index,
                                                               const RmtMemoryEventHistoryVirtualMemoryMapEventInfo** out_event_info)
{
    return EventHistoryImpl::GetEventInfo(
        history_handle, event_index, RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryMapped, out_event_info);
}

RmtErrorCode RmtMemoryEventHistoryGetVirtualMemoryUnmapEventInfo(const RmtMemoryEventHistoryHandle                        history_handle,
                                                                 const RmtMemoryEventHistoryEventIndex                    event_index,
                                                                 const RmtMemoryEventHistoryVirtualMemoryUnmapEventInfo** out_event_info)
{
    return EventHistoryImpl::GetEventInfo(
        history_handle, event_index, RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryUnmapped, out_event_info);
}

// Structure that defines a memory region.
typedef struct MemoryBlock
{
    RmtGpuAddress start_address;  ///< The starting address of the memory block.
    uint64_t      size;           ///< The size of the memory block.
} MemoryBlock;

// Type definition for a list of memory blocks.
typedef std::vector<MemoryBlock> MemoryBlockList;

// Structure used to determine if an address is located within a memory block range.
typedef struct RangeComparator
{
    // Constructor
    RangeComparator(const RmtGpuAddress address)
        : address_to_find(address)
    {
    }

    // Function operator
    bool operator()(const MemoryBlock& block)
    {
        // Check to see if the specified address is within the range of a memory block.
        return RmtAllocationsOverlap(address_to_find, 1, block.start_address, block.size);
    }

    RmtGpuAddress address_to_find;  ///< The address to look for in a memory block.
} RangeComparator;

// Structure used to determine if an address matches the start of a memory block.
typedef struct StartMatchComparator
{
    // Constructor
    StartMatchComparator(const RmtGpuAddress address)
        : address_to_find(address)
    {
    }

    // Function operator
    bool operator()(const MemoryBlock& block)
    {
        // Check to see if the specified address matches the start of a memory block.
        return (block.start_address == address_to_find);
    }

    RmtGpuAddress address_to_find;  ///< The address to compare against the start of a memory block.
} StartMatchComparator;

// Determine if an address matches the start of any memory block in a list.
static bool CheckVirtualMemoryAllocationMatch(const MemoryBlockList& allocations, RmtGpuAddress address)
{
    return (std::find_if(allocations.begin(), allocations.end(), StartMatchComparator(address)) != allocations.end());
}

// Determine if an address is located within any memory block range in a list.
static bool CheckVirtualMemoryBlockOverlap(const MemoryBlockList& allocations, RmtGpuAddress address)
{
    return (std::find_if(allocations.begin(), allocations.end(), RangeComparator(address)) != allocations.end());
}

// A structure used to hold virtual allocation related data.
typedef struct AllocationInfo
{
    RmtGpuAddress base_address;                                ///< Starting address of the allocation.
    RmtHeapType   heap_preferences[RMT_NUM_HEAP_PREFERENCES];  ///< The heap preferences for the virtual allocation.
} AllocationInfo;

RmtErrorCode RmtMemoryEventHistoryGenerateResourceHistoryForId(RmtDataSet*                  data_set,
                                                               const RmtResourceIdentifier  resource_identifier,
                                                               bool                         hide_duplicate_page_table_events,
                                                               RmtMemoryEventHistoryHandle* out_history_handle)
{
    // Set the handle default to null in case an error occurs.
    *out_history_handle = nullptr;

    std::unordered_set<RmtGpuAddress> bound_virtual_addresses;

    RMT_ASSERT(data_set != nullptr);
    RMT_RETURN_ON_ERROR(data_set != nullptr, kRmtErrorInvalidPointer);

    if (hide_implicit_resources && RmtResourceUserDataIsResourceImplicit(resource_identifier))
    {
        return kRmtErrorNoResourceFound;
    }

    // Instantiate a new event history object.  The user is responsible for deleting this with the RmtMemoryEventHistoryFreeHistory() API function.
    EventHistoryImpl* history                       = new EventHistoryImpl();
    *out_history_handle                             = reinterpret_cast<RmtMemoryEventHistoryHandle*>(history);
    RmtMemoryEventHistoryUsageParameters parameters = {};
    parameters.filter_type                          = RmtMemoryEventHistoryFilterType::kRmtMemoryEventHistoryFilterTypeResourceHistoryForId;
    parameters.resource_identifier                  = resource_identifier;
    parameters.virtual_address                      = 0;
    parameters.hide_duplicate_page_table_events     = hide_duplicate_page_table_events;
    parameters.include_resources_in_all_allocations = false;

    history->SetUsageParameters(parameters);

    // Create a list of virtual allocations.
    RmtVirtualAllocationList virtual_allocation_list;
    const size_t             virtual_allocation_buffer_size =
        RmtVirtualAllocationListGetBufferSize(data_set->data_profile.total_virtual_allocation_count, data_set->data_profile.max_concurrent_resources);

    // Allocate storage for the virtual allocation list.  This list is freed at the end of this function once processing has completed.
    void*        virtual_allocation_buffer = malloc(virtual_allocation_buffer_size);
    RmtErrorCode error_code                = RmtVirtualAllocationListInitialize(&virtual_allocation_list,
                                                                 virtual_allocation_buffer,
                                                                 virtual_allocation_buffer_size,
                                                                 data_set->data_profile.max_virtual_allocation_count,
                                                                 data_set->data_profile.max_concurrent_resources,
                                                                 data_set->data_profile.total_virtual_allocation_count);
    RMT_ASSERT(error_code == kRmtOk);
    if (error_code == kRmtOk)
    {
        RmtResource resource = {};

        // List of IDs for virtual allocations bound to the resource (the byte offset in the token stream is used to uniquely identify the virtual allocation token.
        std::unordered_set<uint64_t> bound_allocation_token_id_list;
        MemoryBlockList              resource_memory_blocks;

        // Saved virtual allocation information.  The stream offset of the Resource Bind token is used as the lookup key.
        std::unordered_map<size_t, AllocationInfo> allocation_info_lookup;

        // Reset the RMT stream parsers.
        RmtStreamMergerReset(&data_set->stream_merger, data_set->file_handle);

        // First pass: build virtual allocation list and list of resource binds.
        while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
        {
            // Grab the next token from the heap.
            RmtToken current_token;
            error_code = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
            RMT_ASSERT(error_code == kRmtOk);

            switch (current_token.type)
            {
            case kRmtTokenTypeVirtualAllocate:
            {
                // The byte offset of the token in the data stream is used to uniquely identify this allocation.
                // The offset is used rather than the virtual allocation address in case there are allocations/frees then another allocation with the same base address.
                uint64_t     allocation_identifier = current_token.common.offset;
                RmtErrorCode add_allocation_result = RmtVirtualAllocationListAddAllocation(&virtual_allocation_list,
                                                                                           current_token.common.timestamp,
                                                                                           current_token.virtual_allocate_token.virtual_address,
                                                                                           (int32_t)(current_token.virtual_allocate_token.size_in_bytes >> 12),
                                                                                           current_token.virtual_allocate_token.preference,
                                                                                           current_token.virtual_allocate_token.owner_type,
                                                                                           allocation_identifier);
                RMT_ASSERT(add_allocation_result == kRmtOk);
                break;
            }

            case kRmtTokenTypeResourceBind:
            {
                if (current_token.resource_bind_token.resource_identifier == resource_identifier)
                {
                    RmtErrorCode get_allocation_result = RmtVirtualAllocationListGetAllocationForAddress(
                        &virtual_allocation_list, current_token.resource_bind_token.virtual_address, &resource.bound_allocation);

                    if (get_allocation_result == kRmtOk)
                    {
                        auto& allocation_info        = allocation_info_lookup[current_token.common.offset];
                        allocation_info.base_address = resource.bound_allocation->base_address;
                        memcpy(allocation_info.heap_preferences, resource.bound_allocation->heap_preferences, sizeof(allocation_info.heap_preferences));
                        MemoryBlock memory_block = {current_token.resource_bind_token.virtual_address, current_token.resource_bind_token.size_in_bytes};
                        resource_memory_blocks.push_back(memory_block);
                        bound_allocation_token_id_list.insert(resource.bound_allocation->allocation_identifier);
                        resource.address       = current_token.resource_bind_token.virtual_address;
                        resource.size_in_bytes = current_token.resource_bind_token.size_in_bytes;
                    }
                }
                break;
            }

            case kRmtTokenTypeVirtualFree:
            {
                RmtVirtualAllocationListRemoveAllocation(&virtual_allocation_list, current_token.virtual_free_token.virtual_address);
                break;
            }

            default:
                break;
            }
        }

        // Reset the RMT stream parsers ready to load the data.
        RmtStreamMergerReset(&data_set->stream_merger, data_set->file_handle);

        // Reset the list of virtual allocations.  In this second pass, only add allocations that are relevant (i.e. those included in the bound_allocation_token_id_list).
        error_code = RmtVirtualAllocationListInitialize(&virtual_allocation_list,
                                                        virtual_allocation_buffer,
                                                        virtual_allocation_buffer_size,
                                                        data_set->data_profile.max_virtual_allocation_count,
                                                        data_set->data_profile.max_concurrent_resources,
                                                        data_set->data_profile.total_virtual_allocation_count);
        RMT_ASSERT(error_code == kRmtOk);
        if (error_code == kRmtOk)
        {
            // Second pass: build the history for the resource.
            while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
            {
                // Grab the next token from the heap.
                RmtToken current_token;
                error_code = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
                RMT_ASSERT(error_code == kRmtOk);

                switch (current_token.type)
                {
                case kRmtTokenTypeResourceCreate:
                    if (current_token.resource_create_token.resource_identifier == resource_identifier)
                    {
                        history->AddEvent(current_token.resource_create_token);
                    }
                    break;

                case kRmtTokenTypeResourceDestroy:
                    if (current_token.resource_destroy_token.resource_identifier == resource_identifier)
                    {
                        history->AddEvent(current_token.resource_destroy_token);
                    }
                    break;

                case kRmtTokenTypeResourceBind:
                    if (current_token.resource_bind_token.resource_identifier == resource_identifier)
                    {
                        // Get the index of the bind event that is about to be added to the event history using the current event count.
                        // The event count matches the index of this next event to be added to the history since the index is zero based.
                        // Once the bind event is added, use the bind index to update the event's allocation virtual address.
                        const RmtMemoryEventHistoryEventIndex bind_event_index = history->GetEventCount();
                        history->AddEvent(current_token.resource_bind_token);

                        const auto allocation_address_iterator = allocation_info_lookup.find(current_token.common.offset);
                        if (allocation_address_iterator != allocation_info_lookup.end())
                        {
                            // Update the bind event's heap preferences for the resource.
                            RmtErrorCode update_result =
                                history->UpdateResourceHeapPreferences(bind_event_index, allocation_address_iterator->second.heap_preferences);
                            RMT_ASSERT(update_result == kRmtOk);

                            // Update the bind event's allocation virtual address.
                            update_result = history->UpdateResourceAllocationVirtualAddress(bind_event_index, allocation_address_iterator->second.base_address);
                            RMT_ASSERT(update_result == kRmtOk);
                            RMT_UNUSED(update_result);
                        }
                    }
                    break;

                case kRmtTokenTypeVirtualAllocate:
                {
                    // The byte offset of the token in the data stream is used to uniquely identify this allocation.
                    // The offset is used rather than the virtual allocation address in case there are allocations/frees then another allocation with the same base address.
                    uint64_t allocation_identifier = current_token.virtual_allocate_token.common.offset;
                    if (bound_allocation_token_id_list.find(allocation_identifier) != bound_allocation_token_id_list.end())
                    {
                        // If the allocation is one that was at any time bound to the resource being tracked, add it to the list.
                        RmtVirtualAllocationListAddAllocation(&virtual_allocation_list,
                                                              current_token.common.timestamp,
                                                              current_token.virtual_allocate_token.virtual_address,
                                                              (int32_t)(current_token.virtual_allocate_token.size_in_bytes >> 12),
                                                              current_token.virtual_allocate_token.preference,
                                                              current_token.virtual_allocate_token.owner_type,
                                                              allocation_identifier);
                        history->AddEvent(current_token.virtual_allocate_token);
                    }
                    break;
                }

                case kRmtTokenTypeVirtualFree:
                {
                    // Find the allocation that this free belongs to.  If the allocation wasn't bound to the resource, it won't be found in the list and can be ignored.
                    RmtErrorCode get_allocation_result = RmtVirtualAllocationListGetAllocationForAddress(
                        &virtual_allocation_list, current_token.virtual_free_token.virtual_address, &resource.bound_allocation);
                    if (get_allocation_result == kRmtOk)
                    {
                        history->AddEvent(current_token.virtual_free_token);
                        RmtVirtualAllocationListRemoveAllocation(&virtual_allocation_list, current_token.virtual_free_token.virtual_address);
                    }
                    break;
                }

                case kRmtTokenTypeResourceReference:
                {
                    RmtErrorCode get_allocation_result = RmtVirtualAllocationListGetAllocationForAddress(
                        &virtual_allocation_list, current_token.resource_reference.virtual_address, &resource.bound_allocation);
                    if (get_allocation_result == kRmtOk)
                    {
                        // NOTE: PAL can only make resident/evict a full virtual allocation on CPU, not just a single resource.
                        if (current_token.resource_reference.virtual_address == resource.bound_allocation->base_address)
                        {
                            // Get the index of the Resource Reference event that is about to be added to the event history using the current event count.
                            // The event count matches the index of this next event to be added to the history since the index is zero based.
                            // Once the Resource Reference event is added, use the index to update the event's resource list.
                            const RmtMemoryEventHistoryEventIndex resource_reference_event_index = history->GetEventCount();
                            history->AddEvent(current_token.resource_reference);
                            std::vector<RmtResourceIdentifier> bound_resource;

                            // For this type of history, there will only be one resource affected by the make resident/evict event (the one specified).
                            bound_resource.push_back(resource_identifier);
                            history->UpdateAffectedResourcesForMemorySwapEvent(resource_reference_event_index, bound_resource);
                        }
                    }
                    break;
                }

                case kRmtTokenTypeCpuMap:
                {
                    RmtErrorCode get_allocation_result = RmtVirtualAllocationListGetAllocationForAddress(
                        &virtual_allocation_list, current_token.cpu_map_token.virtual_address, &resource.bound_allocation);
                    if (get_allocation_result == kRmtOk)
                    {
                        // NOTE: PAL can only map/unmap a full virtual allocation on CPU, not just a resource.
                        if (current_token.cpu_map_token.virtual_address == resource.bound_allocation->base_address)
                        {
                            history->AddEvent(current_token.cpu_map_token);
                        }
                    }
                    break;
                }

                case kRmtTokenTypePageTableUpdate:
                {
                    // Check for overlap between the resource VA range and this change to the PA mappings.
                    const uint64_t size_in_bytes =
                        RmtGetAllocationSizeInBytes(current_token.page_table_update_token.size_in_pages, current_token.page_table_update_token.page_size);

                    for (auto resource_memory : resource_memory_blocks)
                    {
                        if (RmtAllocationsOverlap(
                                current_token.page_table_update_token.virtual_address, size_in_bytes, resource_memory.start_address, resource_memory.size))
                        {
                            // See if a virtual allocation is active at this time within the stream.
                            RmtErrorCode get_allocation_result = RmtVirtualAllocationListGetAllocationForAddress(
                                &virtual_allocation_list, resource_memory.start_address, &resource.bound_allocation);
                            if (get_allocation_result == kRmtOk)
                            {
                                history->AddEvent(current_token.page_table_update_token);
                            }
                            break;
                        }
                    }
                }
                break;

                default:
                    break;
                }
            }
        }
    }

    // Free the virtual allocation list.
    free(virtual_allocation_buffer);

    if (error_code != kRmtOk)
    {
        RmtMemoryEventHistoryFreeHistory(out_history_handle);
    }
    return error_code;
}

RmtErrorCode RmtMemoryEventHistoryGenerateFullAllocationHistory(RmtDataSet*                  data_set,
                                                                const RmtGpuAddress          virtual_address,
                                                                const bool                   hide_duplicate_page_table_events,
                                                                const bool                   include_resources_in_all_allocations,
                                                                RmtMemoryEventHistoryHandle* out_history_handle)
{
    // Set the handle default to null in case an error occurs.
    *out_history_handle = nullptr;

    RmtErrorCode                              error_code = kRmtOk;
    MemoryBlockList                           virtual_allocations;
    std::unordered_set<RmtResourceIdentifier> resource_identifiers;

    RMT_ASSERT(data_set != nullptr);
    RMT_RETURN_ON_ERROR(data_set != nullptr, kRmtErrorInvalidPointer);

    // The bound_resources lookup map is used to locate a list of resource identifiers bound to a virtual allocation.
    // A virtual allocation unique identifier is used as the key.
    std::unordered_map<uint64_t, std::vector<RmtResourceIdentifier>> bound_resources;

    // The allocation_resource_map is used to find a virtual allocation's unique identifier bound to a resource with the resource identifier as the key.
    std::unordered_map<RmtResourceIdentifier, uint64_t> allocation_resource_map;

    // The heap_resource_identifiers set is a list of resource identifiers which are heap resources.
    std::unordered_set<RmtResourceIdentifier> heap_resource_identifiers;

    // Instantiate a new event history object.  The user is responsible for deleting this with the RmtMemoryEventHistoryFreeHistory() API function.
    EventHistoryImpl* history                       = new EventHistoryImpl();
    *out_history_handle                             = reinterpret_cast<RmtMemoryEventHistoryHandle*>(history);
    RmtMemoryEventHistoryUsageParameters parameters = {};
    parameters.filter_type                          = RmtMemoryEventHistoryFilterType::kRmtMemoryEventHistoryFilterTypeFullAllcationHistory;
    parameters.resource_identifier                  = 0;
    parameters.virtual_address                      = virtual_address;
    parameters.hide_duplicate_page_table_events     = hide_duplicate_page_table_events;
    parameters.include_resources_in_all_allocations = include_resources_in_all_allocations;
    history->SetUsageParameters(parameters);

    // Reset the RMT stream parsers.
    RmtStreamMergerReset(&data_set->stream_merger, data_set->file_handle);

    // First pass: build virtual allocation list
    while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
    {
        // Grab the next token from the heap.
        RmtToken current_token;
        error_code = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
        RMT_ASSERT(error_code == kRmtOk);

        switch (current_token.type)
        {
        case kRmtTokenTypeVirtualAllocate:
        {
            if (RmtAllocationsOverlap(
                    current_token.virtual_allocate_token.virtual_address, current_token.virtual_allocate_token.size_in_bytes, virtual_address, 1))
            {
                MemoryBlock allocation;
                allocation.start_address = current_token.virtual_allocate_token.virtual_address;
                allocation.size          = current_token.virtual_allocate_token.size_in_bytes;
                virtual_allocations.push_back(allocation);
            }
            break;
        }

        default:
            break;
        }
    }

    // Reset the RMT stream parsers.
    RmtStreamMergerReset(&data_set->stream_merger, data_set->file_handle);

    // Second pass: build list of resource IDs with matching virtual address or are contained in any of the virtual allocations gathered in the first pass (depending on include_all_resources).
    while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
    {
        // Grab the next token from the heap.
        RmtToken current_token;
        error_code = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
        RMT_ASSERT(error_code == kRmtOk);

        switch (current_token.type)
        {
        case kRmtTokenTypeResourceBind:
        {
            // If the API is configured to hide implicit resources and the resource is marked implicit, then don't include the event in history.  In all other cases, the event can be considered for inclusion.
            if (!hide_implicit_resources || (!RmtResourceUserDataIsResourceImplicit(current_token.resource_bind_token.resource_identifier)))
            {
                if (include_resources_in_all_allocations)
                {
                    if (CheckVirtualMemoryBlockOverlap(virtual_allocations, current_token.resource_bind_token.virtual_address))
                    {
                        // This resource is contained within one of the virtual allocation blocks found to contain the virtual address being searched for.  Save this resource ID.
                        resource_identifiers.insert(current_token.resource_bind_token.resource_identifier);
                    }
                }
                else
                {
                    if (RmtAllocationsOverlap(
                            current_token.resource_bind_token.virtual_address, current_token.resource_bind_token.size_in_bytes, virtual_address, 1))
                    {
                        // The virtual address being searched for overlaps with this resource.  Save this resource ID.
                        resource_identifiers.insert(current_token.resource_bind_token.resource_identifier);
                    }
                }
            }
            break;
        }

        default:
            break;
        }
    }

    // Reset the RMT stream parsers.
    RmtStreamMergerReset(&data_set->stream_merger, data_set->file_handle);

    // Third pass: build history.

    // Create a list of virtual allocations.
    RmtVirtualAllocationList virtual_allocation_list;
    const size_t             virtual_allocation_buffer_size =
        RmtVirtualAllocationListGetBufferSize(data_set->data_profile.total_virtual_allocation_count, data_set->data_profile.max_concurrent_resources);

    // Allocate storage for the virtual allocation list.  This list is freed at the end of this function once processing has completed.
    void* virtual_allocation_buffer = malloc(virtual_allocation_buffer_size);
    error_code                      = RmtVirtualAllocationListInitialize(&virtual_allocation_list,
                                                    virtual_allocation_buffer,
                                                    virtual_allocation_buffer_size,
                                                    data_set->data_profile.max_virtual_allocation_count,
                                                    data_set->data_profile.max_concurrent_resources,
                                                    data_set->data_profile.total_virtual_allocation_count);

    // Create a list of virtual allocations that could be potentially bound a heap resource.
    RmtVirtualAllocationList virtual_allocation_list_for_heaps;

    // Allocate storage for the virtual allocation list used to track heap resources.
    // This list is freed at the end of this function once processing has completed.
    void* virtual_allocation_buffer_for_heaps = malloc(virtual_allocation_buffer_size);
    error_code                                = RmtVirtualAllocationListInitialize(&virtual_allocation_list_for_heaps,
                                                    virtual_allocation_buffer_for_heaps,
                                                    virtual_allocation_buffer_size,
                                                    data_set->data_profile.max_virtual_allocation_count,
                                                    data_set->data_profile.max_concurrent_resources,
                                                    data_set->data_profile.total_virtual_allocation_count);

    RMT_ASSERT(error_code == kRmtOk);
    if (error_code == kRmtOk)
    {
        while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
        {
            // Grab the next token from the heap.
            RmtToken current_token;
            error_code = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
            RMT_ASSERT(error_code == kRmtOk);

            switch (current_token.type)
            {
            case kRmtTokenTypeResourceCreate:
            {
                if (resource_identifiers.find(current_token.resource_create_token.resource_identifier) != resource_identifiers.end())
                {
                    if (current_token.resource_create_token.resource_type == RmtResourceType::kRmtResourceTypeHeap)
                    {
                        heap_resource_identifiers.insert(current_token.resource_create_token.resource_identifier);
                    }
                    history->AddEvent(current_token.resource_create_token);
                }
                break;
            }

            case kRmtTokenTypeResourceBind:
            {
                const RmtVirtualAllocation* virtual_allocation   = nullptr;
                bool                        add_event_to_history = false;

                if (heap_resource_identifiers.find(current_token.resource_bind_token.resource_identifier) != heap_resource_identifiers.end())
                {
                    // Handle the case where a heap resource is bound to a virtual allocation.

                    const RmtErrorCode get_allocation_result = RmtVirtualAllocationListGetAllocationForAddress(
                        &virtual_allocation_list_for_heaps, current_token.resource_bind_token.virtual_address, &virtual_allocation);

                    if (get_allocation_result == kRmtOk)
                    {
                        add_event_to_history = true;
                    }
                }
                else if (resource_identifiers.find(current_token.resource_bind_token.resource_identifier) != resource_identifiers.end())
                {
                    // Handle the case where a non-heap resource is being bound.

                    const RmtErrorCode get_allocation_result = RmtVirtualAllocationListGetAllocationForAddress(
                        &virtual_allocation_list, current_token.resource_bind_token.virtual_address, &virtual_allocation);

                    if (get_allocation_result == kRmtOk)
                    {
                        add_event_to_history = true;
                    }
                }

                if (add_event_to_history)
                {
                    // Get the index of the bind event that is about to be added to the event history using the current event count.
                    // The event count matches the index of this next event to be added to the history since the index is zero based.
                    // Once the bind event is added, use the bind index to update the event's allocation virtual address.
                    const RmtMemoryEventHistoryEventIndex bind_event_index = history->GetEventCount();
                    history->AddEvent(current_token.resource_bind_token);

                    // Update the bind event's heap preferences for the resource.
                    RmtErrorCode update_result = history->UpdateResourceHeapPreferences(bind_event_index, virtual_allocation->heap_preferences);
                    RMT_ASSERT(update_result == kRmtOk);

                    // Update the bind event's allocation virtual address.
                    update_result = history->UpdateResourceAllocationVirtualAddress(bind_event_index, virtual_allocation->base_address);
                    RMT_ASSERT(update_result == kRmtOk);
                    RMT_UNUSED(update_result);

                    auto virtual_allocation_bind_list = bound_resources.find(virtual_allocation->allocation_identifier);
                    if (virtual_allocation_bind_list != bound_resources.end())
                    {
                        virtual_allocation_bind_list->second.push_back(current_token.resource_bind_token.resource_identifier);
                    }
                }
                break;
            }

            case kRmtTokenTypeResourceDestroy:
            {
                if (resource_identifiers.find(current_token.resource_destroy_token.resource_identifier) != resource_identifiers.end())
                {
                    history->AddEvent(current_token.resource_destroy_token);
                }
                break;
            }

            case kRmtTokenTypeVirtualAllocate:
            {
                uint64_t allocation_identifier = current_token.common.offset;

                // All allocations need to be tracked in case they are needed for a heap resource (re)bind.
                // Add the allocation that is potentially bound to a heap resource.
                RmtVirtualAllocationListAddAllocation(&virtual_allocation_list_for_heaps,
                                                      current_token.common.timestamp,
                                                      current_token.virtual_allocate_token.virtual_address,
                                                      (int32_t)(current_token.virtual_allocate_token.size_in_bytes >> 12),
                                                      current_token.virtual_allocate_token.preference,
                                                      current_token.virtual_allocate_token.owner_type,
                                                      allocation_identifier);

                if (CheckVirtualMemoryAllocationMatch(virtual_allocations, current_token.virtual_allocate_token.virtual_address))
                {
                    history->AddEvent(current_token.virtual_allocate_token);

                    RmtErrorCode add_allocation_result =
                        RmtVirtualAllocationListAddAllocation(&virtual_allocation_list,
                                                              current_token.common.timestamp,
                                                              current_token.virtual_allocate_token.virtual_address,
                                                              (int32_t)(current_token.virtual_allocate_token.size_in_bytes >> 12),
                                                              current_token.virtual_allocate_token.preference,
                                                              current_token.virtual_allocate_token.owner_type,
                                                              allocation_identifier);
                    RMT_ASSERT(add_allocation_result == kRmtOk);

                    // Add allocation identifier to bound_resources map.
                    bound_resources[allocation_identifier];
                }
                break;
            }

            case kRmtTokenTypeVirtualFree:
            {
                // All allocations need to be tracked in case they are needed for a heap resource (re)bind.
                // Remove the freed allocation that is potentially bound to a heap resource.
                RmtVirtualAllocationListRemoveAllocation(&virtual_allocation_list_for_heaps, current_token.virtual_free_token.virtual_address);

                if (CheckVirtualMemoryAllocationMatch(virtual_allocations, current_token.virtual_free_token.virtual_address))
                {
                    history->AddEvent(current_token.virtual_free_token);
                    const RmtVirtualAllocation* virtual_allocation;
                    const RmtErrorCode          get_allocation_result = RmtVirtualAllocationListGetAllocationForAddress(
                        &virtual_allocation_list, current_token.virtual_free_token.virtual_address, &virtual_allocation);

                    if (get_allocation_result == kRmtOk)
                    {
                        // Remove allocation identifier from bound_resources map
                        bound_resources.erase(virtual_allocation->allocation_identifier);
                    }
                    RmtVirtualAllocationListRemoveAllocation(&virtual_allocation_list, current_token.virtual_free_token.virtual_address);
                }
                break;
            }

            case kRmtTokenTypeResourceReference:
            {
                if (CheckVirtualMemoryBlockOverlap(virtual_allocations, current_token.resource_reference.virtual_address))
                {
                    // Get the index of the Resource Reference event that is about to be added to the event history using the current event count.
                    // The event count matches the index of this next event to be added to the history since the index is zero based.
                    // Once the Resource Reference event is added, use the index to update the event's resource list.
                    const RmtMemoryEventHistoryEventIndex resource_reference_event_index = history->GetEventCount();
                    history->AddEvent(current_token.resource_reference);
                    const RmtVirtualAllocation* virtual_allocation;
                    const RmtErrorCode          get_allocation_result = RmtVirtualAllocationListGetAllocationForAddress(
                        &virtual_allocation_list, current_token.resource_reference.virtual_address, &virtual_allocation);

                    if (get_allocation_result == kRmtOk)
                    {
                        auto virtual_allocation_bind_list = bound_resources.find(virtual_allocation->allocation_identifier);
                        if (virtual_allocation_bind_list != bound_resources.end())
                        {
                            history->UpdateAffectedResourcesForMemorySwapEvent(resource_reference_event_index, virtual_allocation_bind_list->second);
                        }
                    }
                }
                break;
            }

            case kRmtTokenTypeCpuMap:
            {
                if (CheckVirtualMemoryBlockOverlap(virtual_allocations, current_token.cpu_map_token.virtual_address))
                {
                    history->AddEvent(current_token.cpu_map_token);
                }
                break;
            }

            case kRmtTokenTypePageTableUpdate:
            {
                if (CheckVirtualMemoryBlockOverlap(virtual_allocations, current_token.page_table_update_token.virtual_address))
                {
                    history->AddEvent(current_token.page_table_update_token);
                }
                break;
            }

            default:
                break;
            }
        }
    }

    // Free the virtual allocation lists.
    free(virtual_allocation_buffer);
    free(virtual_allocation_buffer_for_heaps);

    if (error_code != kRmtOk)
    {
        RmtMemoryEventHistoryFreeHistory(out_history_handle);
    }

    return error_code;
}

RmtErrorCode RmtMemoryEventHistoryGenerateBasicAllocationHistory(RmtDataSet*                  data_set,
                                                                 const RmtGpuAddress          virtual_address,
                                                                 RmtMemoryEventHistoryHandle* out_history_handle)
{
    // Set the handle default to null in case an error occurs.
    *out_history_handle = nullptr;

    std::unordered_set<RmtGpuAddress> virtual_allocations;

    RMT_ASSERT(data_set != nullptr);
    RMT_RETURN_ON_ERROR(data_set != nullptr, kRmtErrorInvalidPointer);

    RmtErrorCode error_code = kRmtOk;

    // Instantiate a new event history object.  The user is responsible for deleting this with the RmtMemoryEventHistoryFreeHistory() API function.
    EventHistoryImpl* history                       = new EventHistoryImpl();
    *out_history_handle                             = reinterpret_cast<RmtMemoryEventHistoryHandle*>(history);
    RmtMemoryEventHistoryUsageParameters parameters = {};
    parameters.filter_type                          = RmtMemoryEventHistoryFilterType::kRmtMemoryEventHistoryFilterTypeBasicAllocationHistory;
    parameters.resource_identifier                  = 0;
    parameters.virtual_address                      = virtual_address;
    parameters.hide_duplicate_page_table_events     = false;
    parameters.include_resources_in_all_allocations = false;
    history->SetUsageParameters(parameters);

    // Reset the RMT stream parsers.
    RmtStreamMergerReset(&data_set->stream_merger, data_set->file_handle);

    while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
    {
        // Grab the next token from the heap.
        RmtToken current_token;
        error_code = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
        RMT_ASSERT(error_code == kRmtOk);

        switch (current_token.type)
        {
        case kRmtTokenTypeVirtualAllocate:
        {
            RmtGpuAddress allocation_start_address = current_token.virtual_allocate_token.virtual_address;
            if (RmtAllocationsOverlap(allocation_start_address, current_token.virtual_allocate_token.size_in_bytes, virtual_address, 1))
            {
                history->AddEvent(current_token.virtual_allocate_token);
                virtual_allocations.insert(allocation_start_address);
            }
            break;
        }

        case kRmtTokenTypeVirtualFree:
        {
            if (virtual_allocations.find(current_token.virtual_free_token.virtual_address) != virtual_allocations.end())
            {
                history->AddEvent(current_token.virtual_free_token);
            }
            break;
        }

        default:
            break;
        }
    }

    if (error_code != kRmtOk)
    {
        RmtMemoryEventHistoryFreeHistory(out_history_handle);
    }
    return error_code;
}

RmtErrorCode RmtMemoryEventHistoryGenerateResourceCreateHistoryForAddress(RmtDataSet*                  data_set,
                                                                          const RmtGpuAddress          virtual_address,
                                                                          const bool                   include_resources_in_all_allocations,
                                                                          RmtMemoryEventHistoryHandle* out_history_handle)
{
    // Set the handle default to null in case an error occurs.
    *out_history_handle = nullptr;

    RmtErrorCode                              error_code = kRmtOk;
    MemoryBlockList                           virtual_allocations;
    std::unordered_set<RmtResourceIdentifier> resource_identifiers;

    RMT_ASSERT(data_set != nullptr);
    RMT_RETURN_ON_ERROR(data_set != nullptr, kRmtErrorInvalidPointer);

    // Instantiate a new event history object.  The user is responsible for deleting this with the RmtMemoryEventHistoryFreeHistory() API function.
    EventHistoryImpl* history                       = new EventHistoryImpl();
    *out_history_handle                             = reinterpret_cast<RmtMemoryEventHistoryHandle*>(history);
    RmtMemoryEventHistoryUsageParameters parameters = {};
    parameters.filter_type                          = RmtMemoryEventHistoryFilterType::kRmtMemoryEventHistoryFilterTypeResourceCreationHistoryForAddress;
    parameters.resource_identifier                  = 0;
    parameters.virtual_address                      = virtual_address;
    parameters.hide_duplicate_page_table_events     = false;
    parameters.include_resources_in_all_allocations = include_resources_in_all_allocations;
    history->SetUsageParameters(parameters);

    // Reset the RMT stream parsers.
    RmtStreamMergerReset(&data_set->stream_merger, data_set->file_handle);

    // First pass: build virtual allocation list
    while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
    {
        // Grab the next token from the heap.
        RmtToken current_token;
        error_code = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
        RMT_ASSERT(error_code == kRmtOk);

        switch (current_token.type)
        {
        case kRmtTokenTypeVirtualAllocate:
        {
            if (RmtAllocationsOverlap(
                    current_token.virtual_allocate_token.virtual_address, current_token.virtual_allocate_token.size_in_bytes, virtual_address, 1))
            {
                MemoryBlock allocation;
                allocation.start_address = current_token.virtual_allocate_token.virtual_address;
                allocation.size          = current_token.virtual_allocate_token.size_in_bytes;
                virtual_allocations.push_back(allocation);
            }
            break;
        }

        default:
            break;
        }
    }

    // Reset the RMT stream parsers.
    RmtStreamMergerReset(&data_set->stream_merger, data_set->file_handle);

    // Second pass: build list of resource IDs with matching virtual address or are contained in any of the virtual allocations gathered in the first pass (depending on include_all_resources).
    while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
    {
        // Grab the next token from the heap.
        RmtToken current_token;
        error_code = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
        RMT_ASSERT(error_code == kRmtOk);

        switch (current_token.type)
        {
        case kRmtTokenTypeResourceBind:
        {
            // If the API is configured to hide implicit resources and the resource is marked implicit, then don't include the event in history.  In all other cases, the event can be considered for inclusion.
            if (!hide_implicit_resources || (!RmtResourceUserDataIsResourceImplicit(current_token.resource_bind_token.resource_identifier)))
            {
                if (include_resources_in_all_allocations)
                {
                    if (CheckVirtualMemoryBlockOverlap(virtual_allocations, current_token.resource_bind_token.virtual_address))
                    {
                        // This resource is contained within one of the virtual allocation blocks found to contain the virtual address being searched for.  Save this resource ID.
                        resource_identifiers.insert(current_token.resource_bind_token.resource_identifier);
                    }
                }
                else
                {
                    if (RmtAllocationsOverlap(
                            current_token.resource_bind_token.virtual_address, current_token.resource_bind_token.size_in_bytes, virtual_address, 1))
                    {
                        // The virtual address being searched for overlaps with this resource.  Save this resource ID.
                        resource_identifiers.insert(current_token.resource_bind_token.resource_identifier);
                    }
                }
            }
            break;
        }

        default:
            break;
        }
    }

    // Reset the RMT stream parsers.
    RmtStreamMergerReset(&data_set->stream_merger, data_set->file_handle);

    // Third pass: build history.
    while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
    {
        // Grab the next token from the heap.
        RmtToken current_token;
        error_code = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
        RMT_ASSERT(error_code == kRmtOk);

        switch (current_token.type)
        {
        case kRmtTokenTypeResourceCreate:
        {
            if (resource_identifiers.find(current_token.resource_create_token.resource_identifier) != resource_identifiers.end())
            {
                history->AddEvent(current_token.resource_create_token);
            }
            break;
        }

        default:
            break;
        }
    }

    if (error_code != kRmtOk)
    {
        RmtMemoryEventHistoryFreeHistory(out_history_handle);
    }

    return error_code;
}

RmtErrorCode RmtMemoryEventHistoryGenerateHistoryForAllResources(RmtDataSet* data_set, RmtMemoryEventHistoryHandle* out_history_handle)
{
    // Set the handle default to null in case an error occurs.
    *out_history_handle = nullptr;

    RmtErrorCode error_code = kRmtOk;

    RMT_ASSERT(data_set != nullptr);
    RMT_RETURN_ON_ERROR(data_set != nullptr, kRmtErrorInvalidPointer);

    // The bound_resources lookup map is used to locate a list of resource identifiers bound to a virtual allocation.
    // A virtual allocation unique identifier is used as the key.
    std::unordered_map<uint64_t, std::vector<RmtResourceIdentifier>> bound_resources;

    // The allocation_resource_map is used to find a virtual allocation's unique identifier bound to a resource with the resource identifier as the key.
    std::unordered_map<RmtResourceIdentifier, uint64_t> allocation_resource_map;

    EventHistoryImpl* history                       = new EventHistoryImpl();
    *out_history_handle                             = reinterpret_cast<RmtMemoryEventHistoryHandle*>(history);
    RmtMemoryEventHistoryUsageParameters parameters = {};
    parameters.filter_type                          = RmtMemoryEventHistoryFilterType::kRmtMemoryEventHistoryFilterTypeAllResources;
    parameters.resource_identifier                  = 0;
    parameters.virtual_address                      = 0;
    parameters.hide_duplicate_page_table_events     = false;
    parameters.include_resources_in_all_allocations = false;
    history->SetUsageParameters(parameters);

    // Reset the RMT stream parsers.
    RmtStreamMergerReset(&data_set->stream_merger, data_set->file_handle);

    // Create a list of virtual allocations.
    RmtVirtualAllocationList virtual_allocation_list;
    const size_t             virtual_allocation_buffer_size =
        RmtVirtualAllocationListGetBufferSize(data_set->data_profile.total_virtual_allocation_count, data_set->data_profile.max_concurrent_resources);

    // Allocate memory for the VA list (freed when processing completes in this function).
    void* virtual_allocation_buffer = malloc(virtual_allocation_buffer_size);
    error_code                      = RmtVirtualAllocationListInitialize(&virtual_allocation_list,
                                                    virtual_allocation_buffer,
                                                    virtual_allocation_buffer_size,
                                                    data_set->data_profile.max_virtual_allocation_count,
                                                    data_set->data_profile.max_concurrent_resources,
                                                    data_set->data_profile.total_virtual_allocation_count);
    RMT_ASSERT(error_code == kRmtOk);

    if (error_code == kRmtOk)
    {
        // Build the resource history.
        while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
        {
            // Grab the next token from the heap.
            RmtToken current_token;
            error_code = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
            RMT_ASSERT(error_code == kRmtOk);

            switch (current_token.type)
            {
            case kRmtTokenTypeResourceCreate:
            {
                // If the API is configured to hide implicit resources and the resource is marked implicit, then don't include the event in history.  In all other cases, the event can be included.
                if (!hide_implicit_resources || (!RmtResourceUserDataIsResourceImplicit(current_token.resource_destroy_token.resource_identifier)))
                {
                    history->AddEvent(current_token.resource_create_token);
                }
                break;
            }

            case kRmtTokenTypeResourceBind:
            {
                // If the API is configured to hide implicit resources and the resource is marked implicit, then don't include the event in history.  In all other cases, the event can be considered for inclusion.
                if (!hide_implicit_resources || (!RmtResourceUserDataIsResourceImplicit(current_token.resource_destroy_token.resource_identifier)))
                {
                    const RmtVirtualAllocation* virtual_allocation;
                    const RmtErrorCode          get_allocation_result = RmtVirtualAllocationListGetAllocationForAddress(
                        &virtual_allocation_list, current_token.resource_bind_token.virtual_address, &virtual_allocation);

                    if (get_allocation_result == kRmtOk)
                    {
                        // Get the index of the bind event that is about to be added to the event history using the current event count.
                        // The event count matches the index of this next event to be added to the history since the index is zero based.
                        // Once the bind event is added, use the bind index to update the event's allocation virtual address.
                        const RmtMemoryEventHistoryEventIndex bind_event_index = history->GetEventCount();
                        history->AddEvent(current_token.resource_bind_token);

                        // Update the bind event's heap preferences for the resource.
                        RmtErrorCode update_result = history->UpdateResourceHeapPreferences(bind_event_index, virtual_allocation->heap_preferences);
                        RMT_ASSERT(update_result == kRmtOk);

                        // Update the bind event's allocation virtual address.
                        update_result = history->UpdateResourceAllocationVirtualAddress(bind_event_index, virtual_allocation->base_address);
                        RMT_UNUSED(update_result);

                        auto virtual_allocation_bind_list = bound_resources.find(virtual_allocation->allocation_identifier);
                        if (virtual_allocation_bind_list != bound_resources.end())
                        {
                            virtual_allocation_bind_list->second.push_back(current_token.resource_bind_token.resource_identifier);

                            // Use the resource identifier as a key to map the bound resource to a virtual allocation's unique identifier.
                            allocation_resource_map[current_token.resource_bind_token.resource_identifier] = virtual_allocation->allocation_identifier;
                        }
                    }
                }
                break;
            }

            case kRmtTokenTypeResourceDestroy:
            {
                // If the API is configured to hide implicit resources and the resource is marked implicit, then don't include the event in history.  In all other cases, the event can be included.
                if (!hide_implicit_resources || (!RmtResourceUserDataIsResourceImplicit(current_token.resource_destroy_token.resource_identifier)))
                {
                    history->AddEvent(current_token.resource_destroy_token);
                    // Remove the resource ID from the bound allocation in the bound_resources list.
                    if (!allocation_resource_map.empty())
                    {
                        const auto allocation_resource_entry = allocation_resource_map.find(current_token.resource_destroy_token.resource_identifier);
                        if (allocation_resource_entry != allocation_resource_map.end())
                        {
                            uint64_t allocation_identifier = allocation_resource_entry->second;
                            auto     bound_resource_entry  = bound_resources.find(allocation_identifier);
                            if (bound_resource_entry != bound_resources.end())
                            {
                                bound_resources.erase(allocation_identifier);
                            }
                        }
                        allocation_resource_map.erase(current_token.resource_destroy_token.resource_identifier);
                    }
                }

                break;
            }

            case kRmtTokenTypeVirtualAllocate:
            {
                const uint64_t     allocation_identifier = current_token.common.offset;
                const RmtErrorCode add_allocation_result =
                    RmtVirtualAllocationListAddAllocation(&virtual_allocation_list,
                                                          current_token.common.timestamp,
                                                          current_token.virtual_allocate_token.virtual_address,
                                                          (int32_t)(current_token.virtual_allocate_token.size_in_bytes >> 12),
                                                          current_token.virtual_allocate_token.preference,
                                                          current_token.virtual_allocate_token.owner_type,
                                                          allocation_identifier);
                RMT_ASSERT(add_allocation_result == kRmtOk);

                // Add allocation identifier to bound_resources map.
                bound_resources[allocation_identifier];
                break;
            }

            case kRmtTokenTypeVirtualFree:
            {
                const RmtVirtualAllocation* virtual_allocation;
                const RmtErrorCode          get_allocation_result = RmtVirtualAllocationListGetAllocationForAddress(
                    &virtual_allocation_list, current_token.virtual_free_token.virtual_address, &virtual_allocation);

                if (get_allocation_result == kRmtOk)
                {
                    // Remove allocation identifier from bound_resources map
                    bound_resources.erase(virtual_allocation->allocation_identifier);
                }
                RmtVirtualAllocationListRemoveAllocation(&virtual_allocation_list, current_token.virtual_free_token.virtual_address);

                break;
            }

            case kRmtTokenTypeResourceReference:
            {
                // Get the index of the Resource Reference event that is about to be added to the event history using the current event count.
                // The event count matches the index of this next event to be added to the history since the index is zero based.
                // Once the Resource Reference event is added, use the index to update the event's resource list.
                const RmtMemoryEventHistoryEventIndex resource_reference_event_index = history->GetEventCount();
                history->AddEvent(current_token.resource_reference);

                const RmtVirtualAllocation* virtual_allocation;
                const RmtErrorCode          get_allocation_result = RmtVirtualAllocationListGetAllocationForAddress(
                    &virtual_allocation_list, current_token.resource_reference.virtual_address, &virtual_allocation);

                if (get_allocation_result == kRmtOk)
                {
                    auto virtual_allocation_bind_list = bound_resources.find(virtual_allocation->allocation_identifier);
                    if (virtual_allocation_bind_list != bound_resources.end())
                    {
                        history->UpdateAffectedResourcesForMemorySwapEvent(resource_reference_event_index, virtual_allocation_bind_list->second);
                    }
                }

                break;
            }

            case kRmtTokenTypeCpuMap:
            {
                history->AddEvent(current_token.cpu_map_token);
                break;
            }

            default:
                break;
            }
        }
    }

    // Free the virtual allocation list.
    free(virtual_allocation_buffer);

    if (error_code != kRmtOk)
    {
        RmtMemoryEventHistoryFreeHistory(out_history_handle);
    }

    return error_code;
}
