//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author
/// \brief  Error codes.
//=============================================================================

#ifndef RMV_PARSER_RMT_ERROR_H_
#define RMV_PARSER_RMT_ERROR_H_

#include <stdint.h>

#ifndef _WIN32
#include <stdlib.h>
#include <errno.h>
#endif

/// The operation completed successfully.
#define RMT_OK (0)

/// The operation failed due to an invalid pointer.
#define RMT_ERROR_INVALID_POINTER (0x80000000)

/// The operation failed due to an invalid pointer.
#define RMT_ERROR_INVALID_ALIGNMENT (0x80000001)

/// The operation failed due to an invalid size.
#define RMT_ERROR_INVALID_SIZE (0x80000002)

/// The end of the file was encountered.
#define RMT_EOF (0x80000003)

/// The operation failed because the specified path was invalid.
#define RMT_ERROR_INVALID_PATH (0x80000004)

/// The operation failed because end of file was reached.
#define RMT_END_OF_FILE (0x80000005)

/// The operation failed because of some malformed data.
#define RMT_ERROR_MALFORMED_DATA (0x80000006)

/// The operation failed because a file was not open.
#define RMT_ERROR_FILE_NOT_OPEN (0x80000007)

/// The operation failed because it ran out memory.
#define RMT_ERROR_OUT_OF_MEMORY (0x80000008)

/// The operation failed because of an invalid page size.
#define RMT_ERROR_INVALID_PAGE_SIZE (0x80000009)

/// The operation failed because a timestamp was out of bounds.
#define RMT_ERROR_TIMESTAMP_OUT_OF_BOUNDS (0x8000000a)

/// The operation failed because a platform-specific function failed.
#define RMT_ERROR_PLATFORM_FUNCTION_FAILED (0x8000000b)

/// The operation failed because an index was out of range.
#define RMT_ERROR_INDEX_OUT_OF_RANGE (0x8000000c)

/// The operation failed because an allocation could not be found.
#define RMT_ERROR_NO_ALLOCATION_FOUND (0x8000000d)

/// The operation failed because a resource could not be found.
#define RMT_ERROR_NO_RESOURCE_FOUND (0x8000000e)

/// The operation failed because an address was not mapped.
#define RMT_ERROR_ADDRESS_NOT_MAPPED (0x8000000f)

/// The operation failed because an address was already mapped.
#define RMT_ERROR_ADDRESS_ALREADY_MAPPED (0x80000010)

/// An allocation was not found due to it being an external shared allocation.
#define RMT_ERROR_SHARED_ALLOCATION_NOT_FOUND (0x80000011)

/// A resource was already bound to a virtual memory range.
#define RMT_ERROR_RESOURCE_ALREADY_BOUND (0x80000012)

/// The operation failed because a file was already opened.
#define RMT_ERROR_FILE_ALREADY_OPENED (0x80000013)

/// Helper macro to return error code y from a function when a specific condition, x, is not met.
#define RMT_RETURN_ON_ERROR(x, y) \
    if (!(x))                     \
    {                             \
        return (y);               \
    }

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// Typedef for error codes returned from functions in the RMT backend.
typedef int32_t RmtErrorCode;

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_PARSER_RMT_ERROR_H_
