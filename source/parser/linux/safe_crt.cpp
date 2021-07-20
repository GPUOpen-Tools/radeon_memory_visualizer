//==============================================================================
// Copyright (c) 2016-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Linux implementation of Windows safe CRT functions.
//==============================================================================

#if !defined(_WIN32)

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "safe_crt.h"
#include "../parser/rmt_util.h"

errno_t fopen_s(FILE** file, const char* filename, const char* mode)
{
    if (file == 0 || filename == 0 || mode == 0)
    {
        return EINVAL;
    }

    *file = fopen(filename, mode);

    if (*file)
    {
        return errno;
    }

    return 0;
}

int sprintf_s(char* buffer, size_t size_of_buffer, const char* format, ...)
{
    int     ret_val;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    ret_val = vsnprintf(buffer, size_of_buffer, format, arg_ptr);
    va_end(arg_ptr);
    return ret_val;
}

int fprintf_s(FILE* stream, const char* format, ...)
{
    int     ret_val;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    ret_val = vfprintf(stream, format, arg_ptr);
    va_end(arg_ptr);
    return ret_val;
}

size_t fread_s(void* buffer, size_t buffer_size, size_t element_size, size_t count, FILE* stream)
{
    RMT_UNUSED(count);
    return fread(buffer, element_size, buffer_size, stream);
}

errno_t strcpy_s(char* destination, size_t size, const char* source)
{
    RMT_UNUSED(size);
    strcpy(destination, source);
    return 0;
}

errno_t strcat_s(char* destination, size_t size, const char* source)
{
    RMT_UNUSED(size);
    strcat(destination, source);
    return 0;
}

#endif  // !_WIN32
