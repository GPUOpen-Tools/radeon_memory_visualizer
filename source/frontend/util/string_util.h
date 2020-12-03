//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Definition of a number of string utilities.
//=============================================================================

#ifndef RMV_UTIL_STRING_UTIL_H_
#define RMV_UTIL_STRING_UTIL_H_

#include <QString>
#include <stdint.h>

#include "rmt_resource_list.h"

namespace rmv
{
    namespace string_util
    {
        /// Using Qt's toUpper() throus a warning so just do this manually here.
        /// Assumes ASCII and really only intended to capitalize hex number representations.
        /// \param string The string to capitalize.
        /// \return capitalized string.
        QString ToUpperCase(const QString& string);

        /// Construct a string representation of a shader address.
        /// \param upper_bits bits on the left.
        /// \param lower_bits bits on the right.
        /// \return a flat string.
        QString Convert128BitHashToString(uint64_t upper_bits, uint64_t lower_bits);

        /// Given an integer, return a string localized to English format.
        /// \param value The value to convert.
        QString LocalizedValue(int64_t value);

        /// Given an integer, return a string localized to English format.
        /// \param value The value to convert.
        QString LocalizedValuePrecise(double value);

        /// Get the localized string as a memory size. Append the memory units to the
        /// end of the string.
        /// \param value The value to display.
        /// \param base_10 Whether to use base 10 values. If true, memory is returned
        /// assuming factors of 1000. If false, use 1024 (base 2 values). Units are
        /// appended to display XB for base 10 or XiB for base 2.
        /// \return The localized string.
        QString LocalizedValueMemory(double value, bool base_10, bool use_round);

        /// Format an address for printing.
        /// \param address The address to convert.
        /// \return The localized string.
        QString LocalizedValueAddress(RmtGpuAddress address);

        /// Helper function to fetch a string representation of a resource usage type.
        /// \param usage_type resource usage type.
        /// \return A string representation of a resource usage.
        QString GetResourceUsageString(RmtResourceUsageType usage_type);

    }  // namespace string_util
}  // namespace rmv

#endif  // RMV_UTIL_STRING_UTIL_H_
