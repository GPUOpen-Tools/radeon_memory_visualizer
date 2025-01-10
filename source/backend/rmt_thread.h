//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Abstraction of a thread.
//=============================================================================

#ifndef RMV_BACKEND_RMT_THREAD_H_
#define RMV_BACKEND_RMT_THREAD_H_

#include "rmt_error.h"

/// Define the size of the thread handle structure (in DWORDS).
#define RMT_THREAD_SIZE (24)

#ifdef _WIN32
/// The calling convention used by thread functions on Windows platform.
#define RMT_THREAD_FUNC __stdcall
#else
#define RMT_THREAD_FUNC
#endif  // #ifdef _WIN32

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// A handle for a thread.
typedef struct RmtThread
{
    uint32_t data[RMT_THREAD_SIZE];
} RmtThread;

/// A type for the thread function.
typedef uint32_t(RMT_THREAD_FUNC* RmtThreadFunc)(void* input_data);

/// Create a new thread.
///
/// @param [in,out] thread                      A pointer to a <c><i>RmtThread</i></c> structure to contain a handle to the thread.
/// @param [in]     thread_func                 The function to execute as the thread's main function.
/// @param [in]     input_data                  Data to pass to the threads <c><i>inputData</i></c> parameter.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The parameter <c><i>thread</i></c> or <c><i>threadFunc</i></c> was NULL.
/// @retval
/// kRmtErrorPlatformFunctionFailed             A platform-specific function failed.
///
RmtErrorCode RmtThreadCreate(RmtThread* thread, RmtThreadFunc thread_func, void* input_data);

/// Wait for the thread to exit.
///
/// @param [in]     thread                      A pointer to a <c><i>RmtThread</i></c> structure.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The parameter <c><i>thread</i></c> was NULL.
///
RmtErrorCode RmtThreadWaitForExit(RmtThread* thread);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_THREAD_H_
