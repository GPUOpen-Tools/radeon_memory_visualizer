//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Functions working on a snapshot.
//=============================================================================

#include "rmt_data_snapshot.h"

#include <stdlib.h>  // for free()

#include "rmt_address_helper.h"
#include "rmt_assert.h"
#include "rmt_data_set.h"
#include "rmt_print.h"
#include "rmt_resource_history.h"
#include "rmt_token.h"

#ifndef _WIN32
#include "linux/safe_crt.h"
#endif

// do the first pass over the RMT data, figure out the resource-based events, and
// virtual memory-based events, and also build a list of physical address ranges
// that the resource interacts with during its life. This list will be used in the
// 2nd pass of the algorithm.
static RmtErrorCode ProcessTokensIntoResourceHistory(RmtDataSet* data_set, const RmtResource* resource, RmtResourceHistory* out_resource_history)
{
    // Reset the RMT stream parsers ready to load the data.
    RmtErrorCode result = RmtStreamMergerReset(&data_set->stream_merger, data_set->file_handle);

    while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
    {
        // grab the next token from the heap.
        RmtToken     current_token;
        RmtErrorCode error_code = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
        RMT_ASSERT(error_code == kRmtOk);

        // interested in tokens that directly reference resources.
        switch (current_token.type)
        {
        case kRmtTokenTypeResourceCreate:
            if (current_token.resource_create_token.resource_identifier != resource->identifier)
            {
                break;
            }

            RmtResourceHistoryAddEvent(out_resource_history,
                                       kRmtResourceHistoryEventResourceCreated,
                                       current_token.common.thread_id,
                                       current_token.common.timestamp,
                                       0,
                                       0,
                                       0,
                                       0,
                                       false);
            break;

        case kRmtTokenTypeResourceDestroy:
            if (current_token.resource_destroy_token.resource_identifier != resource->identifier)
            {
                break;
            }

            RmtResourceHistoryAddEvent(out_resource_history,
                                       kRmtResourceHistoryEventResourceDestroyed,
                                       current_token.common.thread_id,
                                       current_token.common.timestamp,
                                       0,
                                       0,
                                       0,
                                       0,
                                       false);
            break;

        case kRmtTokenTypeResourceBind:
            if (current_token.resource_bind_token.resource_identifier != resource->identifier)
            {
                break;
            }

            RmtResourceHistoryAddEvent(out_resource_history,
                                       kRmtResourceHistoryEventResourceBound,
                                       current_token.common.thread_id,
                                       current_token.common.timestamp,
                                       current_token.resource_bind_token.virtual_address,
                                       0,
                                       0,
                                       0,
                                       false);
            break;

        case kRmtTokenTypeVirtualAllocate:
        {
            const RmtGpuAddress address_of_last_byte_allocation =
                (current_token.virtual_allocate_token.virtual_address + current_token.virtual_allocate_token.size_in_bytes) - 1;
            if (!RmtResourceOverlapsVirtualAddressRange(resource, current_token.virtual_allocate_token.virtual_address, address_of_last_byte_allocation))
            {
                break;
            }

            RmtResourceHistoryAddEvent(out_resource_history,
                                       kRmtResourceHistoryEventVirtualMemoryAllocated,
                                       current_token.common.thread_id,
                                       current_token.common.timestamp,
                                       current_token.virtual_allocate_token.virtual_address,
                                       0,
                                       current_token.virtual_allocate_token.size_in_bytes,
                                       0,
                                       false);
        }
        break;

        case kRmtTokenTypeResourceReference:

            if (out_resource_history->base_allocation == nullptr)
            {
                break;
            }

            // NOTE: PAL can only make resident/evict a full virtual allocation on CPU, not just a single resource.
            if (current_token.resource_reference.virtual_address != out_resource_history->base_allocation->base_address)
            {
                break;
            }

            if (current_token.resource_reference.residency_update_type == kRmtResidencyUpdateTypeAdd)
            {
                RmtResourceHistoryAddEvent(out_resource_history,
                                           kRmtResourceHistoryEventVirtualMemoryMakeResident,
                                           current_token.common.thread_id,
                                           current_token.common.timestamp,
                                           current_token.resource_reference.virtual_address,
                                           0,
                                           0,
                                           0,
                                           false);
            }
            else
            {
                RmtResourceHistoryAddEvent(out_resource_history,
                                           kRmtResourceHistoryEventVirtualMemoryEvict,
                                           current_token.common.thread_id,
                                           current_token.common.timestamp,
                                           current_token.resource_reference.virtual_address,
                                           0,
                                           0,
                                           0,
                                           false);
            }
            break;

        case kRmtTokenTypeCpuMap:

            if (out_resource_history->base_allocation == nullptr)
            {
                break;
            }

            // NOTE: PAL can only map/unmap a full virtual allocation on CPU, not just a resource.
            if (current_token.cpu_map_token.virtual_address != out_resource_history->base_allocation->base_address)
            {
                break;
            }

            if (current_token.cpu_map_token.is_unmap)
            {
                RmtResourceHistoryAddEvent(out_resource_history,
                                           kRmtResourceHistoryEventVirtualMemoryUnmapped,
                                           current_token.common.thread_id,
                                           current_token.common.timestamp,
                                           current_token.cpu_map_token.virtual_address,
                                           0,
                                           0,
                                           0,
                                           false);
            }
            else
            {
                RmtResourceHistoryAddEvent(out_resource_history,
                                           kRmtResourceHistoryEventVirtualMemoryMapped,
                                           current_token.common.thread_id,
                                           current_token.common.timestamp,
                                           current_token.cpu_map_token.virtual_address,
                                           0,
                                           0,
                                           0,
                                           false);
            }
            break;

        case kRmtTokenTypeVirtualFree:
        {
            if (out_resource_history->base_allocation == nullptr)
            {
                break;
            }

            if (current_token.virtual_free_token.virtual_address != out_resource_history->base_allocation->base_address)
            {
                break;
            }

            const uint64_t      size_in_bytes = RmtGetAllocationSizeInBytes(out_resource_history->base_allocation->size_in_4kb_page, kRmtPageSize4Kb);
            const RmtGpuAddress address_start = current_token.virtual_free_token.virtual_address;
            const RmtGpuAddress address_end   = address_start + size_in_bytes - 1;
            if (!RmtResourceOverlapsVirtualAddressRange(resource, address_start, address_end))
            {
                break;
            }

            RmtResourceHistoryAddEvent(out_resource_history,
                                       kRmtResourceHistoryEventVirtualMemoryFree,
                                       current_token.common.thread_id,
                                       current_token.common.timestamp,
                                       current_token.virtual_free_token.virtual_address,
                                       0,
                                       size_in_bytes,
                                       0,
                                       false);
        }
        break;

        case kRmtTokenTypePageTableUpdate:
        {
            if (out_resource_history->base_allocation == nullptr)
            {
                break;
            }

            // check for overlap between the resource VA range and this change to the PA mappings.
            const uint64_t size_in_bytes =
                RmtGetAllocationSizeInBytes(current_token.page_table_update_token.size_in_pages, current_token.page_table_update_token.page_size);
            const uint64_t page_size_in_bytes = RmtGetPageSize(current_token.page_table_update_token.page_size);

            if (!RmtAllocationsOverlap(current_token.page_table_update_token.virtual_address,
                                       size_in_bytes,
                                       out_resource_history->resource->address,
                                       out_resource_history->resource->size_in_bytes))
            {
                break;
            }

            if (current_token.page_table_update_token.is_unmapping)
            {
                RmtResourceHistoryAddEvent(out_resource_history,
                                           kRmtResourceHistoryEventPhysicalUnmap,
                                           current_token.common.thread_id,
                                           current_token.common.timestamp,
                                           current_token.page_table_update_token.virtual_address,
                                           current_token.page_table_update_token.physical_address,
                                           size_in_bytes,
                                           page_size_in_bytes,
                                           true);
            }
            else
            {
                if (current_token.page_table_update_token.physical_address == 0)
                {
                    RmtResourceHistoryAddEvent(out_resource_history,
                                               kRmtResourceHistoryEventPhysicalMapToHost,
                                               current_token.common.thread_id,
                                               current_token.common.timestamp,
                                               current_token.page_table_update_token.virtual_address,
                                               current_token.page_table_update_token.physical_address,
                                               size_in_bytes,
                                               page_size_in_bytes,
                                               true);
                }
                else
                {
                    RmtResourceHistoryAddEvent(out_resource_history,
                                               kRmtResourceHistoryEventPhysicalMapToLocal,
                                               current_token.common.thread_id,
                                               current_token.common.timestamp,
                                               current_token.page_table_update_token.virtual_address,
                                               current_token.page_table_update_token.physical_address,
                                               size_in_bytes,
                                               page_size_in_bytes,
                                               true);
                }
            }
        }
        break;

        default:
            break;
        }
    }

    if (result == kRmtOk)
    {
        // Name UserData tokens have correlation IDs or 32 bit driver resource IDs so they can't be filtered by the internal resource ID the way other tokens are.
        // Instead, The ResourceUserData module is levereged to populate the resource history with the ResourceNamed events.
        result = RmtResourceUserdataUpdateNamedResourceHistoryEvents(out_resource_history);
    }
    return result;
}

// Helper functo call the correct free function.
static void PerformFree(RmtDataSet* data_set, void* pointer)
{
    if (data_set->free_func == nullptr)
    {
        return free(pointer);
    }

    return (data_set->free_func)(pointer);
}

RmtErrorCode RmtDataSnapshotGenerateResourceHistory(RmtDataSnapshot* snapshot, const RmtResource* resource, RmtResourceHistory* out_resource_history)
{
    RMT_ASSERT(snapshot);
    RMT_ASSERT(resource);
    RMT_ASSERT(out_resource_history);
    RMT_RETURN_ON_ERROR(snapshot, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(resource, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_resource_history, kRmtErrorInvalidPointer);

    // stash the pointer to the resource and the underlaying VA.
    out_resource_history->resource        = resource;
    out_resource_history->base_allocation = resource->bound_allocation;
    out_resource_history->event_count     = 0;

    RmtErrorCode error_code = kRmtOk;

    error_code = ProcessTokensIntoResourceHistory(snapshot->data_set, resource, out_resource_history);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    return kRmtOk;
}

RmtErrorCode RmtDataSnapshotDestroy(RmtDataSnapshot* snapshot)
{
    RMT_RETURN_ON_ERROR(snapshot, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(snapshot->data_set, kRmtErrorMalformedData);

    // free the memory allocated for the snapshot.
    PerformFree(snapshot->data_set, snapshot->virtual_allocation_buffer);
    PerformFree(snapshot->data_set, snapshot->resource_list_buffer);
    PerformFree(snapshot->data_set, snapshot->region_stack_buffer);

    return kRmtOk;
}

uint64_t RmtDataSnapshotGetLargestResourceSize(const RmtDataSnapshot* snapshot)
{
    RMT_RETURN_ON_ERROR(snapshot, 0);

    uint64_t latest_resource_size_in_bytes = 0;

    for (int32_t current_resource_index = 0; current_resource_index < snapshot->resource_list.resource_count; ++current_resource_index)
    {
        const RmtResource* current_resource = &snapshot->resource_list.resources[current_resource_index];
        latest_resource_size_in_bytes       = RMT_MAXIMUM(latest_resource_size_in_bytes, current_resource->size_in_bytes);
    }

    return latest_resource_size_in_bytes;
}

uint64_t RmtDataSnapshotGetLargestUnboundResourceSize(const RmtDataSnapshot* snapshot)
{
    RMT_RETURN_ON_ERROR(snapshot, 0);

    uint64_t latest_unbound_resource_size_in_bytes = 0;

    int32_t allocation_count = snapshot->virtual_allocation_list.allocation_count;
    for (int32_t current_virtual_allocation_index = 0; current_virtual_allocation_index < allocation_count; ++current_virtual_allocation_index)
    {
        const RmtVirtualAllocation* current_virtual_allocation = &snapshot->virtual_allocation_list.allocation_details[current_virtual_allocation_index];

        int32_t unbound_region_count = current_virtual_allocation->unbound_memory_region_count;
        for (int32_t unbound_region_index = 0; unbound_region_index < unbound_region_count; ++unbound_region_index)
        {
            uint64_t size = current_virtual_allocation->unbound_memory_regions[unbound_region_index].size;
            if (size > latest_unbound_resource_size_in_bytes)
            {
                latest_unbound_resource_size_in_bytes = size;
            }
        }
    }
    return latest_unbound_resource_size_in_bytes;
}

/// Get the smallest resource size (in bytes) seen in a snapshot.
uint64_t RmtDataSnapshotGetSmallestResourceSize(const RmtDataSnapshot* snapshot)
{
    RMT_RETURN_ON_ERROR(snapshot, 0);

    uint64_t smallest_resource_size_in_bytes = UINT64_MAX;

    for (int32_t current_resource_index = 0; current_resource_index < snapshot->resource_list.resource_count; ++current_resource_index)
    {
        const RmtResource* current_resource = &snapshot->resource_list.resources[current_resource_index];
        smallest_resource_size_in_bytes     = RMT_MINIMUM(smallest_resource_size_in_bytes, current_resource->adjusted_size_in_bytes);
    }

    if (smallest_resource_size_in_bytes == UINT64_MAX)
    {
        return 0;
    }

    return smallest_resource_size_in_bytes;
}

RmtErrorCode RmtDataSnapshotGetSegmentStatus(const RmtDataSnapshot* snapshot, RmtHeapType heap_type, RmtSegmentStatus* out_segment_status)
{
    RMT_RETURN_ON_ERROR(snapshot, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_segment_status, kRmtErrorInvalidPointer);

    RMT_ASSERT(heap_type != kRmtHeapTypeUnknown);
    RMT_ASSERT(heap_type != kRmtHeapTypeNone);

    out_segment_status->heap_type = heap_type;

    // all this stuff is flagged.
    switch (heap_type)
    {
    case kRmtHeapTypeInvisible:
        out_segment_status->flags |= kRmtSegmentStatusFlagVram;
        out_segment_status->flags |= kRmtSegmentStatusFlagGpuVisible;
        out_segment_status->flags |= kRmtSegmentStatusFlagGpuCached;
        break;
    case kRmtHeapTypeLocal:
        out_segment_status->flags |= kRmtSegmentStatusFlagVram;
        out_segment_status->flags |= kRmtSegmentStatusFlagGpuVisible;
        out_segment_status->flags |= kRmtSegmentStatusFlagGpuCached;
        out_segment_status->flags |= kRmtSegmentStatusFlagCpuVisible;
        break;
    case kRmtHeapTypeSystem:
        out_segment_status->flags |= kRmtSegmentStatusFlagHost;
        out_segment_status->flags |= kRmtSegmentStatusFlagGpuVisible;
        out_segment_status->flags |= kRmtSegmentStatusFlagGpuCached;
        out_segment_status->flags |= kRmtSegmentStatusFlagCpuVisible;
        out_segment_status->flags |= kRmtSegmentStatusFlagCpuCached;
        break;

    default:
        break;
    }

    out_segment_status->total_physical_size        = snapshot->data_set->segment_info[heap_type].size;
    out_segment_status->total_bound_virtual_memory = 0;
    out_segment_status->allocation_count           = 0;
    out_segment_status->resource_count             = 0;

    // calculate data for the segment info fields.
    uint64_t max_virtual_allocation_size      = 0;
    uint64_t min_virtual_allocation_size      = UINT64_MAX;
    uint64_t total_virtual_memory_requested   = 0;
    uint64_t total_physical_mapped_by_process = snapshot->page_table.mapped_per_heap[heap_type];
    uint64_t allocation_count                 = 0;

    // set the resource committed memory values.
    for (int32_t current_resource_usage_index = 0; current_resource_usage_index < kRmtResourceUsageTypeCount; ++current_resource_usage_index)
    {
        out_segment_status->physical_bytes_per_resource_usage[current_resource_usage_index] = 0;
    }

    for (int32_t current_virtual_allocation_index = 0; current_virtual_allocation_index < snapshot->virtual_allocation_list.allocation_count;
         ++current_virtual_allocation_index)
    {
        const RmtVirtualAllocation* current_virtual_allocation = &snapshot->virtual_allocation_list.allocation_details[current_virtual_allocation_index];

        if (current_virtual_allocation->heap_preferences[0] == heap_type)
        {
            const uint64_t size_in_bytes = RmtGetAllocationSizeInBytes(current_virtual_allocation->size_in_4kb_page, kRmtPageSize4Kb);

            total_virtual_memory_requested += size_in_bytes;
            max_virtual_allocation_size = RMT_MAXIMUM(max_virtual_allocation_size, size_in_bytes);
            min_virtual_allocation_size = RMT_MINIMUM(min_virtual_allocation_size, size_in_bytes);
            allocation_count++;

            // Get the size of the resources in the allocation.
            uint64_t memory_region_size = RmtVirtualAllocationGetTotalResourceMemoryInBytes(snapshot, current_virtual_allocation);
            out_segment_status->total_bound_virtual_memory += memory_region_size;
            RMT_ASSERT(size_in_bytes >= memory_region_size);

            out_segment_status->resource_count += current_virtual_allocation->non_heap_resource_count;
        }

        // Walk each resource in the allocation and work out what heap each resource is in.
        for (int32_t current_resource_index = 0; current_resource_index < current_virtual_allocation->resource_count; ++current_resource_index)
        {
            const RmtResource* current_resource = current_virtual_allocation->resources[current_resource_index];

            if (current_resource->resource_type == kRmtResourceTypeHeap)
            {
                continue;
            }

            // Process the resource.
            const RmtResourceUsageType current_resource_usage = RmtResourceGetUsageType(current_resource);

            // Calculate the histogram of where each resource has its memory committed.
            uint64_t resource_histogram[kRmtResourceBackingStorageCount] = {0};
            RmtResourceGetBackingStorageHistogram(snapshot, current_resource, resource_histogram);
            out_segment_status->physical_bytes_per_resource_usage[current_resource_usage] += resource_histogram[heap_type];
        }
    }

    if (min_virtual_allocation_size == UINT64_MAX)
    {
        min_virtual_allocation_size = 0;
    }

    // fill out the structure fields.
    out_segment_status->total_virtual_memory_requested           = total_virtual_memory_requested;
    out_segment_status->total_physical_mapped_by_process         = total_physical_mapped_by_process;
    out_segment_status->total_physical_mapped_by_other_processes = 0;
    out_segment_status->max_allocation_size                      = max_virtual_allocation_size;
    out_segment_status->min_allocation_size                      = min_virtual_allocation_size;
    out_segment_status->committed_size                           = snapshot->snapshot_point->committed_memory[heap_type];
    out_segment_status->allocation_count                         = allocation_count;
    if (allocation_count > 0)
    {
        out_segment_status->mean_allocation_size = total_virtual_memory_requested / allocation_count;
    }
    else
    {
        out_segment_status->mean_allocation_size = 0;
    }

    return kRmtOk;
}

// calculate the seg. status.
RmtSegmentSubscriptionStatus RmtSegmentStatusGetOversubscribed(const RmtSegmentStatus* segment_status)
{
    const uint64_t close_limit = (uint64_t)((double)segment_status->total_physical_size * 0.8);
    if (segment_status->total_virtual_memory_requested > segment_status->total_physical_size)
    {
        return kRmtSegmentSubscriptionStatusOverLimit;
    }
    if (segment_status->total_virtual_memory_requested > close_limit)
    {
        return kRmtSegmentSubscriptionStatusCloseToLimit;
    }

    return kRmtSegmentSubscriptionStatusUnderLimit;
}

// get the heap type for a physical address.
RmtHeapType RmtDataSnapshotGetSegmentForAddress(const RmtDataSnapshot* snapshot, RmtGpuAddress gpu_address)
{
    RMT_RETURN_ON_ERROR(snapshot, kRmtHeapTypeUnknown);

    // special case for system memory.
    if (gpu_address == 0)
    {
        return kRmtHeapTypeSystem;
    }

    for (int32_t current_segment_index = 0; current_segment_index < snapshot->data_set->segment_info_count; ++current_segment_index)
    {
        const RmtGpuAddress start_address = snapshot->data_set->segment_info->base_address;
        const RmtGpuAddress end_address =
            snapshot->data_set->segment_info[current_segment_index].base_address + snapshot->data_set->segment_info[current_segment_index].size;

        if (start_address <= gpu_address && gpu_address < end_address)
        {
            return snapshot->data_set->segment_info[current_segment_index].heap_type;
        }
    }

    return kRmtHeapTypeUnknown;
}
