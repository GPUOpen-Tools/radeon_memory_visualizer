//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Game Engineering Group
/// \brief  The segment information captured for the target process.
//=============================================================================

#ifndef RMV_BACKEND_RMT_SEGMENT_INFO_H_
#define RMV_BACKEND_RMT_SEGMENT_INFO_H_

#include <rmt_error.h>
#include <rmt_types.h>
#include <rmt_format.h>

#ifdef __cpluplus
extern "C" {
#endif  // #ifdef __cplusplus

/// A structure encapsulating the information about a segment.
typedef struct RmtSegmentInfo
{
    RmtGpuAddress base_address;  ///< The base address of the segment.
    uint64_t      size;          ///< The size of the segment (in bytes).
    RmtHeapType   heap_type;     ///< The heap type where the segment resides.
    int32_t       index;         ///< The index of the segment.
} RmtSegmentInfo;

#ifdef __cpluplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_SEGMENT_INFO_H_
