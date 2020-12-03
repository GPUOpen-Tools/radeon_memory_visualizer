//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author
/// \brief Implementations of the mutex abstraction.
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
    RMT_RETURN_ON_ERROR(mutex, RMT_ERROR_INVALID_POINTER);

    RmtMutexInternal* mutex_internal = (RmtMutexInternal*)mutex;

    // if this fails it means the external opaque structure isn't large enough for an internal mutex.
    RMT_STATIC_ASSERT(sizeof(RmtMutex) >= sizeof(RmtMutexInternal));

#ifdef _WIN32
    /* create the mutex for win32 */
    mutex_internal->handle = CreateMutexA(NULL, FALSE, name);

    RMT_RETURN_ON_ERROR(mutex_internal->handle != NULL, RMT_ERROR_PLATFORM_FUNCTION_FAILED);
#else
    RMT_UNUSED(name);
    mutex_internal->mutex = new (&mutex_internal->buffer) std::mutex;
#endif  // #ifdef _WIN32

    return RMT_OK;
}

RmtErrorCode RmtMutexLock(RmtMutex* mutex)
{
    RMT_ASSERT_MESSAGE(mutex, "Parameter mutex is NULL.");
    RMT_RETURN_ON_ERROR(mutex, RMT_ERROR_INVALID_POINTER);

    RmtMutexInternal* mutex_internal = (RmtMutexInternal*)mutex;

#ifdef _WIN32
    const DWORD error_code = WaitForSingleObject(mutex_internal->handle, INFINITE);
    RMT_RETURN_ON_ERROR(error_code == WAIT_OBJECT_0, RMT_ERROR_PLATFORM_FUNCTION_FAILED);
#else
    mutex_internal->mutex->lock();
#endif  // #ifdef _WIN32

    return RMT_OK;
}

RmtErrorCode RmtMutexUnlock(RmtMutex* mutex)
{
    RMT_ASSERT_MESSAGE(mutex, "Parameter mutex is NULL.");
    RMT_RETURN_ON_ERROR(mutex, RMT_ERROR_INVALID_POINTER);

    RmtMutexInternal* mutex_internal = (RmtMutexInternal*)mutex;

#ifdef _WIN32
    BOOL error_code = ReleaseMutex(mutex_internal->handle);
    RMT_RETURN_ON_ERROR(error_code, RMT_ERROR_PLATFORM_FUNCTION_FAILED);
#else
    mutex_internal->mutex->unlock();
#endif  // #ifdef _WIN32

    return RMT_OK;
}

RmtErrorCode RmtMutexDestroy(RmtMutex* mutex)
{
    RMT_ASSERT_MESSAGE(mutex, "Parameter mutex is NULL.");
    RMT_RETURN_ON_ERROR(mutex, RMT_ERROR_INVALID_POINTER);

    RmtMutexInternal* mutex_internal = (RmtMutexInternal*)mutex;

#ifdef _WIN32
    BOOL error_code = CloseHandle(mutex_internal->handle);
    RMT_RETURN_ON_ERROR(error_code, RMT_ERROR_PLATFORM_FUNCTION_FAILED);
#else
    mutex_internal->mutex->~mutex();
#endif  // #ifdef _WIN32

    return RMT_OK;
}
