//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of types used throughout the RMT code.
//=============================================================================

#ifndef RMV_PARSER_RMT_TYPES_H_
#define RMV_PARSER_RMT_TYPES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// An enumeration of different address maps.
typedef enum RmtAddressMapType
{
    kRmtAddressMapTypeVirtual  = 0,  ///< The virtual address map.
    kRmtAddressMapTypePhysical = 1   ///< The physical address map.
} RmtAddressMapType;

/// An enumeration of the different page sizes.
typedef enum RmtPageSize
{
    kRmtPageSizeUnmapped  = 0,  ///< The memory was unmapped.
    kRmtPageSize4Kb       = 1,  ///< 4KiB page size.
    kRmtPageSize64Kb      = 2,  ///< 64KiB page size.
    kRmtPageSize256Kb     = 3,  ///< 256KiB page size.
    kRmtPageSize1Mb       = 4,  ///< 1MB page size.
    kRmtPageSize2Mb       = 5,  ///< 2MB page size.
    kRmtPageSizeReserved0 = 6,  ///< Reserved for future expansion.
    kRmtPageSizeReserved1 = 7,  ///< Reserved for future expansion.

    // add above this
    kRmtPageSizeCount
} RmtPageSize;

/// An enumeration of the heap types.
typedef enum RmtHeapType
{
    kRmtHeapTypeUnknown   = -1,  ///< The heap is not known.
    kRmtHeapTypeLocal     = 0,   ///< The heap in local memory. This is mapped on the CPU.
    kRmtHeapTypeInvisible = 1,   ///< The heap in local memory. This is not mappable on the CPU.
    kRmtHeapTypeSystem =
        2,  ///< The heap is in host memory. Uncached Speculative Write Combine - it is intended for write-only data on the CPU side, it does not use the CPU cache.
    kRmtHeapTypeNone = 3,  ///< There is no heap used.

    kRmtHeapTypeCount
} RmtHeapType;

/// The type of page table update operation.
typedef enum RmtPageTableUpdateType
{
    kRmtPageTableUpdateTypeDiscard  = 0,  ///< The physical page table entry is being discarded.
    kRmtPageTableUpdateTypeUpdate   = 1,  ///< The physical page table entry is a regular page table mapping.
    kRmtPageTableUpdateTypeTransfer = 2,  ///< The physical page table entry is some memory being transfered from one pool to another.
    kRmtPageTableUpdateTypeReserved = 3   ///< Reserved for future expansion.
} RmtPageTableUpdateType;

/// The type of system driving page table updates.
typedef enum RmtPageTableController
{
    kRmtPageTableControllerOperatingSystem = 0,  ///< Decisions about page table updating strategy are controlled by the OS.
    kRmtPageTableControllerKmd             = 1   ///< Decisions about page table updating strategy are controlled by KMD.
} RmtPageTableController;

/// A typedef for a resource identifier.
typedef uint64_t RmtResourceIdentifier;

/// A typedef for a resource correlation ID.
typedef uint64_t RmtCorrelationIdentifier;

/// A typedef for a queue.
typedef uint64_t RmtQueue;

/// A typedef for an address.
typedef uint64_t RmtGpuAddress;

/// A typedef for a process ID.
typedef uint64_t RmtProcessId;

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_PARSER_RMT_TYPES_H_
