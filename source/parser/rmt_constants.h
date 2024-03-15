//=============================================================================
// Copyright (c) 2023-2024 Advanced Micro Devices, Inc. All rights reserved.
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

static const uint32_t kGfx10AsicFamily = 0x8F;

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // RMV_PARSER_RMT_CONSTANTS_H_
