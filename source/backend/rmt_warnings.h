//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of structures and functions for RMT warnings.
//=============================================================================

#ifndef RMV_BACKEND_RMT_WARNINGS_H_
#define RMV_BACKEND_RMT_WARNINGS_H_

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// An enumeration of all warnings.
typedef enum RmtWarningType
{
    kRmtWarningTypeVirtualAllocationHighDedicatedResourceCount  = 0,
    kRmtWarningTypeVirtualAllocationHighFragmentationAllocation = 1,
    kRmtWarningTypeHeapOversubscribed                           = 2,
    kRmtWarningTypeResourceCriticalInNonlocal                   = 3,
    kRmtWarningTypeResourceInNonPreferredHeap                   = 4,
    kRmtWarningTypeResourceOrphaned                             = 5,

    // add above this
    kRmtWarningTypeCount
} RmtWarningType;

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_WARNINGS_H_
