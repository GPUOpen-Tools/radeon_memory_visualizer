//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test log file implementation
//=============================================================================

#include "log.h"

#include <stdarg.h>
#include <iostream>
#include <string>

#ifdef _LINUX
#include "linux/safe_crt.h"
#endif

#include "rmt_constants.h"

namespace backend_test
{
    // Suffix added to the trace filename to produce the log filename.
    static const char* kRMVLogFileSuffix = "_test.log";

    Log::Log()
        : log_file_handle_(nullptr)
    {
    }

    Log::~Log()
    {
        if (log_file_handle_ != nullptr)
        {
            fclose(log_file_handle_);
        }
    }

    void Log::Open(const char* rmv_filename)
    {
        if (rmv_filename != nullptr)
        {
            bool        success = false;
            std::string output_filename(rmv_filename);
            std::string original_suffix = kRMVTraceFileExtension;
            size_t      offset          = output_filename.find(kRMVTraceFileExtension);

            if (offset == std::string::npos)
            {
                offset          = output_filename.find(kRGDTraceFileExtension);
                original_suffix = kRGDTraceFileExtension;
            }

            if (offset != std::string::npos)
            {
                std::string new_suffix = kRMVLogFileSuffix;

                output_filename.replace(offset, original_suffix.length(), new_suffix);

                errno_t err = fopen_s(&log_file_handle_, output_filename.c_str(), "wt");
                if (err == 0)
                {
                    success = true;
                }
            }

            if (!success)
            {
                WriteConsole("Error: Unable to open log file");
            }
        }
    }

    void Log::Write(const char* log_message, ...) const
    {
        va_list arg_ptr;
        va_start(arg_ptr, log_message);
        WriteImpl(log_message, arg_ptr, false);
        va_end(arg_ptr);
    }

    void Log::WriteConsole(const char* log_message, ...) const
    {
        va_list arg_ptr;
        va_start(arg_ptr, log_message);
        WriteImpl(log_message, arg_ptr, true);
        va_end(arg_ptr);
    }

    void Log::WriteImpl(const char* log_message, va_list arg_ptr, const bool write_to_console) const
    {
        if (log_message != nullptr)
        {
            if (write_to_console)
            {
                vprintf(log_message, arg_ptr);
                printf("\n");
            }

            if (log_file_handle_ != nullptr)
            {
                vfprintf(log_file_handle_, log_message, arg_ptr);
                fprintf(log_file_handle_, "\n");
            }
        }
    }
}  // namespace backend_test
