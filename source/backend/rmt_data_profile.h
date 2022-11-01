//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Structures for profiling the RMT data for future parsing.
//=============================================================================

#ifndef RMV_BACKEND_RMT_DATA_PROFILE_H_
#define RMV_BACKEND_RMT_DATA_PROFILE_H_

#include "rmt_types.h"
#include "rmt_configuration.h"
#include "rmt_file_format.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// A structure encapsulating a profile of an RMT file.
typedef struct RmtDataProfile
{
    int32_t process_count;        ///< The number of processes seen in the RMT streams.
    int32_t stream_count;         ///< The number of RMT streams.
    int32_t snapshot_count;       ///< The number of snapshots in the RMT streams.
    int32_t snapshot_name_count;  ///< The number of bytes required to store all snapshot names.
    int32_t current_virtual_allocation_count;
    int32_t max_virtual_allocation_count;  ///< The maximum number of allocations seen concurrently.
    int32_t current_resource_count;
    int32_t max_concurrent_resources;        ///< The maximum number of resources seen concurrently.
    int32_t total_resource_count;            ///< The total number of resources seent at any time.
    int32_t total_virtual_allocation_count;  ///< The total number of virtual allocations seen at any time.

} RmtDataProfile;

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_DATA_PROFILE_H_
