//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test log file header.
///
/// The log file will contain detailed information about the test run so the
/// exact failure can be determined.
//=============================================================================

#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>

namespace backend_test
{
    class Log
    {
    public:
        /// @brief Constructor.
        Log();

        /// @brief Destructor.
        ~Log();

        /// @brief Open / create the log file.
        ///
        /// @param [in] filename The name of the trace file being tested.
        void Open(const char* filename);

        /// @brief Write a string to the log file.
        ///
        /// @param [in] log_message The string to write.
        void Write(const char* log_message, ...) const;

        /// @brief Write a string to both the console and the log file.
        ///
        /// @param [in] log_message The string to write.
        /// @param [in] ...         Additional arguments dependent on <c><i>log_message</i></c> formatting.
        void WriteConsole(const char* log_message, ...) const;

    private:
        /// @brief Write a string to the log file and optionally the console.
        ///
        /// @param [in] log_message         The string to write.
        /// @param [in] arg_ptr             Additional arguments dependent on <c><i>log_message</i></c> formatting.
        /// @paran [in] write_to_console    If true, the message is output to the console in addition to the log file.
        void WriteImpl(const char* log_message, va_list arg_ptr, const bool write_to_console) const;

    private:
        FILE* log_file_handle_;  ///< Handle to the log file.
    };
}  // namespace backend_test

#endif  // LOG_H_
