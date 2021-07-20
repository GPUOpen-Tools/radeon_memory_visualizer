//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementations of the mutex abstraction.
//=============================================================================

#ifndef _WIN32
#include <mutex>
#endif  // #ifndef _WIN32

#include <rmt_assert.h>
#include "rmt_mutex.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif  // #ifdef _WIN32

// structure for a mutex
typedef struct RmtMutexInternal
{
#ifdef _WIN32
    HANDLE handle;
#else
    std::mutex* mutex;   // pointer to mutex object
    std::mutex  buffer;  // buffer to store the mutex object (using placement new)
#endif  // #ifdef _WIN32
} RmtMutexInternal;

// create a mutex
RmtErrorCode RmtMutexCreate(RmtMutex* mutex, const char* name)
{
    RMT_ASSERT_MESSAGE(mutex, "Parameter mutex is NULL.");
    RMT_RETURN_ON_ERROR(mutex, kRmtErrorInvalidPointer);

    RmtMutexInternal* mutex_internal = (RmtMutexInternal*)mutex;

    // if this fails it means the external opaque structure isn't large enough for an internal mutex.
    RMT_STATIC_ASSERT(sizeof(RmtMutex) >= sizeof(RmtMutexInternal));

#ifdef _WIN32
    /* create the mutex for win32 */
    mutex_internal->handle = CreateMutexA(NULL, FALSE, name);

    RMT_RETURN_ON_ERROR(mutex_internal->handle != NULL, kRmtErrorPlatformFunctionFailed);
#else
    RMT_UNUSED(name);
    mutex_internal->mutex = new (&mutex_internal->buffer) std::mutex;
#endif  // #ifdef _WIN32

    return kRmtOk;
}

RmtErrorCode RmtMutexLock(RmtMutex* mutex)
{
    RMT_ASSERT_MESSAGE(mutex, "Parameter mutex is NULL.");
    RMT_RETURN_ON_ERROR(mutex, kRmtErrorInvalidPointer);

    RmtMutexInternal* mutex_internal = (RmtMutexInternal*)mutex;

#ifdef _WIN32
    const DWORD error_code = WaitForSingleObject(mutex_internal->handle, INFINITE);
    RMT_RETURN_ON_ERROR(error_code == WAIT_OBJECT_0, kRmtErrorPlatformFunctionFailed);
#else
    mutex_internal->mutex->lock();
#endif  // #ifdef _WIN32

    return kRmtOk;
}

RmtErrorCode RmtMutexUnlock(RmtMutex* mutex)
{
    RMT_ASSERT_MESSAGE(mutex, "Parameter mutex is NULL.");
    RMT_RETURN_ON_ERROR(mutex, kRmtErrorInvalidPointer);

    RmtMutexInternal* mutex_internal = (RmtMutexInternal*)mutex;

#ifdef _WIN32
    BOOL error_code = ReleaseMutex(mutex_internal->handle);
    RMT_RETURN_ON_ERROR(error_code, kRmtErrorPlatformFunctionFailed);
#else
    mutex_internal->mutex->unlock();
#endif  // #ifdef _WIN32

    return kRmtOk;
}

RmtErrorCode RmtMutexDestroy(RmtMutex* mutex)
{
    RMT_ASSERT_MESSAGE(mutex, "Parameter mutex is NULL.");
    RMT_RETURN_ON_ERROR(mutex, kRmtErrorInvalidPointer);

    RmtMutexInternal* mutex_internal = (RmtMutexInternal*)mutex;

#ifdef _WIN32
    BOOL error_code = CloseHandle(mutex_internal->handle);
    RMT_RETURN_ON_ERROR(error_code, kRmtErrorPlatformFunctionFailed);
#else
    mutex_internal->mutex->~mutex();
#endif  // #ifdef _WIN32

    return kRmtOk;
}
