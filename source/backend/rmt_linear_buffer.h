//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  A linear allocator structure and helper functions.
//=============================================================================

#ifndef RMV_BACKEND_RMT_LINEAR_BUFFER_H_
#define RMV_BACKEND_RMT_LINEAR_BUFFER_H_

#include "rmt_error.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// A structure encapsulating state for a linear allocator.
typedef struct RmtAllocLinearBuffer
{
    void*  buffer_base;  ///< A pointer to the buffer.
    size_t buffer_size;  ///< The total size of <c><i>bufferBase</i></c> in bytes.
    size_t offset;       ///< The current offset into <c><i>bufferBase</i></c> in bytes.
} RmtAllocLinearBuffer;

/// Initialize the linear allocator.
///
/// @param [in,out] linear_buffer       A pointer to a <c><i>RmtAllocLinearBuffer</i></c> structure.
/// @param [in] buffer                  A pointer to a memory buffer to manage allocations for.
/// @param [in] buffer_size             The size (in bytes) of the buffer pointed to by <c><i>buffer</i></c>.
///
/// @retval
/// kRmtOk                          The operation completed succesfully.
/// @retval
/// kRmtErrorInvalidPointer         The operation failed because either <c><i>linearBuffer</i></c> or <c><i>buffer</i></c> was NULL.
///
RmtErrorCode RmtAllocLinearBufferInitialize(RmtAllocLinearBuffer* linear_buffer, void* buffer, size_t buffer_size);

/// Allocate the next free block from the pool.
///
/// @param [in,out] linear_buffer       A pointer to a <c><i>RmtAllocLinearBuffer</i></c> structure.
/// @param [in] size                    The size (in bytes) of the allocation.
///
/// @returns
/// A pointer to the next allocated block. If the buffer is empty then <c><i>NULL</i></c> is returned.
///
void* RmtAllocLinearBufferAllocate(RmtAllocLinearBuffer* linear_buffer, size_t size);

/// Get the base address of the linear buffer.
///
/// @param [in] linear_buffer       A pointer to a <c><i>RmtAllocLinearBuffer</i></c> structure.
///
/// @returns
/// A pointer to the linear buffer base address. If linear_buffer is <c><i>NULL</i></c> then <c><i>NULL</i></c> is returned.
void* RmtAllocLinearBufferGetBaseAddress(RmtAllocLinearBuffer* linear_buffer);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_LINEAR_BUFFER_H_
