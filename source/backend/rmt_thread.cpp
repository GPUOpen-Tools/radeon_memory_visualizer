//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of platform-specific thread.
//=============================================================================

#include "rmt_thread.h"

#ifndef _WIN32
#include <new>
#include <thread>
#endif  // #ifndef _WIN32

#include <stdint.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif /* #ifdef _WIN32 */

#include "rmt_assert.h"

/* structure for a thread. */
typedef struct RmtThreadInternal
{
#ifdef _WIN32
    HANDLE   handle;     // handle to the thread
    uint32_t thread_id;  // the thread ID
#else
    std::thread* thread;  // pointer to the thread object
    std::thread  buffer;  // buffer to store the thread object (using placement new)
#endif  // #ifdef _WIN32
} RmtThreadInternal;

/* create a thread */
RmtErrorCode RmtThreadCreate(RmtThread* thread, RmtThreadFunc thread_func, void* input_data)
{
    RMT_ASSERT_MESSAGE(thread, "Parameter thread is NULL.");
    RMT_ASSERT_MESSAGE(thread, "Parameter threadFunc is NULL.");
    RMT_RETURN_ON_ERROR(thread, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(thread_func, kRmtErrorInvalidPointer);

    // if this fails it means the external opaque structure isn't large enough for an internal thread.
    RMT_STATIC_ASSERT(sizeof(RmtThread) >= sizeof(RmtThreadInternal));

    // conver the pointer to the internal representation
    RmtThreadInternal* thread_internal = (RmtThreadInternal*)thread;

#ifdef _WIN32
    thread_internal->handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_func, input_data, 0, (LPDWORD)&thread_internal->thread_id);
    RMT_RETURN_ON_ERROR(thread_internal->handle != NULL, kRmtErrorPlatformFunctionFailed);
#else
    thread_internal->thread = new (&thread_internal->buffer) std::thread(thread_func, input_data);
#endif  // #ifdef _WIN32

    return kRmtOk;
}

/* wait for the thread to exit */
RmtErrorCode RmtThreadWaitForExit(RmtThread* thread)
{
    RMT_ASSERT_MESSAGE(thread, "Parameter thread is NULL.");
    RMT_RETURN_ON_ERROR(thread, kRmtErrorInvalidPointer);

    // convert the pointer to the internal representation
    RmtThreadInternal* thread_internal = (RmtThreadInternal*)thread;

    // wait for the thread to exit
#ifdef _WIN32
    WaitForSingleObject(thread_internal->handle, INFINITE);
#else
    thread_internal->thread->join();
    thread_internal->thread->~thread();
#endif  // #ifdef _WIN32

    return kRmtOk;
}
