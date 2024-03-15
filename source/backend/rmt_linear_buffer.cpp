//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the linear allocator helper functions.
//=============================================================================

#include "rmt_linear_buffer.h"
#include "rmt_assert.h"

RmtErrorCode RmtAllocLinearBufferInitialize(RmtAllocLinearBuffer* linear_buffer, void* buffer, size_t buffer_size)
{
    RMT_RETURN_ON_ERROR(linear_buffer && buffer, kRmtErrorInvalidPointer);

    linear_buffer->buffer_base = buffer;
    linear_buffer->buffer_size = buffer_size;
    linear_buffer->offset      = 0;
    return kRmtOk;
}

void* RmtAllocLinearBufferAllocate(RmtAllocLinearBuffer* linear_buffer, size_t size)
{
    RMT_ASSERT_MESSAGE(linear_buffer, "Parameter linearBuffer is NULL.");
    RMT_RETURN_ON_ERROR(linear_buffer, NULL);
    // check there is enough space in the buffer for the allocation.
    RMT_RETURN_ON_ERROR((linear_buffer->offset + size) <= linear_buffer->buffer_size, NULL);

    const uintptr_t address = (uintptr_t)linear_buffer->buffer_base + linear_buffer->offset;
    linear_buffer->offset += size;
    return (void*)address;
}

void* RmtAllocLinearBufferGetBaseAddress(RmtAllocLinearBuffer* linear_buffer)
{
    RMT_ASSERT_MESSAGE(linear_buffer, "Parameter linearBuffer is NULL.");
    RMT_RETURN_ON_ERROR(linear_buffer, NULL);

    return linear_buffer->buffer_base;
}