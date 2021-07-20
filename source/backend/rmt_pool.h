//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of structures and functions for a fixed-size pool
///         memory allocator.
//=============================================================================

#ifndef RMV_BACKEND_RMT_ALLOC_POOL_H_
#define RMV_BACKEND_RMT_ALLOC_POOL_H_

#include <rmt_error.h>

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// A structure encapsulating state for a pool allocator.
///
/// A pool allocate allocates blocks of a fixed size with a single,
/// contiguous buffer. No guarantee is made about the order that blocks
/// are not allocated within the buffer, they are not always allocated
/// contiguously.
///
/// Allocating and freeing is a relatively cheap, fixed cost operation.
///
/// The contents of freed memory blocks is not changed when blocks are
/// returned to the pool, other than the first sizeof(uintptr_t) bytes.
///
/// NOTE: The minimum supported size of a block in this pool allocator is
/// the same as the sizeof(uintptr_t). This limitation is so the additional
/// memory overhead for the pool allocator is kept to a minimum as no
/// free block list is required. The memory for this is unioned with free
/// blocks in the memory buffer.
///
typedef struct RmtPool
{
    void*  head;
    void*  buffer;
    size_t buffer_size;
    size_t block_size;
    size_t allocated;
} RmtPool;

/// Initialize the pool allocator.
///
/// @param [in,out] pool                A pointer to a <c><i>RmtPool</i></c> structure.
/// @param [in] buffer                  A pointer to a memory buffer to manage allocations for.
/// @param [in] buffer_size             The size (in bytes) of the buffer pointed to by <c><i>buffer</i></c>.
/// @param [in] block_size              The size of a single block to allocate. Must be >= sizeof(uintptr_t).
///
/// @retval
/// kRmtOk                          The operation completed succesfully.
/// @retval
/// kRmtErrorInvalidPointer         The operation failed because either <c><i>pool</i></c> or <c><i>buffer</i></c> was NULL.
/// @retval
/// kRmtErrorInvalidSize            The operation failed because <c><i>chunkSize</i></c> was not greater than or equal to sizeof(uintptr_t).
///
RmtErrorCode RmtPoolInitialize(RmtPool* pool, void* buffer, size_t buffer_size, size_t block_size);

/// Allocate the next free block from the pool.
///
/// @param [in,out] pool                A pointer to a <c><i>RmtPool</i></c> structure.
///
/// @returns
/// A pointer to the next block. If the pool is empty then <c><i>NULL</i></c> is returned.
///
void* RmtPoolAllocate(RmtPool* pool);

/// Free a block that was previously allocated from the pool.
///
/// @param [in,out] pool                A pointer to a <c><i>RmtPool</i></c> structure.
/// @param [in] address                 The address of the block to free back to the pool.
///
void RmtPoolFree(RmtPool* pool, void* address);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_ALLOC_POOL_H_
