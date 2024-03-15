//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of functions related to RMT token structures.
//=============================================================================

#include "rmt_assert.h"
#include "rmt_string_memory_pool.h"
#include "rmt_token.h"

// 1MB memory pool for resource name strings.
static const uint64_t kMemoryPoolBlockSize = 1024 * 1024;

// Memory pool for managing resource name text string allocations.
static RmtStringMemoryPool resource_name_string_pool(kMemoryPoolBlockSize);

uint8_t* RmtTokenAllocatePayloadCache(size_t size)
{
    char* buffer = nullptr;
    resource_name_string_pool.Allocate(size, &buffer);
    return reinterpret_cast<uint8_t*>(buffer);
}

/// @brief Clean up the allocated memory for the userdata payloads.
void RmtTokenClearPayloadCaches()
{
    resource_name_string_pool.FreeAll();
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
        dest->userdata_token.time_delay                   = src->userdata_token.time_delay;
        dest->userdata_token.implicit_resource_type       = src->userdata_token.implicit_resource_type;

        if ((dest->userdata_token.userdata_type == kRmtUserdataTypeName || dest->userdata_token.userdata_type == kRmtUserdataTypeName_V2) &&
            (dest->userdata_token.size_in_bytes > 0))
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
