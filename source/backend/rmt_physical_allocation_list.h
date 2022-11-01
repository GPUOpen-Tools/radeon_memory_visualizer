//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Structures and functions for managing a physical allocation list.
//=============================================================================

#ifndef RMV_BACKEND_RMT_PHYSICAL_ALLOCATION_LIST_H_
#define RMV_BACKEND_RMT_PHYSICAL_ALLOCATION_LIST_H_

#include "rmt_types.h"
#include "rmt_error.h"
#include "rmt_configuration.h"
#include "rmt_pool.h"
#include "rmt_format.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// A structure encapsulating critical allocation identifier information.
typedef struct RmtPhysicalAllocationInterval
{
    RmtGpuAddress base_address;       ///< The base address of the physical allocation.
    int32_t       size_in_4kb_pages;  ///< The size of the allocation in 4KiB pages.
    int32_t       padding;            ///< Padding for the node.
} RmtPhysicalAllocationInterval;

/// An enumeration of all flags for physical allocations.
typedef enum RmtPhysicaAllocationFlags
{
    kRmtPhysicalAllocationFlagTransferred = 1 << 0,  ///< Indicates that the physical allocation has previously been transfered.
} RmtPhysicaAllocationFlags;

/// A structure encapsulating extra details about a physical allocation.
typedef struct RmtPhysicalAllocation
{
    uint64_t base_address;      ///< The base address of the physical allocation.
    int32_t  size_in_4kb_page;  ///< The size of the physical allocation.
    int32_t  guid;              ///< A GUID for the this physical allocation.
    uint32_t flags;             ///< A set of flags for the physical allocation.

    uint64_t     timestamp;   ///< The timestamp when the physical allocation was made.
    RmtProcessId process_id;  ///< The ID of the process which made this physical allocation.
    RmtHeapType  heap_type;   ///< The type of heap the physical allocation resides in.
} RmtPhysicalAllocation;

/// Get the size (in bytes) of a physical allocation.
///
/// @param [in] physical_allocation                 A pointer to a <c><i>RmtPhysicalAllocation</i></c> structure.
///
/// @returns
/// The size (in bytes) of the <c><i>physicalAllocation</i></c>.
uint64_t RmtPhysicalAllocationGetSizeInBytes(const RmtPhysicalAllocation* physical_allocation);

/// A structure encapsulating a list of allocations.
typedef struct RmtPhysicalAllocationList
{
    RmtPhysicalAllocationInterval* allocation_intervals;            ///< A buffer of allocation intervals.
    RmtPhysicalAllocation*         allocation_details;              ///< A buffer of extra allocation details.
    int32_t                        allocation_count;                ///< The number of live allocations in the list.
    int32_t                        next_allocation_guid;            ///< The next allocation GUID to assign.
    int32_t                        maximum_concurrent_allocations;  ///< The maximum number of concurrent allocations.
} RmtPhysicalAllocationList;

/// Calculate the size of the workling buffer required for a specific number of concurrent allocations.
///
/// @param [in] maximum_concurrent_allocations          The maximum number of concurrent allocations that can be in flight at any one time.
///
/// @returns
/// The size of the physical allocation list buffer that is required to initialize a <c><i>RmtPhysicalAllocationList</i></c> structure using <c><i>rmtPhysicalAllocationListInitialize</i></c>.
size_t RmtPhysicalAllocationListGetBufferSize(int32_t maximum_concurrent_allocations);

/// Initialize the physical allocation list.
///
/// @param [in] physical_allocation_list                A pointer to a <c><i>RmtPhysicalAllocationList</i></c> structure.
/// @param [in] buffer                                  A pointer to a memory buffer to use for the list contents.
/// @param [in] buffer_size                             The size (in bytes) of the memory region pointed to by <c><i>buffer</i></c>.
/// @param [in] maximum_concurrent_allocations          The maximum number of concurrent allocations that can be in flight at any one time.
///
/// @retval
/// kRmtOk                                  The operation completed successfully.
/// @retval
/// @kRmtErrorInvalidPointer                The operation failed because <c><i>physicalAllocationList</i></c> or <c><i>buffer</i></c> was <c><i>NULL</i></c>.
/// @retval
/// @kRmtErrorInvalidSize                   The operation failed because <c><i>buffer</i></c> was not large enough to contain the number of concurrent allocations specified.
RmtErrorCode RmtPhysicalAllocationListInitialize(RmtPhysicalAllocationList* physical_allocation_list,
                                                 void*                      buffer,
                                                 size_t                     buffer_size,
                                                 int32_t                    maximum_concurrent_allocations);

/// Add a physical allocation to the list.
///
/// @param [in] physical_allocation_list                A pointer to a <c><i>RmtPhysicalAllocationList</i></c> structure.
/// @param [in] timestamp                               The time at which the allocation occurred.
/// @param [in] address                                 The physical allocation address to add.
/// @param [in] size_in_4kb_pages                       The size of the allocation in 4KiB page chunks.
/// @param [in] heap_type                               The type of heap the physical address is in.
/// @param [in] process_id                              The process which allocated this physical memory.
///
/// @retval
/// kRmtOk                                  The operation completed successfully.
/// @retval
/// @kRmtErrorInvalidPointer                The operation failed because <c><i>physicalAllocationList</i></c>  was <c><i>NULL</i></c>.
RmtErrorCode RmtPhysicalAllocationListAddAllocation(RmtPhysicalAllocationList* physical_allocation_list,
                                                    uint64_t                   timestamp,
                                                    RmtGpuAddress              address,
                                                    int32_t                    size_in_4kb_pages,
                                                    RmtHeapType                heap_type,
                                                    RmtProcessId               process_id);

/// Remove a physical allocation from the list.
RmtErrorCode RmtPhysicalAllocationListDiscardAllocation(RmtPhysicalAllocationList* physical_allocation_list, RmtGpuAddress address);

/// Find base address of a physical allocation from address.
///
/// @param [in] physical_allocation_list                A pointer to a <c><i>RmtPhysicalAllocationList</i></c> structure.
/// @param [in] address                                 The physical allocation address to search for.
/// @param [out] out_allocation                         The address of a pointer which will be filled to point at an <c><i>RmtPhysicalAllocation</i></c> structure.
///
/// @retval
/// kRmtOk                                  The operation completed successfully.
/// @retval
/// @kRmtErrorInvalidPointer                The operation failed because <c><i>physicalAllocationList</i></c> or <c><i>outAllocation</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtPhysicalAllocationListGetAllocationForAddress(const RmtPhysicalAllocationList* physical_allocation_list,
                                                              RmtGpuAddress                    address,
                                                              const RmtPhysicalAllocation**    out_allocation);

/// Get a physical allocation at a specific index.
///
/// @param [in] physical_allocation_list                A pointer to a <c><i>RmtPhysicalAllocationList</i></c> structure.
/// @paran [in] index                                   The index of the physical allocation to return.
/// @param [out] out_allocation                         The address of a pointer to a <c><i>RmtPhysicalAllocationList</i></c> structure.
///
/// @retval
/// kRmtOk                                  The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                 The operation failed because <c><i>physicalAllocationList</i></c> or <c><i>outAllocation</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtPhysicalAllocationListGetAllocationAtIndex(const RmtPhysicalAllocationList* physical_allocation_list,
                                                           int32_t                          index,
                                                           const RmtPhysicalAllocation**    out_allocation);

/// Get the total size (in bytes) of the memory in a physical allocation list.
///
/// @param [in] physical_allocation_list                A pointer to a <c><i>RmtPhysicalAllocationList</i></c> structure.
///
/// @returns
/// The size (in bytes) of all physical allocations contained in <c><i>physicalAllocationList</i></c>.
uint64_t RmtPhysicalAllocationListGetTotalSizeInBytes(const RmtPhysicalAllocationList* physical_allocation_list);

/// Get the total size (in bytes) of the memory in a physical allocation list for a specific process.
///
/// @param [in] physical_allocation_list                A pointer to a <c><i>RmtPhysicalAllocationList</i></c> structure.
/// @param [in] process_id                              The 32bit process ID.
///
/// @returns
/// The size (in bytes) of all physical allocations contained in <c><i>physicalAllocationList</i></c>.
uint64_t RmtPhysicalAllocationListGetTotalSizeInBytesForProcess(const RmtPhysicalAllocationList* physical_allocation_list, RmtProcessId process_id);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_PHYSICAL_ALLOCATION_LIST_H_
