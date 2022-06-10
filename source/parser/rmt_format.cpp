//=============================================================================
// Copyright (c) 2019-2022 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of functions related to RMT token structures.
//=============================================================================

#include "rmt_assert.h"
#include "rmt_format.h"

uint8_t* AllocatePayloadCache(size_t size)
{
    return new uint8_t[size];
}

void DeallocatePayloadCache(uint8_t* payload_cache)
{
    delete[] payload_cache;
}

RmtTokenUserdata::RmtTokenUserdata()
{
    common                       = {};
    original_resource_identifier = 0;
    resource_identifier          = 0;
    correlation_identifier       = 0;
    size_in_bytes                = 0;
    payload_cache                = nullptr;
    userdata_type                = kRmtUserdataTypeName;
}

RmtTokenUserdata::~RmtTokenUserdata()
{
    if (payload_cache != nullptr)
    {
        delete[] payload_cache;
    }
}

RmtTokenUserdata& RmtTokenUserdata::operator=(const RmtTokenUserdata& object)
{
    common                       = object.common;
    resource_identifier          = object.resource_identifier;
    correlation_identifier       = object.correlation_identifier;
    original_resource_identifier = object.original_resource_identifier;
    size_in_bytes                = object.size_in_bytes;
    userdata_type                = object.userdata_type;

    if ((userdata_type == kRmtUserdataTypeName) && (size_in_bytes > 0))
    {
        payload_cache = new uint8_t[size_in_bytes];
        memcpy(payload_cache, object.payload_cache, size_in_bytes);
        payload_cache[size_in_bytes-1] = 0;
    }

    return *this;
}

RmtToken::RmtToken()
{
    memset(reinterpret_cast<void*>(this), 0, sizeof(RmtToken));
}

RmtToken::~RmtToken()
{
}

RmtToken& RmtToken::operator = (const RmtToken& object)
{
    common = object.common;
    type   = object.type;

    switch (object.type)
    {
    case RmtTokenType::kRmtTokenTypeCpuMap:
        cpu_map_token = object.cpu_map_token;
        break;

    case RmtTokenType::kRmtTokenTypeMisc:
        misc_token = object.misc_token;
        break;

    case RmtTokenType::kRmtTokenTypePageReference:
        page_reference_token = object.page_reference_token;
        break;

    case RmtTokenType::kRmtTokenTypePageTableUpdate:
        page_table_update_token = object.page_table_update_token;
        break;

    case RmtTokenType::kRmtTokenTypeProcessEvent:
        process_event_token = object.process_event_token;
        break;

    case RmtTokenType::kRmtTokenTypeResourceBind:
        resource_bind_token = object.resource_bind_token;
        break;

    case RmtTokenType::kRmtTokenTypeResourceCreate:
        resource_create_token = object.resource_create_token;
        break;

    case RmtTokenType::kRmtTokenTypeResourceDestroy:
        resource_destroy_token = object.resource_destroy_token;
        break;

    case RmtTokenType::kRmtTokenTypeResourceReference:
        resource_reference = object.resource_reference;
        break;

    case RmtTokenType::kRmtTokenTypeTimeDelta:
        time_delta_token = object.time_delta_token;
        break;

    case RmtTokenType::kRmtTokenTypeTimestamp:
        timestamp_token = object.timestamp_token;
        break;

    case RmtTokenType::kRmtTokenTypeUserdata:
        userdata_token = object.userdata_token;
        break;

    case RmtTokenType::kRmtTokenTypeVirtualAllocate:
        virtual_allocate_token = object.virtual_allocate_token;
        break;

    case RmtTokenType::kRmtTokenTypeVirtualFree:
        virtual_free_token = object.virtual_free_token;
        break;

    default:
        RMT_ASSERT_MESSAGE(false, "Unsupported token type");
        break;
    }

    return *this;
}
