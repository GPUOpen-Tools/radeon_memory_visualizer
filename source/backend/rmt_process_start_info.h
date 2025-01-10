//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of the process start info structures and helper functions.
//=============================================================================

#ifndef RMV_BACKEND_RMT_PROCESS_START_INFO_H_
#define RMV_BACKEND_RMT_PROCESS_START_INFO_H_

#include "rmt_error.h"
#include "rmt_types.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// A structure encapsulating the process start info.
typedef struct RmtProcessStartInfo
{
    uint64_t process_id;                 ///< The process ID.
    uint64_t physical_memory_allocated;  ///< The amount of physical memory allocated (in bytes).
} RmtProcessStartInfo;

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_PROCESS_START_INFO_H_
