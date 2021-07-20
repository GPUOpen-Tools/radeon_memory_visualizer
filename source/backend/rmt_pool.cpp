//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of functions for a fixed-size pool memory allocator.
//=============================================================================

#include "rmt_pool.h"
#include <rmt_assert.h>

#define POOL_ALLOC_TRASH_DEALLOCATED_MEMORY
#ifdef POOL_ALLOC_TRASH_DEALLOCATED_MEMORY
#include <string.h>  // for memset()
#endif               // #ifdef POOL_ALLOC_TRASH_DEALLOCATED_MEMORY

// initialize the pool
RmtErrorCode RmtPoolInitialize(RmtPool* pool, void* buffer, size_t buffer_size, size_t block_size)
{
    // validate pool parameters
    RMT_RETURN_ON_ERROR(pool, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(buffer, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR((buffer_size >= block_size) && (buffer_size % block_size) == 0, kRmtErrorInvalidSize);
    RMT_RETURN_ON_ERROR(block_size >= sizeof(uintptr_t), kRmtErrorInvalidSize);

    // setup structure fields
    pool->buffer      = buffer;
    pool->buffer_size = buffer_size;
    pool->block_size  = block_size;
    pool->head        = buffer;
    pool->allocated   = 0;

    // initialize the free list
    const size_t block_count = (buffer_size / block_size) - 1;

    for (size_t block_index = 0; block_index < block_count; ++block_index)
    {
        void* curr_chunk          = (void*)((uintptr_t)buffer + (block_index * block_size));
        *((uintptr_t*)curr_chunk) = (uintptr_t)curr_chunk + block_size;
    }

    // write the terminal null
    uintptr_t* terminal_null = (uintptr_t*)((uintptr_t)buffer + (block_count * block_size));
    *terminal_null           = (uintptr_t)NULL;
    return kRmtOk;
}

// allocate a new block
void* RmtPoolAllocate(RmtPool* pool)
{
    RMT_RETURN_ON_ERROR(pool, NULL);
    RMT_RETURN_ON_ERROR(pool->head, NULL);

    // deference the head pointer and store it back in head
    void* current_ptr = pool->head;
    pool->head        = (void*)(*((uint8_t**)(pool->head)));
    pool->allocated++;
    return current_ptr;
}

// free a block
void RmtPoolFree(RmtPool* pool, void* address)
{
    if ((pool == nullptr) || (address == nullptr))
    {
        return;
    }

#ifdef POOL_ALLOC_TRASH_DEALLOCATED_MEMORY
    memset(address, 0xcdcdcdcd, pool->block_size);
#endif  // #ifdef POOL_ALLOC_TRASH_DEALLOCATED_MEMORY

    *((uintptr_t*)address) = (uintptr_t)pool->head;
    pool->head             = (void*)address;
    pool->allocated--;
}
