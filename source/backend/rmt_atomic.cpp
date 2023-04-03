//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of atomic helper functions.
//=============================================================================

#include <stdint.h>
#include "rmt_atomic.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "rmt_assert.h"

uint64_t RmtThreadAtomicRead(volatile uint64_t* address)
{
    return InterlockedOr(address, 0);
}

uint64_t RmtThreadAtomicWrite(volatile uint64_t* address, uint64_t value)
{
    return InterlockedExchange(address, value);
}

uint64_t RmtThreadAtomicOr(volatile uint64_t* address, uint64_t value)
{
    return InterlockedOr(address, value);
}

int32_t RmtThreadAtomicAdd(volatile int32_t* address, int32_t value)
{
    RMT_STATIC_ASSERT(sizeof(LONG) == sizeof(int32_t));
    return InterlockedAdd((volatile LONG*)address, (LONG)value);
}

int64_t RmtThreadAtomicAdd64(volatile int64_t* address, int64_t value)
{
    RMT_STATIC_ASSERT(sizeof(LONG64) == sizeof(uint64_t));
    return InterlockedAdd64((volatile LONG64*)address, (LONG64)value);
}

int64_t RmtThreadAtomicMax64(volatile int64_t* address, int64_t value)
{
    RMT_STATIC_ASSERT(sizeof(LONG64) == sizeof(uint64_t));

    int64_t previous_value = *address;
    while (previous_value < value && InterlockedCompareExchange64(address, value, previous_value) != previous_value)
    {
        ;
    }

    return value;
}

#else
// Linux-specific atomic functions. Uses the extensions provided by gcc
// The memory order parameter. Set to "sequentially consistent". See
static const int memorder = __ATOMIC_SEQ_CST;
uint64_t         RmtThreadAtomicRead(volatile uint64_t* address)
{
    return __atomic_or_fetch(address, 0, memorder);
}

uint64_t RmtThreadAtomicWrite(volatile uint64_t* address, uint64_t value)
{
    return __atomic_exchange_n(address, value, memorder);
}

uint64_t RmtThreadAtomicOr(volatile uint64_t* address, uint64_t value)
{
    return __atomic_or_fetch(address, value, memorder);
}

int32_t RmtThreadAtomicAdd(volatile int32_t* address, int32_t value)
{
    return __atomic_add_fetch(address, value, memorder);
}

int64_t RmtThreadAtomicAdd64(volatile int64_t* address, int64_t value)
{
    return __atomic_add_fetch(address, value, memorder);
}

#endif  // #ifdef _WIN32
