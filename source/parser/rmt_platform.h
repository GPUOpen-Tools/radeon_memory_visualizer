//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Platform specific utilities.
//=============================================================================

#ifndef RMV_PARSER_RMT_PLATFORM_H_
#define RMV_PARSER_RMT_PLATFORM_H_

#include <stdint.h>

#ifdef _WIN32
#define RMT_FORCEINLINE __forceinline
#else
#define RMT_FORCEINLINE __attribute__((always_inline)) inline
#endif  // #ifdef _WIN32

#ifdef _WIN32
#define RMT_UNUSED_FUNC
#else
#define RMT_UNUSED_FUNC __attribute__((unused))
#endif

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// Utility function to get the clock frequency.
///
/// @returns
/// The current clock frquency of the CPU.
///
uint64_t RmtGetClockFrequency();

/// Utility function to get the current timestamp.
///
/// @returns
/// The current clock frquency of the CPU.
///
uint64_t RmtGetCurrentTimestamp();

/// Utility function to sleep.
///
/// @param [in] timeout         The sleep time, in milliseconds.
///
/// @returns
/// The current clock frquency of the CPU.
///
void RmtSleep(uint32_t timeout);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_PARSER_RMT_PLATFORM_H_
