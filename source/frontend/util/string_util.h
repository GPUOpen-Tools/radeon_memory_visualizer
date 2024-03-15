//=============================================================================
// Copyright (c) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of a number of string utilities.
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
        /// @brief Convert string to upper case.
        ///
        /// Using Qt's toUpper() throws a warning so just do this manually here.
        /// Assumes ASCII and really only intended to capitalize hex number representations.
        ///
        /// @param [in] string The string to capitalize.
        ///
        /// @return capitalized string.
        QString ToUpperCase(const QString& string);

        /// @brief Construct a string representation of a shader address.
        ///
        /// @param [in] upper_bits bits on the left.
        /// @param [in] lower_bits bits on the right.
        ///
        /// @return a flat string.
        QString Convert128BitHashToString(uint64_t upper_bits, uint64_t lower_bits);

        /// @brief Given an integer, return a string localized to English format.
        ///
        /// @param [in] value The value to convert.
        ///
        /// @return The localized string.
        QString LocalizedValue(int64_t value);

        /// @brief Given an integer, return a string localized to English format.
        ///
        /// @param [in] value The value to convert.
        ///
        /// @return The localized string.
        QString LocalizedValuePrecise(double value);

        /// @brief Get the localized string as a memory size.
        ///
        /// Append the memory units to the end of the string. Base 10 or base 2 can
        /// be selected. Base 2 uses 1024 rather than 1000. Units are appended to
        /// display XB for base 10 or XiB for base 2.
        ///
        /// @param [in] value                       The value to display.
        /// @param [in] base_10                     If true, use base 10 values, otherwise base 2.
        /// @param [in] use_round                   If true and units aren't in bytes, round up the value.
        /// @param [in] include_decimal             If true, include the fractional part of the value.
        ///
        /// @return The localized string.
        QString LocalizedValueMemory(const double value, const bool base_10, const bool use_round, const bool include_decimal = true);

        /// @brief Format an address for printing.
        ///
        /// @param [in] address The address to convert.
        ///
        /// @return The localized string.
        QString LocalizedValueAddress(RmtGpuAddress address);

        /// @brief Get the localized string as a memory size in bytes.
        ///
        /// Append the memory units to the end of the string.
        ///
        /// @param value The value to display.
        ///
        /// @return The localized string.
        QString LocalizedValueBytes(int64_t value);

        /// @brief Builds a range string for the memory size thresholds.
        ///
        /// @param [in] min_memory_size           The lower value of the range.
        /// @param [in] max_memory_size           The upper value of the range.
        ///
        /// @return The localized string.
        QString GetMemoryRangeString(const uint64_t min_memory_size, const uint64_t max_memory_size);

    }  // namespace string_util
}  // namespace rmv

#endif  // RMV_UTIL_STRING_UTIL_H_
