//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \file
/// \brief Profile utils.
//=============================================================================

#ifndef RMV_PARSER_RMT_PROFILE_H_
#define RMV_PARSER_RMT_PROFILE_H_

#include "rmt_platform.h"
#include <inttypes.h>

#define RMT_PROFILE_START(name)                                  \
    const uint64_t name##___freq_value = rmtGetClockFrequency(); \
    const uint64_t name##___strt_value = rmtGetCurrentTimestamp();

#define RMT_PROFILE_STOP(name)                                                          \
    do                                                                                  \
    {                                                                                   \
        const uint64_t name##___stop_value = rmtGetCurrentTimestamp();                  \
        const uint64_t name##___delta      = name##___stop_value - name##___strt_value; \
        const uint64_t name##___micro      = name##___delta * 1000000;                  \
        const uint64_t name##___final      = name##___micro / name##___freq_value;      \
        printf("%" PRIu64 "ms\n", (name##___final / 1000));                             \
    } while (0)

#endif  // #ifndef RMV_PARSER_RMT_PROFILE_H_
