//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Helpers for working with addresses.
//=============================================================================

#ifndef RMV_PARSER_RMT_ADDRESS_HELPER_H_
#define RMV_PARSER_RMT_ADDRESS_HELPER_H_

#include "rmt_format.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// Calculate the size of a page in bytes.
///
/// @param [in]     page_size               An RmtPageSize defining the page size.
/// @returns
/// The page size, in bytes.
uint64_t RmtGetPageSize(RmtPageSize page_size);

/// Calculate the size of an allocation in bytes from its pages count and page size.
///
/// @param [in]     size_in_pages           The size of the allocation, in pages.
/// @param [in]     page_size               An RmtPageSize defining the page size.
/// @returns
/// The allocation size, in bytes.
uint64_t RmtGetAllocationSizeInBytes(uint64_t size_in_pages, RmtPageSize page_size);

/// Check if two ranges in an address space overlap.
///
/// @param [in]     base_address1           The base address of the first allocation.
/// @param [in]     size_in_bytes1          The size, in bytes, of the first allocation.
/// @param [in]     base_address2           The base address of the second allocation.
/// @param [in]     size_in_bytes2          The size, in bytes, of the second allocation.
/// @returns
/// If the allocations overlap, true is returned, otherwise false.
bool RmtAllocationsOverlap(RmtGpuAddress base_address1, uint64_t size_in_bytes1, RmtGpuAddress base_address2, uint64_t size_in_bytes2);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_PARSER_RMT_ADDRESS_HELPER_H_
