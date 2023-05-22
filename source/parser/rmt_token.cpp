//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of functions related to RMT token structures.
//=============================================================================

#include "rmt_assert.h"
#include "rmt_token.h"

#include <vector>

// The array of user data allocations, so they can be cleared up later.
static std::vector<uint8_t*> alloc_cache;

// 1MB memory pool for resource name strings.
static const uint64_t kMemSize = 1024 * 1024;

// Housekeeping variables for the current memory pool.
static uint8_t* buffer_ptr    = nullptr;
static size_t   buffer_offset = 0;

/// @brief Allocate memory for a userdata payload.
///
/// These are typically very small strings so internally use a memory pool.
/// Since items are only ever added to the buffer, there is no need to add
/// functionality to deal with deleting or defragmenting memory.
///
/// @@param [in] size   The size of memory to allocate, in bytes.
///
/// @return a pointer to the allocated memory.
uint8_t* RmtTokenAllocatePayloadCache(size_t size)
{
    // Enough room in the buffer? If not, set things up to allocate a new one.
    if ((buffer_offset + size) >= kMemSize)
    {
        buffer_ptr = nullptr;
    }

    // If no buffer, allocate one and add it to the buffer cache.
    if (buffer_ptr == nullptr)
    {
        buffer_ptr = new uint8_t[kMemSize];
        alloc_cache.push_back(buffer_ptr);
        buffer_offset = 0;
    }

    // Generate memory pointer from the buffer.
    uint8_t* mem_ptr = buffer_ptr + buffer_offset;
    buffer_offset += size;
    return mem_ptr;
}

/// @brief Clean up the allocated memory for the userdata payloads.
void RmtTokenClearPayloadCaches()
{
    buffer_offset = 0;
    buffer_ptr    = nullptr;
    for (auto alloc : alloc_cache)
    {
        delete[](alloc);
    }
    alloc_cache.clear();
}

void RmtTokenCopy(RmtToken* dest, const RmtToken* src)
{
    if (src == dest)
    {
        return;
    }

    dest->type = src->type;

    memcpy(&dest->common, &src->common, sizeof(RmtTokenCommon));

    switch (src->type)
    {
    case RmtTokenType::kRmtTokenTypeCpuMap:
        memcpy(&dest->cpu_map_token, &src->cpu_map_token, sizeof(RmtTokenCpuMap));
        break;

    case RmtTokenType::kRmtTokenTypeMisc:
        memcpy(&dest->misc_token, &src->misc_token, sizeof(RmtTokenMisc));
        break;

    case RmtTokenType::kRmtTokenTypePageReference:
        memcpy(&dest->page_reference_token, &src->page_reference_token, sizeof(RmtTokenPageReference));
        break;

    case RmtTokenType::kRmtTokenTypePageTableUpdate:
        memcpy(&dest->page_table_update_token, &src->page_table_update_token, sizeof(RmtTokenPageTableUpdate));
        break;

    case RmtTokenType::kRmtTokenTypeProcessEvent:
        memcpy(&dest->process_event_token, &src->process_event_token, sizeof(RmtTokenProcessEvent));
        break;

    case RmtTokenType::kRmtTokenTypeResourceBind:
        memcpy(&dest->resource_bind_token, &src->resource_bind_token, sizeof(RmtTokenResourceBind));
        break;

    case RmtTokenType::kRmtTokenTypeResourceCreate:
        memcpy(&dest->resource_create_token, &src->resource_create_token, sizeof(RmtTokenResourceCreate));
        break;

    case RmtTokenType::kRmtTokenTypeResourceDestroy:
        memcpy(&dest->resource_destroy_token, &src->resource_destroy_token, sizeof(RmtTokenResourceDestroy));
        break;

    case RmtTokenType::kRmtTokenTypeResourceReference:
        memcpy(&dest->resource_reference, &src->resource_reference, sizeof(RmtTokenResourceReference));
        break;

    case RmtTokenType::kRmtTokenTypeTimeDelta:
        memcpy(&dest->time_delta_token, &src->time_delta_token, sizeof(RmtTokenTimeDelta));
        break;

    case RmtTokenType::kRmtTokenTypeTimestamp:
        memcpy(&dest->timestamp_token, &src->timestamp_token, sizeof(RmtTokenTimestamp));
        break;

    case RmtTokenType::kRmtTokenTypeUserdata:
        dest->userdata_token.resource_identifier          = src->userdata_token.resource_identifier;
        dest->userdata_token.correlation_identifier       = src->userdata_token.correlation_identifier;
        dest->userdata_token.original_resource_identifier = src->userdata_token.original_resource_identifier;
        dest->userdata_token.size_in_bytes                = src->userdata_token.size_in_bytes;
        dest->userdata_token.userdata_type                = src->userdata_token.userdata_type;

        if ((dest->userdata_token.userdata_type == kRmtUserdataTypeName) && (dest->userdata_token.size_in_bytes > 0))
        {
            dest->userdata_token.payload_cache = src->userdata_token.payload_cache;
        }
        break;

    case RmtTokenType::kRmtTokenTypeVirtualAllocate:
        memcpy(&dest->virtual_allocate_token, &src->virtual_allocate_token, sizeof(RmtTokenVirtualAllocate));
        break;

    case RmtTokenType::kRmtTokenTypeVirtualFree:
        memcpy(&dest->virtual_free_token, &src->virtual_free_token, sizeof(RmtTokenVirtualFree));
        break;

    case RmtTokenType::kRmtTokenTypeResourceUpdate:
        memcpy(&dest->resource_update_token, &src->resource_update_token, sizeof(RmtTokenResourceUpdate));
        break;

    default:
        break;
    }
}
