//=============================================================================
// Copyright (c) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Intended to hold globally-known definitions.
//=============================================================================

#ifndef RMV_PARSER_RMT_CONSTANTS_H_
#define RMV_PARSER_RMT_CONSTANTS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

// The file extensions for trace files.
static const char* kRMVTraceFileExtension = ".rmv";
static const char* kRGDTraceFileExtension = ".rgd";

static constexpr uint32_t kFamilyNavi  = 0x8F;
static constexpr uint32_t kFamilyNavi4 = 0x98;

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // RMV_PARSER_RMT_CONSTANTS_H_
