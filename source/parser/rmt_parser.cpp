//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of functions for parsing RMT data.
//=============================================================================

#include <string.h>  // for memcpy()

#include "rmt_parser.h"
#include <rmt_print.h>
#include "rmt_format.h"
#include "rmt_util.h"
#include "rmt_assert.h"
#include "rmt_token_heap.h"


// size in bytes of each token.
#define RMT_TOKEN_SIZE_TIMESTAMP (96 / 8)           ///< Timestamp Token Size, in bytes
#define RMT_TOKEN_SIZE_RESERVED_0 (0 / 8)           ///< Reserved_0 Token Size, in bytes
#define RMT_TOKEN_SIZE_RESERVED_1 (0 / 8)           ///< Reserved_1 Token Size, in bytes
#define RMT_TOKEN_SIZE_PAGE_TABLE_UPDATE (144 / 8)  ///< Page Table Update Token Size, in bytes
#define RMT_TOKEN_SIZE_USERDATA (32 / 8)            ///< Userdata Token Size, in bytes
#define RMT_TOKEN_SIZE_MISC (16 / 8)                ///< Misc Token Size, in bytes
#define RMT_TOKEN_SIZE_RESOURCE_REFERENCE (64 / 8)  ///< Resource Reference Token Size, in bytes
#define RMT_TOKEN_SIZE_RESOURCE_BIND (136 / 8)      ///< Resource Bind Token Size, in bytes
#define RMT_TOKEN_SIZE_PROCESS_EVENT (48 / 8)       ///< Process Event Token Size, in bytes
#define RMT_TOKEN_SIZE_PAGE_REFERENCE (80 / 8)      ///< Page Reference Token Size, in bytes
#define RMT_TOKEN_SIZE_CPU_MAP (64 / 8)             ///< CPU Map Token Size, in bytes
#define RMT_TOKEN_SIZE_VIRTUAL_FREE (56 / 8)        ///< Virtual Free Token Size, in bytes
#define RMT_TOKEN_SIZE_VIRTUAL_ALLOCATE (96 / 8)    ///< Virtual Allocate Token Size, in bytes
#define RMT_TOKEN_SIZE_RESOURCE_CREATE (56 / 8)     ///< Resource Create Token Size, in bytes
#define RMT_TOKEN_SIZE_RESOURCE_DESTROY (40 / 8)    ///< Resource Destroy Token Size, in bytes

#define IMAGE_RESOURCE_TOKEN_SIZE (304 / 8)                 ///< Image Resource Token Size
#define IMAGE_RESOURCE_TOKEN_SIZE_V1_6 (312 / 8)            ///< Image Resource Token Size for V1.6 onwards
#define BUFFER_RESOURCE_TOKEN_SIZE (88 / 8)                 ///< Buffer Resource Token Size
#define GPU_EVENT_RESOURCE_TOKEN_SIZE (8 / 8)               ///< GPU Event Resource Token Size
#define BORDER_COLOR_PALETTE_RESOURCE_TOKEN_SIZE (8 / 8)    ///< Border Color Palette Resource Token Size
#define INDIRECT_CMD_GENERATOR_RESOURCE_TOKEN_SIZE (0 / 8)  ///< Indirect Cmd Generator Resource Token Size
#define MOTION_ESTIMATOR_RESOURCE_TOKEN_SIZE (0 / 8)        ///< Motion Estimator Resource Token Size
#define PERF_EXPERIMENT_RESOURCE_TOKEN_SIZE (96 / 8)        ///< Perf Experiment Resource Token Size
#define QUERY_HEAP_RESOURCE_TOKEN_SIZE (8 / 8)              ///< Query Heap Resource Token Size
#define VIDEO_DECODER_RESOURCE_TOKEN_SIZE (32 / 8)          ///< Video Decoder Resource Token Size
#define VIDEO_ENCODER_RESOURCE_TOKEN_SIZE (48 / 8)          ///< Video Encoder Resource Token Size
#define TIMESTAMP_RESOURCE_TOKEN_SIZE (0 / 8)               ///< Timestamp Resource Token Size
#define HEAP_RESOURCE_TOKEN_SIZE (80 / 8)                   ///< Heap Resource Token Size
#define PIPELINE_RESOURCE_TOKEN_SIZE (152 / 8)              ///< Pipeline Resource Token Size
#define DESCRIPTOR_HEAP_RESOURCE_TOKEN_SIZE (32 / 8)        ///< Descriptor Heap Resource Token Size
#define DESCRIPTOR_POOL_RESOURCE_TOKEN_SIZE (24 / 8)        ///< Descriptor Pool Resource Token Size
#define CMD_ALLOCATOR_RESOURCE_TOKEN_SIZE (352 / 8)         ///< Cmd Allocator Resource Token Size
#define MISC_INTERNAL_RESOURCE_TOKEN_SIZE (8 / 8)           ///< Misc Internal Resource Token Size

#define DESCRIPTOR_POOL_DESCRIPTION_SIZE (32 / 8)  ///< Descriptor Pool description size

#define TIMESTAMP_QUANTA (32)

// Is the file version greater than or equal to the version passed in.
// @param rmt_parser The parser containing the current chunk version.
// @param major_version The major version requested.
// @param minor_version The minor version requested.
// @return true if version >= file version, false otherwise.
static bool FileVersionGreaterOrEqual(const RmtParser* rmt_parser, int32_t major_version, int32_t minor_version)
{
    int32_t file_version = (rmt_parser->major_version * 10) + rmt_parser->minor_version;
    int32_t requested_version = (major_version * 10) + minor_version;
    if (file_version >= requested_version)
    {
        return true;
    }
    return false;
}

// read unsigned 8 bit value from the parser's buffer.
static RmtErrorCode ReadUInt8(const RmtParser* rmt_parser, uint8_t* value, size_t offset)
{
    RMT_RETURN_ON_ERROR(rmt_parser->stream_current_offset + sizeof(uint8_t) <= rmt_parser->stream_size, kRmtEof);
    RMT_RETURN_ON_ERROR((size_t)(rmt_parser->file_buffer_offset + offset + sizeof(uint8_t)) <= (size_t)rmt_parser->file_buffer_actual_size,
                        kRmtErrorInvalidSize);

    const uintptr_t base_address = (uintptr_t)rmt_parser->file_buffer + rmt_parser->file_buffer_offset + offset;
    const uint8_t*  base_ptr     = (const uint8_t*)base_address;
    *value                       = *base_ptr;
    return kRmtOk;
}

// read an unsigned 16bit value from the parser's buffer
static RmtErrorCode ReadUInt16(const RmtParser* rmt_parser, uint16_t* value, size_t offset)
{
    // validate that there is enough data to read from the buffers
    RMT_RETURN_ON_ERROR(rmt_parser->stream_current_offset + sizeof(uint16_t) <= rmt_parser->stream_size, kRmtEof);
    RMT_RETURN_ON_ERROR((size_t)(rmt_parser->file_buffer_offset + offset + sizeof(uint16_t)) <= (size_t)rmt_parser->file_buffer_actual_size,
                        kRmtErrorInvalidSize);

    const uintptr_t base_address = (uintptr_t)rmt_parser->file_buffer + rmt_parser->file_buffer_offset + offset;
    const uint16_t* base_ptr     = (const uint16_t*)base_address;
    *value                       = *base_ptr;
    return kRmtOk;
}

// read an unsigned 32bit value from the parser's buffer
static RmtErrorCode ReadUInt32(const RmtParser* rmt_parser, uint32_t* value, size_t offset)
{
    // validate that there is enough data to read from the buffers
    RMT_RETURN_ON_ERROR(rmt_parser->stream_current_offset + sizeof(uint32_t) <= rmt_parser->stream_size, kRmtEof);
    RMT_RETURN_ON_ERROR((size_t)(rmt_parser->file_buffer_offset + offset + sizeof(uint32_t)) <= (size_t)rmt_parser->file_buffer_actual_size,
                        kRmtErrorInvalidSize);

    const uintptr_t base_address = (uintptr_t)rmt_parser->file_buffer + rmt_parser->file_buffer_offset + offset;
    const uint32_t* base_ptr     = (const uint32_t*)base_address;
    *value                       = *base_ptr;
    return kRmtOk;
}

// read an unsigned 64bit value from the parser's buffer
static RmtErrorCode ReadUInt64(RmtParser* rmt_parser, uint64_t* value, size_t offset)
{
    // validate that there is enough data to read from the buffers
    RMT_RETURN_ON_ERROR(rmt_parser->stream_current_offset + sizeof(uint64_t) <= rmt_parser->stream_size, kRmtEof);
    RMT_RETURN_ON_ERROR((size_t)(rmt_parser->file_buffer_offset + offset + sizeof(uint64_t)) <= (size_t)rmt_parser->file_buffer_actual_size,
                        kRmtErrorInvalidSize);

    const uintptr_t base_address = (uintptr_t)rmt_parser->file_buffer + rmt_parser->file_buffer_offset + offset;
    const uint8_t*  base_ptr     = (const uint8_t*)base_address;
    *value = ((uint64_t)base_ptr[7] << 56) | ((uint64_t)base_ptr[6] << 48) | ((uint64_t)base_ptr[5] << 40) | ((uint64_t)base_ptr[4] << 32) |
             ((uint64_t)base_ptr[3] << 24) | ((uint64_t)base_ptr[2] << 16) | ((uint64_t)base_ptr[1] << 8) | ((uint64_t)base_ptr[0]);

    return kRmtOk;
}

// read an array of unsigned 8 bit value from the parser's buffer.
static RmtErrorCode ReadBytes(const RmtParser* rmt_parser, uint8_t* value, size_t offset, size_t size)
{
    RMT_RETURN_ON_ERROR(rmt_parser->stream_current_offset + (sizeof(uint8_t) * size) <= rmt_parser->stream_size, kRmtEof);
    RMT_RETURN_ON_ERROR((size_t)(rmt_parser->file_buffer_offset + offset + (sizeof(uint8_t) * size)) <= (size_t)rmt_parser->file_buffer_actual_size,
                        kRmtErrorInvalidSize);

    const uintptr_t base_address = (uintptr_t)rmt_parser->file_buffer + rmt_parser->file_buffer_offset + offset;
    const uint8_t*  base_ptr     = (const uint8_t*)base_address;
    for (size_t current_byte_index = 0; current_byte_index < size; ++current_byte_index)
    {
        value[current_byte_index] = base_ptr[current_byte_index];
    }

    return kRmtOk;
}

// Get the specified bits from the provided source data, up to 64 bits.
// NOTE: pDstVal will be cleared prior to the copy, any existing data will be lost.
static uint64_t ReadBitsFromBuffer(const uint8_t* buffer, size_t buffer_size, uint32_t end_bit, uint32_t start_bit)
{
    const uint32_t num_bits         = end_bit - start_bit + 1;
    const uint32_t start_byte       = start_bit / 8;
    const uint8_t  start_byte_shift = (start_bit % 8);
    const uint8_t  start_byte_bits  = (uint8_t)RMT_MINIMUM(((uint32_t)8 - start_byte_shift), num_bits);
    const uint8_t  start_byte_mask  = ((1 << start_byte_bits) - 1);
    const uint32_t end_byte         = end_bit / 8;
    const uint8_t  end_byte_bits    = (uint8_t)RMT_MINIMUM(((end_bit % 8) + 1), num_bits);
    const uint8_t  end_byte_mask    = (1 << end_byte_bits) - 1;
    const uint32_t num_bytes        = end_byte - start_byte + 1;

    uint64_t ret_val = 0;
    if (((start_byte + num_bytes) <= buffer_size) && (buffer != nullptr))
    {
        int8_t total_bits_copied = 0;

        for (uint32_t i = 0; i < num_bytes; ++i)
        {
            uint32_t src_idx   = (start_byte + i);
            uint8_t  src_mask  = 0xFF;
            uint8_t  src_shift = 0;
            uint8_t  bits      = 8;
            if (i == 0)
            {
                src_mask  = start_byte_mask;
                bits      = start_byte_bits;
                src_shift = start_byte_shift;
            }
            else if (i == (num_bytes - 1))
            {
                src_mask = end_byte_mask;
                bits     = end_byte_bits;
            }

            // Get the source byte
            uint8_t src_byte = buffer[src_idx];

            // Mask off the target bits, in most cases this will be all of them but for the first
            // or last byte it may be less
            src_byte = (src_byte >> src_shift) & src_mask;

            ret_val |= (static_cast<uint64_t>(src_byte) << total_bits_copied);

            total_bits_copied += bits;
        }
    }

    return ret_val;
}

// update the parser's notion of time
static void UpdateTimeState(RmtParser* rmt_parser, const uint16_t token_header)
{
    // work out the token type.
    const RmtTokenType token_type = (RmtTokenType)(token_header & 0xf);

    if (rmt_parser->seen_timestamp == 0)
    {
        if (token_type == kRmtTokenTypeTimestamp)
        {
            uint64_t     timestamp  = 0;
            RmtErrorCode error_code = ReadUInt64(rmt_parser, &timestamp, 0);
            if (error_code != kRmtOk)
            {
                return;
            }

            uint32_t frequency = 0;
            error_code         = ReadUInt32(rmt_parser, &frequency, 8);
            if (error_code != kRmtOk)
            {
                return;
            }

            rmt_parser->start_timestamp   = ((timestamp >> 4) * TIMESTAMP_QUANTA);
            rmt_parser->current_timestamp = rmt_parser->start_timestamp;
            rmt_parser->cpu_frequency     = frequency;
            rmt_parser->seen_timestamp    = 1;
        }
    }
    else
    {
        switch (token_type)
        {
        case kRmtTokenTypeCpuMap:
        case kRmtTokenTypeVirtualFree:
        case kRmtTokenTypeMisc:
        case kRmtTokenTypePageTableUpdate:
        case kRmtTokenTypeProcessEvent:
        case kRmtTokenTypeResourceBind:
        case kRmtTokenTypeResourceCreate:
        case kRmtTokenTypeResourceReference:
        case kRmtTokenTypeUserdata:
        case kRmtTokenTypeVirtualAllocate:
        {
            const uint64_t delta = (((token_header >> 4) & 0xf) * TIMESTAMP_QUANTA);
            rmt_parser->current_timestamp += delta;
        }
        break;

        case kRmtTokenTypeTimeDelta:
        {
            uint8_t      num_delta_bytes = 0;
            RmtErrorCode error_code      = ReadUInt8(rmt_parser, &num_delta_bytes, 0);
            if (error_code != kRmtOk)
            {
                return;
            }
            num_delta_bytes = (num_delta_bytes >> 4) & 7;

            uint64_t delta = 0;
            error_code     = ReadBytes(rmt_parser, (uint8_t*)&delta, 1, num_delta_bytes);
            if (error_code != kRmtOk)
            {
                return;
            }

            delta *= TIMESTAMP_QUANTA;
            rmt_parser->current_timestamp += delta;
        }
        break;

        case kRmtTokenTypeTimestamp:
        {
            uint64_t           timestamp  = 0;
            const RmtErrorCode error_code = ReadUInt64(rmt_parser, &timestamp, 0);
            if (error_code != kRmtOk)
            {
                return;
            }
            timestamp = (timestamp >> 4) * TIMESTAMP_QUANTA;

            // calculate delta from start timestamp.
            const uint64_t delta          = timestamp;
            rmt_parser->current_timestamp = delta;
        }
        break;

        default:
            break;
        }
    }
}

// Calculate the size of the resource description size from the type.
static int32_t GetResourceDescriptionSize(RmtParser* rmt_parser, RmtResourceType resource_type)
{
    switch (resource_type)
    {
    case kRmtResourceTypeImage:
        // Image format changed at V1.6.
        if (FileVersionGreaterOrEqual(rmt_parser, 1, 6))
        {
            return IMAGE_RESOURCE_TOKEN_SIZE_V1_6;
        }
        else
        {
            return IMAGE_RESOURCE_TOKEN_SIZE;
        }
    case kRmtResourceTypeBuffer:
        return BUFFER_RESOURCE_TOKEN_SIZE;
    case kRmtResourceTypeGpuEvent:
        return GPU_EVENT_RESOURCE_TOKEN_SIZE;
    case kRmtResourceTypeBorderColorPalette:
        return BORDER_COLOR_PALETTE_RESOURCE_TOKEN_SIZE;
    case kRmtResourceTypePerfExperiment:
        return PERF_EXPERIMENT_RESOURCE_TOKEN_SIZE;
    case kRmtResourceTypeQueryHeap:
        return QUERY_HEAP_RESOURCE_TOKEN_SIZE;
    case kRmtResourceTypeVideoDecoder:
        return VIDEO_DECODER_RESOURCE_TOKEN_SIZE;
    case kRmtResourceTypeVideoEncoder:
        return VIDEO_ENCODER_RESOURCE_TOKEN_SIZE;
    case kRmtResourceTypeHeap:
        return HEAP_RESOURCE_TOKEN_SIZE;
    case kRmtResourceTypePipeline:
        return PIPELINE_RESOURCE_TOKEN_SIZE;
    case kRmtResourceTypeDescriptorHeap:
        return DESCRIPTOR_HEAP_RESOURCE_TOKEN_SIZE;
    case kRmtResourceTypeDescriptorPool:
    {
        uint32_t           pool_size_count = 0;
        const RmtErrorCode error_code      = ReadUInt32(rmt_parser, &pool_size_count, RMT_TOKEN_SIZE_RESOURCE_CREATE);
        RMT_UNUSED(error_code);
        pool_size_count = (pool_size_count >> 16) & 0xff;
        return 3 + (pool_size_count * 4);
    }
    case kRmtResourceTypeCommandAllocator:
        return CMD_ALLOCATOR_RESOURCE_TOKEN_SIZE;
    case kRmtResourceTypeMiscInternal:
        return MISC_INTERNAL_RESOURCE_TOKEN_SIZE;

        // all of the the rest have no payload
    default:
        return 0;
    }
}

// Calculate the size of the token from the header and the data in the parser.
static int32_t GetTokenSize(RmtParser* rmt_parser, const uint16_t token_header)
{
    // work out the token type (bottom 4 bits)
    const RmtTokenType token_type = (RmtTokenType)(token_header & 0xf);

    switch (token_type)
    {
    case kRmtTokenTypeTimestamp:
        return RMT_TOKEN_SIZE_TIMESTAMP;
    case kRmtTokenTypeVirtualFree:
        return RMT_TOKEN_SIZE_VIRTUAL_FREE;
    case kRmtTokenTypePageTableUpdate:
        return RMT_TOKEN_SIZE_PAGE_TABLE_UPDATE;
    case kRmtTokenTypeUserdata:
    {
        uint32_t           payload_length = 0;
        const RmtErrorCode error_code     = ReadUInt32(rmt_parser, &payload_length, 0);  // [31:12]
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, 0);
        payload_length = (payload_length >> 12) & 0xfffff;
        return RMT_TOKEN_SIZE_USERDATA + payload_length;
    }
    case kRmtTokenTypeMisc:
        return RMT_TOKEN_SIZE_MISC;
    case kRmtTokenTypeResourceReference:
        return RMT_TOKEN_SIZE_RESOURCE_REFERENCE;
    case kRmtTokenTypeResourceBind:
        return RMT_TOKEN_SIZE_RESOURCE_BIND;
    case kRmtTokenTypeProcessEvent:
        return RMT_TOKEN_SIZE_PROCESS_EVENT;
    case kRmtTokenTypePageReference:
        return RMT_TOKEN_SIZE_PAGE_REFERENCE;
    case kRmtTokenTypeCpuMap:
        return RMT_TOKEN_SIZE_CPU_MAP;
    case kRmtTokenTypeVirtualAllocate:
        return RMT_TOKEN_SIZE_VIRTUAL_ALLOCATE;
    case kRmtTokenTypeResourceCreate:
    {
        const int32_t      base_size_of_resource_description = RMT_TOKEN_SIZE_RESOURCE_CREATE;
        uint8_t            resource_type_byte                = 0;
        const RmtErrorCode error_code                        = ReadUInt8(rmt_parser, &resource_type_byte, 6);  // [53:48]
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, 0);
        const RmtResourceType resource_type                        = (RmtResourceType)resource_type_byte;
        const int32_t         size_of_resource_description_payload = GetResourceDescriptionSize(rmt_parser, resource_type);
        return base_size_of_resource_description + size_of_resource_description_payload;
    }
    case kRmtTokenTypeTimeDelta:
    {
        uint8_t            num_delta_bytes = 0;
        const RmtErrorCode error_code      = ReadUInt8(rmt_parser, &num_delta_bytes, 0);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, 0);
        num_delta_bytes = (num_delta_bytes >> 4) & 7;
        return 1 + num_delta_bytes;
    }
    case kRmtTokenTypeResourceDestroy:
        return RMT_TOKEN_SIZE_RESOURCE_DESTROY;
    default:
        return 1;  // Advance by a byte to try to recover.
    }
}

// populate the common fields of all tokens.
static void PopulateCommonFields(RmtParser* rmt_parser, RmtTokenCommon* common)
{
    common->offset       = rmt_parser->stream_start_offset + rmt_parser->stream_current_offset;
    common->timestamp    = rmt_parser->current_timestamp;
    common->thread_id    = rmt_parser->thread_id;
    common->process_id   = rmt_parser->process_id;  // sometimes overriden.
    common->stream_index = rmt_parser->stream_index;
}

// parse a timestamp token
static RmtErrorCode ParseTimestamp(RmtParser* rmt_parser, const uint16_t token_header, RmtTimestampToken* out_timestamp_token)
{
    RMT_UNUSED(token_header);

    PopulateCommonFields(rmt_parser, &out_timestamp_token->common);

    // token-specific fields.
    uint8_t            data[RMT_TOKEN_SIZE_TIMESTAMP];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, 0, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    uint64_t timestamp             = ReadBitsFromBuffer(data, sizeof(data), 63, 4);
    out_timestamp_token->timestamp = (timestamp >> 4) * TIMESTAMP_QUANTA;
    out_timestamp_token->frequency = (uint32_t)ReadBitsFromBuffer(data, sizeof(data), 95, 64);
    return kRmtOk;
}

// parse a virtual free token
static RmtErrorCode ParseVirtualFree(RmtParser* rmt_parser, const uint16_t token_header, RmtTokenVirtualFree* out_free_token)
{
    RMT_UNUSED(token_header);

    PopulateCommonFields(rmt_parser, &out_free_token->common);

    uint8_t            data[RMT_TOKEN_SIZE_VIRTUAL_FREE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, 0, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    out_free_token->virtual_address = ReadBitsFromBuffer(data, sizeof(data), 55, 8);

    return kRmtOk;
}

// parse a page table update
static RmtErrorCode ParsePageTableUpdate(RmtParser* rmt_parser, const uint16_t token_header, RmtTokenPageTableUpdate* out_page_table_update_token)
{
    RMT_UNUSED(token_header);

    PopulateCommonFields(rmt_parser, &out_page_table_update_token->common);

    // token-specific fields.
    uint8_t            data[RMT_TOKEN_SIZE_PAGE_TABLE_UPDATE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, 0, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    const uint64_t virtual_address               = ReadBitsFromBuffer(data, sizeof(data), 43, 8);
    out_page_table_update_token->virtual_address = virtual_address << 12;

    const uint64_t physical_address               = ReadBitsFromBuffer(data, sizeof(data), 79, 44);
    out_page_table_update_token->physical_address = physical_address << 12;

    const uint64_t size_in_pages               = ReadBitsFromBuffer(data, sizeof(data), 99, 80);
    out_page_table_update_token->size_in_pages = size_in_pages;
    const RmtPageSize page_size                = (RmtPageSize)(ReadBitsFromBuffer(data, sizeof(data), 102, 100));
    out_page_table_update_token->page_size     = page_size;

    const bool is_unmap                       = (bool)(ReadBitsFromBuffer(data, sizeof(data), 103, 103));
    out_page_table_update_token->is_unmapping = is_unmap;

    const uint64_t process_id                      = ReadBitsFromBuffer(data, sizeof(data), 135, 104);
    out_page_table_update_token->common.process_id = process_id;  // override the procID from the token.

    const RmtPageTableUpdateType update_type = (RmtPageTableUpdateType)(ReadBitsFromBuffer(data, sizeof(data), 137, 136));
    out_page_table_update_token->update_type = update_type;

    const RmtPageTableController controller = (RmtPageTableController)(ReadBitsFromBuffer(data, sizeof(data), 138, 138));
    out_page_table_update_token->controller = controller;

    return kRmtOk;
}

// parse a user data blob
static RmtErrorCode ParseUserdata(RmtParser* rmt_parser, const uint16_t token_header, RmtTokenUserdata* out_userdata_token)
{
    RMT_UNUSED(token_header);

    PopulateCommonFields(rmt_parser, &out_userdata_token->common);

    uint32_t           payload_length = 0;
    const RmtErrorCode error_code     = ReadUInt32(rmt_parser, &payload_length, 0);  // [31:12]
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, 0);
    out_userdata_token->userdata_type = (RmtUserdataType)((payload_length & 0xf00) >> 8);
    out_userdata_token->size_in_bytes = (payload_length >> 12) & 0xfffff;

    // If the payload extends beyond the end of the buffer, return an end of file error.  Processing of tokens will continue.
    RMT_RETURN_ON_ERROR((rmt_parser->stream_current_offset + (sizeof(uint8_t) * out_userdata_token->size_in_bytes) <= rmt_parser->stream_size), kRmtEof);

    void* payload                     = (void*)((uintptr_t)rmt_parser->file_buffer + rmt_parser->file_buffer_offset + sizeof(uint32_t));
    out_userdata_token->payload_cache = nullptr;

    if (out_userdata_token->size_in_bytes > 4 && out_userdata_token->userdata_type == kRmtUserdataTypeName)
    {
        // Allocate memory for the payload cache.  This will be deleted by the token destructor.
        out_userdata_token->payload_cache = AllocatePayloadCache(out_userdata_token->size_in_bytes);
        memcpy(out_userdata_token->payload_cache, payload, out_userdata_token->size_in_bytes);

        const uintptr_t id_address = ((uintptr_t)payload) + (out_userdata_token->size_in_bytes - 4);
        const uint32_t  id_value   = *((uint32_t*)id_address);

        // Handle DX12 trace.
        const RmtCorrelationIdentifier correlation_identifier = (RmtCorrelationIdentifier)id_value;
        out_userdata_token->correlation_identifier            = correlation_identifier;

        // Handle Vulkan trace.
        const RmtResourceIdentifier resource_identifier = (RmtResourceIdentifier)id_value;
        out_userdata_token->resource_identifier         = resource_identifier;
    }
    else if (out_userdata_token->size_in_bytes == 8 && out_userdata_token->userdata_type == kRmtUserdataTypeCorrelation)
    {
        const uintptr_t             resource_id_address = ((uintptr_t)payload) + (out_userdata_token->size_in_bytes - 8);
        const uint32_t              resource_id_value   = *((uint32_t*)resource_id_address);
        const RmtResourceIdentifier resource_identifier = (RmtResourceIdentifier)resource_id_value;

        const uintptr_t                correlation_id_address = ((uintptr_t)payload) + (out_userdata_token->size_in_bytes - 4);
        const uint32_t                 correlation_id_value   = *((uint32_t*)correlation_id_address);
        const RmtCorrelationIdentifier correlation_identifier = (RmtCorrelationIdentifier)correlation_id_value;
        out_userdata_token->resource_identifier               = resource_identifier;
        out_userdata_token->correlation_identifier            = correlation_identifier;
    }
    else if (out_userdata_token->userdata_type == kRmtUserdataTypeMarkImplicitResource)
    {
        const uintptr_t             resource_id_address = (reinterpret_cast<uintptr_t>(payload) + (out_userdata_token->size_in_bytes - sizeof(int32_t)));
        const uint32_t              resource_id_value   = *(reinterpret_cast<uint32_t*>(resource_id_address));
        const RmtResourceIdentifier resource_identifier = static_cast<RmtResourceIdentifier>(resource_id_value);
        out_userdata_token->resource_identifier         = resource_identifier;
#ifdef _IMPLICIT_RESOURCE_LOGGING
        RmtPrint("ParseUserdata() - Store implicit resource ID: 0x%llx", out_userdata_token->resource_identifier);
#endif  // _IMPLICIT_RESOURCE_LOGGING
    }

    return kRmtOk;
}

// parse a misc token
static RmtErrorCode ParseMisc(RmtParser* rmt_parser, const uint16_t token_header, RmtTokenMisc* out_misc_token)
{
    RMT_UNUSED(token_header);

    PopulateCommonFields(rmt_parser, &out_misc_token->common);

    uint8_t            data[RMT_TOKEN_SIZE_MISC];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, 0, sizeof(data));
    RMT_UNUSED(error_code);
    out_misc_token->type = (RmtMiscType)ReadBitsFromBuffer(data, sizeof(data), 11, 8);
    return kRmtOk;
}

// parse a resource reference token
static RmtErrorCode ParserResourceReference(RmtParser* rmt_parser, const uint16_t token_header, RmtTokenResourceReference* out_residency_update)
{
    RMT_UNUSED(token_header);

    PopulateCommonFields(rmt_parser, &out_residency_update->common);

    uint8_t            data[RMT_TOKEN_SIZE_RESOURCE_REFERENCE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, 0, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    out_residency_update->residency_update_type = (RmtResidencyUpdateType)ReadBitsFromBuffer(data, sizeof(data), 8, 8);
    out_residency_update->virtual_address       = ReadBitsFromBuffer(data, sizeof(data), 56, 9);
    out_residency_update->queue                 = ReadBitsFromBuffer(data, sizeof(data), 63, 57);
    return kRmtOk;
}

// parse a resource bind token
static RmtErrorCode ParseResourceBind(RmtParser* rmt_parser, const uint16_t token_header, RmtTokenResourceBind* out_resource_bind)
{
    RMT_UNUSED(token_header);

    PopulateCommonFields(rmt_parser, &out_resource_bind->common);

    uint8_t            data[RMT_TOKEN_SIZE_RESOURCE_BIND];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, 0, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    out_resource_bind->virtual_address     = ReadBitsFromBuffer(data, sizeof(data), 55, 8);
    out_resource_bind->size_in_bytes       = ReadBitsFromBuffer(data, sizeof(data), 99, 56);
    out_resource_bind->is_system_memory    = (ReadBitsFromBuffer(data, sizeof(data), 100, 100) != 0U);
    out_resource_bind->resource_identifier = ReadBitsFromBuffer(data, sizeof(data), 135, 104);
    return kRmtOk;
}

// parse a process event token
static RmtErrorCode ParseProcessEvent(RmtParser* rmt_parser, const uint16_t token_header, RmtTokenProcessEvent* out_process_event)
{
    RMT_UNUSED(token_header);

    PopulateCommonFields(rmt_parser, &out_process_event->common);

    uint8_t            data[RMT_TOKEN_SIZE_PROCESS_EVENT];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, 0, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    out_process_event->common.process_id = (RmtProcessId)ReadBitsFromBuffer(data, sizeof(data), 39, 8);
    out_process_event->event_type        = (RmtProcessEventType)ReadBitsFromBuffer(data, sizeof(data), 47, 40);

    return kRmtOk;
}

// parse a page reference token
static RmtErrorCode ParsePageReference(RmtParser* rmt_parser, const uint16_t token_header, RmtTokenPageReference* out_page_reference)
{
    RMT_UNUSED(token_header);

    PopulateCommonFields(rmt_parser, &out_page_reference->common);

    uint8_t            data[RMT_TOKEN_SIZE_PAGE_REFERENCE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, 0, sizeof(data));
    RMT_UNUSED(error_code);

    out_page_reference->page_size      = (RmtPageSize)ReadBitsFromBuffer(data, sizeof(data), 10, 8);
    const bool     is_compressed       = (bool)ReadBitsFromBuffer(data, sizeof(data), 11, 11);
    const uint64_t page_reference_data = ReadBitsFromBuffer(data, sizeof(data), 75, 16);

    RMT_UNUSED(is_compressed);
    RMT_UNUSED(page_reference_data);

    return kRmtOk;
}

// parse a CPU map token
static RmtErrorCode ParseCpuMap(RmtParser* rmt_parser, const uint16_t token_header, RmtTokenCpuMap* out_cpu_map)
{
    RMT_UNUSED(token_header);

    PopulateCommonFields(rmt_parser, &out_cpu_map->common);

    uint8_t            data[RMT_TOKEN_SIZE_CPU_MAP];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, 0, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    out_cpu_map->virtual_address = ReadBitsFromBuffer(data, sizeof(data), 55, 8);
    out_cpu_map->is_unmap        = (ReadBitsFromBuffer(data, sizeof(data), 56, 56) != 0U);

    return kRmtOk;
}

// Parse a virtual allocation.
static RmtErrorCode ParseVirtualAllocate(RmtParser* rmt_parser, const uint16_t token_header, RmtTokenVirtualAllocate* out_virtual_allocate)
{
    RMT_UNUSED(token_header);

    PopulateCommonFields(rmt_parser, &out_virtual_allocate->common);

    uint8_t            data[RMT_TOKEN_SIZE_VIRTUAL_ALLOCATE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, 0, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    const uint64_t size_in_pages_minus_one = ReadBitsFromBuffer(data, sizeof(data), 31, 8);
    out_virtual_allocate->size_in_bytes    = ((size_in_pages_minus_one + 1) * (4 * 1024));
    out_virtual_allocate->owner_type       = (RmtOwnerType)ReadBitsFromBuffer(data, sizeof(data), 33, 32);
    out_virtual_allocate->virtual_address  = ReadBitsFromBuffer(data, sizeof(data), 81, 34);
    out_virtual_allocate->preference[0]    = (RmtHeapType)ReadBitsFromBuffer(data, sizeof(data), 83, 82);
    out_virtual_allocate->preference[1]    = (RmtHeapType)ReadBitsFromBuffer(data, sizeof(data), 85, 84);
    out_virtual_allocate->preference[2]    = (RmtHeapType)ReadBitsFromBuffer(data, sizeof(data), 87, 86);
    out_virtual_allocate->preference[3]    = (RmtHeapType)ReadBitsFromBuffer(data, sizeof(data), 89, 88);

    // Handle flattening of GART_CACHABLE and GART_USWC.
    for (int32_t current_heap_index = 0; current_heap_index < RMT_NUM_HEAP_PREFERENCES; ++current_heap_index)
    {
        if (out_virtual_allocate->preference[current_heap_index] == 3)
        {
            out_virtual_allocate->preference[current_heap_index] = kRmtHeapTypeSystem;
        }
    }

    // Handle cases where preferred heap is not required (V1.6 and higher).
    if (FileVersionGreaterOrEqual(rmt_parser, 1, 6))
    {
        // The heap importance count indicates how many heap preferences should be considered. A value
        // of 0 indicates that there are no heap preferences.
        // This value can be used as a start index of heap preferences that can be set to kRmtHeapTypeNone.
        uint64_t heap_importance_count = ReadBitsFromBuffer(data, sizeof(data), 92, 90);
        for (uint64_t index = heap_importance_count; index < RMT_NUM_HEAP_PREFERENCES; index++)
        {
            out_virtual_allocate->preference[index] = kRmtHeapTypeNone;
        }
    }
    return kRmtOk;
}

// Parse a image description payload.
static RmtErrorCode ParseResourceDescriptionPayloadImage(RmtParser* rmt_parser, RmtResourceDescriptionImage* out_image)
{
    uint8_t            data[IMAGE_RESOURCE_TOKEN_SIZE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // FLAGS [19:0] Creation flags describing how the image was created.
    out_image->create_flags = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 19, 0));

    // USAGE_FLAGS [34:20] Usage flags describing how the image is used by the application.
    out_image->usage_flags = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 34, 20));

    // TYPE [36:35] The type of the image
    out_image->image_type = (RmtImageType)(ReadBitsFromBuffer(data, sizeof(data), 36, 35));

    // DIMENSION_X [49:37] The dimension of the image in the X dimension, minus 1.
    int32_t dimension      = (int32_t)ReadBitsFromBuffer(data, sizeof(data), 49, 37);
    out_image->dimension_x = dimension + 1;

    // DIMENSION_Y [62:50] The dimension of the image in the Y dimension, minus 1.
    dimension              = (int32_t)ReadBitsFromBuffer(&data[0], sizeof(data), 62, 50);
    out_image->dimension_y = (int32_t)(dimension + 1);

    // DIMENSION_Z [75:63] The dimension of the image in the Z dimension, minus 1.
    dimension              = (int32_t)ReadBitsFromBuffer(data, sizeof(data), 75, 63);
    out_image->dimension_z = (int32_t)(dimension + 1);

    // FORMAT [95:76] The format of the image.
    uint64_t format = ReadBitsFromBuffer(data, sizeof(data), 95, 76);
    //   SWIZZLE_X [2:0]
    out_image->format.swizzle_x = (RmtChannelSwizzle)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 2, 0);
    //   SWIZZLE_Y [5:3]
    out_image->format.swizzle_y = (RmtChannelSwizzle)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 5, 3);
    //   SWIZZLE_Z [8:6]
    out_image->format.swizzle_z = (RmtChannelSwizzle)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 8, 6);
    //   SWIZZLE_W [11:9]
    out_image->format.swizzle_w = (RmtChannelSwizzle)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 11, 9);
    //   NUM_FORMAT [19:12]
    out_image->format.format = (RmtFormat)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 19, 12);

    // MIPS [99:96] The number of mip-map levels in the image.
    out_image->mip_levels = (int32_t)(ReadBitsFromBuffer(data, sizeof(data), 99, 96));

    // SLICES [110:100] The number of slices in the image minus one. The maximum this can be in the range [1..2048].
    int32_t slices    = (int32_t)(ReadBitsFromBuffer(data, sizeof(data), 110, 100));
    out_image->slices = slices + 1;

    // SAMPLES [113:111] The Log2(n) of the sample count for the image.
    int32_t log2_samples    = (int32_t)(ReadBitsFromBuffer(data, sizeof(data), 113, 111));
    out_image->sample_count = (1 << log2_samples);

    // FRAGMENTS [115:114] The Log2(n) of the fragment count for the image.
    int32_t log2_fragments    = (int32_t)(ReadBitsFromBuffer(data, sizeof(data), 115, 114));
    out_image->fragment_count = (1 << log2_fragments);

    // TILING_TYPE [117:116] The tiling type used for the image
    out_image->tiling_type = (RmtTilingType)(ReadBitsFromBuffer(data, sizeof(data), 117, 116));

    // TILING_OPT_MODE [119:118] The tiling optimisation mode for the image
    out_image->tiling_optimization_mode = (RmtTilingOptimizationMode)(ReadBitsFromBuffer(data, sizeof(data), 119, 118));

    // METADATA_MODE [121:120] The metadata mode for the image
    out_image->metadata_mode = (RmtMetadataMode)(ReadBitsFromBuffer(&data[0], sizeof(data), 121, 120));

    // MAX_BASE_ALIGNMENT [126:122] The alignment of the image resource. This is stored as the Log2(n) of the
    //                                alignment, it is therefore possible to encode alignments from [1Byte..2MiB].
    uint64_t log2_alignment       = ReadBitsFromBuffer(data, sizeof(data), 126, 122);
    out_image->max_base_alignment = (1ULL << log2_alignment);

    // PRESENTABLE [127] This bit is set to 1 if the image is presentable.
    out_image->presentable = (bool)(ReadBitsFromBuffer(data, sizeof(data), 127, 127));

    // IMAGE_SIZE [159:128] The size of the core image data inside the resource.
    out_image->image_size = ReadBitsFromBuffer(data, sizeof(data), 159, 128);

    // METADATA_OFFSET [191:160] The offset from the base virtual address of the resource to the metadata of
    //                           the image.
    out_image->metadata_tail_offset = ReadBitsFromBuffer(data, sizeof(data), 191, 160);

    // METADATA_SIZE [223:192] The size of the metadata inside the resource.
    out_image->metadata_tail_size = ReadBitsFromBuffer(data, sizeof(data), 223, 192);

    // METADATA_HEADER_OFFSET [255:224] The offset from the base virtual address of the resource to the
    //                                  metadata header.
    out_image->metadata_head_offset = ReadBitsFromBuffer(data, sizeof(data), 255, 224);

    // METADATA_HEADER_SIZE [287:256] The size of the metadata header inside the resource.
    out_image->metadata_head_size = ReadBitsFromBuffer(data, sizeof(data), 287, 256);

    // IMAGE_ALIGN [292:288] The alignment of the core image data within the resource’s virtual address allocation.
    //                       This is stored as the Log2(n) of the alignment.
    log2_alignment             = ReadBitsFromBuffer(data, sizeof(data), 292, 288);
    out_image->image_alignment = (1ULL << log2_alignment);

    // METADATA_ALIGN [297:293] The alignment of the metadata within the resource’s virtual address allocation.
    //                          This is stored as the Log2(n) of the alignment.
    log2_alignment                     = ReadBitsFromBuffer(data, sizeof(data), 297, 293);
    out_image->metadata_tail_alignment = (1ULL << log2_alignment);

    // METADATA_HEADER_ALIGN [302:298] The alignment of the metadata header within the resource’s virtual address
    //                                 allocation. This is stored as the Log2(n) of the alignment.
    log2_alignment                     = ReadBitsFromBuffer(data, sizeof(data), 302, 298);
    out_image->metadata_head_alignment = (1ULL << log2_alignment);

    // FULLSCREEN [303] This bit is set to 1 if the image is fullscreen presentable.
    out_image->fullscreen = (bool)(ReadBitsFromBuffer(data, sizeof(data), 303, 303));
    return kRmtOk;
}

// Parse a image description payload for chunk file version > 1.6. This requires an extra bit added to the X,Y,Z
// image dimensions, which shifts everything else out by 3 bits. The payload size is also increased by 1 byte
// to accommodate these extra bits.
static RmtErrorCode ParseResourceDescriptionPayloadImageV1_6(RmtParser* rmt_parser, RmtResourceDescriptionImage* out_image)
{
    uint8_t            data[IMAGE_RESOURCE_TOKEN_SIZE_V1_6];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // FLAGS [19:0] Creation flags describing how the image was created.
    out_image->create_flags = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 19, 0));

    // USAGE_FLAGS [34:20] Usage flags describing how the image is used by the application.
    out_image->usage_flags = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 34, 20));

    // TYPE [36:35] The type of the image
    out_image->image_type = (RmtImageType)(ReadBitsFromBuffer(data, sizeof(data), 36, 35));

    // DIMENSION_X [50:37] The dimension of the image in the X dimension, minus 1.
    int32_t dimension      = (int32_t)ReadBitsFromBuffer(data, sizeof(data), 50, 37);
    out_image->dimension_x = dimension + 1;

    // DIMENSION_Y [64:51] The dimension of the image in the Y dimension, minus 1.
    dimension              = (int32_t)ReadBitsFromBuffer(&data[0], sizeof(data), 64, 51);
    out_image->dimension_y = (int32_t)(dimension + 1);

    // DIMENSION_Z [78:65] The dimension of the image in the Z dimension, minus 1.
    dimension              = (int32_t)ReadBitsFromBuffer(data, sizeof(data), 78, 65);
    out_image->dimension_z = (int32_t)(dimension + 1);

    // FORMAT [98:79] The format of the image.
    uint64_t format = ReadBitsFromBuffer(data, sizeof(data), 98, 79);
    //   SWIZZLE_X [2:0]
    out_image->format.swizzle_x = (RmtChannelSwizzle)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 2, 0);
    //   SWIZZLE_Y [5:3]
    out_image->format.swizzle_y = (RmtChannelSwizzle)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 5, 3);
    //   SWIZZLE_Z [8:6]
    out_image->format.swizzle_z = (RmtChannelSwizzle)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 8, 6);
    //   SWIZZLE_W [11:9]
    out_image->format.swizzle_w = (RmtChannelSwizzle)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 11, 9);
    //   NUM_FORMAT [19:12]
    out_image->format.format = (RmtFormat)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 19, 12);

    // MIPS [102:99] The number of mip-map levels in the image.
    out_image->mip_levels = (int32_t)(ReadBitsFromBuffer(data, sizeof(data), 102, 99));

    // SLICES [113:103] The number of slices in the image minus one. The maximum this can be in the range [1..2048].
    int32_t slices    = (int32_t)(ReadBitsFromBuffer(data, sizeof(data), 113, 103));
    out_image->slices = slices + 1;

    // SAMPLES [116:114] The Log2(n) of the sample count for the image.
    int32_t log2_samples    = (int32_t)(ReadBitsFromBuffer(data, sizeof(data), 116, 114));
    out_image->sample_count = (1 << log2_samples);

    // FRAGMENTS [118:117] The Log2(n) of the fragment count for the image.
    int32_t log2_fragments    = (int32_t)(ReadBitsFromBuffer(data, sizeof(data), 118, 117));
    out_image->fragment_count = (1 << log2_fragments);

    // TILING_TYPE [120:119] The tiling type used for the image
    out_image->tiling_type = (RmtTilingType)(ReadBitsFromBuffer(data, sizeof(data), 120, 119));

    // TILING_OPT_MODE [122:121] The tiling optimisation mode for the image
    out_image->tiling_optimization_mode = (RmtTilingOptimizationMode)(ReadBitsFromBuffer(data, sizeof(data), 122, 121));

    // METADATA_MODE [124:123] The metadata mode for the image
    out_image->metadata_mode = (RmtMetadataMode)(ReadBitsFromBuffer(&data[0], sizeof(data), 124, 123));

    // MAX_BASE_ALIGNMENT [129:125] The alignment of the image resource. This is stored as the Log2(n) of the
    //                                alignment, it is therefore possible to encode alignments from [1Byte..2MiB].
    uint64_t log2_alignment       = ReadBitsFromBuffer(data, sizeof(data), 129, 125);
    out_image->max_base_alignment = (1ULL << log2_alignment);

    // PRESENTABLE [130] This bit is set to 1 if the image is presentable.
    out_image->presentable = (bool)(ReadBitsFromBuffer(data, sizeof(data), 130, 130));

    // IMAGE_SIZE [162:131] The size of the core image data inside the resource.
    out_image->image_size = ReadBitsFromBuffer(data, sizeof(data), 162, 131);

    // METADATA_OFFSET [194:163] The offset from the base virtual address of the resource to the metadata of
    //                           the image.
    out_image->metadata_tail_offset = ReadBitsFromBuffer(data, sizeof(data), 194, 163);

    // METADATA_SIZE [226:195] The size of the metadata inside the resource.
    out_image->metadata_tail_size = ReadBitsFromBuffer(data, sizeof(data), 226, 195);

    // METADATA_HEADER_OFFSET [258:227] The offset from the base virtual address of the resource to the
    //                                  metadata header.
    out_image->metadata_head_offset = ReadBitsFromBuffer(data, sizeof(data), 258, 227);

    // METADATA_HEADER_SIZE [290:259] The size of the metadata header inside the resource.
    out_image->metadata_head_size = ReadBitsFromBuffer(data, sizeof(data), 290, 259);

    // IMAGE_ALIGN [295:291] The alignment of the core image data within the resource’s virtual address allocation.
    //                       This is stored as the Log2(n) of the alignment.
    log2_alignment             = ReadBitsFromBuffer(data, sizeof(data), 295, 291);
    out_image->image_alignment = (1ULL << log2_alignment);

    // METADATA_ALIGN [300:296] The alignment of the metadata within the resource’s virtual address allocation.
    //                          This is stored as the Log2(n) of the alignment.
    log2_alignment                     = ReadBitsFromBuffer(data, sizeof(data), 300, 296);
    out_image->metadata_tail_alignment = (1ULL << log2_alignment);

    // METADATA_HEADER_ALIGN [305:301] The alignment of the metadata header within the resource’s virtual address
    //                                 allocation. This is stored as the Log2(n) of the alignment.
    log2_alignment                     = ReadBitsFromBuffer(data, sizeof(data), 305, 301);
    out_image->metadata_head_alignment = (1ULL << log2_alignment);

    // FULLSCREEN [306] This bit is set to 1 if the image is fullscreen presentable.
    out_image->fullscreen = (bool)(ReadBitsFromBuffer(data, sizeof(data), 306, 306));
    return kRmtOk;
}

// parse a buffer description payload.
static RmtErrorCode ParseResourceDescriptionPayloadBuffer(RmtParser* rmt_parser, RmtResourceDescriptionBuffer* out_buffer)
{
    uint8_t            data[BUFFER_RESOURCE_TOKEN_SIZE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // CREATE_FLAGS [7:0] The create flags for a buffer.
    out_buffer->create_flags = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 7, 0));
    // USAGE_FLAGS [23:8] The usage flags for a buffer.
    out_buffer->usage_flags = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 23, 8));
    // SIZE [87:24] The size in bytes of the buffer.
    out_buffer->size_in_bytes = ReadBitsFromBuffer(data, sizeof(data), 87, 24);
    return kRmtOk;
}

// parse a gpu event description payload.
static RmtErrorCode ParseResourceDescriptionPayloadGpuEvent(RmtParser* rmt_parser, RmtResourceDescriptionGpuEvent* out_gpu_event)
{
    uint8_t            data[GPU_EVENT_RESOURCE_TOKEN_SIZE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_UNUSED(error_code);

    // FLAGS [7:0] The flags used to create the GPU event.
    out_gpu_event->flags = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 7, 0));
    return kRmtOk;
}

// parse a border palette description payload.
static RmtErrorCode ParseResourceDescriptionPayloadBorderColorPalette(RmtParser* rmt_parser, RmtResourceDescriptionBorderColorPalette* out_border_color_palette)
{
    uint8_t            data[BORDER_COLOR_PALETTE_RESOURCE_TOKEN_SIZE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // NUM_ENTRIES [7:0] The number of entries in the border color palette.
    out_border_color_palette->size_in_entries = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 7, 0));
    return kRmtOk;
}

// parse perf experiment description payload.
static RmtErrorCode ParseResourceDescriptionPayloadPerfExperiment(RmtParser* rmt_parser, RmtResourceDescriptionPerfExperiment* out_perf_experiment)
{
    uint8_t            data[PERF_EXPERIMENT_RESOURCE_TOKEN_SIZE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // SPM_SIZE [31:0] The size in bytes for the amount of memory allocated for SPM counter streaming.
    out_perf_experiment->spm_size = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 31, 0));

    // SQTT_SIZE [63:32] The size in bytes for the amount of memory allocated for SQTT data streaming.
    out_perf_experiment->sqtt_size = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 63, 32));

    // COUNTER_SIZE [95:64] The size in bytes for the amount of memory allocated for per-draw counter data.
    out_perf_experiment->counter_size = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 95, 64));
    return kRmtOk;
}

// parse query heap description payload.
static RmtErrorCode ParseResourceDescriptionPayloadQueryHeap(RmtParser* rmt_parser, RmtResourceDescriptionQueryHeap* out_query_heap)
{
    uint8_t            data[QUERY_HEAP_RESOURCE_TOKEN_SIZE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // TYPE [1:0] The type of the query heap. See RMT_QUERY_HEAP_TYPE.
    out_query_heap->heap_type = (RmtQueryHeapType)(ReadBitsFromBuffer(data, sizeof(data), 1, 0));

    // ENABLE_CPU_ACCESS [2] Set to 1 if CPU access is enabled.
    out_query_heap->enable_cpu_access = (bool)(ReadBitsFromBuffer(data, sizeof(data), 2, 2));
    return kRmtOk;
}

// parse video decoder description payload.
static RmtErrorCode ParseResourceDescriptionPayloadVideoDecoder(RmtParser* rmt_parser, RmtResourceDescriptionVideoDecoder* out_video_decoder)
{
    uint8_t            data[VIDEO_DECODER_RESOURCE_TOKEN_SIZE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // ENGINE_TYPE [3:0] The type of engine that the video decoder will run on.
    out_video_decoder->engine_type = (RmtEngineType)(ReadBitsFromBuffer(data, sizeof(data), 3, 0));

    // VIDEO_DECODER_TYPE [7:4] The type of decoder being run.
    out_video_decoder->decoder_type = (RmtVideoDecoderType)(ReadBitsFromBuffer(data, sizeof(data), 7, 4));

    // WIDTH [19:8] The width of the video minus one.
    uint32_t width           = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 19, 8));
    out_video_decoder->width = width + 1;

    // HEIGHT [31:20] The height of the video minus one.
    uint32_t height           = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 31, 20));
    out_video_decoder->height = height + 1;
    return kRmtOk;
}

// parse video encoder description payload.
static RmtErrorCode ParseResourceDescriptionPayloadVideoEncoder(RmtParser* rmt_parser, RmtResourceDescriptionVideoEncoder* out_video_encoder)
{
    uint8_t            data[VIDEO_ENCODER_RESOURCE_TOKEN_SIZE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // ENGINE_TYPE [3:0] The type of engine that the video encoder will run on.
    out_video_encoder->engine_type = (RmtEngineType)(ReadBitsFromBuffer(data, sizeof(data), 3, 0));

    // VIDEO_ENCODER_TYPE [4] The type of encoder being run.
    out_video_encoder->encoder_type = (RmtVideoEncoderType)(ReadBitsFromBuffer(data, sizeof(data), 4, 4));

    // WIDTH [16:5] The width of the video minus one.
    uint16_t width           = (uint16_t)(ReadBitsFromBuffer(data, sizeof(data), 16, 5));
    out_video_encoder->width = width + 1;

    // HEIGHT [28:17] The height of the video minus one.
    uint16_t height           = (uint16_t)(ReadBitsFromBuffer(data, sizeof(data), 28, 17));
    out_video_encoder->height = height + 1;

    // IMAGE_FORMAT [47:29] Image format
    uint64_t format = ReadBitsFromBuffer(data, sizeof(data), 47, 29);
    //   SWIZZLE_X [2:0]
    out_video_encoder->format.swizzle_x = (RmtChannelSwizzle)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 2, 0);
    //   SWIZZLE_Y [5:3]
    out_video_encoder->format.swizzle_y = (RmtChannelSwizzle)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 5, 3);
    //   SWIZZLE_Z [8:6]
    out_video_encoder->format.swizzle_z = (RmtChannelSwizzle)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 8, 6);
    //   SWIZZLE_W [11:9]
    out_video_encoder->format.swizzle_w = (RmtChannelSwizzle)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 11, 9);
    //   NUM_FORMAT [19:12]
    out_video_encoder->format.format = (RmtFormat)ReadBitsFromBuffer((uint8_t*)&format, sizeof(format), 19, 12);

    return kRmtOk;
}

// parse heap description payload.
static RmtErrorCode ParseResourceDescriptionPayloadHeap(RmtParser* rmt_parser, RmtResourceDescriptionHeap* out_heap)
{
    uint8_t            data[HEAP_RESOURCE_TOKEN_SIZE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // FLAGS [4:0] The flags used to create the heap.
    out_heap->flags = (uint8_t)(ReadBitsFromBuffer(data, sizeof(data), 4, 0));

    // SIZE [68:5] The size of the heap in bytes.
    out_heap->size = ReadBitsFromBuffer(data, sizeof(data), 68, 5);

    // ALIGNMENT [73:69] The alignment of the heap. This will always match a page size, and therefore is encoded as RmtPageSize.
    out_heap->alignment = (RmtPageSize)(ReadBitsFromBuffer(data, sizeof(data), 73, 69));

    // SEGMENT_INDEX [77:74] The segment index where the heap was requested to be created.
    out_heap->segment_index = (uint8_t)(ReadBitsFromBuffer(data, sizeof(data), 77, 74));

    return kRmtOk;
}

// parse a pipeline.
static RmtErrorCode ParseResourceDescriptionPayloadPipeline(RmtParser* rmt_parser, RmtResourceDescriptionPipeline* out_pipeline)
{
    uint8_t            data[PIPELINE_RESOURCE_TOKEN_SIZE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // CREATE_FLAGS [7:0] Describes the creation flags for the pipeline.
    out_pipeline->create_flags = (uint8_t)(ReadBitsFromBuffer(data, sizeof(data), 7, 0));

    // PIPELINE_HASH [135:8] The 128bit pipeline hash of the code object.
    out_pipeline->internal_pipeline_hash_hi = ReadBitsFromBuffer(data, sizeof(data), 71, 8);
    out_pipeline->internal_pipeline_hash_lo = ReadBitsFromBuffer(data, sizeof(data), 135, 72);

    // Pipeline Stages [143:136].
    out_pipeline->stage_mask = (uint32_t)(ReadBitsFromBuffer(data, sizeof(data), 143, 136));

    // IS_NGG [144] The bit is set to true if the pipeline was compiled in NGG mode.
    out_pipeline->is_ngg = (bool)(ReadBitsFromBuffer(data, sizeof(data), 144, 144));

    return kRmtOk;
}

// parse descriptor heap experiment description payload.
static RmtErrorCode ParseResourceDescriptionPayloadDescriptorHeap(RmtParser* rmt_parser, RmtResourceDescriptionDescriptorHeap* out_descriptor_heap)
{
    uint8_t            data[DESCRIPTOR_HEAP_RESOURCE_TOKEN_SIZE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // TYPE [3:0] The type of descriptors in the heap.
    out_descriptor_heap->descriptor_type = (RmtDescriptorType)(ReadBitsFromBuffer(data, sizeof(data), 3, 0));

    // SHADER_VISIBLE [4] Flag indicating whether the heap is shader-visible.
    out_descriptor_heap->shader_visible = (bool)(ReadBitsFromBuffer(data, sizeof(data), 4, 4));

    // GPU_MASK [12:5] Bitmask to identify which adapters the heap applies to.
    out_descriptor_heap->gpu_mask = (uint8_t)(ReadBitsFromBuffer(data, sizeof(data), 12, 5));

    // NUM_DESCRIPTORS [28:13] The number of descriptors in the heap.
    out_descriptor_heap->num_descriptors = (uint16_t)(ReadBitsFromBuffer(data, sizeof(data), 28, 13));

    return kRmtOk;
}

// parse descriptor pool experiment description payload.
static RmtErrorCode ParseResourceDescriptionPayloadDescriptorPool(RmtParser* rmt_parser, RmtResourceDescriptionDescriptorPool* out_descriptor_pool)
{
    uint8_t      data[DESCRIPTOR_POOL_RESOURCE_TOKEN_SIZE];
    RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // MAX_SETS [11:0] Maximum number of descriptor sets that can be allocated from the pool.
    out_descriptor_pool->max_sets = (uint16_t)(ReadBitsFromBuffer(data, sizeof(data), 15, 0));

    // POOL_SIZE_COUNT [15:12] The number of pool size structs.
    out_descriptor_pool->pools_count = (uint8_t)(ReadBitsFromBuffer(data, sizeof(data), 23, 16));

    size_t offset = RMT_TOKEN_SIZE_RESOURCE_CREATE + DESCRIPTOR_POOL_RESOURCE_TOKEN_SIZE;
    for (uint8_t i = 0; i < out_descriptor_pool->pools_count; ++i)
    {
        uint8_t pool_desc_data[DESCRIPTOR_POOL_DESCRIPTION_SIZE];
        error_code = ReadBytes(rmt_parser, pool_desc_data, offset, sizeof(pool_desc_data));
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

        // TYPE [15:0] Descriptor type this pool can hold.
        out_descriptor_pool->pools[i].type = (RmtDescriptorType)(ReadBitsFromBuffer(pool_desc_data, sizeof(pool_desc_data), 15, 0));

        // NUM_DESCRIPTORS [31:16] Number of descriptors to be allocated by this pool.
        out_descriptor_pool->pools[i].num_descriptors = (RmtDescriptorType)(ReadBitsFromBuffer(pool_desc_data, sizeof(pool_desc_data), 31, 16));

        offset += sizeof(pool_desc_data);
    }

    return kRmtOk;
}

// parse command allocator experiment description payload.
static RmtErrorCode ParseResourceDescriptionPayloadCmdAllocator(RmtParser* rmt_parser, RmtResourceDescriptionCommandAllocator* out_command_allocator)
{
    uint8_t            data[CMD_ALLOCATOR_RESOURCE_TOKEN_SIZE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // FLAGS [3:0] Describes the creation flags for the command allocator.
    out_command_allocator->flags = (uint8_t)(ReadBitsFromBuffer(data, sizeof(data), 3, 0));

    // CMD_DATA_PREFERRED_HEAP [7:4] The preferred allocation heap for executable command data
    out_command_allocator->cmd_data_heap = (RmtHeapType)(ReadBitsFromBuffer(data, sizeof(data), 7, 4));

    // CMD_DATA_ALLOC_SIZE [63:8] Size of the base memory allocations the command allocator will make for executable command data. Expressed as 4kB chunks
    out_command_allocator->cmd_data_size = ReadBitsFromBuffer(data, sizeof(data), 63, 8);

    // CMD_DATA_SUBALLOC_SIZE [119:64] Size, in bytes, of the chunks the command allocator will give to command buffers for executable command data. Expressed as 4kB chunks
    out_command_allocator->cmd_data_suballoc_size = ReadBitsFromBuffer(data, sizeof(data), 119, 64);

    // EMBEDDED_DATA_PREFERRED_HEAP [123:120] The preferred allocation heap for embedded command data
    out_command_allocator->embed_data_heap = (RmtHeapType)(ReadBitsFromBuffer(data, sizeof(data), 123, 120));

    // EMBEDDED_DATA_ALLOC_SIZE [179:124] Size, in bytes, of the base memory allocations the command allocator will make for embedded command data. Expressed as 4kB chunks
    out_command_allocator->embed_data_size = ReadBitsFromBuffer(data, sizeof(data), 179, 124);

    // EMBEDDED_DATA_SUBALLOC_SIZE [235:180] Size, in bytes, of the chunks the command allocator will give to command buffers for embedded command data. Expressed as 4kB chunks
    out_command_allocator->embed_data_suballoc_size = ReadBitsFromBuffer(data, sizeof(data), 235, 180);

    // GPU_SCRATCH_MEM_PREFERRED_HEAP [239:236] The preferred allocation heap for GPU scratch memory.
    out_command_allocator->gpu_scratch_heap = (RmtHeapType)(ReadBitsFromBuffer(data, sizeof(data), 239, 236));

    // GPU_SCRATCH_MEM_ALLOC_SIZE [295:240] Size, in bytes, of the base memory allocations the command allocator will make for GPU scratch memory. Expressed as 4kB chunks
    out_command_allocator->gpu_scratch_size = ReadBitsFromBuffer(data, sizeof(data), 295, 240);

    // GPU_SCRATCH_MEM_SUBALLOC_SIZE [351:296] Size, in bytes, of the chunks the command allocator will give to command buffers for GPU scratch memory. Expressed as 4kB chunks
    out_command_allocator->gpu_scratch_suballoc_size = ReadBitsFromBuffer(data, sizeof(data), 351, 296);

    return kRmtOk;
}

// parse a misc internal resource.
static RmtErrorCode ParseResourceDescriptionPayloadMiscInternal(RmtParser* rmt_parser, RmtResourceDescriptionMiscInternal* out_misc_internal)
{
    uint8_t            data[MISC_INTERNAL_RESOURCE_TOKEN_SIZE];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, RMT_TOKEN_SIZE_RESOURCE_CREATE, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    out_misc_internal->type = (RmtResourceMiscInternalType)data[0];
    return kRmtOk;
}

// parse a resource description
static RmtErrorCode ParseResourceCreate(RmtParser* rmt_parser, const uint16_t token_header, RmtTokenResourceCreate* out_resource_description)
{
    RMT_UNUSED(token_header);

    // common fields.
    PopulateCommonFields(rmt_parser, &out_resource_description->common);

    uint8_t      data[RMT_TOKEN_SIZE_RESOURCE_CREATE];
    RmtErrorCode error_code = ReadBytes(rmt_parser, data, 0, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    out_resource_description->resource_identifier = ReadBitsFromBuffer(data, sizeof(data), 39, 8);
    out_resource_description->owner_type          = (RmtOwnerType)ReadBitsFromBuffer(data, sizeof(data), 41, 40);
    //outResourceDescription->ownerCategoryType = readBitsFromBuffer(data, sizeof(data), 45, 42);
    out_resource_description->commit_type   = (RmtCommitType)ReadBitsFromBuffer(data, sizeof(data), 47, 46);
    out_resource_description->resource_type = (RmtResourceType)ReadBitsFromBuffer(data, sizeof(data), 53, 48);

    // Parse per-type data.
    switch (out_resource_description->resource_type)
    {
    case kRmtResourceTypeImage:
        // Image format changed at V1.6.
        if (FileVersionGreaterOrEqual(rmt_parser, 1, 6))
        {
            error_code = ParseResourceDescriptionPayloadImageV1_6(rmt_parser, &out_resource_description->image);
        }
        else
        {
            error_code = ParseResourceDescriptionPayloadImage(rmt_parser, &out_resource_description->image);
        }
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;
    case kRmtResourceTypeBuffer:
        error_code = ParseResourceDescriptionPayloadBuffer(rmt_parser, &out_resource_description->buffer);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;
    case kRmtResourceTypeGpuEvent:
        error_code = ParseResourceDescriptionPayloadGpuEvent(rmt_parser, &out_resource_description->gpu_event);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;
    case kRmtResourceTypeBorderColorPalette:
        error_code = ParseResourceDescriptionPayloadBorderColorPalette(rmt_parser, &out_resource_description->border_color_palette);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;
    case kRmtResourceTypePerfExperiment:
        error_code = ParseResourceDescriptionPayloadPerfExperiment(rmt_parser, &out_resource_description->perf_experiment);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;
    case kRmtResourceTypeQueryHeap:
        error_code = ParseResourceDescriptionPayloadQueryHeap(rmt_parser, &out_resource_description->query_heap);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;
    case kRmtResourceTypeVideoDecoder:
        error_code = ParseResourceDescriptionPayloadVideoDecoder(rmt_parser, &out_resource_description->video_decoder);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;
    case kRmtResourceTypeVideoEncoder:
        error_code = ParseResourceDescriptionPayloadVideoEncoder(rmt_parser, &out_resource_description->video_encoder);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;
    case kRmtResourceTypeHeap:
        error_code = ParseResourceDescriptionPayloadHeap(rmt_parser, &out_resource_description->heap);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;
    case kRmtResourceTypePipeline:
        error_code = ParseResourceDescriptionPayloadPipeline(rmt_parser, &out_resource_description->pipeline);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;
    case kRmtResourceTypeDescriptorHeap:
        error_code = ParseResourceDescriptionPayloadDescriptorHeap(rmt_parser, &out_resource_description->descriptor_heap);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;
    case kRmtResourceTypeDescriptorPool:
        error_code = ParseResourceDescriptionPayloadDescriptorPool(rmt_parser, &out_resource_description->descriptor_pool);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;
    case kRmtResourceTypeCommandAllocator:
        error_code = ParseResourceDescriptionPayloadCmdAllocator(rmt_parser, &out_resource_description->command_allocator);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;
    case kRmtResourceTypeMiscInternal:
        error_code = ParseResourceDescriptionPayloadMiscInternal(rmt_parser, &out_resource_description->misc_internal);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        break;

    default:
        break;
    }

    return kRmtOk;
}

// parse a time delta
static RmtErrorCode ParseTimeDelta(RmtParser* rmt_parser, const uint16_t token_header, RmtTokenTimeDelta* out_time_delta)
{
    RMT_UNUSED(token_header);

    PopulateCommonFields(rmt_parser, &out_time_delta->common);

    // token-specific fields.
    uint8_t      num_delta_bytes = 0;
    RmtErrorCode error_code      = ReadUInt8(rmt_parser, &num_delta_bytes, 0);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    num_delta_bytes = (num_delta_bytes >> 4) & 7;

    uint64_t delta = 0;
    error_code     = ReadUInt64(rmt_parser, &delta, 1);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    out_time_delta->delta = ((delta >> (8 - num_delta_bytes)) * TIMESTAMP_QUANTA) / 1000000;
    return kRmtOk;
}

// parse a resource destroy
static RmtErrorCode ParseResourceDestroy(RmtParser* rmt_parser, const uint16_t token_header, RmtTokenResourceDestroy* out_resource_destroy)
{
    RMT_UNUSED(token_header);

    PopulateCommonFields(rmt_parser, &out_resource_destroy->common);

    uint8_t            data[kRmtTokenTypeResourceDestroy];
    const RmtErrorCode error_code = ReadBytes(rmt_parser, data, 0, sizeof(data));
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    out_resource_destroy->resource_identifier = ReadBitsFromBuffer(data, sizeof(data), 39, 8);
    return kRmtOk;
}

RmtErrorCode RmtParserInitialize(RmtParser* rmt_parser,
                                 FILE*      file_handle,
                                 int32_t    file_offset,
                                 int32_t    stream_size,
                                 void*      file_buffer,
                                 int32_t    file_buffer_size,
                                 int32_t    major_version,
                                 int32_t    minor_version,
                                 int32_t    stream_index,
                                 uint64_t   process_id,
                                 uint64_t   thread_id)
{
    RMT_RETURN_ON_ERROR(rmt_parser, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(file_handle, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR((stream_size >= 1), kRmtErrorInvalidSize);

    rmt_parser->start_timestamp         = 0;
    rmt_parser->current_timestamp       = 0;
    rmt_parser->seen_timestamp          = 0;
    rmt_parser->file_handle             = file_handle;
    rmt_parser->stream_current_offset   = 0;
    rmt_parser->stream_start_offset     = file_offset;
    rmt_parser->stream_size             = stream_size;
    rmt_parser->file_buffer_offset      = 0;
    rmt_parser->file_buffer             = file_buffer;
    rmt_parser->file_buffer_size        = file_buffer_size;
    rmt_parser->file_buffer_actual_size = 0;
    rmt_parser->major_version           = major_version;
    rmt_parser->minor_version           = minor_version;
    rmt_parser->process_id              = process_id;
    rmt_parser->thread_id               = thread_id;
    rmt_parser->stream_index            = stream_index;
    return kRmtOk;
}

RmtErrorCode RmtParserAdvance(RmtParser* rmt_parser, RmtToken* out_token, RmtParserPosition* out_parser_position)
{
    RMT_RETURN_ON_ERROR(rmt_parser, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_token, kRmtErrorInvalidPointer);

    if (out_parser_position != nullptr)
    {
        out_parser_position->seen_timestamp          = rmt_parser->seen_timestamp;
        out_parser_position->timestamp               = rmt_parser->current_timestamp;
        out_parser_position->stream_start_offset     = rmt_parser->stream_start_offset;
        out_parser_position->stream_current_offset   = rmt_parser->stream_current_offset;
        out_parser_position->file_buffer_actual_size = rmt_parser->file_buffer_actual_size;
        out_parser_position->file_buffer_offset      = rmt_parser->file_buffer_offset;
    }

    // If we have less than 64 bytes in the buffer, fetch some more data..
    if (rmt_parser->file_buffer_offset >= (rmt_parser->file_buffer_actual_size - 64))
    {
        if (rmt_parser->file_buffer_actual_size == 0 || (rmt_parser->file_buffer_actual_size == rmt_parser->file_buffer_size))
        {
            fseek(rmt_parser->file_handle, rmt_parser->stream_start_offset + rmt_parser->stream_current_offset, SEEK_SET);
            const int32_t read_bytes = (int32_t)fread(rmt_parser->file_buffer, 1, rmt_parser->file_buffer_size, rmt_parser->file_handle);
            //printf("Read %d bytes from file [%d..%d]\n", read_bytes, rmt_parser->stream_current_offset + rmt_parser->stream_start_offset, rmt_parser->stream_current_offset + rmt_parser->stream_start_offset + rmt_parser->file_buffer_size);
            rmt_parser->file_buffer_actual_size = read_bytes;
            rmt_parser->file_buffer_offset      = 0;
        }
    }

    // Figure out what the token is we have to parse.
    uint16_t     token_header = 0;
    RmtErrorCode error_code   = ReadUInt16(rmt_parser, &token_header, 0);
    if (error_code != kRmtOk)
    {
        return error_code;
    }

    UpdateTimeState(rmt_parser, token_header);

    // token type encoded in [3:0]
    const RmtTokenType token_type = (RmtTokenType)(token_header & 0xf);
    out_token->type               = token_type;

    switch (token_type)
    {
    case kRmtTokenTypeTimestamp:
        error_code = ParseTimestamp(rmt_parser, token_header, &out_token->timestamp_token);
        break;
    case kRmtTokenTypePageTableUpdate:
        error_code = ParsePageTableUpdate(rmt_parser, token_header, &out_token->page_table_update_token);
        break;
    case kRmtTokenTypeUserdata:
        error_code = ParseUserdata(rmt_parser, token_header, &out_token->userdata_token);
        break;
    case kRmtTokenTypeMisc:
        error_code = ParseMisc(rmt_parser, token_header, &out_token->misc_token);
        break;
    case kRmtTokenTypeResourceReference:
        error_code = ParserResourceReference(rmt_parser, token_header, &out_token->resource_reference);
        break;
    case kRmtTokenTypeResourceBind:
        error_code = ParseResourceBind(rmt_parser, token_header, &out_token->resource_bind_token);
        break;
    case kRmtTokenTypeProcessEvent:
        error_code = ParseProcessEvent(rmt_parser, token_header, &out_token->process_event_token);
        break;
    case kRmtTokenTypePageReference:
        error_code = ParsePageReference(rmt_parser, token_header, &out_token->page_reference_token);
        break;
    case kRmtTokenTypeCpuMap:
        error_code = ParseCpuMap(rmt_parser, token_header, &out_token->cpu_map_token);
        break;
    case kRmtTokenTypeVirtualFree:
        error_code = ParseVirtualFree(rmt_parser, token_header, &out_token->virtual_free_token);
        break;
    case kRmtTokenTypeVirtualAllocate:
        error_code = ParseVirtualAllocate(rmt_parser, token_header, &out_token->virtual_allocate_token);
        break;
    case kRmtTokenTypeResourceCreate:
        error_code = ParseResourceCreate(rmt_parser, token_header, &out_token->resource_create_token);
        break;
    case kRmtTokenTypeTimeDelta:
        error_code = ParseTimeDelta(rmt_parser, token_header, &out_token->time_delta_token);
        break;
    case kRmtTokenTypeResourceDestroy:
        error_code = ParseResourceDestroy(rmt_parser, token_header, &out_token->resource_destroy_token);
        break;
    default:
        RMT_ASSERT(0);
        error_code = kRmtErrorMalformedData;  // corrupted file.
        break;
    }

    // if there was an error during the parsing of the packet, then return it.
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // advance the stream by the size of the token.
    const int32_t token_size = GetTokenSize(rmt_parser, token_header);
    rmt_parser->stream_current_offset += token_size;
    rmt_parser->file_buffer_offset += token_size;

    return kRmtOk;
}

RmtErrorCode RmtParserSetPosition(RmtParser* rmt_parser, const RmtParserPosition* parser_position)
{
    RMT_RETURN_ON_ERROR(rmt_parser, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(parser_position, kRmtErrorInvalidPointer);

    // set the position up
    rmt_parser->stream_current_offset   = parser_position->stream_current_offset;
    rmt_parser->current_timestamp       = parser_position->timestamp;
    rmt_parser->stream_start_offset     = parser_position->stream_start_offset;
    rmt_parser->seen_timestamp          = parser_position->seen_timestamp;
    rmt_parser->file_buffer_actual_size = parser_position->file_buffer_actual_size;
    rmt_parser->file_buffer_offset      = parser_position->file_buffer_offset;

    return kRmtOk;
}

bool RmtParserIsCompleted(RmtParser* rmt_parser)
{
    RMT_RETURN_ON_ERROR(rmt_parser, false);

    RmtParserPosition  parser_pos;
    RmtToken           token;
    const RmtErrorCode error_code = RmtParserAdvance(rmt_parser, &token, &parser_pos);
    RmtParserSetPosition(rmt_parser, &parser_pos);

    return error_code != kRmtOk;
}

RmtErrorCode RmtParserReset(RmtParser* rmt_parser)
{
    RMT_RETURN_ON_ERROR(rmt_parser, kRmtErrorInvalidPointer);

    // state related values
    rmt_parser->start_timestamp       = 0;
    rmt_parser->stream_current_offset = 0;
    rmt_parser->seen_timestamp        = 0;

    // initialize time-related values.
    rmt_parser->current_timestamp = 0;

    // make sure we re-read the data from the start.
    rmt_parser->file_buffer_actual_size = 0;
    rmt_parser->file_buffer_offset      = 0;

    return kRmtOk;
}
