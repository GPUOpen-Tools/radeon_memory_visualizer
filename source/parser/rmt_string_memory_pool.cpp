//=============================================================================
// Copyright (c) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of functions for a dynamic memory pool
///         allocator optimized for small text strings.
//=============================================================================

#include "rmt_string_memory_pool.h"

#include "rmt_assert.h"
#include "rmt_error.h"

RmtStringMemoryPool::RmtStringMemoryPool(const size_t block_size)
{
    memory_pool_block_size_    = block_size;
    memory_pool_buffer_        = nullptr;
    memory_pool_buffer_offset_ = 0;
}

RmtStringMemoryPool::~RmtStringMemoryPool()
{
    FreeAll();
}

RmtErrorCode RmtStringMemoryPool::Allocate(const size_t buffer_size, char** out_buffer)
{
    RMT_RETURN_ON_ERROR(out_buffer != nullptr, kRmtErrorInvalidPointer);

    // Check to see if there is enough room in the buffer.
    // If not, set things up to allocate a new buffer block.
    if ((memory_pool_buffer_offset_ + buffer_size) >= memory_pool_block_size_)
    {
        memory_pool_buffer_ = nullptr;
    }

    // If no buffer, allocate one and add it to the buffer cache.
    if (memory_pool_buffer_ == nullptr)
    {
        memory_pool_buffer_ = new char[memory_pool_block_size_];
        memory_pool_blocks_.push_back(memory_pool_buffer_);
        memory_pool_buffer_offset_ = 0;
    }

    // Generate memory pointer from the buffer.
    *out_buffer = memory_pool_buffer_ + memory_pool_buffer_offset_;
    memory_pool_buffer_offset_ += buffer_size;

    return kRmtOk;
}

RmtErrorCode RmtStringMemoryPool::FreeAll()
{
    memory_pool_buffer_offset_ = 0;
    memory_pool_buffer_        = nullptr;
    for (auto memory_block : memory_pool_blocks_)
    {
        delete[] (memory_block);
    }
    memory_pool_blocks_.clear();

    return kRmtOk;
}
