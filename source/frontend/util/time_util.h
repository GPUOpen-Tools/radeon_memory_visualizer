//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of a number of time-related utilities.
//=============================================================================

#ifndef RMV_UTIL_TIME_UTIL_H_
#define RMV_UTIL_TIME_UTIL_H_

#include <QString>

#include "rmt_data_set.h"

namespace rmv
{
    namespace time_util
    {
        /// @brief Convert a clock to a time unit and output as string.
        ///
        /// @param [in] clk input clock to convert.
        ///
        /// @return A string representing a clock value.
        QString ClockToTimeUnit(uint64_t clk);

        /// @brief Get the ratio of time units to clock units.
        ///
        /// Used to convert from time to clocks and vice versa.
        ///
        /// @return The ratio of clock to time.
        double TimeToClockRatio();
    }  // namespace time_util
}  // namespace rmv

#endif  // RMV_UTIL_TIME_UTIL_H_
