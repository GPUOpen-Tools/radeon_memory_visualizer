//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Utility macro definitions.
//=============================================================================

#ifndef RMV_PARSER_RMT_UTIL_H_
#define RMV_PARSER_RMT_UTIL_H_

#include "rmt_types.h"

/// Helper macro to avoid warnings about unused variables.
#define RMT_UNUSED(x) ((void)(x))

/// Helper macro to align an integer to the specified power of 2 boundary
#define RMT_ALIGN_UP(x, y) (((x) + ((y)-1)) & ~((y)-1))

/// Helper macro to check if a value is aligned.
#define RMT_IS_ALIGNED(x) (((x) != 0) && ((x) & ((x)-1)))

/// Helper macro to stringify a value.
#define RMT_STR(s) RMT_XSTR(s)
#define RMT_XSTR(s) #s

/// Helper macro to forward declare a structure.
#define RMT_FORWARD_DECLARE(x) typedef struct x x

/// Helper macro to return the maximum of two values.
#define RMT_MAXIMUM(x, y) (((x) > (y)) ? (x) : (y))

/// Helper macro to return the minimum of two values.
#define RMT_MINIMUM(x, y) (((x) < (y)) ? (x) : (y))

/// Helper macro to do safe free on a pointer.
#define RMT_SAFE_FREE(x) \
    if (x)               \
    free(x)

/// Helper macro to return the abs of an integer value.
#define RMT_ABSOLUTE(x) (((x) < 0) ? (-(x)) : (x))

/// Helper macro to return sign of a value.
#define RMT_SIGN(x) (((x) < 0) ? -1 : 1)

/// Helper macro to work out the number of elements in an array.
#define RMT_ARRAY_ELEMENTS(x) (int32_t)((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

#endif  // #ifndef RMV_PARSER_RMT_UTIL_H_
