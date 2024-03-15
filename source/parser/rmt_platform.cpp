//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of platform-specific utils.
//=============================================================================

#include "rmt_platform.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <time.h>
#endif /* #ifdef _WIN32 */

static const uint64_t kOneBillion = 1000000000;

uint64_t RmtGetClockFrequency()
{
#ifdef _WIN32
    LARGE_INTEGER freq = {0};
    QueryPerformanceFrequency(&freq);
    return (uint64_t)freq.QuadPart;
#else
    // clock_gettime on Linux is always in nanoseconds
    return kOneBillion;
#endif /* #ifdef _WIN32 */
}

uint64_t RmtGetCurrentTimestamp()
{
#ifdef _WIN32
    LARGE_INTEGER clock_value = {0};
    QueryPerformanceCounter(&clock_value);
    return (uint64_t)clock_value.QuadPart;
#else
    struct timespec clock_value;
    clock_gettime(CLOCK_REALTIME, &clock_value);
    return (clock_value.tv_sec * kOneBillion) + clock_value.tv_nsec;
#endif /* #ifdef _WIN32 */
}

void RmtSleep(uint32_t timeout)
{
#ifdef WIN32
    Sleep(timeout);
#else
    timespec     time;
    const time_t sec = static_cast<time_t>(timeout / 1000);
    time.tv_sec      = sec;
    time.tv_nsec     = (timeout - (sec * 1000)) * 1000000;
    nanosleep(&time, nullptr);
#endif
}
