//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of platform-specific thread events.
//=============================================================================

#include "rmt_assert.h"
#include "rmt_thread_event.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include "../third_party/pevents/pevents.h"
#endif  // #ifdef _WIN32

// structure for thread event
typedef struct RmtThreadEventInternal
{
#ifdef _WIN32
    HANDLE handle;
#else
    neosmart::neosmart_event_t handle;
#endif  // #ifdef _WIN32
} RmtThreadEventInternal;

// create a thread event
RmtErrorCode RmtThreadEventCreate(RmtThreadEvent* thread_event, bool initial_value, bool manual_reset, const char* name)
{
    RMT_ASSERT_MESSAGE(thread_event, "Parameter threadEvent is NULL.");
    RMT_RETURN_ON_ERROR(thread_event, kRmtErrorInvalidPointer);

    // convert the pointer to the internal representation
    RmtThreadEventInternal* thread_event_internal = (RmtThreadEventInternal*)thread_event;

    // if this fails it means the external opaque structure isn't large enough for an internal thread event.
    RMT_STATIC_ASSERT(sizeof(RmtThreadEvent) >= sizeof(RmtThreadEventInternal));

#ifdef _WIN32
    thread_event_internal->handle = CreateEventA(NULL, manual_reset, initial_value, name);
#else
    RMT_UNUSED(name);
    thread_event_internal->handle = neosmart::CreateEvent(manual_reset, initial_value);
#endif  // #ifdef _WIN32

    RMT_RETURN_ON_ERROR(thread_event_internal->handle != NULL, kRmtErrorPlatformFunctionFailed);
    return kRmtOk;
}

// signal a thread event
RmtErrorCode RmtThreadEventSignal(RmtThreadEvent* thread_event)
{
    RMT_ASSERT_MESSAGE(thread_event, "Parameter threadEvent is NULL.");
    RMT_RETURN_ON_ERROR(thread_event, kRmtErrorInvalidPointer);

    // convert the pointer to the internal representation
    RmtThreadEventInternal* thread_event_internal = (RmtThreadEventInternal*)thread_event;

#ifdef _WIN32
    // signal the event
    BOOL result = SetEvent(thread_event_internal->handle);
#else
    int     result                = neosmart::SetEvent(thread_event_internal->handle);
#endif  // #ifdef _WIN32

    RMT_RETURN_ON_ERROR(result, kRmtErrorPlatformFunctionFailed);
    return kRmtOk;
}

/* wait for a thread event */
RmtErrorCode RmtThreadEventWait(RmtThreadEvent* thread_event)
{
    RMT_ASSERT_MESSAGE(thread_event, "Parameter threadEvent is NULL.");
    RMT_RETURN_ON_ERROR(thread_event, kRmtErrorInvalidPointer);

    // convert the pointer to the internal representation
    RmtThreadEventInternal* thread_event_internal = (RmtThreadEventInternal*)thread_event;

#ifdef _WIN32
    const uint32_t result = WaitForSingleObject(thread_event_internal->handle, INFINITE);
    RMT_RETURN_ON_ERROR(result == WAIT_OBJECT_0, kRmtErrorPlatformFunctionFailed);
#else
    int32_t result                = neosmart::WaitForEvent(thread_event_internal->handle);
    RMT_RETURN_ON_ERROR(result == 0, kRmtErrorPlatformFunctionFailed);
#endif  // #ifdef _WIN32

    return kRmtOk;
}

/* reset a thread event */
RmtErrorCode RmtThreadEventReset(RmtThreadEvent* thread_event)
{
    RMT_ASSERT_MESSAGE(thread_event, "Parameter threadEvent is NULL.");
    RMT_RETURN_ON_ERROR(thread_event, kRmtErrorInvalidPointer);

    // convert the pointer to the internal representation
    RmtThreadEventInternal* thread_event_internal = (RmtThreadEventInternal*)thread_event;

#ifdef _WIN32
    const uint32_t result = ResetEvent(thread_event_internal->handle);
    RMT_RETURN_ON_ERROR(result == WAIT_OBJECT_0, kRmtErrorPlatformFunctionFailed);
#else
    int result = neosmart::ResetEvent(thread_event_internal->handle);

    RMT_RETURN_ON_ERROR(result == 0, kRmtErrorPlatformFunctionFailed);
#endif  // #ifdef _WIN32

    return kRmtOk;
}

RmtErrorCode RmtThreadEventDestroy(RmtThreadEvent* thread_event)
{
    RMT_ASSERT_MESSAGE(thread_event, "Parameter threadEvent is NULL.");
    RMT_RETURN_ON_ERROR(thread_event, kRmtErrorInvalidPointer);

    // convert the pointer to the internal representation
    RmtThreadEventInternal* thread_event_internal = (RmtThreadEventInternal*)thread_event;

#ifdef _WIN32
    BOOL error_code = CloseHandle(thread_event_internal->handle);
#else
    int error_code = neosmart::DestroyEvent(thread_event_internal->handle);
#endif  // #ifdef _WIN32

    RMT_RETURN_ON_ERROR(error_code, kRmtErrorPlatformFunctionFailed);
    return kRmtOk;
}
