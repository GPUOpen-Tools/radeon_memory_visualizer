//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of structures and functions for a multi-level page table.
//=============================================================================

#ifndef RMV_BACKEND_RMT_PAGE_TABLE_H_
#define RMV_BACKEND_RMT_PAGE_TABLE_H_

#include "rmt_error.h"
#include "rmt_types.h"
#include "rmt_configuration.h"
#include "rmt_pool.h"
#include "rmt_segment_info.h"

/// The number of entries in the multi-level page directory for level 0. Enough for 10-bit radix.
#define RMT_PAGE_DIRECTORY_LEVEL_0_SIZE (1024)

/// The number of entries in the multi-level page directory for level 1. Enough for 10-bit radix.
#define RMT_PAGE_DIRECTORY_LEVEL_1_SIZE (1024)

/// The number of entries in the multi-level page directory for level 2. Enough for 8-bit radix.
#define RMT_PAGE_DIRECTORY_LEVEL_2_SIZE (256)

/// The number of entries in the multi-level page directory for level 3. Enough for 8-bit radix.
#define RMT_PAGE_DIRECTORY_LEVEL_3_SIZE (256)

/// The size (in bytes) of a set of 256 x 48-bit physical offsets for the leaf node.
/// NOTE: This could be optimized down to 36-bit later.
#define RMT_MAXIMUM_PAGE_TABLE_LEAF_SIZE (RMT_PAGE_DIRECTORY_LEVEL_3_SIZE * 6)

/// The maximum size the page table can encode in physical address space.
#define RMT_PAGE_TABLE_MAX_SIZE_OF_PHYSICAL_SPACE_IN_BYTES (16 * 1024 * 1024 * 1024)

/// The number of level 0 nodes to keep space for.
#define RMT_PAGE_DIRECTORY_LEVEL_0_COUNT 1024

/// The number of level 1 nodes to keep space for.
#define RMT_PAGE_DIRECTORY_LEVEL_1_COUNT 1024

/// The number of level 2 nodes to keep space for.
#define RMT_PAGE_DIRECTORY_LEVEL_2_COUNT 4096

/// The number of level 3 nodes to keep space for. This value maps exactly to number of MB of VA space than can be mapped at once.
#define RMT_PAGE_DIRECTORY_LEVEL_3_COUNT (64 * 1024)

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

typedef struct RmtVirtualAllocation RmtVirtualAllocation;
typedef struct RmtResource          RmtResource;

/// A structure encapsulating the leaf node of a page table.
typedef struct RmtPageDirectoryLevel3
{
    // NOTE: Moving to 36 bit offsets in here would save 25MB at 64GB.

    uint8_t physical_addresses[RMT_MAXIMUM_PAGE_TABLE_LEAF_SIZE];  ///< An array of bytes to store 256 x 48bit physical addresses.
    uint8_t is_mapped[RMT_PAGE_DIRECTORY_LEVEL_3_SIZE / 8];        ///< A bit field indicating if the slot in <c><i>physicalAddresses</i></c> is used or not.
} RmtPageDirectoryLevel3;

/// A structure to encapsulate a level 2 page directory structure.
typedef struct RmtPageDirectoryLevel2
{
    RmtPageDirectoryLevel3* page_directory[RMT_PAGE_DIRECTORY_LEVEL_2_SIZE];  ///< An array of pointers to level 3 page directory structures.
} RmtPageDirectoryLevel2;

/// A structure to encapsulate
typedef struct RmtPageDirectoryLevel1
{
    RmtPageDirectoryLevel2* page_directory[RMT_PAGE_DIRECTORY_LEVEL_1_SIZE];  ///< An array of pointers to level 2 page directory structures.
} RmtPageDirectoryLevel1;

/// A structure encapsulating a multi-level page table.
///
/// This is implemented a trie data structure. The virtual address is decompossed into
/// a different size radix at each level of the tree.
///
///   |XXXXXXXXXX|XXXXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXXXXXX|
///   |----------|----------|--------|--------|------------|
///         |          |         |        |     lower bits
///      Lvl.0       Lvl.1     Lvl.2    Lvl.3
///     (10bit)     (10bit)   (8bit)   (8bit)
///
/// This means to traverse the tree we use the different parts of the virtual address
/// to index in to the array of pointers at the different tree levels. At the leaf of
/// the trie, we are storing a compacted array of 48bit physical address pointers.
///
/// On top of this the <c><i>RmtPageTable</i></c> structure also implements a small
/// TLB-like cache, which keeps a small buffer of the most recently accessed Lvl. 3
/// nodes from the trie. This means that iterating through virtual address space
/// will only result in a tree traversal every 1MB of VA range.
///
typedef struct RmtPageTable
{
    RmtPageDirectoryLevel1* level0[RMT_PAGE_DIRECTORY_LEVEL_0_SIZE];  ///< An array of pointers to level 1 page directory structures.

    // Storage and allocators for level 2..3 structures and what not.
    RmtPageDirectoryLevel1 level1_nodes[RMT_PAGE_DIRECTORY_LEVEL_1_COUNT];  ///< An array of <c><i>RmtPageDirectoryLevel1</i></c> structures.
    RmtPageDirectoryLevel2 level2_nodes[RMT_PAGE_DIRECTORY_LEVEL_2_COUNT];  ///< An array of <c><i>RmtPageDirectoryLevel2</i></c> structures.
    RmtPageDirectoryLevel3 level3_nodes[RMT_PAGE_DIRECTORY_LEVEL_3_COUNT];  ///< An array of <c><i>RmtPageDirectoryLevel3</i></c> structures.
    RmtPool                level1_allocator;                                ///< A <c><i>RmtPool</i></c> allocator structure for level 1 nodes.
    RmtPool                level2_allocator;                                ///< A <c><i>RmtPool</i></c> allocator structure for level 2 nodes.
    RmtPool                level3_allocator;                                ///< A <c><i>RmtPool</i></c> allocator structure for level 3 nodes.

    uint64_t       mapped_per_heap[kRmtHeapTypeCount];  ///< An array of <c><i>uint64_t</i></c> each containing the number of bytes per heap currently mapped.
    RmtSegmentInfo segment_info[RMT_MAXIMUM_SEGMENTS];  ///< An array of segment information.
    int32_t        segment_info_count;                  ///< The number of segments.

    uint64_t target_process_id;  ///< The process ID of the process we're tracing for UMD data.
} RmtPageTable;

/// Initialize the page table.
///
/// @param [in] page_table                  A pointer to a <c><i>RmtPageTable</i></c> structure to initialize.
/// @param [in] segment_info                A pointer to an array of segment info structures.
/// @param [in] segment_info_count          The number of segment info structures in the structures.
/// @param [in] target_process_id           The target process being traced.
///
/// @retval
/// kRmtOk                              The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer             The operation failed as <c><i>pageTable</i></c> structure was <c><i>NULL</i></c>.
RmtErrorCode RmtPageTableInitialize(RmtPageTable* page_table, const RmtSegmentInfo* segment_info, int32_t segment_info_count, uint64_t target_process_id);

/// Map some virtual memory to an underlaying physical range.
///
/// If <c><i>pageSize</i></c> is set to <c><i>RMT_PAGE_SIZE_UNAMPPED</i></c> then this function will
/// interpret this as an unmapping operation and call <c><i>rmtPageTableUnmap</i></c>.
///
/// @param [in] page_table                  A pointer to a <c><i>RmtPageTable</i></c> structure to update.
/// @param [in] virtual_address             The virtual address of the page table mapping.
/// @param [in] physical_address            The physical address that the virtual address is being mapped to. Only valid when <c><i>pageSize</i></c> is not <c><i>RMT_PAGE_SIZE_UNMAPPED</i></c>.
/// @param [in] size_in_pages               The size of the the mapping specified in pages. The page size is determined by <c><i>pageSize</i></c>.
/// @param [in] page_size                   The size of each page being mapped.
/// @param [in] is_unmapping                A boolean value indicating if the mapping update is an unmap operation.
/// @param [in] update_type                 The type of the update operation.
/// @param [in] process_id                  The process id.
///
/// @retval
/// kRmtOk                              The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer             The operation failed as <c><i>pageTable</i></c> structure was <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorAddressAlreadyMapped       The operation failed because some memory in this range was already mapped.
RmtErrorCode RmtPageTableUpdateMemoryMappings(RmtPageTable*          page_table,
                                              RmtGpuAddress          virtual_address,
                                              RmtGpuAddress          physical_address,
                                              size_t                 size_in_pages,
                                              RmtPageSize            page_size,
                                              bool                   is_unmapping,
                                              RmtPageTableUpdateType update_type,
                                              uint64_t               process_id);

/// Find the physical mapping for the specified virtual address.
///
/// @param [in] page_table              A pointer to a <c><i>RmtPageTable</i></c> structure to search.
/// @param [in] virtual_address         The virtual address of the page table mapping.
/// @param [out] out_physical_address   The physical address for the specified virtual address.
///
/// @retval
/// kRmtOk                              The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer             The operation failed as <c><i>pageTable</i></c> structure was <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorAddressNotMapped           The operation failed as <c><i>virtualAddress</i></c> was not mapped to a physical address.
RmtErrorCode RmtPageTableGetPhysicalAddressForVirtualAddress(const RmtPageTable* page_table,
                                                             RmtGpuAddress       virtual_address,
                                                             RmtGpuAddress*      out_physical_address);

/// Check if a virtual allocation is completed backed by physical memory.
///
/// @param [in] page_table                  A pointer to a <c><i>RmtPageTable</i></c> structure.
/// @param [in] virtual_allocation          A pointer to the <c><i>RmtVirtualAllocation</i></c> structure to check the mapping of.
///
bool RmtPageTableIsEntireVirtualAllocationPhysicallyMapped(const RmtPageTable* page_table, const RmtVirtualAllocation* virtual_allocation);

/// Check if a resource is completed backed by physical memory.
///
/// @param [in] page_table                  A pointer to a <c><i>RmtPageTable</i></c> structure.
/// @param [in] resource                    A pointer to the <c><i>RmtResource</i></c> structure to check the mapping of.
///
bool RmtPageTableIsEntireResourcePhysicallyMapped(const RmtPageTable* page_table, const RmtResource* resource);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_PAGE_TABLE_H_
