//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the physical allocation list functions.
//=============================================================================

#include "rmt_physical_allocation_list.h"
#include <rmt_assert.h>
#include <string.h>  // memcpy
#include <math.h>    // for sqrt

// calculate the size of an allocation in bytes.
uint64_t RmtPhysicalAllocationGetSizeInBytes(const RmtPhysicalAllocation* physical_allocation)
{
    RMT_ASSERT(physical_allocation);
    RMT_RETURN_ON_ERROR(physical_allocation, 0);

    return ((uint64_t)physical_allocation->size_in_4kb_page) << 12;
}

// NOTE: likely that we are going to do more queries than insert/delete, may want to accelerate the lookup using a tree.

static int32_t FindAllocationIndexForAddress(const RmtPhysicalAllocationList* physical_allocation_list, RmtGpuAddress address)
{
    for (int32_t current_allocation_index = 0; current_allocation_index < physical_allocation_list->allocation_count; ++current_allocation_index)
    {
        const RmtPhysicalAllocationInterval* current_allocation_interval = &physical_allocation_list->allocation_intervals[current_allocation_index];
        const int64_t                        size_in_bytes               = ((uint64_t)current_allocation_interval->size_in_4kb_pages) << 12;
        if (current_allocation_interval->base_address <= address && address < (current_allocation_interval->base_address + size_in_bytes))
        {
            return current_allocation_index;
        }
    }

    return -1;
}

// get the size of the buffer required for an allocation list, given a specific number of concurrent allocations.
size_t RmtPhysicalAllocationListGetBufferSize(int32_t maximum_concurrent_allocations)
{
    return maximum_concurrent_allocations * (sizeof(RmtPhysicalAllocationInterval) + sizeof(RmtPhysicalAllocation));
}

// initialize the data structure.
RmtErrorCode RmtPhysicalAllocationListInitialize(RmtPhysicalAllocationList* physical_allocation_list,
                                                 void*                      buffer,
                                                 size_t                     buffer_size,
                                                 int32_t                    maximum_concurrent_allocations)
{
    RMT_ASSERT(physical_allocation_list);
    RMT_RETURN_ON_ERROR(physical_allocation_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(buffer, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(buffer_size, kRmtErrorInvalidSize);
    RMT_RETURN_ON_ERROR(RmtPhysicalAllocationListGetBufferSize(maximum_concurrent_allocations) <= buffer_size, kRmtErrorInvalidSize);

    // dice up the buffer
    physical_allocation_list->allocation_intervals = (RmtPhysicalAllocationInterval*)buffer;
    physical_allocation_list->allocation_details =
        (RmtPhysicalAllocation*)(((uintptr_t)buffer) + (maximum_concurrent_allocations * sizeof(RmtPhysicalAllocationInterval)));
    physical_allocation_list->allocation_count               = 0;
    physical_allocation_list->next_allocation_guid           = 0;
    physical_allocation_list->maximum_concurrent_allocations = maximum_concurrent_allocations;

    return kRmtOk;
}

// add an allocation to the list.
RmtErrorCode RmtPhysicalAllocationListAddAllocation(RmtPhysicalAllocationList* physical_allocation_list,
                                                    uint64_t                   timestamp,
                                                    RmtGpuAddress              address,
                                                    int32_t                    size_in_4kb_pages,
                                                    RmtHeapType                heap_type,
                                                    RmtProcessId               process_id)
{
    RMT_ASSERT(physical_allocation_list);
    RMT_RETURN_ON_ERROR(physical_allocation_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(size_in_4kb_pages, kRmtErrorInvalidSize);
    RMT_RETURN_ON_ERROR((address >> 12) + size_in_4kb_pages < RMT_PAGE_TABLE_MAX_SIZE, kRmtErrorInvalidSize);

    const int32_t next_allocation_index = physical_allocation_list->allocation_count++;

    // fill out the allocation interval
    RmtPhysicalAllocationInterval* allocation_interval = &physical_allocation_list->allocation_intervals[next_allocation_index];
    allocation_interval->base_address                  = address;
    allocation_interval->size_in_4kb_pages             = size_in_4kb_pages;

    // fill out the details.
    RmtPhysicalAllocation* allocation_details = &physical_allocation_list->allocation_details[next_allocation_index];
    allocation_details->base_address          = address;
    allocation_details->size_in_4kb_page      = size_in_4kb_pages;
    allocation_details->guid                  = physical_allocation_list->next_allocation_guid++;
    allocation_details->flags                 = 0;
    allocation_details->timestamp             = timestamp;
    allocation_details->process_id            = process_id;
    allocation_details->heap_type             = heap_type;

    return kRmtOk;
}

// tradtional free of an allocation from the list.
RmtErrorCode RmtPhysicalAllocationListDiscardAllocation(RmtPhysicalAllocationList* physical_allocation_list, RmtGpuAddress address)
{
    RMT_ASSERT(physical_allocation_list);
    RMT_RETURN_ON_ERROR(physical_allocation_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(physical_allocation_list->allocation_count, kRmtErrorNoAllocationFound);

    // find the allocation index.
    const int32_t index = FindAllocationIndexForAddress(physical_allocation_list, address);
    if (index < 0)
    {
        return kRmtErrorNoAllocationFound;
    }

    const int32_t last_physica_allocation_index = physical_allocation_list->allocation_count - 1;

    // special case for deleting the last element of the list.
    if (index == last_physica_allocation_index)
    {
        physical_allocation_list->allocation_count--;
        return kRmtOk;
    }

    // copy over.
    RmtPhysicalAllocation*       current_physical_allocation = &physical_allocation_list->allocation_details[index];
    const RmtPhysicalAllocation* last_physical_allocation    = &physical_allocation_list->allocation_details[last_physica_allocation_index];
    memcpy(current_physical_allocation, last_physical_allocation, sizeof(RmtPhysicalAllocation));
    physical_allocation_list->allocation_count--;

    return kRmtOk;
}

// transfer of memory to system.
RmtErrorCode RmtPhysicaAllocationListTransferAllocation(RmtPhysicalAllocationList* physical_allocation_list, RmtGpuAddress address)
{
    RMT_ASSERT(physical_allocation_list);
    RMT_RETURN_ON_ERROR(physical_allocation_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(physical_allocation_list->allocation_count, kRmtErrorNoAllocationFound);

    const int32_t index = FindAllocationIndexForAddress(physical_allocation_list, address);
    if (index < 0)
    {
        return kRmtErrorNoAllocationFound;
    }

    RmtPhysicalAllocation* allocation = &physical_allocation_list->allocation_details[index];
    allocation->flags |= kRmtPhysicalAllocationFlagTransferred;

    return kRmtOk;
}

// find an allocation for an address.
RmtErrorCode RmtPhysicalAllocationListGetAllocationForAddress(const RmtPhysicalAllocationList* physical_allocation_list,
                                                              RmtGpuAddress                    address,
                                                              const RmtPhysicalAllocation**    out_allocation)
{
    RMT_ASSERT(physical_allocation_list);
    RMT_ASSERT(out_allocation);
    RMT_RETURN_ON_ERROR(physical_allocation_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_allocation, kRmtErrorInvalidPointer);

    const int32_t index = FindAllocationIndexForAddress(physical_allocation_list, address);
    if (index < 0)
    {
        return kRmtErrorNoAllocationFound;
    }

    const RmtPhysicalAllocation* allocation = &physical_allocation_list->allocation_details[index];
    *out_allocation                         = allocation;
    return kRmtOk;
}

RmtErrorCode RmtPhysicalAllocationListGetAllocationAtIndex(const RmtPhysicalAllocationList* physical_allocation_list,
                                                           int32_t                          index,
                                                           const RmtPhysicalAllocation**    out_allocation)
{
    RMT_ASSERT(physical_allocation_list);
    RMT_ASSERT(out_allocation);
    RMT_RETURN_ON_ERROR(physical_allocation_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_allocation, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(0 <= index && index < physical_allocation_list->allocation_count, kRmtErrorIndexOutOfRange);

    const RmtPhysicalAllocation* allocation = &physical_allocation_list->allocation_details[index];
    *out_allocation                         = allocation;
    return kRmtOk;
}

uint64_t RmtPhysicalAllocationListGetTotalSizeInBytes(const RmtPhysicalAllocationList* physical_allocation_list)
{
    RMT_ASSERT(physical_allocation_list);
    RMT_RETURN_ON_ERROR(physical_allocation_list, 0);

    uint64_t total_size_in_bytes = 0;

    for (int32_t current_allocation_index = 0; current_allocation_index < physical_allocation_list->allocation_count; ++current_allocation_index)
    {
        const RmtPhysicalAllocation* current_physical_allocation = &physical_allocation_list->allocation_details[current_allocation_index];
        total_size_in_bytes += RmtPhysicalAllocationGetSizeInBytes(current_physical_allocation);
    }

    return total_size_in_bytes;
}
