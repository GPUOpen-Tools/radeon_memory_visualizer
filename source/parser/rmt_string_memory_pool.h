//=============================================================================
// Copyright (c) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of structures and functions for a dynamic memory pool
///         allocator optimized for small text strings.
//=============================================================================

#ifndef RMV_PARSER_RMT_STRING_MEMORY_POOL_H_
#define RMV_PARSER_RMT_STRING_MEMORY_POOL_H_

#include "rmt_error.h"

#include <vector>

/// A class for allocating text string memory buffers.
/// These are typically very small strings so internally a memory pool is used.
/// Since items are only ever added to the buffer, there is no need to add
/// functionality to deal with deleting or defragmenting memory until all allocated
/// memory is freed.
class RmtStringMemoryPool
{
public:
    /// Constructor for initializing the RmtStringMemoryPool class.
    ///
    /// @param [in] block_size                              The size of a single block to allocate.
    RmtStringMemoryPool(const size_t block_size);

    /// Deletes all allocated memory blocks from the pool.
    ~RmtStringMemoryPool();

    /// Allocate a memory buffer for a text string.
    ///
    /// @param [in]     buffer_size                         The size of the memory buffer requested.
    /// @param [in/out] out_buffer                          A pointer to the allocated memory buffer.
    ///
    /// @retval
    /// kRmtOk                          The operation completed succesfully.
    /// @retval
    /// kRmtErrorInvalidPointer         The operation failed because <c><i>out_buffer</i></c> is invalid.
    RmtErrorCode Allocate(const size_t buffer_size, char** out_buffer);

    /// Delete all allocated memory blocks for the memory pool.
    ///
    /// @retval
    /// kRmtOk                          The operation completed succesfully.
    RmtErrorCode FreeAll();

private:
    std::vector<char*> memory_pool_blocks_;         ///< A list of all memory blocks that have been allocated.
    size_t             memory_pool_block_size_;     ///< The size of memory block allocations.
    char*              memory_pool_buffer_;         ///< A pointer to the current memory block.
    size_t             memory_pool_buffer_offset_;  ///< The offset in the current memory block being allocated.
};
#endif  // #ifndef RMV_PARSER_RMT_STRING_MEMORY_POOL_H_
