//=============================================================================
// Copyright (c) 2017-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of assert.
//=============================================================================

#include "rmt_assert.h"
#include <stdlib.h>  // for malloc()

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>  // required for OutputDebugString()
#include <stdio.h>    // required for sprintf_s
#endif                // #ifndef _WIN32

static RmtAssertCallback s_assert_callback;

// set the printing callback function
void RmtAssertSetPrintingCallback(RmtAssertCallback callback)
{
    s_assert_callback = callback;
    return;
}

// implementation of assert reporting
bool RmtAssertReport(const char* file, int32_t line, const char* condition, const char* message)
{
    if (!file)
    {
        return true;
    }

#ifdef _WIN32
    // form the final assertion string and output to the TTY.
    const size_t bufferSize = snprintf(NULL, 0, "%s(%d): ASSERTION FAILED. %s\n", file, line, message ? message : condition) + 1;
    char*        tempBuf    = (char*)malloc(bufferSize);
    if (!tempBuf)
    {
        return true;
    }

    if (!message)
    {
        sprintf_s(tempBuf, bufferSize, "%s(%d): ASSERTION FAILED. %s\n", file, line, condition);
    }
    else
    {
        sprintf_s(tempBuf, bufferSize, "%s(%d): ASSERTION FAILED. %s\n", file, line, message);
    }

    if (!s_assert_callback)
    {
        OutputDebugStringA(tempBuf);
    }
    else
    {
        s_assert_callback(tempBuf);
    }

    // free the buffer.
    free(tempBuf);

#else
    RMT_UNUSED(line);
    RMT_UNUSED(condition);
    RMT_UNUSED(message);
#endif

    return true;
}
