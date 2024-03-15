//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Error codes.
//=============================================================================

#ifndef RMV_PARSER_RMT_ERROR_H_
#define RMV_PARSER_RMT_ERROR_H_

#include <stdint.h>

#ifndef _WIN32
#include <stdlib.h>
#include <errno.h>
#endif

/// Typedef for error codes returned from functions in the RMT backend.
typedef int32_t RmtErrorCode;

static const RmtErrorCode kRmtOk                            = 0;           /// The operation completed successfully.
static const RmtErrorCode kRmtErrorInvalidPointer           = 0x80000000;  /// The operation failed due to an invalid pointer.
static const RmtErrorCode kRmtErrorInvalidAlignment         = 0x80000001;  /// The operation failed due to an invalid alignment.
static const RmtErrorCode kRmtErrorInvalidSize              = 0x80000002;  /// The operation failed due to an invalid size.
static const RmtErrorCode kRmtEof                           = 0x80000003;  /// The end of the file was encountered.
static const RmtErrorCode kRmtErrorInvalidPath              = 0x80000004;  /// The operation failed because the specified path was invalid.
static const RmtErrorCode kRmtEndOfFile                     = 0x80000005;  /// The operation failed because end of file was reached.
static const RmtErrorCode kRmtErrorMalformedData            = 0x80000006;  /// The operation failed because of some malformed data.
static const RmtErrorCode kRmtErrorFileNotOpen              = 0x80000007;  /// The operation failed because a file was not open.
static const RmtErrorCode kRmtErrorOutOfMemory              = 0x80000008;  /// The operation failed because it ran out memory.
static const RmtErrorCode kRmtErrorInvalidPageSize          = 0x80000009;  /// The operation failed because of an invalid page size.
static const RmtErrorCode kRmtErrorTimestampOutOfBounds     = 0x8000000a;  /// The operation failed because a timestamp was out of bounds.
static const RmtErrorCode kRmtErrorPlatformFunctionFailed   = 0x8000000b;  /// The operation failed because a platform-specific function failed.
static const RmtErrorCode kRmtErrorIndexOutOfRange          = 0x8000000c;  /// The operation failed because an index was out of range.
static const RmtErrorCode kRmtErrorNoAllocationFound        = 0x8000000d;  /// The operation failed because an allocation could not be found.
static const RmtErrorCode kRmtErrorNoResourceFound          = 0x8000000e;  /// The operation failed because a resource could not be found.
static const RmtErrorCode kRmtErrorAddressNotMapped         = 0x8000000f;  /// The operation failed because an address was not mapped.
static const RmtErrorCode kRmtErrorAddressAlreadyMapped     = 0x80000010;  /// The operation failed because an address was already mapped.
static const RmtErrorCode kRmtErrorSharedAllocationNotFound = 0x80000011;  /// An allocation was not found due to it being an external shared allocation.
static const RmtErrorCode kRmtErrorResourceAlreadyBound     = 0x80000012;  /// A resource was already bound to a virtual memory range.
static const RmtErrorCode kRmtErrorFileAlreadyOpened        = 0x80000013;  /// The operation failed because a file was already opened.
static const RmtErrorCode kRmtErrorFileAccessFailed         = 0x80000014;  /// The operation failed because a file couldn't be accessed.
static const RmtErrorCode kRmtErrorPageTableSizeExceeded    = 0x80000015;  /// The operation failed because the page table size was exceeded.
static const RmtErrorCode kRmtErrorTraceFileNotSupported    = 0x80000016;  /// The operation failed because a trace file is unsupported.

/// Helper macro to return error code y from a function when a specific condition, x, is not met.
#define RMT_RETURN_ON_ERROR(x, y) \
    if (!(x))                     \
    {                             \
        return (y);               \
    }

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_PARSER_RMT_ERROR_H_
