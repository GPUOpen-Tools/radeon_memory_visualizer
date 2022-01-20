//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of functions for working with a data set.
//=============================================================================

#include <map>
#include <set>
#include <stdlib.h>  // for malloc() / free()
#include <string>
#include <string.h>  // for memcpy()
#include <time.h>

#include "rmt_data_set.h"
#include "rmt_data_timeline.h"
#include <rmt_util.h>
#include <rmt_assert.h>
#include "rmt_linear_buffer.h"
#include "rmt_data_snapshot.h"
#include "rmt_resource_history.h"
#include <rmt_file_format.h>
#include <rmt_print.h>
#include <rmt_address_helper.h>

#ifndef _WIN32
#include "linux/safe_crt.h"
#include <fstream>
#include <stddef.h>  // for offsetof macro.
#include <unistd.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// Determine if a file is read only.
static bool IsFileReadOnly(const char* file_path)
{
#ifdef _WIN32
    DWORD file_attributes = GetFileAttributes(file_path);
    return ((file_attributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY);
#else
    // Test to see if the file is writable.
    // The access() function will return 0 if successful, -1 on failure.
    return access(file_path, W_OK) == -1;
#endif
}

// Portable copy file function.
static bool CopyTraceFile(const char* existing_file_path, const char* new_file_path)
{
#ifdef _WIN32
    return CopyFile(existing_file_path, new_file_path, FALSE);
#else
    std::ifstream source_stream(existing_file_path, std::ios::binary);
    std::ofstream destination_stream(new_file_path, std::ios::binary);

    destination_stream << source_stream.rdbuf();
    bool result = destination_stream.good();
    return result;
#endif
}

// Portable move file function.
static bool MoveTraceFile(const char* existing_file_path, const char* new_file_path)
{
#ifdef _WIN32
    return MoveFileEx(existing_file_path, new_file_path, MOVEFILE_REPLACE_EXISTING);
#else
    bool result = false;
    if (CopyTraceFile(existing_file_path, new_file_path))
    {
        result = remove(existing_file_path);
    }

    return result;
#endif
}

static void CopyString(char* destination, size_t size_in_bytes, char const* source, size_t max_count)
{
#ifndef _WIN32
    strncpy(destination, source, std::min(size_in_bytes, max_count));
#else
    strncpy_s(destination, size_in_bytes, source, max_count);
#endif  // _WIN32
}

struct RmtResourceNameData
{
    RmtResourceIdentifier resource_id;
    std::string           name;
};

// Map used to associate correlation IDs with Resource IDs.
static std::map<RmtCorrelationIdentifier, RmtResourceIdentifier> resource_correlation_lookup_map;

// Map used to look up resource name data.
static std::map<RmtCorrelationIdentifier, RmtResourceNameData> resource_name_lookup_map;

// Map used to lookup unique resource ID hash using the original resource ID as the key.
static std::map<RmtResourceIdentifier, RmtResourceIdentifier> unique_resource_id_lookup_map;

// A list of implicit resources to be removed from a snapshot.
static std::set<RmtResourceIdentifier> implicit_resource_list;

// A flag used to indicate that the list of implicit resources has been created.
// This list is created when the trace is first parsed and the timeline is built.
// The list is used when parsing the trace for building snapshots.
static bool implicit_resource_list_created = false;

// Replaces original resource IDs in lookup maps with the unique resource ID hash values generated when resource objects are created.
void UpdateResourceNameIds()
{
    for (auto& resource_name_data : resource_name_lookup_map)
    {
        RmtCorrelationIdentifier correlation_identifier               = resource_name_data.first;
        auto                     resource_correlation_lookup_iterator = resource_correlation_lookup_map.find(correlation_identifier);
        if (resource_correlation_lookup_iterator != resource_correlation_lookup_map.end())
        {
            // Take the unique resource IDs from the correlation USERDATA tokens and copy them to the resource name data.
            resource_name_data.second.resource_id = resource_correlation_lookup_iterator->second;
        }

        // Update original resource IDs to unique resource IDs.
        auto unique_resource_id_iterator = unique_resource_id_lookup_map.find(resource_name_data.second.resource_id);
        if (unique_resource_id_iterator != unique_resource_id_lookup_map.end())
        {
            if (resource_correlation_lookup_iterator != resource_correlation_lookup_map.end())
            {
                resource_correlation_lookup_iterator->second = unique_resource_id_iterator->second;
            }
            resource_name_data.second.resource_id = unique_resource_id_iterator->second;
        }
    }
}

// this buffer is used by the parsers to read chunks of data into RAM for processing. The larger this
// buffer the better the parsing performance, but the larger the memory footprint.
//
// NOTE: If we new the total stream count ahead of time, we could divide this more intelligently.
// From very quick tests you probably don't want to go less than 128KB per stream.
static uint8_t s_file_read_buffer[16 * 1024 * 1024];

// create a stream for the RMT chunk
static RmtErrorCode ParseRmtDataChunk(RmtDataSet* data_set, RmtFileChunkHeader* file_chunk)
{
    RMT_ASSERT(data_set);
    RMT_ASSERT(file_chunk);

    // read the RmtFileChunkRmtData from the file.
    RmtFileChunkRmtData data_chunk;
    const size_t        read_size = fread(&data_chunk, 1, sizeof(RmtFileChunkRmtData), (FILE*)data_set->file_handle);
    RMT_ASSERT(read_size == sizeof(RmtFileChunkRmtData));
    RMT_RETURN_ON_ERROR(read_size == sizeof(RmtFileChunkRmtData), kRmtErrorMalformedData);

    const int32_t offset = ftell((FILE*)data_set->file_handle);
    const int32_t size   = file_chunk->size_in_bytes - (sizeof(RmtFileChunkRmtData) + sizeof(RmtFileChunkHeader));

    // ignore 0 sized chunks.
    if (size == 0)
    {
        return kRmtOk;
    }

    // create an RMT parser for this stream with a file handle and offset.
    RmtParser*         parser     = &data_set->streams[data_set->stream_count];
    const RmtErrorCode error_code = RmtParserInitialize(parser,
                                                        (FILE*)data_set->file_handle,
                                                        offset,
                                                        size,
                                                        s_file_read_buffer + (data_set->stream_count * (sizeof(s_file_read_buffer) / RMT_MAXIMUM_STREAMS)),
                                                        (sizeof(s_file_read_buffer) / RMT_MAXIMUM_STREAMS),
                                                        file_chunk->version_major,
                                                        file_chunk->version_minor,
                                                        data_set->stream_count,
                                                        data_chunk.process_id,
                                                        data_chunk.thread_id);

    // set the target process.
    if (data_chunk.process_id != 0 && data_set->target_process_id == 0)
    {
        data_set->target_process_id = data_chunk.process_id;
    }

    RMT_UNUSED(error_code);
    // read for next allocation.
    data_set->stream_count++;

    return kRmtOk;
}

// handle setting up segment info chunks.
static RmtErrorCode ParseSegmentInfoChunk(RmtDataSet* data_set, RmtFileChunkHeader* current_file_chunk)
{
    RMT_UNUSED(current_file_chunk);

    RMT_ASSERT((data_set->segment_info_count + 1) < RMT_MAXIMUM_SEGMENTS);
    RMT_RETURN_ON_ERROR((data_set->segment_info_count + 1) < RMT_MAXIMUM_SEGMENTS, kRmtErrorInvalidSize);

    // Read the RmtSegmentInfo from the file.
    RmtFileChunkSegmentInfo segment_info_chunk;
    const size_t            read_size = fread(&segment_info_chunk, 1, sizeof(RmtFileChunkSegmentInfo), (FILE*)data_set->file_handle);
    RMT_ASSERT(read_size == sizeof(RmtFileChunkSegmentInfo));
    RMT_RETURN_ON_ERROR(read_size == sizeof(RmtFileChunkSegmentInfo), kRmtErrorMalformedData);

    // Fill out the segment info.
    RmtSegmentInfo* next_segment_info = &data_set->segment_info[data_set->segment_info_count++];
    next_segment_info->base_address   = segment_info_chunk.base_address;
    next_segment_info->size           = segment_info_chunk.size_in_bytes;
    next_segment_info->heap_type      = (RmtHeapType)segment_info_chunk.heap_type;
    next_segment_info->index          = segment_info_chunk.memory_index;
    return kRmtOk;
}

// handle setting up process start info.
static RmtErrorCode ParseProcessStartInfo(RmtDataSet* data_set, RmtFileChunkHeader* current_file_chunk)
{
    RMT_UNUSED(current_file_chunk);

    RMT_ASSERT((data_set->process_start_info_count + 1) < RMT_MAXIMUM_PROCESS_COUNT);
    RMT_RETURN_ON_ERROR((data_set->process_start_info_count + 1) < RMT_MAXIMUM_PROCESS_COUNT, kRmtErrorInvalidSize);

    RmtProcessStartInfo* next_process_start_info       = &data_set->process_start_info[data_set->process_start_info_count++];
    next_process_start_info->process_id                = 0;
    next_process_start_info->physical_memory_allocated = 0;
    return kRmtOk;
}

// handle reading in any static adapter info.
static RmtErrorCode ParseAdapterInfoChunk(RmtDataSet* data_set, RmtFileChunkHeader* current_file_chunk)
{
    RMT_UNUSED(current_file_chunk);

    RmtFileChunkAdapterInfo adapter_info_chunk;
    const size_t            read_size = fread(&adapter_info_chunk, 1, sizeof(RmtFileChunkAdapterInfo), (FILE*)data_set->file_handle);
    RMT_ASSERT(read_size == sizeof(RmtFileChunkAdapterInfo));
    RMT_RETURN_ON_ERROR(read_size == sizeof(RmtFileChunkAdapterInfo), kRmtErrorMalformedData);

    // these should always match.
    RMT_STATIC_ASSERT(sizeof(adapter_info_chunk.name) == sizeof(data_set->adapter_info.name));

    // fill out adapter info.
    memcpy(data_set->adapter_info.name, adapter_info_chunk.name, RMT_MAX_ADAPTER_NAME_LENGTH);
    data_set->adapter_info.pcie_family_id              = adapter_info_chunk.pcie_family_id;
    data_set->adapter_info.pcie_revision_id            = adapter_info_chunk.pcie_revision_id;
    data_set->adapter_info.device_id                   = adapter_info_chunk.device_id;
    data_set->adapter_info.minimum_engine_clock        = adapter_info_chunk.minimum_engine_clock;
    data_set->adapter_info.maximum_engine_clock        = adapter_info_chunk.maximum_engine_clock;
    data_set->adapter_info.memory_type                 = (RmtAdapterInfoMemoryType)adapter_info_chunk.memory_type;
    data_set->adapter_info.memory_operations_per_clock = adapter_info_chunk.memory_operations_per_clock;
    data_set->adapter_info.memory_bus_width            = adapter_info_chunk.memory_bus_width;
    data_set->adapter_info.memory_bandwidth            = adapter_info_chunk.memory_bandwidth;
    data_set->adapter_info.minimum_memory_clock        = adapter_info_chunk.minimum_memory_clock;
    data_set->adapter_info.maximum_memory_clock        = adapter_info_chunk.maximum_memory_clock;
    return kRmtOk;
}

// handle reading a snapshot.
static RmtErrorCode ParseSnapshotInfoChunk(RmtDataSet* data_set, RmtFileChunkHeader* current_file_chunk)
{
    RMT_UNUSED(current_file_chunk);

    RmtFileChunkSnapshotInfo snapshot_info_chunk;

    const size_t file_offset = ftell((FILE*)data_set->file_handle);
    size_t       read_size   = fread(&snapshot_info_chunk, 1, sizeof(RmtFileChunkSnapshotInfo), (FILE*)data_set->file_handle);
    RMT_ASSERT(read_size == sizeof(RmtFileChunkSnapshotInfo));
    RMT_RETURN_ON_ERROR(read_size == sizeof(RmtFileChunkSnapshotInfo), kRmtErrorMalformedData);

    // Allocate some buffer in the snapshot names.
    const int32_t snapshot_index = data_set->snapshot_count;
    if (snapshot_index >= RMT_MAXIMUM_SNAPSHOT_POINTS)
    {
        return kRmtErrorOutOfMemory;
    }

    // ignore snapshots of 0-length name, these are deleted snapshots.
    if (snapshot_info_chunk.name_length_in_bytes == 0)
    {
        return kRmtOk;
    }

    // read the name into the snapshot point.
    const size_t capped_name_length = RMT_MINIMUM(RMT_MAXIMUM_NAME_LENGTH, snapshot_info_chunk.name_length_in_bytes);
    void*        name_buffer        = (void*)&data_set->snapshots[snapshot_index].name;
    read_size                       = fread(name_buffer, 1, capped_name_length, (FILE*)data_set->file_handle);
    RMT_ASSERT(read_size == capped_name_length);
    RMT_RETURN_ON_ERROR(read_size == capped_name_length, kRmtErrorMalformedData);

    // set the time.
    data_set->snapshots[snapshot_index].timestamp   = snapshot_info_chunk.snapshot_time;
    data_set->snapshots[snapshot_index].file_offset = file_offset;
    data_set->snapshot_count++;
    return kRmtOk;
}

// helper function to parse the chunks of the RMT file into the data set.
static RmtErrorCode ParseChunks(RmtDataSet* data_set)
{
    data_set->stream_count             = 0;
    data_set->segment_info_count       = 0;
    data_set->process_start_info_count = 0;

    RmtFileParser rmt_file_parser;
    RmtErrorCode  error_code = RmtFileParserCreateFromHandle(&rmt_file_parser, (FILE*)data_set->file_handle);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // Check if file is supported.
    error_code = RmtFileParserIsFileSupported(&rmt_file_parser.header);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // Store the time the trace was taken.
    struct tm      create_time;
    RmtFileHeader* header = &rmt_file_parser.header;
    create_time.tm_hour   = header->hour;
    create_time.tm_min    = header->minute;
    create_time.tm_sec    = header->second;
    create_time.tm_wday   = header->day_in_week;
    create_time.tm_mday   = header->day_in_month;
    create_time.tm_mon    = header->month;
    create_time.tm_year   = header->year;
    create_time.tm_yday   = header->day_in_year;
    create_time.tm_isdst  = header->is_daylight_savings;
    data_set->create_time = mktime(&create_time);

    // Process all the chunks in the rmt file.
    RmtFileChunkHeader* current_file_chunk = NULL;
    while (RmtFileParserParseNextChunk(&rmt_file_parser, &current_file_chunk) == kRmtOk)
    {
        // Ensure that the chunk is valid to read.
        RMT_ASSERT(current_file_chunk);
        if (current_file_chunk == nullptr)
        {
            break;
        }

        if ((size_t)rmt_file_parser.next_chunk_offset > data_set->file_size_in_bytes)
        {
            return kRmtErrorMalformedData;
        }

        if (((size_t)current_file_chunk->size_in_bytes < sizeof(RmtFileChunkHeader)) ||
            ((size_t)current_file_chunk->size_in_bytes > data_set->file_size_in_bytes))
        {
            return kRmtErrorMalformedData;
        }

        // Depending on the type of chunk, handle pre-processing it.
        switch (current_file_chunk->chunk_identifier.chunk_type)
        {
        case kRmtFileChunkTypeAsicInfo:
            break;

        case kRmtFileChunkTypeApiInfo:
            break;

        case kRmtFileChunkTypeSystemInfo:
            break;

        case kRmtFileChunkTypeRmtData:
            error_code = ParseRmtDataChunk(data_set, current_file_chunk);
            RMT_ASSERT(error_code == kRmtOk);
            RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
            break;

        case kRmtFileChunkTypeSegmentInfo:
            error_code = ParseSegmentInfoChunk(data_set, current_file_chunk);
            RMT_ASSERT(error_code == kRmtOk);
            RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
            break;

        case kRmtFileChunkTypeProcessStart:
            error_code = ParseProcessStartInfo(data_set, current_file_chunk);
            RMT_ASSERT(error_code == kRmtOk);
            RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
            break;

        case kRmtFileChunkTypeAdapterInfo:
            error_code = ParseAdapterInfoChunk(data_set, current_file_chunk);
            RMT_ASSERT(error_code == kRmtOk);
            RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
            break;

        case kRmtFileChunkTypeSnapshotInfo:
            error_code = ParseSnapshotInfoChunk(data_set, current_file_chunk);
            RMT_ASSERT(error_code == kRmtOk);
            RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
            break;

        default:
            break;
        }
    }

    // Initialize the token heap for k-way merging.
    error_code = RmtStreamMergerInitialize(&data_set->stream_merger, data_set->streams, data_set->stream_count);
    RMT_ASSERT(error_code == kRmtOk);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // Rebase any snapshot times to be relative to the minimum timestamp.
    for (int32_t current_snapshot_index = 0; current_snapshot_index < data_set->snapshot_count; ++current_snapshot_index)
    {
        data_set->snapshots[current_snapshot_index].timestamp -= data_set->stream_merger.minimum_start_timestamp;
    }

    return kRmtOk;
}

static void BuildDataProfileParseUserdata(RmtDataSet* data_set, const RmtToken* current_token)
{
    RMT_ASSERT(current_token->type == kRmtTokenTypeUserdata);

    const RmtTokenUserdata* userdata = (const RmtTokenUserdata*)&current_token->userdata_token;

    if (userdata->userdata_type != kRmtUserdataTypeSnapshot)
    {
        return;  // we only care about snapshots.
    }

    data_set->data_profile.snapshot_count++;
    data_set->data_profile.snapshot_name_count += userdata->size_in_bytes + 1;  // +1 for /0
}

static void BuildDataProfileParseProcessEvent(RmtDataSet* data_set, const RmtToken* current_token)
{
    RMT_ASSERT(current_token->type == kRmtTokenTypeProcessEvent);

    const RmtTokenProcessEvent* process_event = (const RmtTokenProcessEvent*)&current_token->process_event_token;

    if (process_event->event_type != kRmtProcessEventTypeStart)
    {
        return;  // we only care about process start.
    }

    // Add to the process map.
    RmtProcessMapAddProcess(&data_set->process_map, process_event->common.process_id);
    data_set->data_profile.process_count++;
}

static void BuildDataProfileParseVirtualFree(RmtDataSet* data_set, const RmtToken* current_token)
{
    RMT_ASSERT(current_token->type == kRmtTokenTypeVirtualFree);

    data_set->data_profile.current_virtual_allocation_count--;
}

static void BuildDataProfileParseVirtualAllocate(RmtDataSet* data_set, const RmtToken* current_token)
{
    RMT_ASSERT(current_token->type == kRmtTokenTypeVirtualAllocate);

    data_set->data_profile.current_virtual_allocation_count++;
    data_set->data_profile.total_virtual_allocation_count++;
    data_set->data_profile.max_virtual_allocation_count =
        RMT_MAXIMUM(data_set->data_profile.max_virtual_allocation_count, data_set->data_profile.current_virtual_allocation_count);
}

static void BuildDataProfileParseResourceCreate(RmtDataSet* data_set, const RmtToken* current_token)
{
    RMT_ASSERT(current_token->type == kRmtTokenTypeResourceCreate);

    data_set->data_profile.current_resource_count++;
    data_set->data_profile.total_resource_count++;
    data_set->data_profile.max_concurrent_resources =
        RMT_MAXIMUM(data_set->data_profile.max_concurrent_resources, data_set->data_profile.current_resource_count);

    // Add one to the allocation count if the resource being created is a shareable image, since we might need to create a dummy allocation token
    // if we don't see one in the token stream.
    if ((current_token->resource_create_token.resource_type == kRmtResourceTypeImage) &&
        ((current_token->resource_create_token.image.create_flags & kRmtImageCreationFlagShareable) == kRmtImageCreationFlagShareable))
    {
        data_set->data_profile.current_virtual_allocation_count++;
        data_set->data_profile.total_virtual_allocation_count++;
        data_set->data_profile.max_virtual_allocation_count =
            RMT_MAXIMUM(data_set->data_profile.max_virtual_allocation_count, data_set->data_profile.current_virtual_allocation_count);
    }
}

static void BuildDataProfileParseResourceDestroy(RmtDataSet* data_set, const RmtToken* current_token)
{
    RMT_ASSERT(current_token->type == kRmtTokenTypeResourceDestroy);

    data_set->data_profile.current_resource_count--;
}

// Build a data profile which can be used by all subsequent parsing.
static RmtErrorCode BuildDataProfile(RmtDataSet* data_set)
{
    // get the stream count from the loader, and initialize all the counters
    data_set->data_profile.stream_count        = data_set->stream_count;
    data_set->data_profile.process_count       = data_set->process_start_info_count;
    data_set->data_profile.snapshot_count      = 0;
    data_set->data_profile.snapshot_name_count = 0;
    data_set->maximum_timestamp                = 0;

    // Push processes into the process map.
    for (int32_t current_process_start_index = 0; current_process_start_index < data_set->process_start_info_count; ++current_process_start_index)
    {
        RmtProcessMapAddProcess(&data_set->process_map, data_set->process_start_info[current_process_start_index].process_id);
    }

    // if the heap has something there, then add it.
    while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
    {
        // grab the next token from the heap.
        RmtToken           current_token;
        const RmtErrorCode error_code = RmtStreamMergerAdvance(&data_set->stream_merger, &current_token);
        RMT_ASSERT(error_code == kRmtOk);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

        data_set->maximum_timestamp = RMT_MAXIMUM(data_set->maximum_timestamp, current_token.common.timestamp);

        // process the token.
        switch (current_token.type)
        {
        case kRmtTokenTypeUserdata:
            BuildDataProfileParseUserdata(data_set, &current_token);
            break;

        case kRmtTokenTypeProcessEvent:
            BuildDataProfileParseProcessEvent(data_set, &current_token);
            break;

        case kRmtTokenTypeVirtualFree:
            BuildDataProfileParseVirtualFree(data_set, &current_token);
            break;

        case kRmtTokenTypeVirtualAllocate:
            BuildDataProfileParseVirtualAllocate(data_set, &current_token);
            break;

        case kRmtTokenTypeResourceCreate:
            BuildDataProfileParseResourceCreate(data_set, &current_token);
            break;

        case kRmtTokenTypeResourceDestroy:
            BuildDataProfileParseResourceDestroy(data_set, &current_token);
            break;

        default:
            break;
        }
    }

    data_set->cpu_frequency = data_set->streams[0].cpu_frequency;

    // Create an allocator for the token heap to use for generating unique resource IDs
    size_t size_required = (data_set->data_profile.total_resource_count * sizeof(ResourceIdMapNode)) + sizeof(ResourceIdMapAllocator);

    void* data = calloc(size_required, 1);
    RMT_ASSERT(data != nullptr);
    data_set->p_resource_id_map_allocator                  = static_cast<ResourceIdMapAllocator*>(data);
    data_set->p_resource_id_map_allocator->allocation_base = (void*)(static_cast<const uint8_t*>(data) + sizeof(ResourceIdMapAllocator));
    data_set->p_resource_id_map_allocator->allocation_size = size_required - sizeof(ResourceIdMapAllocator);
    data_set->p_resource_id_map_allocator->resource_count  = 0;
    data_set->stream_merger.allocator                      = data_set->p_resource_id_map_allocator;
    return kRmtOk;
}

// Helper function call the correct allocation function.
static void* PerformAllocation(RmtDataSet* data_set, size_t size_in_bytes, size_t alignment)
{
    if (data_set->allocate_func == nullptr)
    {
        return malloc(size_in_bytes);
    }

    return (data_set->allocate_func)(size_in_bytes, alignment);
}

// Helper functo call the correct free function.
static void PerformFree(RmtDataSet* data_set, void* pointer)
{
    if (data_set->free_func == nullptr)
    {
        return free(pointer);
    }

    return (data_set->free_func)(pointer);
}

// Allocate memory for a snapshot.
static RmtErrorCode AllocateMemoryForSnapshot(RmtDataSet* data_set, RmtDataSnapshot* out_snapshot)
{
    // Set a pointer to parent data set.
    out_snapshot->data_set = data_set;

    // Initialize the virtual allocation list.
    const size_t virtual_allocation_buffer_size =
        RmtVirtualAllocationListGetBufferSize(data_set->data_profile.total_virtual_allocation_count, data_set->data_profile.max_concurrent_resources + 200);
    if (virtual_allocation_buffer_size > 0)
    {
        out_snapshot->virtual_allocation_buffer = PerformAllocation(data_set, virtual_allocation_buffer_size, sizeof(uint32_t));
        RMT_ASSERT(out_snapshot->virtual_allocation_buffer);
        RMT_RETURN_ON_ERROR(out_snapshot->virtual_allocation_buffer, kRmtErrorOutOfMemory);
        const RmtErrorCode error_code = RmtVirtualAllocationListInitialize(&out_snapshot->virtual_allocation_list,
                                                                           out_snapshot->virtual_allocation_buffer,
                                                                           virtual_allocation_buffer_size,
                                                                           data_set->data_profile.max_virtual_allocation_count,
                                                                           data_set->data_profile.max_concurrent_resources + 200,
                                                                           data_set->data_profile.total_virtual_allocation_count);
        RMT_ASSERT(error_code == kRmtOk);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }

    // create the resource list.
    const size_t resource_list_buffer_size = RmtResourceListGetBufferSize(data_set->data_profile.max_concurrent_resources + 200);
    if (resource_list_buffer_size > 0)
    {
        out_snapshot->resource_list_buffer = PerformAllocation(data_set, resource_list_buffer_size, sizeof(uint32_t));
        RMT_ASSERT(out_snapshot->resource_list_buffer);
        RMT_RETURN_ON_ERROR(out_snapshot->resource_list_buffer, kRmtErrorOutOfMemory);
        const RmtErrorCode error_code = RmtResourceListInitialize(&out_snapshot->resource_list,
                                                                  out_snapshot->resource_list_buffer,
                                                                  resource_list_buffer_size,
                                                                  &out_snapshot->virtual_allocation_list,
                                                                  data_set->data_profile.max_concurrent_resources + 200);
        RMT_ASSERT(error_code == kRmtOk);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }

    // initialize the region stack
    out_snapshot->region_stack_buffer = NULL;
    out_snapshot->region_stack_count  = 0;

    return kRmtOk;
}

// Verify that a string contains only valid characters.
static bool IsValidTextString(const char* text, size_t length)
{
    int character_position = 0;
    while ((character_position < length) && (text[character_position] != '\0'))
    {
        if ((text[character_position] <= 0x1f) || text[character_position] == 0x7f)
        {
            return false;
        }
        character_position++;
    }

    return true;
}

// consume next RMT token for snapshot generation.
static RmtErrorCode ProcessTokenForSnapshot(RmtDataSet* data_set, RmtToken* current_token, RmtDataSnapshot* out_snapshot)
{
    RmtErrorCode error_code = kRmtOk;

    RMT_UNUSED(data_set);

    switch (current_token->type)
    {
    case kRmtTokenTypeVirtualFree:
    {
#ifdef PRINT_TOKENS
        RmtPrint("%lld - VIRTUAL_FREE - 0x%010llx", current_token->common.timestamp, current_token->virtual_free_token.virtual_address);
#endif

        error_code = RmtVirtualAllocationListRemoveAllocation(&out_snapshot->virtual_allocation_list, current_token->virtual_free_token.virtual_address);
        RMT_ASSERT(error_code == kRmtOk);
        //RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }
    break;

    case kRmtTokenTypePageTableUpdate:
    {
#ifdef PRINT_TOKENS
        RmtPrint("%lld - PAGE_TABLE_UPDATE - [0x%010llx..0x%010llx] => [0x%010llx..0x%010llx] (%lld * %lld) %d %s",
                 current_token->common.timestamp,
                 current_token->page_table_update_token.virtual_address,
                 current_token->page_table_update_token.virtual_address +
                     (current_token->page_table_update_token.size_in_pages * RmtGetPageSize(current_token->page_table_update_token.page_size)),
                 current_token->page_table_update_token.physical_address,
                 current_token->page_table_update_token.physical_address +
                     (current_token->page_table_update_token.size_in_pages * RmtGetPageSize(current_token->page_table_update_token.page_size)),
                 current_token->page_table_update_token.size_in_pages,
                 RmtGetPageSize(current_token->page_table_update_token.page_size),
                 current_token->page_table_update_token.update_type,
                 current_token->page_table_update_token.is_unmapping ? "UM" : "M");
#endif

        if (!current_token->page_table_update_token.is_unmapping)
        {
            const uint64_t size_in_bytes =
                current_token->page_table_update_token.size_in_pages * RmtGetPageSize(current_token->page_table_update_token.page_size);
            RmtProcessMapAddCommittedMemoryForProcessId(&out_snapshot->process_map, current_token->common.process_id, size_in_bytes);
        }
        else
        {
            const uint64_t size_in_bytes =
                current_token->page_table_update_token.size_in_pages * RmtGetPageSize(current_token->page_table_update_token.page_size);
            RmtProcessMapRemoveCommittedMemoryForProcessId(&out_snapshot->process_map, current_token->common.process_id, size_in_bytes);
        }

        // Filter is done in the page table such that we only build it for target PID.
        error_code = RmtPageTableUpdateMemoryMappings(&out_snapshot->page_table,
                                                      current_token->page_table_update_token.virtual_address,
                                                      current_token->page_table_update_token.physical_address,
                                                      current_token->page_table_update_token.size_in_pages,
                                                      current_token->page_table_update_token.page_size,
                                                      current_token->page_table_update_token.is_unmapping,
                                                      current_token->page_table_update_token.update_type,
                                                      current_token->common.process_id);
        RMT_ASSERT(error_code == kRmtOk);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }
    break;

    case kRmtTokenTypeUserdata:
    {
        if (current_token->userdata_token.userdata_type == kRmtUserdataTypeName)
        {
            char name[RMT_MAXIMUM_NAME_LENGTH];

            // Calculate the size of the resource name string.  Subtract the size of the ID in the Name USERDATA token and one byte for the termination character to get the name length.
            int  name_length = RMT_MINIMUM(current_token->userdata_token.size_in_bytes - (sizeof(uint32_t)), RMT_MAXIMUM_NAME_LENGTH);

            memcpy(name, current_token->userdata_token.payload_cache, name_length);
            // Check for invalid characters.  Invalid characters may indicate a parsing issue.
            RMT_ASSERT(IsValidTextString(name, name_length));

            name[name_length-1]           = 0;

            // If a matching resource isn't found, attempt to match later using the correlation ID.
            RmtResourceNameData resource_name_data;
            resource_name_data.name                                                        = name;
            resource_name_data.resource_id                                                 = current_token->userdata_token.resource_identifier;
            resource_name_lookup_map[current_token->userdata_token.correlation_identifier] = resource_name_data;

            current_token->userdata_token.payload_cache = nullptr;
        }
        else if (current_token->userdata_token.userdata_type == kRmtUserdataTypeCorrelation)
        {
            // store values in lookup map.
            resource_correlation_lookup_map[current_token->userdata_token.correlation_identifier] = current_token->userdata_token.resource_identifier;
        }
        else if (current_token->userdata_token.userdata_type == kRmtUserdataTypeMarkImplicitResource)
        {
#ifdef _IMPLICIT_RESOURCE_LOGGING
            if (implicit_resource_list_created == false)
            {
                RmtPrint("ProcessTokenForSnapshot() - add implicit resource: %lld", current_token->userdata_token.resource_identifier);
#endif  // _IMPLICIT_RESOURCE_LOGGING
                implicit_resource_list.insert(current_token->userdata_token.resource_identifier);
            }

#ifdef PRINT_TOKENS
#endif
    }
    break;

    case kRmtTokenTypeMisc:
    {
#ifdef PRINT_TOKENS
        RmtPrint("%lld - MISC - %s", current_token->common.timestamp, RmtGetMiscTypeNameFromMiscType(current_token->misc_token.type));
#endif
    }
    break;

    case kRmtTokenTypeResourceReference:
    {
        error_code = RmtVirtualAllocationListAddResourceReference(&out_snapshot->virtual_allocation_list,
                                                                  current_token->common.timestamp,
                                                                  current_token->resource_reference.virtual_address,
                                                                  current_token->resource_reference.residency_update_type,
                                                                  current_token->resource_reference.queue);
        //RMT_ASSERT(error_code == kRmtOk);
        //RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }
    break;

    case kRmtTokenTypeResourceBind:
    {
#ifdef PRINT_TOKENS
        RmtPrint("%lld - RESOURCE_BIND - %lld 0x%010llx %lld",
                 current_token->common.timestamp,
                 current_token->resource_bind_token.resource_identifier,
                 current_token->resource_bind_token.virtual_address,
                 current_token->resource_bind_token.size_in_bytes);
#endif

        error_code = RmtResourceListAddResourceBind(&out_snapshot->resource_list, &current_token->resource_bind_token);

        if (error_code == kRmtErrorSharedAllocationNotFound)
        {
            // This is not a true error, it just means that we encountered a shareable resource without the matching virtual
            // alloc token.  This is an expected case as that allocation is owned outside the target process, so we'll add
            // the allocation to the list so future resource tokens can find it.
            static const RmtHeapType kDummyHeapPref[4] = {kRmtHeapTypeInvisible, kRmtHeapTypeInvisible, kRmtHeapTypeInvisible, kRmtHeapTypeInvisible};
            error_code                                 = RmtVirtualAllocationListAddAllocation(&out_snapshot->virtual_allocation_list,
                                                               current_token->common.timestamp,
                                                               current_token->resource_bind_token.virtual_address,
                                                               (int32_t)(current_token->resource_bind_token.size_in_bytes >> 12),
                                                               kDummyHeapPref,
                                                               RmtOwnerType::kRmtOwnerTypeClientDriver);
        }
        else if (error_code == kRmtErrorResourceAlreadyBound)
        {
            // duplicate the command allocator resource we have at this resource ID this is because command allocators are
            // bound to multiple chunks of virtual address space simultaneously.
            const RmtResource* matching_resource = NULL;
            error_code                           = RmtResourceListGetResourceByResourceId(
                &out_snapshot->resource_list, current_token->resource_bind_token.resource_identifier, &matching_resource);
            if (error_code == kRmtOk)
            {
                // form the token.
                RmtTokenResourceCreate resource_create_token;
                resource_create_token.resource_identifier = matching_resource->identifier;
                resource_create_token.owner_type          = matching_resource->owner_type;
                resource_create_token.commit_type         = matching_resource->commit_type;
                resource_create_token.resource_type = kRmtResourceTypeCommandAllocator;
                memcpy(&resource_create_token.common, &current_token->common, sizeof(RmtTokenCommon));
                memcpy(&resource_create_token.command_allocator, &matching_resource->command_allocator, sizeof(RmtResourceDescriptionCommandAllocator));

                // Create the resource.
                error_code = RmtResourceListAddResourceCreate(&out_snapshot->resource_list, &resource_create_token);
                RMT_ASSERT(error_code == kRmtOk);

                if (!(current_token->resource_bind_token.is_system_memory && current_token->resource_bind_token.virtual_address == 0))
                {
                    error_code = RmtResourceListAddResourceBind(&out_snapshot->resource_list, &current_token->resource_bind_token);
                    RMT_ASSERT(error_code == kRmtOk);
                }
            }
        }

        RMT_ASSERT(error_code == kRmtOk);
        //RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }
    break;

    case kRmtTokenTypeProcessEvent:
    {
        if (current_token->process_event_token.event_type != kRmtProcessEventTypeStart)
        {
            break;  // we only care about process start.
        }

        // Add to the process map.
        RmtProcessMapAddProcess(&data_set->process_map, current_token->common.process_id);
    }
    break;

    case kRmtTokenTypePageReference:
    {
    }
    break;

    case kRmtTokenTypeCpuMap:
    {
#ifdef PRINT_TOKENS
        RmtPrint("%lld - CPU_MAP - 0x%010llx", current_token->common.timestamp, current_token->cpu_map_token.virtual_address);
#endif

        if (current_token->cpu_map_token.is_unmap)
        {
            error_code = RmtVirtualAllocationListAddCpuUnmap(
                &out_snapshot->virtual_allocation_list, current_token->common.timestamp, current_token->cpu_map_token.virtual_address);
        }
        else
        {
            error_code = RmtVirtualAllocationListAddCpuMap(
                &out_snapshot->virtual_allocation_list, current_token->common.timestamp, current_token->cpu_map_token.virtual_address);
        }

        RMT_ASSERT(error_code == kRmtOk);
        //RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }
    break;

    case kRmtTokenTypeVirtualAllocate:
    {
#ifdef PRINT_TOKENS
        RmtPrint("%lld - VIRTUAL_ALLOCATE - 0x%010llx %lld",
                 current_token->common.timestamp,
                 current_token->virtual_allocate_token.virtual_address,
                 current_token->virtual_allocate_token.size_in_bytes);
#endif

        error_code = RmtVirtualAllocationListAddAllocation(&out_snapshot->virtual_allocation_list,
                                                           current_token->common.timestamp,
                                                           current_token->virtual_allocate_token.virtual_address,
                                                           (int32_t)(current_token->virtual_allocate_token.size_in_bytes >> 12),
                                                           current_token->virtual_allocate_token.preference,
                                                           current_token->virtual_allocate_token.owner_type);
        RMT_ASSERT(error_code == kRmtOk);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }
    break;

    case kRmtTokenTypeResourceCreate:
    {
        // For testing purposes, define _DISABLE_IMPLICIT_RESOURCE_FILTERING to prevent implicitly created resource from being removed.
#ifndef _DISABLE_IMPLICIT_RESOURCE_FILTERING
        if ((!implicit_resource_list_created) ||
            implicit_resource_list.find(current_token->resource_create_token.original_resource_identifier) == implicit_resource_list.end())
#endif  // _DISABLE_IMPLICIT_RESOURCE_FILTERING
        {
#ifdef PRINT_TOKENS
            char buf[128];
            sprintf_s(buf,
                      "%lld - RESOURCE_CREATE - %lld %d",
                      current_token->common.timestamp,
                      current_token->resource_create_token.resource_identifier,
                      current_token->resource_create_token.resource_type);
#endif
            error_code = RmtResourceListAddResourceCreate(&out_snapshot->resource_list, &current_token->resource_create_token);

            RMT_ASSERT(error_code == kRmtOk);

            unique_resource_id_lookup_map[current_token->resource_create_token.original_resource_identifier] =
                current_token->resource_create_token.resource_identifier;
        }

#ifdef PRINT_TOKENS
        if (error_code != kRmtOk)
        {
            RmtPrint("%s - FAILED", buf);
        }
        else
        {
            RmtPrint(buf);
        }
#endif
        //RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }
    break;

    case kRmtTokenTypeResourceDestroy:
    {
#ifdef PRINT_TOKENS
        char buf[128];
        sprintf_s(buf,
                  "%lld - RESOURCE_DESTROY - %lld",
                  current_token->resource_destroy_token.common.timestamp,
                  current_token->resource_destroy_token.resource_identifier);
#endif
        error_code = RmtResourceListAddResourceDestroy(&out_snapshot->resource_list, &current_token->resource_destroy_token);
        //RMT_ASSERT(error_code == kRmtOk);

#ifdef PRINT_TOKENS
        if (error_code != kRmtOk)
        {
            RmtPrint("%s - FAILED", buf);
        }
        else
        {
            RmtPrint(buf);
        }
#endif
        //RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }
    break;

    default:
        break;
    }

    return kRmtOk;
}

// helper function that mirrors the .bak file to the original.
static void CommitTemporaryFileEdits(RmtDataSet* data_set, bool remove_temporary)
{
    RMT_ASSERT(data_set);
    if (data_set->read_only)
    {
        return;
    }

    if (data_set->file_handle != nullptr)
    {
        fflush((FILE*)data_set->file_handle);
        fclose((FILE*)data_set->file_handle);
        data_set->file_handle = NULL;
    }

    if (remove_temporary)
    {
        bool success = MoveTraceFile(data_set->temporary_file_path, data_set->file_path);
        RMT_ASSERT(success);
    }
    else
    {
        // for a mirror without remove, we need to recopy the temp
        bool success = MoveTraceFile(data_set->temporary_file_path, data_set->file_path);
        RMT_ASSERT(success);
        success = CopyTraceFile(data_set->file_path, data_set->temporary_file_path);
        RMT_ASSERT(success);

        data_set->file_handle = NULL;
        errno_t error_no      = fopen_s((FILE**)&data_set->file_handle, data_set->temporary_file_path, "rb+");
        RMT_ASSERT(data_set->file_handle);
        RMT_ASSERT(error_no == 0);
    }
}

// initialize the data set by reading the header chunks, and setting up the streams.
RmtErrorCode RmtDataSetInitialize(const char* path, RmtDataSet* data_set)
{
    RMT_ASSERT(path);
    RMT_ASSERT(data_set);
    RMT_RETURN_ON_ERROR(path, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);

    // copy the path
    const size_t path_length = strlen(path);
    memcpy(data_set->file_path, path, RMT_MINIMUM(RMT_MAXIMUM_FILE_PATH, path_length));
    memcpy(data_set->temporary_file_path, path, RMT_MINIMUM(RMT_MAXIMUM_FILE_PATH, path_length));

    data_set->file_handle = NULL;
    data_set->read_only   = false;
    errno_t error_no;

    if (IsFileReadOnly(path))
    {
        data_set->read_only = true;
    }
    else
    {
        // copy the entire input file to a temporary.
        strcat_s(data_set->temporary_file_path, sizeof(data_set->temporary_file_path), ".bak");
        CopyTraceFile(data_set->file_path, data_set->temporary_file_path);

        // Load the data from the RMT file.
        error_no = fopen_s((FILE**)&data_set->file_handle, data_set->temporary_file_path, "rb+");
        if ((data_set->file_handle == nullptr) || error_no != 0)
        {
            data_set->read_only = true;
        }
    }

    if (data_set->read_only)
    {
        // file is read-only so just read the original rmv trace file
        error_no = fopen_s((FILE**)&data_set->file_handle, data_set->file_path, "rb");
        if ((data_set->file_handle == nullptr) || error_no != 0)
        {
            return kRmtErrorFileNotOpen;
        }
    }

    // get the size of the file.
    const size_t current_stream_offset = ftell((FILE*)data_set->file_handle);
    fseek((FILE*)data_set->file_handle, 0L, SEEK_END);
    data_set->file_size_in_bytes = ftell((FILE*)data_set->file_handle);
    fseek((FILE*)data_set->file_handle, (int32_t)current_stream_offset, SEEK_SET);
    if (data_set->file_size_in_bytes == 0U)
    {
        fclose((FILE*)data_set->file_handle);
        data_set->file_handle = NULL;
        return kRmtErrorFileNotOpen;
    }

    // check that the file is larger enough at least for the RMT file header.
    if (data_set->file_size_in_bytes < sizeof(RmtFileHeader))
    {
        return kRmtErrorFileNotOpen;
    }

    // parse all the chunk headers from the file.
    RmtErrorCode error_code = ParseChunks(data_set);
    RMT_ASSERT(error_code == kRmtOk);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // construct the data profile for subsequent data parsing.
    error_code = BuildDataProfile(data_set);
    RMT_ASSERT(error_code == kRmtOk);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    return kRmtOk;
}

// destroy the data set.
RmtErrorCode RmtDataSetDestroy(RmtDataSet* data_set)
{
    // flush writes and close the handle.
    fflush((FILE*)data_set->file_handle);
    fclose((FILE*)data_set->file_handle);
    data_set->file_handle = NULL;

    CommitTemporaryFileEdits(data_set, true);

    data_set->file_handle = NULL;
    return kRmtOk;
}

// get the number of series from the timeline type
static int32_t GetSeriesCountFromTimelineType(const RmtDataSet* data_set, RmtDataTimelineType timeline_type)
{
    // NOTE: We always have 2 additional buckets, one for unallocated physical pages
    // and one for physical pages belonging to any other process.
    const int32_t additional_buckets_for_non_process_grouping_mode = 2;

    switch (timeline_type)
    {
    case kRmtDataTimelineTypeProcess:
        return data_set->process_map.process_count;

    case kRmtDataTimelineTypePageSize:
        return kRmtPageSizeReserved0 + additional_buckets_for_non_process_grouping_mode;

    case kRmtDataTimelineTypeCommitted:
        return kRmtHeapTypeCount;

    case kRmtDataTimelineTypeResourceUsageCount:
        return kRmtResourceUsageTypeCount;  // NOTE: should be top 7..8..9 with 1 bucket for "other" use a bitfield to test inclusion in top 8.

    case kRmtDataTimelineTypeResourceUsageVirtualSize:
        return kRmtResourceUsageTypeCount;

    case kRmtDataTimelineTypePaging:
        return 1;  // NOTE: Could be per driver model, could also be done per heap.

    case kRmtDataTimelineTypeVirtualMemory:
        return kRmtHeapTypeCount;

    case kRmtDataTimelineTypeResourceNonPreferred:
        return kRmtResourceUsageTypeCount;

    default:
        return 0;
    }
}

// calculate the grouping value for a collection
static int32_t UpdateSeriesValuesFromCurrentSnapshot(const RmtDataSnapshot* current_snapshot,
                                                     RmtDataTimelineType    timeline_type,
                                                     int32_t                last_value_index,
                                                     RmtDataTimeline*       out_timeline)
{
    // calculate the index within the level-0 series for the value
    const int32_t value_index = RmtDataSetGetSeriesIndexForTimestamp(current_snapshot->data_set, current_snapshot->timestamp);

    // Smeer from lastValueIndex until valueIndex if its >1 step away.
    if (last_value_index > 0)
    {
        for (int32_t current_value_index = last_value_index + 1; current_value_index < value_index; ++current_value_index)
        {
            for (int32_t current_series_index = 0; current_series_index < out_timeline->series_count; ++current_series_index)
            {
                const uint64_t value = out_timeline->series[current_series_index].levels[0].values[last_value_index];
                out_timeline->series[current_series_index].levels[0].values[current_value_index] = value;
            }
        }
    }

    // handle the values for this timeline type.
    switch (timeline_type)
    {
    case kRmtDataTimelineTypeProcess:
    {
        for (int32_t current_process_index = 0; current_process_index < current_snapshot->process_map.process_count; ++current_process_index)
        {
            const uint64_t comitted_memory_for_process = current_snapshot->process_map.process_committed_memory[current_process_index];
            out_timeline->series[current_process_index].levels[0].values[value_index] = comitted_memory_for_process;
        }
    }
    break;

    case kRmtDataTimelineTypePageSize:
    {
    }
    break;

    case kRmtDataTimelineTypeCommitted:
    {
        for (int32_t current_heap_type_index = 0; current_heap_type_index < kRmtHeapTypeCount; ++current_heap_type_index)
        {
            const uint64_t heap_type_count                                              = current_snapshot->page_table.mapped_per_heap[current_heap_type_index];
            out_timeline->series[current_heap_type_index].levels[0].values[value_index] = heap_type_count;
        }
    }
    break;

    case kRmtDataTimelineTypeResourceUsageCount:
    {
        for (int32_t current_resource_index = 0; current_resource_index < kRmtResourceUsageTypeCount; ++current_resource_index)
        {
            const int32_t resource_count_for_usage_type = current_snapshot->resource_list.resource_usage_count[current_resource_index];

            // Write this to the correct slot in the series.
            if (current_resource_index == kRmtResourceUsageTypeHeap)
            {
                out_timeline->series[current_resource_index].levels[0].values[value_index] = 0;
            }
            else
            {
                out_timeline->series[current_resource_index].levels[0].values[value_index] = resource_count_for_usage_type;
            }
        }
    }
    break;

    case kRmtDataTimelineTypeResourceUsageVirtualSize:
    {
        for (int32_t current_resource_index = 0; current_resource_index < kRmtResourceUsageTypeCount; ++current_resource_index)
        {
            const uint64_t resource_size_for_usage_type = current_snapshot->resource_list.resource_usage_size[current_resource_index];

            // Write this to the correct slot in the series.
            if (current_resource_index == kRmtResourceUsageTypeHeap)
            {
                out_timeline->series[current_resource_index].levels[0].values[value_index] = 0;
            }
            else
            {
                out_timeline->series[current_resource_index].levels[0].values[value_index] = resource_size_for_usage_type;
            }
        }
    }
    break;

    case kRmtDataTimelineTypePaging:
    {
    }
    break;

    case kRmtDataTimelineTypeVirtualMemory:
    {
        for (int32_t current_heap_type_index = 0; current_heap_type_index < kRmtHeapTypeCount; ++current_heap_type_index)
        {
            const uint64_t heap_type_count = current_snapshot->virtual_allocation_list.allocations_per_preferred_heap[current_heap_type_index];
            out_timeline->series[current_heap_type_index].levels[0].values[value_index] = heap_type_count;
        }
    }
    break;

    case kRmtDataTimelineTypeResourceNonPreferred:
    {
    }
    break;

    default:
        break;
    }

    // Sum the values in each series, and take the max against maximumValueInAllSeries
    uint64_t total_for_all_series = 0;
    for (int32_t current_series_index = 0; current_series_index < out_timeline->series_count; ++current_series_index)
    {
        total_for_all_series += out_timeline->series[current_series_index].levels[0].values[value_index];
    }

    // track the max.
    out_timeline->maximum_value_in_all_series = RMT_MAXIMUM(out_timeline->maximum_value_in_all_series, total_for_all_series);

    return value_index;
}

//  Allocate memory for the stuff we counted in the RmtDataProfile.
static RmtErrorCode TimelineGeneratorAllocateMemory(RmtDataSet* data_set, RmtDataTimelineType timeline_type, RmtDataTimeline* out_timeline)
{
    out_timeline->series_count = GetSeriesCountFromTimelineType(data_set, timeline_type);
    RMT_ASSERT(out_timeline->series_count < RMT_MAXIMUM_TIMELINE_DATA_SERIES);

    const int32_t  values_per_top_level_series = RmtDataSetGetSeriesIndexForTimestamp(data_set, data_set->maximum_timestamp) + 1;
    const uint64_t buffer_size                 = values_per_top_level_series * sizeof(uint64_t);
    const size_t   series_memory_buffer_size   = buffer_size * out_timeline->series_count;
    out_timeline->series_memory_buffer         = (int32_t*)PerformAllocation(data_set, series_memory_buffer_size, sizeof(uint64_t));
    RMT_ASSERT(out_timeline->series_memory_buffer);
    RMT_RETURN_ON_ERROR(out_timeline->series_memory_buffer, kRmtErrorOutOfMemory);

    // zero the entire buffer.
    memset(out_timeline->series_memory_buffer, 0, series_memory_buffer_size);

    uint64_t current_series_memory_buffer_start_offset = 0;
    for (int32_t current_series_index = 0; current_series_index < out_timeline->series_count; ++current_series_index)
    {
        // Work out what we needed and increment it.
        const uintptr_t buffer_address                              = (uintptr_t)out_timeline->series_memory_buffer + current_series_memory_buffer_start_offset;
        out_timeline->series[current_series_index].levels[0].values = (uint64_t*)buffer_address;
        out_timeline->series[current_series_index].level_count      = 1;
        out_timeline->series[current_series_index].levels[0].value_count = values_per_top_level_series;

        // Move the buffer along to the next process.
        current_series_memory_buffer_start_offset += buffer_size;
    }

    return kRmtOk;
}

// Load the data into the structures we have allocated.
static RmtErrorCode TimelineGeneratorParseData(RmtDataSet* data_set, RmtDataTimelineType timeline_type, RmtDataTimeline* out_timeline)
{
    RMT_ASSERT(data_set);

    // Reset the list of implicitly created resources.
    implicit_resource_list.clear();
    implicit_resource_list_created = false;

    // Allocate temporary snapshot.
    RmtDataSnapshot* temp_snapshot = (RmtDataSnapshot*)PerformAllocation(data_set, sizeof(RmtDataSnapshot), alignof(RmtDataSnapshot));
    RMT_ASSERT(temp_snapshot);
    RMT_RETURN_ON_ERROR(temp_snapshot, kRmtErrorOutOfMemory);
    RmtErrorCode error_code = AllocateMemoryForSnapshot(data_set, temp_snapshot);
    RMT_ASSERT(error_code == kRmtOk);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // initialize this.
    temp_snapshot->maximum_physical_memory_in_bytes = 0;

    // initialize the page table.
    error_code = RmtPageTableInitialize(&temp_snapshot->page_table, data_set->segment_info, data_set->segment_info_count, data_set->target_process_id);
    RMT_ASSERT(error_code == kRmtOk);

    // initialize the process map.
    error_code = RmtProcessMapInitialize(&temp_snapshot->process_map);
    RMT_ASSERT(error_code == kRmtOk);

    // Initialize resource lookup maps.
    resource_correlation_lookup_map.clear();
    resource_name_lookup_map.clear();
    unique_resource_id_lookup_map.clear();

    // Special case:
    // for timeline type of process, we have to first fill the 0th value of level 0
    // of each series with the total amount of committed memory from the process start
    // information.
    if (timeline_type == kRmtDataTimelineTypeProcess)
    {
        for (int32_t current_process_start_index = 0; current_process_start_index < data_set->process_start_info_count; ++current_process_start_index)
        {
            int32_t series_index = -1;
            RmtProcessMapGetIndexFromProcessId(&data_set->process_map, data_set->process_start_info[current_process_start_index].process_id, &series_index);
            RMT_ASSERT(series_index >= 0);

            const uint64_t value = data_set->process_start_info[current_process_start_index].physical_memory_allocated;

            // Write the value for the process start to 0th value of 0th level each series.
            out_timeline->series[current_process_start_index].levels[0].values[0] = value;
        }
    }

    RmtStreamMergerReset(&data_set->stream_merger);

    // if the heap has something there, then add it.
    int32_t last_value_index = -1;
    while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
    {
        // grab the next token from the heap.
        RmtToken current_token;
        error_code = RmtStreamMergerAdvance(&data_set->stream_merger, &current_token);
        RMT_ASSERT(error_code == kRmtOk);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

        // Update the temporary snapshot with the RMT token.
        error_code = ProcessTokenForSnapshot(data_set, &current_token, temp_snapshot);
        RMT_ASSERT(error_code == kRmtOk);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

        // set the timestamp for the current snapshot
        temp_snapshot->timestamp = current_token.common.timestamp;

        // Generate whatever series values we need for current timeline type from the snapshot.
        last_value_index = UpdateSeriesValuesFromCurrentSnapshot(temp_snapshot, timeline_type, last_value_index, out_timeline);
    }

    // clean up temporary structures we allocated to construct the timeline.
    RmtDataSnapshotDestroy(temp_snapshot);
    PerformFree(data_set, temp_snapshot);
    implicit_resource_list_created = true;
    return kRmtOk;
}

// calculate mip-maps for all levels of all series
static RmtErrorCode TimelineGeneratorCalculateSeriesLevels(RmtDataTimeline* out_timeline)
{
    RMT_UNUSED(out_timeline);

    return kRmtOk;
}

// function to generate a timeline.
RmtErrorCode RmtDataSetGenerateTimeline(RmtDataSet* data_set, RmtDataTimelineType timeline_type, RmtDataTimeline* out_timeline)
{
    RMT_ASSERT(data_set);
    RMT_ASSERT(out_timeline);
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_timeline, kRmtErrorInvalidPointer);

    // points at the parent dataset, which has lots of shared data.
    out_timeline->data_set                    = data_set;
    out_timeline->max_timestamp               = data_set->maximum_timestamp;
    out_timeline->timeline_type               = timeline_type;
    out_timeline->maximum_value_in_all_series = 0;  // this will be calculated as we populate the data/generate mipmaps.

    // Allocate the memory we care about for the timeline.
    RmtErrorCode error_code = TimelineGeneratorAllocateMemory(data_set, timeline_type, out_timeline);
    if (error_code != kRmtOk)
    {
        return error_code;
    }

    // Do the parsing for generating a timeline.
    error_code = TimelineGeneratorParseData(data_set, timeline_type, out_timeline);
    if (error_code != kRmtOk)
    {
        return error_code;
    }

    // Generate mip-map data.
    TimelineGeneratorCalculateSeriesLevels(out_timeline);

    return kRmtOk;
}

// a pass to convert solitary heaps in an allocation into buffers.
static RmtErrorCode SnapshotGeneratorConvertHeapsToBuffers(RmtDataSnapshot* snapshot)
{
    RMT_ASSERT(snapshot);

    for (int32_t current_resource_index = 0; current_resource_index < snapshot->resource_list.resource_count; ++current_resource_index)
    {
        RmtResource*          current_resource           = &snapshot->resource_list.resources[current_resource_index];
        RmtVirtualAllocation* current_virtual_allocation = (RmtVirtualAllocation*)current_resource->bound_allocation;

        if (current_virtual_allocation == nullptr)
        {
            continue;
        }

        // we're only interested in heaps which are the only resource inside an allocation.
        if (current_virtual_allocation->resource_count > 1 || current_resource->resource_type != kRmtResourceTypeHeap)
        {
            continue;
        }

        // NOTE: read things out into temporaries as heap and buffer structures are unioned.
        const size_t heap_size_in_bytes        = current_resource->heap.size;
        current_resource->buffer.create_flags  = 0;
        current_resource->buffer.usage_flags   = 0;
        current_resource->buffer.size_in_bytes = heap_size_in_bytes;
        current_resource->resource_type        = kRmtResourceTypeBuffer;
    }

    return kRmtOk;
}

static int32_t ResourceComparator(const void* a, const void* b)
{
    const RmtResource** resource_pointer_a = (const RmtResource**)a;
    const RmtResource** resource_pointer_b = (const RmtResource**)b;
    const RmtResource*  resource_a         = *resource_pointer_a;
    const RmtResource*  resource_b         = *resource_pointer_b;
    return (resource_a->address > resource_b->address) ? 1 : -1;
}

// add a list of pointers to resources to each allocation.
static RmtErrorCode SnapshotGeneratorAddResourcePointers(RmtDataSnapshot* snapshot)
{
    RMT_ASSERT(snapshot);

    // set up the pointer addresses for each allocation.
    int32_t current_resource_connectivity_index = 0;
    for (int32_t current_virtual_allocation_index = 0; current_virtual_allocation_index < snapshot->virtual_allocation_list.allocation_count;
         ++current_virtual_allocation_index)
    {
        RmtVirtualAllocation* current_virtual_allocation = &snapshot->virtual_allocation_list.allocation_details[current_virtual_allocation_index];

        if ((current_virtual_allocation->flags & kRmtAllocationDetailIsDead) == kRmtAllocationDetailIsDead)
        {
            continue;
        }

        current_virtual_allocation->resources = &snapshot->virtual_allocation_list.resource_connectivity[current_resource_connectivity_index];

        // move the index along by the number of resources inside this allocation.
        current_resource_connectivity_index += current_virtual_allocation->resource_count;
    }

    // iterate over every resource and add pointers to the allocations.
    for (int32_t current_resource_index = 0; current_resource_index < snapshot->resource_list.resource_count; ++current_resource_index)
    {
        RmtResource*          current_resource           = &snapshot->resource_list.resources[current_resource_index];
        RmtVirtualAllocation* current_virtual_allocation = (RmtVirtualAllocation*)current_resource->bound_allocation;

        if (current_virtual_allocation == nullptr)
        {
            continue;
        }

        // if the bound allocation is marked as dead then we don't want to bother patching up
        // its pointers. This is also an indication that we may have a dangling resource. We know
        // that the boundAllocation will be invalid after snapshotGeneratorCompactVirtualAllocations
        // has completed anyway, so we can easily clear them now.
        if ((current_virtual_allocation->flags & kRmtAllocationDetailIsDead) == kRmtAllocationDetailIsDead)
        {
            current_resource->flags |= kRmtResourceFlagDangling;
            current_resource->bound_allocation = NULL;
            continue;
        }

        RMT_ASSERT(current_virtual_allocation->base_address <= current_resource->address);

        // add the pointer
        RmtResource** next_resource_pointer = &current_virtual_allocation->resources[current_virtual_allocation->next_resource_index++];
        *next_resource_pointer              = current_resource;
    }

    // sort the resources into baseAddress order. This will allow subsequent algorithms to
    // operate more efficiently as they can make assumptions about the order of the resources
    // within a virtual allocation.
    for (int32_t current_virtual_allocation_index = 0; current_virtual_allocation_index < snapshot->virtual_allocation_list.allocation_count;
         ++current_virtual_allocation_index)
    {
        RmtVirtualAllocation* current_virtual_allocation = &snapshot->virtual_allocation_list.allocation_details[current_virtual_allocation_index];

        if ((current_virtual_allocation->flags & kRmtAllocationDetailIsDead) == kRmtAllocationDetailIsDead)
        {
            continue;
        }

        qsort(current_virtual_allocation->resources, current_virtual_allocation->resource_count, sizeof(RmtResource*), ResourceComparator);
    }

    return kRmtOk;
}

// add unbound resources to the virtual allocation, there should be one of these for every gap in the VA
// address space.
static RmtErrorCode SnapshotGeneratorAddUnboundResources(RmtDataSnapshot* snapshot)
{
    RMT_ASSERT(snapshot);

    // scan the VA range and look for holes in the address space where no resource is bound.
    int32_t unbound_region_index = 0;
    for (int32_t current_virtual_allocation_index = 0; current_virtual_allocation_index < snapshot->virtual_allocation_list.allocation_count;
         ++current_virtual_allocation_index)
    {
        RmtVirtualAllocation* current_virtual_allocation = &snapshot->virtual_allocation_list.allocation_details[current_virtual_allocation_index];

        // set the pointer to array of unbound regions to next in sequence.
        current_virtual_allocation->unbound_memory_regions      = &snapshot->virtual_allocation_list.unbound_memory_regions[unbound_region_index];
        current_virtual_allocation->unbound_memory_region_count = 0;

        // not interested in soon to be compacted out allocations.
        if ((current_virtual_allocation->flags & kRmtAllocationDetailIsDead) == kRmtAllocationDetailIsDead)
        {
            continue;
        }

        // scan the list of resources looking for holes.
        RmtGpuAddress last_resource_end = current_virtual_allocation->base_address;
        for (int32_t current_resource_index = 0; current_resource_index < current_virtual_allocation->resource_count; ++current_resource_index)
        {
            const RmtResource* current_resource = current_virtual_allocation->resources[current_resource_index];

            // look for holes in the VA range.
            if (current_resource->address > last_resource_end)
            {
                const uint64_t delta = current_resource->address - last_resource_end;

                // Create the unbound region.
                RmtMemoryRegion* next_region = &snapshot->virtual_allocation_list.unbound_memory_regions[unbound_region_index++];
                next_region->offset          = last_resource_end - current_virtual_allocation->base_address;
                next_region->size            = delta;
                current_virtual_allocation->unbound_memory_region_count++;
            }

            last_resource_end = current_resource->address + current_resource->size_in_bytes;
        }

        // check the ending region as a special case.
        const RmtGpuAddress end_address = current_virtual_allocation->base_address + RmtVirtualAllocationGetSizeInBytes(current_virtual_allocation);
        if (end_address > last_resource_end)
        {
            const uint64_t delta = end_address - last_resource_end;

            // Create the unbound region.
            RmtMemoryRegion* next_region = &snapshot->virtual_allocation_list.unbound_memory_regions[unbound_region_index++];
            next_region->offset          = last_resource_end - current_virtual_allocation->base_address;
            next_region->size            = delta;
            current_virtual_allocation->unbound_memory_region_count++;
        }
    }

    return kRmtOk;
}

// compact virtual allocations, removing dead ones.
static RmtErrorCode SnapshotGeneratorCompactVirtualAllocations(RmtDataSnapshot* snapshot)
{
    RMT_ASSERT(snapshot);

    RmtVirtualAllocationListCompact(&snapshot->virtual_allocation_list, true);

    return kRmtOk;
}

// calculate summary data for snapshot
static RmtErrorCode SnapshotGeneratorCalculateSummary(RmtDataSnapshot* snapshot)
{
    RMT_ASSERT(snapshot);

    snapshot->minimum_virtual_address      = UINT64_MAX;
    snapshot->maximum_virtual_address      = 0;
    snapshot->minimum_allocation_timestamp = UINT64_MAX;
    snapshot->maximum_allocation_timestamp = 0;

    for (int32_t current_virtual_allocation_index = 0; current_virtual_allocation_index < snapshot->virtual_allocation_list.allocation_count;
         ++current_virtual_allocation_index)
    {
        RmtVirtualAllocation* current_virtual_allocation = &snapshot->virtual_allocation_list.allocation_details[current_virtual_allocation_index];

        snapshot->minimum_virtual_address = RMT_MINIMUM(snapshot->minimum_virtual_address, current_virtual_allocation->base_address);
        snapshot->maximum_virtual_address = RMT_MAXIMUM(
            snapshot->maximum_virtual_address, current_virtual_allocation->base_address + RmtVirtualAllocationGetSizeInBytes(current_virtual_allocation));
        snapshot->minimum_allocation_timestamp = RMT_MINIMUM(snapshot->minimum_allocation_timestamp, current_virtual_allocation->timestamp);
        snapshot->maximum_allocation_timestamp = RMT_MAXIMUM(snapshot->maximum_allocation_timestamp, current_virtual_allocation->timestamp);
    }

    if (snapshot->minimum_virtual_address == UINT64_MAX)
    {
        snapshot->minimum_virtual_address = 0;
    }

    if (snapshot->minimum_allocation_timestamp == UINT64_MAX)
    {
        snapshot->minimum_allocation_timestamp = 0;
    }

    snapshot->minimum_resource_size_in_bytes         = RmtDataSnapshotGetSmallestResourceSize(snapshot);
    snapshot->maximum_resource_size_in_bytes         = RmtDataSnapshotGetLargestResourceSize(snapshot);
    snapshot->maximum_unbound_resource_size_in_bytes = RmtDataSnapshotGetLargestUnboundResourceSize(snapshot);

    return kRmtOk;
}

// calculate approximate commit type for each resource.
static RmtErrorCode SnapshotGeneratorCalculateCommitType(RmtDataSnapshot* snapshot)
{
    RMT_ASSERT(snapshot);

    for (int32_t current_virtual_allocation_index = 0; current_virtual_allocation_index < snapshot->virtual_allocation_list.allocation_count;
         ++current_virtual_allocation_index)
    {
        RmtVirtualAllocation* current_virtual_allocation = &snapshot->virtual_allocation_list.allocation_details[current_virtual_allocation_index];

        // walk every resources and update the commit type flag.
        for (int32_t current_resource_index = 0; current_resource_index < current_virtual_allocation->resource_count; ++current_resource_index)
        {
            RmtResource* current_resource = current_virtual_allocation->resources[current_resource_index];

            if (current_resource->commit_type != kRmtCommitTypeVirtual)
            {
                current_resource->commit_type = (current_virtual_allocation->non_heap_resource_count <= 1) ? kRmtCommitTypeCommitted : kRmtCommitTypePlaced;
            }
        }
    }

    return kRmtOk;
}

// Allocate the region stack used to calculate the total resource memory in an allocation.
static RmtErrorCode SnapshotGeneratorAllocateRegionStack(RmtDataSnapshot* snapshot)
{
    RMT_ASSERT(snapshot);

    // find the allocation with the largest number of resources
    int32_t max_resource_count = 0;
    for (int32_t current_virtual_allocation_index = 0; current_virtual_allocation_index < snapshot->virtual_allocation_list.allocation_count;
         ++current_virtual_allocation_index)
    {
        RmtVirtualAllocation* current_virtual_allocation = &snapshot->virtual_allocation_list.allocation_details[current_virtual_allocation_index];
        int32_t               current_resource_count     = current_virtual_allocation->resource_count;
        if (current_resource_count > max_resource_count)
        {
            max_resource_count = current_resource_count;
        }
    }

    // allocate the memory and keep track of the max size
    snapshot->region_stack_count = max_resource_count;
    snapshot->region_stack_buffer =
        (RmtMemoryRegion*)PerformAllocation(snapshot->data_set, sizeof(RmtMemoryRegion) * max_resource_count, sizeof(RmtMemoryRegion));

    return kRmtOk;
}

static RmtErrorCode SnapshotGeneratorCalculateSnapshotPointSummary(RmtDataSnapshot* snapshot, RmtSnapshotPoint* out_snapshot_point)
{
    RMT_ASSERT(snapshot);
    RMT_ASSERT(out_snapshot_point);

    out_snapshot_point->virtual_allocations    = snapshot->virtual_allocation_list.allocation_count;
    out_snapshot_point->resource_count         = snapshot->resource_list.resource_count;
    out_snapshot_point->total_virtual_memory   = RmtVirtualAllocationListGetTotalSizeInBytes(&snapshot->virtual_allocation_list);
    out_snapshot_point->bound_virtual_memory   = RmtVirtualAllocationListGetBoundTotalSizeInBytes(snapshot, &snapshot->virtual_allocation_list);
    out_snapshot_point->unbound_virtual_memory = RmtVirtualAllocationListGetUnboundTotalSizeInBytes(snapshot, &snapshot->virtual_allocation_list);

    RmtSegmentStatus heap_status[kRmtHeapTypeCount];
    for (int32_t current_heap_type_index = 0; current_heap_type_index < kRmtHeapTypeNone; ++current_heap_type_index)
    {
        RmtDataSnapshotGetSegmentStatus(snapshot, (RmtHeapType)current_heap_type_index, &heap_status[current_heap_type_index]);
        out_snapshot_point->committed_memory[current_heap_type_index] = heap_status[current_heap_type_index].total_physical_mapped_by_process;
    }

    return kRmtOk;
}

// function to generate a snapshot.
RmtErrorCode RmtDataSetGenerateSnapshot(RmtDataSet* data_set, RmtSnapshotPoint* snapshot_point, RmtDataSnapshot* out_snapshot)
{
    RMT_ASSERT(data_set);
    RMT_ASSERT(out_snapshot);
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_snapshot, kRmtErrorInvalidPointer);

    out_snapshot->snapshot_point = snapshot_point;

    // set up the snapshot.
    memcpy(out_snapshot->name, snapshot_point->name, RMT_MINIMUM(strlen(snapshot_point->name), sizeof(out_snapshot->name)));
    out_snapshot->timestamp = snapshot_point->timestamp;
    RmtErrorCode error_code = AllocateMemoryForSnapshot(data_set, out_snapshot);

    // initialize this.
    out_snapshot->maximum_physical_memory_in_bytes = 0;

    // initialize the page table.
    error_code = RmtPageTableInitialize(
        &out_snapshot->page_table, out_snapshot->data_set->segment_info, out_snapshot->data_set->segment_info_count, out_snapshot->data_set->target_process_id);
    RMT_ASSERT(error_code == kRmtOk);

    // Reset the RMT stream parsers ready to load the data.
    RmtStreamMergerReset(&data_set->stream_merger);

    // process all the tokens
    while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
    {
        // grab the next token from the heap.
        RmtToken current_token;
        error_code = RmtStreamMergerAdvance(&data_set->stream_merger, &current_token);
        RMT_ASSERT(error_code == kRmtOk);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

        // we only want to create the snapshot using events up until a specific moment in time.
        if (current_token.common.timestamp > snapshot_point->timestamp)
        {
            break;
        }

        // handle the token.
        error_code = ProcessTokenForSnapshot(data_set, &current_token, out_snapshot);
        RMT_ASSERT(error_code == kRmtOk);
    }

    SnapshotGeneratorConvertHeapsToBuffers(out_snapshot);
    SnapshotGeneratorAddResourcePointers(out_snapshot);
    SnapshotGeneratorCompactVirtualAllocations(out_snapshot);
    SnapshotGeneratorAddUnboundResources(out_snapshot);
    SnapshotGeneratorCalculateSummary(out_snapshot);
    SnapshotGeneratorCalculateCommitType(out_snapshot);
    SnapshotGeneratorAllocateRegionStack(out_snapshot);
    SnapshotGeneratorCalculateSnapshotPointSummary(out_snapshot, snapshot_point);

    // Update the lookup maps to use unique resource ID hash values.
    UpdateResourceNameIds();

    for (const auto& name_token_item : resource_name_lookup_map)
    {
        RmtResourceIdentifier resource_identifier = name_token_item.second.resource_id;
        RmtResource*          found_resource      = NULL;
        error_code = RmtResourceListGetResourceByResourceId(&out_snapshot->resource_list, resource_identifier, (const RmtResource**)&found_resource);
        if (error_code == kRmtOk)
        {
            CopyString(found_resource->name, sizeof(found_resource->name), name_token_item.second.name.c_str(), name_token_item.second.name.length());
        }
    }
    return kRmtOk;
}

// get the segment info for a physical address
RmtErrorCode RmtDataSetGetSegmentForPhysicalAddress(const RmtDataSet* data_set, RmtGpuAddress physical_address, const RmtSegmentInfo** out_segment_info)
{
    RMT_ASSERT(data_set);
    RMT_ASSERT(out_segment_info);
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_segment_info, kRmtErrorInvalidPointer);

    for (int32_t current_segment_info_index = 0; current_segment_info_index < data_set->segment_info_count; ++current_segment_info_index)
    {
        const RmtSegmentInfo* current_segment_info = &data_set->segment_info[current_segment_info_index];
        if (current_segment_info->base_address <= physical_address && physical_address <= (current_segment_info->base_address + current_segment_info->size))
        {
            *out_segment_info = current_segment_info;
            return kRmtOk;
        }
    }

    return kRmtErrorNoAllocationFound;
}

// Get the time corresponding to the given number of cpu clock cycles
RmtErrorCode RmtDataSetGetCpuClockTimestamp(const RmtDataSet* data_set, uint64_t clk, double* out_cpu_timestamp)
{
    RMT_ASSERT(data_set);
    RMT_ASSERT(out_cpu_timestamp);
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_cpu_timestamp, kRmtErrorInvalidPointer);

    const uint64_t cpu_clock_frequency_in_mhz = data_set->cpu_frequency / 1000000;

    if (cpu_clock_frequency_in_mhz == 0)
    {
        return kRmtErrorTimestampOutOfBounds;
    }

    *out_cpu_timestamp = clk * 1000.0;
    *out_cpu_timestamp /= cpu_clock_frequency_in_mhz;
    return kRmtOk;
}

// Get whether the CPU clock timestamp is valid
RmtErrorCode RmtDataSetGetCpuClockTimestampValid(const RmtDataSet* data_set)
{
    RMT_ASSERT(data_set);
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);

    const uint64_t cpu_clock_frequency_in_mhz = data_set->cpu_frequency / 1000000;

    if (cpu_clock_frequency_in_mhz == 0)
    {
        return kRmtErrorTimestampOutOfBounds;
    }
    return kRmtOk;
}

// guts of adding a snapshot with file ops.
static RmtErrorCode AddSnapshot(RmtDataSet* data_set, const char* name, uint64_t timestamp, RmtSnapshotPoint** out_snapshot_point)
{
    // add it to the snapshot list in the dataset.
    const int32_t snapshot_index = data_set->snapshot_count++;
    if (snapshot_index >= RMT_MAXIMUM_SNAPSHOT_POINTS)
    {
        *out_snapshot_point = NULL;
        return kRmtErrorOutOfMemory;
    }

    const int32_t name_length                     = (int32_t)RMT_MINIMUM(strlen(name), RMT_MAXIMUM_NAME_LENGTH);
    data_set->snapshots[snapshot_index].timestamp = timestamp;
    memset(data_set->snapshots[snapshot_index].name, 0, sizeof(data_set->snapshots[snapshot_index].name));
    memcpy(data_set->snapshots[snapshot_index].name, name, name_length);
    data_set->snapshots[snapshot_index].cached_snapshot        = NULL;
    data_set->snapshots[snapshot_index].virtual_allocations    = 0;
    data_set->snapshots[snapshot_index].resource_count         = 0;
    data_set->snapshots[snapshot_index].total_virtual_memory   = 0;
    data_set->snapshots[snapshot_index].bound_virtual_memory   = 0;
    data_set->snapshots[snapshot_index].unbound_virtual_memory = 0;
    for (int32_t current_heap_type_index = 0; current_heap_type_index < kRmtHeapTypeCount; ++current_heap_type_index)
    {
        data_set->snapshots[snapshot_index].committed_memory[current_heap_type_index] = 0;
    }

    if (!data_set->read_only)
    {
        // jump to the end of the file, and write a new snapshot out.
        fseek((FILE*)data_set->file_handle, 0L, SEEK_END);

        // add the header.
        RmtFileChunkHeader chunk_header;
        chunk_header.chunk_identifier.chunk_type  = kRmtFileChunkTypeSnapshotInfo;
        chunk_header.chunk_identifier.chunk_index = 0;
        chunk_header.chunk_identifier.reserved    = 0;
        chunk_header.version_major                = 1;
        chunk_header.version_minor                = 0;
        chunk_header.padding                      = 0;
        chunk_header.size_in_bytes                = sizeof(RmtFileChunkHeader) + sizeof(RmtFileChunkSnapshotInfo) + name_length;

        size_t write_size = fwrite(&chunk_header, 1, sizeof(RmtFileChunkHeader), (FILE*)data_set->file_handle);
        RMT_ASSERT(write_size == sizeof(RmtFileChunkHeader));

        // add the snapshot payload
        RmtFileChunkSnapshotInfo snapshot_info_chunk;
        snapshot_info_chunk.name_length_in_bytes = name_length;
        snapshot_info_chunk.snapshot_time        = timestamp + data_set->stream_merger.minimum_start_timestamp;  // add the minimum so rebase on load works.
        data_set->snapshots[snapshot_index].file_offset = ftell((FILE*)data_set->file_handle);                   // get offset before write to payload
        write_size                                      = fwrite(&snapshot_info_chunk, 1, sizeof(RmtFileChunkSnapshotInfo), (FILE*)data_set->file_handle);
        RMT_ASSERT(write_size == sizeof(RmtFileChunkSnapshotInfo));

        // write the name
        write_size = fwrite(name, 1, name_length, (FILE*)data_set->file_handle);
        RMT_ASSERT(write_size == name_length);
    }

    // write the pointer back.
    *out_snapshot_point = &data_set->snapshots[snapshot_index];
    return kRmtOk;
}

// add a new snapshot to the end of the file.
RmtErrorCode RmtDataSetAddSnapshot(RmtDataSet* data_set, const char* name, uint64_t timestamp, RmtSnapshotPoint** out_snapshot_point)
{
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(name, kRmtErrorInvalidPointer);

    const RmtErrorCode error_code = AddSnapshot(data_set, name, timestamp, out_snapshot_point);
    RMT_ASSERT(error_code == kRmtOk);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    CommitTemporaryFileEdits(data_set, false);
    return kRmtOk;
}

// guts of removing a snapshot without destroying the cached object, lets this code be shared with rename.
static void RemoveSnapshot(RmtDataSet* data_set, const int32_t snapshot_index)
{
    if (!data_set->read_only)
    {
        // set the length to 0 in the file.
        const uint64_t offset_to_snapshot_chunk = data_set->snapshots[snapshot_index].file_offset;  // offset to snapshot info chunk.
        fseek((FILE*)data_set->file_handle, (int32_t)(offset_to_snapshot_chunk + offsetof(RmtFileChunkSnapshotInfo, name_length_in_bytes)), SEEK_SET);
        const uint32_t value = 0;
        fwrite(&value, 1, sizeof(uint32_t), (FILE*)data_set->file_handle);
    }

    // remove the snapshot from the list of snapshot points in the dataset.
    const int32_t last_snapshot_index = data_set->snapshot_count - 1;
    memcpy(&data_set->snapshots[snapshot_index], &data_set->snapshots[last_snapshot_index], sizeof(RmtSnapshotPoint));
    data_set->snapshot_count--;
}

// remove a snapshot from the data set.
RmtErrorCode RmtDataSetRemoveSnapshot(RmtDataSet* data_set, const int32_t snapshot_index)
{
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(snapshot_index < data_set->snapshot_count, kRmtErrorIndexOutOfRange);

    RmtSnapshotPoint* snapshot_point = &data_set->snapshots[snapshot_index];
    RmtDataSnapshotDestroy(snapshot_point->cached_snapshot);

    RemoveSnapshot(data_set, snapshot_index);

    CommitTemporaryFileEdits(data_set, false);

    return kRmtOk;
}

// rename a snapshot in the data set.
RmtErrorCode RmtDataSetRenameSnapshot(RmtDataSet* data_set, const int32_t snapshot_index, const char* name)
{
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(name, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(snapshot_index < data_set->snapshot_count, kRmtErrorIndexOutOfRange);

    const uint64_t timestamp = data_set->snapshots[snapshot_index].timestamp;

    // add it to the end.
    RmtSnapshotPoint* snapshot_point = NULL;
    RmtErrorCode      error_code     = AddSnapshot(data_set, name, timestamp, &snapshot_point);
    RMT_ASSERT(error_code == kRmtOk);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // copy over the summary stuff from the previous one, and the pointer to the cached dataset.
    snapshot_point->cached_snapshot        = data_set->snapshots[snapshot_index].cached_snapshot;
    snapshot_point->virtual_allocations    = data_set->snapshots[snapshot_index].virtual_allocations;
    snapshot_point->resource_count         = data_set->snapshots[snapshot_index].resource_count;
    snapshot_point->total_virtual_memory   = data_set->snapshots[snapshot_index].total_virtual_memory;
    snapshot_point->bound_virtual_memory   = data_set->snapshots[snapshot_index].bound_virtual_memory;
    snapshot_point->unbound_virtual_memory = data_set->snapshots[snapshot_index].unbound_virtual_memory;
    for (int32_t current_heap_type_index = 0; current_heap_type_index < kRmtHeapTypeCount; ++current_heap_type_index)
    {
        snapshot_point->committed_memory[current_heap_type_index] = data_set->snapshots[snapshot_index].committed_memory[current_heap_type_index];
    }

    // remove it also, has the side effect of copying the new thing we just made back to the original location :D
    RemoveSnapshot(data_set, snapshot_index);

    CommitTemporaryFileEdits(data_set, false);

    return kRmtOk;
}

int32_t RmtDataSetGetSeriesIndexForTimestamp(RmtDataSet* data_set, uint64_t timestamp)
{
    RMT_UNUSED(data_set);

    return (int32_t)(timestamp / 3000);
}
