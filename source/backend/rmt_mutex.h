//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of a mutex abstraction.
//=============================================================================

#ifndef RMV_BACKEND_RMT_MUTEX_H_
#define RMV_BACKEND_RMT_MUTEX_H_

#include "rmt_error.h"

/// Define the size of a mutex handle structure (in DWORDS).
#define RMT_MUTEX_SIZE (24)

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// A handle for a mutex.
typedef struct RmtMutex
{
    uint32_t data[RMT_MUTEX_SIZE];
} RmtMutex;

/// Create a new mutex.
///
/// @param [in]     mutex                       A pointer to a <c><i>RmtMutex</i></c> structure.
/// @param [in]     name                        The name of the mutex.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The parameter <c><i>mutex</i></c> was NULL.
/// @retval
/// kRmtErrorPlatformFunctionFailed             A platform-specific function failed.
///
RmtErrorCode RmtMutexCreate(RmtMutex* mutex, const char* name);

/// Lock (acquire) the mutex.
///
/// @param [in]     mutex                       A pointer to a <c><i>RmtMutex</i></c> structure.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The parameter <c><i>threadEvent</i></c> was NULL.
/// @retval
/// kRmtErrorPlatformFunctionFailed             A platform-specific function failed.
///
RmtErrorCode RmtMutexLock(RmtMutex* mutex);

/// Unlock (release) the mutex.
///
/// @param [in]     mutex                       A pointer to a <c><i>RmtMutex</i></c> structure.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The parameter <c><i>threadEvent</i></c> was NULL.
/// @retval
/// kRmtErrorPlatformFunctionFailed             A platform-specific function failed.
///
RmtErrorCode RmtMutexUnlock(RmtMutex* mutex);

/// Destroy the mutex. Free up any resources the mutex was using
///
/// @param [in]     mutex                       A pointer to a <c><i>RmtMutex</i></c> structure.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The parameter <c><i>threadEvent</i></c> was NULL.
/// @retval
/// kRmtErrorPlatformFunctionFailed             A platform-specific function failed.
///
RmtErrorCode RmtMutexDestroy(RmtMutex* mutex);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_MUTEX_H
