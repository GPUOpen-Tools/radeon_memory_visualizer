//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Game Engineering Group
/// \brief  Implementation of the page table helper functions.
//=============================================================================

#include "rmt_page_table.h"
#include "rmt_address_helper.h"
#include <rmt_assert.h>
#include "rmt_virtual_allocation_list.h"
#include "rmt_resource_list.h"
#include <string.h>  // for memcpy

// helper function to decompose an address
static void DecomposeAddress(RmtGpuAddress virtual_address,
                             int32_t*      out_level0_radix,
                             int32_t*      out_level1_radix,
                             int32_t*      out_level2_radix,
                             int32_t*      out_level3_radix)
{
    RMT_ASSERT((virtual_address >> 48) == 0);

    // Decompose the virtual address into 4 radixes for looking into the trie. The format of the
    // address is 10:10:8:8. First of all we have to calculate a VA page offset as the trie
    // structure deals at 4KiB page granularity.
    const uint64_t virtual_page_offset = virtual_address >> 12;
    const int32_t  level0_radix        = (virtual_page_offset >> 26) & 0x3ff;
    const int32_t  level1_radix        = (virtual_page_offset >> 16) & 0x3ff;
    const int32_t  level2_radix        = (virtual_page_offset >> 8) & 0xff;
    const int32_t  level3_radix        = (virtual_page_offset >> 0) & 0xff;

    // check the radix never goes out of range, this will stomp memory.
    RMT_ASSERT(level0_radix < RMT_PAGE_DIRECTORY_LEVEL_0_SIZE);
    RMT_ASSERT(level1_radix < RMT_PAGE_DIRECTORY_LEVEL_1_SIZE);
    RMT_ASSERT(level2_radix < RMT_PAGE_DIRECTORY_LEVEL_2_SIZE);
    RMT_ASSERT(level3_radix < RMT_PAGE_DIRECTORY_LEVEL_3_SIZE);

    *out_level0_radix = level0_radix;
    *out_level1_radix = level1_radix;
    *out_level2_radix = level2_radix;
    *out_level3_radix = level3_radix;
}

// helper function to work out the physical heap from an physical address.
static RmtHeapType GetHeapTypeFromAddress(RmtPageTable* page_table, RmtGpuAddress physical_address)
{
    // Calculate the heap that we are updating.
    RmtHeapType current_segment = kRmtHeapTypeUnknown;
    if (physical_address == 0)
    {
        current_segment = kRmtHeapTypeSystem;
    }
    else
    {
        for (int32_t current_segment_index = 0; current_segment_index <= kRmtHeapTypeInvisible; ++current_segment_index)
        {
            const RmtGpuAddress end_address =
                page_table->segment_info[current_segment_index].base_address + page_table->segment_info[current_segment_index].size;
            if (page_table->segment_info[current_segment_index].base_address <= physical_address && physical_address < end_address)
            {
                current_segment = (RmtHeapType)current_segment_index;
                break;
            }
        }
    }

    return current_segment;
}

// update mapping for a single 4KB page.
static void UpdateMappingForSingle4KPage(RmtPageTable* page_table,
                                         int32_t       level0_radix,
                                         int32_t       level1_radix,
                                         int32_t       level2_radix,
                                         int32_t       level3_radix,
                                         RmtGpuAddress physical_address,
                                         bool          is_unmapping)
{
    // The first three nodes have a similar idea that they are implementing. If we didn't
    // already have a level 1 node for this radix, then we can allocate one now. When we
    // first allocate a node, we should set all pointers to the next page directory level to NULL.
    RmtPageDirectoryLevel1* level1 = page_table->level0[level0_radix];
    if (level1 == nullptr)
    {
        level1 = (RmtPageDirectoryLevel1*)RmtPoolAllocate(&page_table->level1_allocator);
        RMT_ASSERT(level1);
        for (int32_t current_page_directory_level_index = 0; current_page_directory_level_index < RMT_PAGE_DIRECTORY_LEVEL_1_SIZE;
             ++current_page_directory_level_index)
        {
            level1->page_directory[current_page_directory_level_index] = NULL;
        }
        page_table->level0[level0_radix] = level1;
    }

    RmtPageDirectoryLevel2* level2 = level1->page_directory[level1_radix];
    if (level2 == nullptr)
    {
        level2 = (RmtPageDirectoryLevel2*)RmtPoolAllocate(&page_table->level2_allocator);
        RMT_ASSERT(level2);
        for (int32_t current_page_directory_level_index = 0; current_page_directory_level_index < RMT_PAGE_DIRECTORY_LEVEL_2_SIZE;
             ++current_page_directory_level_index)
        {
            level2->page_directory[current_page_directory_level_index] = NULL;
        }
        level1->page_directory[level1_radix] = level2;
    }

    RmtPageDirectoryLevel3* level3 = level2->page_directory[level2_radix];
    if (level3 == nullptr)
    {
        level3 = (RmtPageDirectoryLevel3*)RmtPoolAllocate(&page_table->level3_allocator);
        RMT_ASSERT(level3);
        for (int32_t current_page_directory_level_index = 0; current_page_directory_level_index < RMT_MAXIMUM_PAGE_TABLE_LEAF_SIZE;
             ++current_page_directory_level_index)
        {
            level3->physical_addresses[current_page_directory_level_index] = (uint8_t)NULL;
        }
        for (size_t current_bit_field_byte = 0; current_bit_field_byte < sizeof(level3->is_mapped); ++current_bit_field_byte)
        {
            level3->is_mapped[current_bit_field_byte] = 0;
        }
        level2->page_directory[level2_radix] = level3;
    }

    // Update specific slot in the level3 node.
    const int32_t byte_offset = level3_radix / 8;
    const int32_t bit_offset  = level3_radix % 8;
    const uint8_t mask        = (1 << bit_offset);

    // remove from mapped totals if it was previously mapped.
    bool was_previously_mapped = ((level3->is_mapped[byte_offset] & mask) == mask);
    if (was_previously_mapped)
    {
        const RmtGpuAddress previous_physical_address = ((RmtGpuAddress)level3->physical_addresses[(level3_radix * 6) + 0] << 40) |
                                                        ((RmtGpuAddress)level3->physical_addresses[(level3_radix * 6) + 1] << 32) |
                                                        ((RmtGpuAddress)level3->physical_addresses[(level3_radix * 6) + 2] << 24) |
                                                        ((RmtGpuAddress)level3->physical_addresses[(level3_radix * 6) + 3] << 16) |
                                                        ((RmtGpuAddress)level3->physical_addresses[(level3_radix * 6) + 4] << 8) |
                                                        ((RmtGpuAddress)level3->physical_addresses[(level3_radix * 6) + 5] << 0);

        const RmtHeapType previous_physical_heap_type = GetHeapTypeFromAddress(page_table, previous_physical_address);
        RMT_ASSERT(previous_physical_heap_type != kRmtHeapTypeUnknown);

        if (page_table->mapped_per_heap[previous_physical_heap_type] > RmtGetPageSize(kRmtPageSize4Kb))
        {
            page_table->mapped_per_heap[previous_physical_heap_type] -= RmtGetPageSize(kRmtPageSize4Kb);
        }
        else
        {
            page_table->mapped_per_heap[previous_physical_heap_type] = 0;
        }
    }

    // Store the physical address.
    if (!is_unmapping)
    {
        // TOOD: Check if it was previously mapped, and if so, find out which segment it was in.
        level3->physical_addresses[(level3_radix * 6) + 0] = ((physical_address >> 40) & 0xff);
        level3->physical_addresses[(level3_radix * 6) + 1] = ((physical_address >> 32) & 0xff);
        level3->physical_addresses[(level3_radix * 6) + 2] = ((physical_address >> 24) & 0xff);
        level3->physical_addresses[(level3_radix * 6) + 3] = ((physical_address >> 16) & 0xff);
        level3->physical_addresses[(level3_radix * 6) + 4] = ((physical_address >> 8) & 0xff);
        level3->physical_addresses[(level3_radix * 6) + 5] = ((physical_address >> 0) & 0xff);
        level3->is_mapped[byte_offset] |= mask;

        const RmtHeapType current_physical_heap_type = GetHeapTypeFromAddress(page_table, physical_address);
        page_table->mapped_per_heap[current_physical_heap_type] += RmtGetPageSize(kRmtPageSize4Kb);
    }
    else
    {
        level3->physical_addresses[(level3_radix * 6) + 0] = 0;
        level3->physical_addresses[(level3_radix * 6) + 1] = 0;
        level3->physical_addresses[(level3_radix * 6) + 2] = 0;
        level3->physical_addresses[(level3_radix * 6) + 3] = 0;
        level3->physical_addresses[(level3_radix * 6) + 4] = 0;
        level3->physical_addresses[(level3_radix * 6) + 5] = 0;
        level3->is_mapped[byte_offset] &= ~mask;
    }
}

// Initialize the page table.
RmtErrorCode RmtPageTableInitialize(RmtPageTable* page_table, const RmtSegmentInfo* segment_info, int32_t segment_info_count, uint64_t target_process_id)
{
    RMT_ASSERT(page_table);
    RMT_RETURN_ON_ERROR(page_table, RMT_ERROR_INVALID_POINTER);

    // copy the segment info over.
    memcpy(page_table->segment_info, segment_info, sizeof(RmtSegmentInfo) * segment_info_count);
    page_table->segment_info_count = segment_info_count;
    page_table->target_process_id  = target_process_id;

    // Initialize the level 1 node pointers to be NULL to denote an empty page table.
    for (int32_t current_level0_node_index = 0; current_level0_node_index < RMT_PAGE_DIRECTORY_LEVEL_0_SIZE; ++current_level0_node_index)
    {
        page_table->level0[current_level0_node_index] = NULL;
    }

    // Initialize the allocators for level 1, 2 and 3 nodes.
    RmtErrorCode error_code = RMT_OK;
    error_code = RmtPoolInitialize(&page_table->level1_allocator, page_table->level1_nodes, sizeof(page_table->level1_nodes), sizeof(RmtPageDirectoryLevel1));
    RMT_ASSERT(error_code == RMT_OK);
    RMT_RETURN_ON_ERROR(error_code == RMT_OK, error_code);
    error_code = RmtPoolInitialize(&page_table->level2_allocator, page_table->level2_nodes, sizeof(page_table->level2_nodes), sizeof(RmtPageDirectoryLevel2));
    RMT_ASSERT(error_code == RMT_OK);
    RMT_RETURN_ON_ERROR(error_code == RMT_OK, error_code);
    error_code = RmtPoolInitialize(&page_table->level3_allocator, page_table->level3_nodes, sizeof(page_table->level3_nodes), sizeof(RmtPageDirectoryLevel3));
    RMT_ASSERT(error_code == RMT_OK);
    RMT_RETURN_ON_ERROR(error_code == RMT_OK, error_code);

    // clear per-heap byte tracking.
    for (int32_t current_heap_index = 0; current_heap_index < kRmtHeapTypeCount; ++current_heap_index)
    {
        page_table->mapped_per_heap[current_heap_index] = 0;
    }

    return RMT_OK;
}

// Map virtual memory.
RmtErrorCode RmtPageTableUpdateMemoryMappings(RmtPageTable*          page_table,
                                              RmtGpuAddress          virtual_address,
                                              RmtGpuAddress          physical_address,
                                              size_t                 size_in_pages,
                                              RmtPageSize            page_size,
                                              bool                   is_unmapping,
                                              RmtPageTableUpdateType update_type,
                                              uint64_t               process_id)
{
    RMT_UNUSED(process_id);

    RMT_ASSERT(page_table);
    RMT_RETURN_ON_ERROR(page_table, RMT_ERROR_INVALID_POINTER);

    // For now, we ignore anything that's not a regular update.
    if (update_type != kRmtPageTableUpdateTypeUpdate)
    {
        return RMT_OK;
    }

    // For a regular mapping operation these must be valid.
    RMT_ASSERT(size_in_pages > 0);

    // NOTE: process filtering, driver doesn't seem to be producing >1 process currently?
    if (process_id <= 1)
    {
        return RMT_OK;
    }

    // Calculate the number of 4KiB pages we require.
    const uint64_t page_size_4kib   = RmtGetPageSize(kRmtPageSize4Kb);
    const uint64_t size_of_page     = RmtGetPageSize(page_size);
    const uint64_t size_in_bytes    = size_in_pages * size_of_page;
    const int32_t  size_in_4k_pages = (int32_t)(size_in_bytes / page_size_4kib);
    RMT_ASSERT(size_in_4k_pages * 4 * 1024 == size_in_bytes);  // make sure no precision lost in division (4KB should always be a factor of other page sizes).

    // Update each page's mapping in the page table.
    RmtGpuAddress current_virtual_address  = virtual_address;
    RmtGpuAddress current_physical_address = physical_address;
    for (int32_t current_page_index = 0; current_page_index < size_in_4k_pages; ++current_page_index)
    {
        int32_t level0_radix;
        int32_t level1_radix;
        int32_t level2_radix;
        int32_t level3_radix = 0;
        DecomposeAddress(current_virtual_address, &level0_radix, &level1_radix, &level2_radix, &level3_radix);

        UpdateMappingForSingle4KPage(page_table, level0_radix, level1_radix, level2_radix, level3_radix, current_physical_address, is_unmapping);

        current_virtual_address += page_size_4kib;
        if (!is_unmapping && (current_physical_address == 0U))
        {
            current_physical_address = 0;  // this means mapped in system RAM.
        }
        else
        {
            current_physical_address += page_size_4kib;
        }
    }

    return RMT_OK;
}

// Get the physical address for a specified virtual address.
RmtErrorCode RmtPageTableGetPhysicalAddressForVirtualAddress(const RmtPageTable* page_table, RmtGpuAddress virtual_address, RmtGpuAddress* out_physical_address)
{
    RMT_RETURN_ON_ERROR(page_table, RMT_ERROR_INVALID_POINTER);

    int32_t level0_radix;
    int32_t level1_radix;
    int32_t level2_radix;
    int32_t level3_radix = 0;
    DecomposeAddress(virtual_address, &level0_radix, &level1_radix, &level2_radix, &level3_radix);

    const RmtPageDirectoryLevel1* level1 = page_table->level0[level0_radix];
    RMT_RETURN_ON_ERROR(level1, RMT_ERROR_ADDRESS_NOT_MAPPED);

    const RmtPageDirectoryLevel2* level2 = level1->page_directory[level1_radix];
    RMT_RETURN_ON_ERROR(level2, RMT_ERROR_ADDRESS_NOT_MAPPED);

    const RmtPageDirectoryLevel3* level3 = level2->page_directory[level2_radix];
    RMT_RETURN_ON_ERROR(level3, RMT_ERROR_ADDRESS_NOT_MAPPED);

    // Each physical address value at level 3 of the page map trie is 36 bits in size.
    const int32_t byte_offset = level3_radix / 8;
    const int32_t bit_offset  = level3_radix % 8;
    const uint8_t mask        = (1 << bit_offset);
    if ((level3->is_mapped[byte_offset] & mask) != mask)
    {
        return RMT_ERROR_ADDRESS_NOT_MAPPED;
    }

    // Look up the physical address value from the level 3 node.
    if (out_physical_address != nullptr)
    {
        *out_physical_address = ((RmtGpuAddress)level3->physical_addresses[(level3_radix * 6) + 0] << 40) |
                                ((RmtGpuAddress)level3->physical_addresses[(level3_radix * 6) + 1] << 32) |
                                ((RmtGpuAddress)level3->physical_addresses[(level3_radix * 6) + 2] << 24) |
                                ((RmtGpuAddress)level3->physical_addresses[(level3_radix * 6) + 3] << 16) |
                                ((RmtGpuAddress)level3->physical_addresses[(level3_radix * 6) + 4] << 8) |
                                ((RmtGpuAddress)level3->physical_addresses[(level3_radix * 6) + 5] << 0);
    }

    return RMT_OK;
}

// check if entire resource physically
bool RmtPageTableIsEntireResourcePhysicallyMapped(const RmtPageTable* page_table, const RmtResource* resource)
{
    RMT_RETURN_ON_ERROR(page_table, false);
    RMT_RETURN_ON_ERROR(resource, false);

    const RmtGpuAddress end_virtual_address     = resource->address + resource->size_in_bytes;
    RmtGpuAddress       current_virtual_address = resource->address;

    // no address, no size, or dangling should be ignored.
    if (resource->address == 0 || resource->size_in_bytes == 0 || (resource->flags & kRmtResourceFlagDangling) == kRmtResourceFlagDangling)
    {
        return false;
    }

    // walk through the resource, and check each page.
    while (current_virtual_address < end_virtual_address)
    {
        RmtGpuAddress      physical_address = 0;
        const RmtErrorCode error_code       = RmtPageTableGetPhysicalAddressForVirtualAddress(page_table, current_virtual_address, &physical_address);
        if (error_code == RMT_ERROR_ADDRESS_NOT_MAPPED)
        {
            return false;
        }

        current_virtual_address += RmtGetPageSize(kRmtPageSize4Kb);
    }

    return true;
}

// check if entire allocation is mapped.
bool RmtPageTableIsEntireVirtualAllocationPhysicallyMapped(const RmtPageTable* page_table, const RmtVirtualAllocation* virtual_allocation)
{
    RMT_RETURN_ON_ERROR(page_table, false);
    RMT_RETURN_ON_ERROR(virtual_allocation, false);

    const RmtGpuAddress end_virtual_address =
        virtual_allocation->base_address + RmtGetAllocationSizeInBytes(virtual_allocation->size_in_4kb_page, kRmtPageSize4Kb);
    RmtGpuAddress current_virtual_address = virtual_allocation->base_address;

    // walk through the virtual resource, and check each page.
    while (current_virtual_address < end_virtual_address)
    {
        RmtGpuAddress      physical_address = 0;
        const RmtErrorCode error_code       = RmtPageTableGetPhysicalAddressForVirtualAddress(page_table, current_virtual_address, &physical_address);
        if (error_code == RMT_ERROR_ADDRESS_NOT_MAPPED)
        {
            return false;
        }

        // move ahead by the page mapping size.
        current_virtual_address += RmtGetPageSize(kRmtPageSize4Kb);
    }

    return true;
}
