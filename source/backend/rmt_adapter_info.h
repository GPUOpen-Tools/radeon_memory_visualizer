//=============================================================================
// Copyright (c) 2020-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Helper function for legacy adapter information captured for the target process.
//=============================================================================

#ifndef RMV_BACKEND_RMT_ADAPTER_INFO_H_
#define RMV_BACKEND_RMT_ADAPTER_INFO_H_

#include "rmt_error.h"
#include "rmt_types.h"
#include "rmt_format.h"
#include "rmt_file_format.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// @brief An enumeration of memory types that can be used with an adapter.
typedef enum RmtAdapterInfoMemoryType
{
    kRmtAdapterInfoMemoryTypeUnknown = 0,   ///< Memory is unknown.
    kRmtAdapterInfoMemoryTypeDdr2    = 1,   ///< System DDR2.
    kRmtAdapterInfoMemoryTypeDdr3    = 2,   ///< System DDR3.
    kRmtAdapterInfoMemoryTypeDdr4    = 3,   ///< System DDR4.
    kRmtAdapterInfoMemoryTypeGddr5   = 4,   ///< Graphics DDR 5.
    kRmtAdapterInfoMemoryTypeGddr6   = 5,   ///< Graphics DDR 6.
    kRmtAdapterInfoMemoryTypeHbm     = 6,   ///< First version of High-Bandwidth Memory.
    kRmtAdapterInfoMemoryTypeHbm2    = 7,   ///< Second version of High-Bandwidth Memory.
    kRmtAdapterInfoMemoryTypeHbm3    = 8,   ///< Third version of High-Bandwidth Memory.
    kRmtAdapterInfoMemoryTypeLpddr4  = 9,   ///< Low power DDR4.
    kRmtAdapterInfoMemoryTypeLpddr5  = 10,  ///< Low power DDR5.
    kRmtAdapterInfoMemoryTypeDdr5    = 11   ///< System DDR5.
} RmtAdapterInfoMemoryType;

/// @brief Get the chip type (in string) of the video memory.
///
/// @param [in]     memory_type             The memory type enumerator.
///
/// @returns
/// Pointer to the memory chip type string.
const char* RmtAdapterInfoGetVideoMemoryType(const RmtAdapterInfoMemoryType memory_type);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_ADAPTER_INFO_H_
