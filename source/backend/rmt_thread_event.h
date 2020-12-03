//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author
/// \brief
//=============================================================================

#ifndef RMV_BACKEND_RMT_THREAD_EVENT_H_
#define RMV_BACKEND_RMT_THREAD_EVENT_H_

#include <rmt_error.h>

/// Define the size of a thread event handle structure (in DWORDS).
#define RMT_THREAD_EVENT_SIZE (4)

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// A handle for a thread event.
typedef struct RmtThreadEvent
{
    uint32_t data[RMT_THREAD_EVENT_SIZE];
} RmtThreadEvent;

/// Create a new thread event.
///
/// @param [in]     thread_event                A pointer to a <c><i>RmtThreadEvent</i></c> structure.
/// @param [in]     initial_value               Boolean value indicating what the set the thread event to.
/// @param [in]     manual_reset                Boolean value indicating if the thread event should require manually reset.
/// @param [in]     name                        The name of the thread event.
///
/// @retval
/// RMT_OK                                      The operation completed successfully.
/// @retval
/// RMT_ERROR_INVALID_POINTER                   The parameter <c><i>threadEvent</i></c> was NULL.
/// @retval
/// RMT_ERROR_PLATFORM_FUNCTION_FAILED          A platform-specific function failed.
///
RmtErrorCode RmtThreadEventCreate(RmtThreadEvent* thread_event, bool initial_value, bool manual_reset, const char* name);

/// Signal a thread event.
///
/// @param [in]     thread_event                A pointer to a <c><i>RmtThreadEvent</i></c> structure.
///
/// @retval
/// RMT_OK                                      The operation completed successfully.
/// @retval
/// RMT_ERROR_INVALID_POINTER                   The parameter <c><i>threadEvent</i></c> was NULL.
/// @retval
/// RMT_ERROR_PLATFORM_FUNCTION_FAILED          A platform-specific function failed.
///
RmtErrorCode RmtThreadEventSignal(RmtThreadEvent* thread_event);

/// Wait for a thread event to be signalled.
///
/// @param [in]     thread_event                A pointer to a <c><i>RmtThreadEvent</i></c> structure.
///
/// @retval
/// RMT_OK                                      The operation completed successfully.
/// @retval
/// RMT_ERROR_INVALID_POINTER                   The parameter <c><i>threadEvent</i></c> was NULL.
/// @retval
/// RMT_ERROR_PLATFORM_FUNCTION_FAILED          A platform-specific function failed.
///
RmtErrorCode RmtThreadEventWait(RmtThreadEvent* thread_event);

/// Wait for a thread event to be reset.
///
/// @param [in]     thread_event                A pointer to a <c><i>RmtThreadEvent</i></c> structure.
///
/// @retval
/// RMT_OK                                      The operation completed successfully.
/// @retval
/// RMT_ERROR_INVALID_POINTER                   The parameter <c><i>threadEvent</i></c> was NULL.
/// @retval
/// RMT_ERROR_PLATFORM_FUNCTION_FAILED          A platform-specific function failed.
///
RmtErrorCode RmtThreadEventReset(RmtThreadEvent* thread_event);

/// Destroy a thread event.
///
/// @param [in]     thread_event                A pointer to a <c><i>RmtThreadEvent</i></c> structure.
///
/// @retval
/// RMT_OK                                      The operation completed successfully.
/// @retval
/// RMT_ERROR_INVALID_POINTER                   The parameter <c><i>threadEvent</i></c> was NULL.
/// @retval
/// RMT_ERROR_PLATFORM_FUNCTION_FAILED          A platform-specific function failed.
///
RmtErrorCode RmtThreadEventDestroy(RmtThreadEvent* thread_event);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_THREAD_EVENT_H_
