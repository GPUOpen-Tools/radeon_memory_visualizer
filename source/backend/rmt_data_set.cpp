//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of functions for working with a data set.
//=============================================================================

#include "rmt_data_set.h"

#include <stdlib.h>  // for malloc() / free()
#include <string.h>  // for memcpy()
#include <time.h>
#include <algorithm>
#include <string>

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "rmt_adapter_info.h"
#include "rmt_address_helper.h"
#include "rmt_assert.h"
#include "rmt_constants.h"
#include "rmt_data_snapshot.h"
#include "rmt_data_timeline.h"
#include "rmt_file_format.h"
#include "rmt_legacy_snapshot_writer.h"
#include "rmt_linear_buffer.h"
#include "rmt_memory_aliasing_timeline.h"
#include "rmt_print.h"
#include "rmt_rdf_file_parser.h"
#include "rmt_resource_history.h"
#include "rmt_snapshot_writer.h"
#include "rmt_token.h"
#include "rmt_util.h"

#ifdef _LINUX
#include "linux/safe_crt.h"
#include "rmt_trace_loader.h"

#include <stddef.h>  // for offsetof macro.
#include <unistd.h>
#include <fstream>
#else
#define WIN32_LEAN_AND_MEAN
#include <corecrt_io.h>
#include <fcntl.h>
#include <windows.h>
#include <fstream>
#endif  // #ifdef _LINUX

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

/// @brief Customizable file open function.
///
/// Provides the options of preventing other processes from inheriting the handle and
/// specifying whether the file should be opened in a shareable mode or exclusive mode.
///
/// @param [out] file_descriptor                 A pointer to the file descriptor to be opened.
/// @param [in]  file_name                       A pointer to the file name to be opened.
/// @param [in]  mode                            The mode to open the file in.
/// @param [in]  prevent_inheritance             A flag that, if true, prevents inheritance of the file handle.
/// @param [in]  is_shareable                    A flag that, if true, allows the file to be shared with other processes.
///
/// @return An error code indicating the result of the operation (0 indicates success).
///
static errno_t OpenFile(FILE** file_descriptor, char const* file_name, char const* mode, const bool prevent_inheritance, const bool is_shareable)
{
#ifdef WIN32
    const DWORD share_mode           = is_shareable ? (FILE_SHARE_READ | FILE_SHARE_WRITE) : 0;
    const DWORD flags_and_attributes = FILE_ATTRIBUTE_NORMAL;
    DWORD       desired_access       = 0;
    DWORD       creation_disposition = 0;

    if (strrchr(mode, 'r') != nullptr)
    {
        desired_access |= GENERIC_READ;
        creation_disposition = OPEN_EXISTING;
    }

    if (strrchr(mode, 'w') != nullptr)
    {
        desired_access |= GENERIC_WRITE;
        creation_disposition = CREATE_ALWAYS;
    }

    if ((strrchr(mode, 'a') != nullptr) || (strrchr(mode, '+') != nullptr))
    {
        desired_access |= GENERIC_WRITE;
        creation_disposition = OPEN_ALWAYS;
    }

    HANDLE object_handle = CreateFile(file_name, desired_access, share_mode, nullptr, creation_disposition, flags_and_attributes, nullptr);

    if (object_handle == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

    if (prevent_inheritance)
    {
        // Disable inheritance of the file handle.
        SetHandleInformation(object_handle, HANDLE_FLAG_INHERIT, 0);
    }

    // Convert the file handle to a file descriptor.
    const int file_handle = _open_osfhandle((intptr_t)object_handle, _O_BINARY);
    *file_descriptor      = _fdopen(file_handle, mode);
    if (*file_descriptor == nullptr)
    {
        CloseHandle(object_handle);
        return GetLastError();
    }

    return 0;
#else
    RMT_UNUSED(prevent_inheritance);
    RMT_UNUSED(is_shareable);

    errno_t error_no = fopen_s(file_descriptor, file_name, mode);
    return error_no;
#endif
}

/// @brief Determine if the trace file is an RGD crash dump.
///
/// @param [in]  path                           A pointer to the trace file path to be checked.
///
/// @return A flag that is true if the file is an RGD crash dump or false otherwise.
///
static bool IsCrashDumpFile(const char* path)
{
    // Workaround for -Werror=unused-variable compiler warning.
    RMT_UNUSED(kRMVTraceFileExtension);

    RMT_ASSERT(path);
    bool result = false;

    // Assuming the path has the ".rgd" extension, calculate the extension start and length.
    size_t path_length      = strlen(path);
    size_t extension_length = strlen(kRGDTraceFileExtension);

    // Only compare the extension if the path has enough characters in it.
    if (path_length >= extension_length)
    {
        size_t extension_start = path_length - extension_length;
#ifdef _WIN32
        if (_strcmpi(path + extension_start, kRGDTraceFileExtension) == 0)
#else
        if (strcasecmp(path + extension_start, kRGDTraceFileExtension) == 0)
#endif
        {
            result = true;
        }
    }

    return result;
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
    // The Windows MoveFileEx() API function is not used here because it requires
    // the destination file to be opened in exclusive mode.  Otherwise, MoveFileEx() will fail
    // with a sharing violation error.  Instead, the destination file is manually opened and
    // the source file is copied to it.

    // Open the source file for reading.
    std::ifstream source_file(existing_file_path, std::ios::binary);
    if (!source_file)
    {
        return false;  // Failed to open the source file.
    }

    // Open the shared destination file for writing with shared read/write access.
    std::ofstream destination_file(new_file_path, std::ios::binary);
    if (!destination_file)
    {
        return false;  // Failed to open the destination file.
    }

    // Copy the contents of the source file to the destination file.
    destination_file << source_file.rdbuf();

    if (!destination_file.good())
    {
        return false;  // Failed to copy the file contents.
    }

    // Close the file handles.
    source_file.close();
    destination_file.close();

    // Delete the source file.
    if (std::remove(existing_file_path) != 0)
    {
        return false;  // Failed to delete the source file.
    }

    return true;  // File moved successfully.
#else
    bool result = false;
    if (rename(existing_file_path, new_file_path) == 0)
    {
        result = true;
    }

    return result;
#endif
}

// Portable delete file function.
static bool DeleteTemporaryFile(const char* file_path)
{
    bool         result           = false;
    const size_t file_path_length = strlen(file_path);
    const size_t extension_length = strlen("bak");

    if (file_path_length > extension_length)
    {
        const char* file_extension = file_path + (file_path_length - extension_length);
        if (strcmp(file_extension, "bak") == 0)
        {
#ifdef _WIN32
            return DeleteFile(file_path);
#else
            if (remove(file_path) == 0)
            {
                result = true;
            }
#endif
        }
    }
    return result;
}

// Convert a snapshot writer handle to a snapshot writer class object pointer.
RmtSnapshotWriter* SnapshotWriterFromHandle(RmtSnapshotWriterHandle handle)
{
    RmtSnapshotWriter* snapshot_writer = reinterpret_cast<RmtSnapshotWriter*>(handle);

    RMT_ASSERT((handle == nullptr) || ((handle != nullptr) && (snapshot_writer != nullptr)));
    return snapshot_writer;
}

// Delete a snapshot writer object associated with a data set.
RmtErrorCode DestroySnapshotWriter(RmtDataSet* data_set)
{
    delete SnapshotWriterFromHandle(data_set->snapshot_writer_handle);
    data_set->snapshot_writer_handle = nullptr;

    return kRmtOk;
}

using namespace RmtMemoryAliasingTimelineAlgorithm;

// Map used to lookup unique resource ID hash using the original resource ID as the key.
static std::unordered_map<RmtResourceIdentifier, RmtResourceIdentifier> unique_resource_id_lookup_map;

// The set of created resources at any point in time.
static std::unordered_set<RmtResourceIdentifier> created_resources;

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
    RMT_STATIC_ASSERT(sizeof(adapter_info_chunk.name) == sizeof(data_set->system_info.name));

    // fill out adapter info.
    strncpy_s(data_set->system_info.name, sizeof(data_set->system_info.name), adapter_info_chunk.name, sizeof(data_set->system_info.name) - 1);

    data_set->system_info.pcie_family_id       = adapter_info_chunk.pcie_family_id;
    data_set->system_info.pcie_revision_id     = adapter_info_chunk.pcie_revision_id;
    data_set->system_info.device_id            = adapter_info_chunk.device_id;
    data_set->system_info.minimum_engine_clock = adapter_info_chunk.minimum_engine_clock;
    data_set->system_info.maximum_engine_clock = adapter_info_chunk.maximum_engine_clock;

    strncpy_s(data_set->system_info.video_memory_type_name,
              sizeof(data_set->system_info.video_memory_type_name),
              RmtAdapterInfoGetVideoMemoryType(static_cast<RmtAdapterInfoMemoryType>(adapter_info_chunk.memory_type)),
              sizeof(data_set->system_info.video_memory_type_name) - 1);

    data_set->system_info.memory_operations_per_clock = adapter_info_chunk.memory_operations_per_clock;
    data_set->system_info.memory_bus_width            = adapter_info_chunk.memory_bus_width;
    data_set->system_info.memory_bandwidth            = adapter_info_chunk.memory_bandwidth;
    data_set->system_info.minimum_memory_clock        = adapter_info_chunk.minimum_memory_clock;
    data_set->system_info.maximum_memory_clock        = adapter_info_chunk.maximum_memory_clock;
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
        switch (current_file_chunk->chunk_identifier.chunk_info.chunk_type)
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
    error_code = RmtStreamMergerInitialize(&data_set->stream_merger, data_set->streams, data_set->stream_count, data_set->file_handle);
    RMT_ASSERT(error_code == kRmtOk);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // Rebase any snapshot times to be relative to the minimum timestamp.
    for (int32_t current_snapshot_index = 0; current_snapshot_index < data_set->snapshot_count; ++current_snapshot_index)
    {
        data_set->snapshots[current_snapshot_index].timestamp -= data_set->stream_merger.minimum_start_timestamp;
    }

    return kRmtOk;
}

// Check for CPU host aperture support.
//
// Supported by default on RDNA 4 hardware.
static void CheckForCpuHostApertureSupport(RmtDataSet* data_set)
{
    if (data_set->system_info.pcie_family_id == kFamilyNavi4)
    {
        data_set->segment_info[kRmtHeapTypeLocal].size += data_set->segment_info[kRmtHeapTypeInvisible].size;
        data_set->segment_info[kRmtHeapTypeInvisible].size = 0;
        data_set->flags.cpu_host_aperture_enabled          = true;
    }
    else
    {
        data_set->flags.cpu_host_aperture_enabled = false;
    }
}

// Check for SAM (Smart access memory) support.
//
// Without SAM support, the local memory size is 256MB. If SAM is enabled, the local memory
// will be the total GPU memory. In addition, the invisible memory available will be 0 bytes.
static void CheckForSAMSupport(RmtDataSet* data_set)
{
    if (data_set->segment_info[kRmtHeapTypeInvisible].size == 0)
    {
        data_set->flags.sam_enabled = true;
    }
    else
    {
        data_set->flags.sam_enabled = false;
    }
}

static void BuildDataProfileParseUserdata(RmtDataSet* data_set, const RmtToken* current_token)
{
    RMT_ASSERT(current_token->type == kRmtTokenTypeUserdata);

    const RmtTokenUserdata* userdata = (const RmtTokenUserdata*)&current_token->userdata_token;

    if (userdata->userdata_type == kRmtUserdataTypeCorrelation)
    {
        data_set->flags.contains_correlation_tokens = true;
    }
    else if (userdata->userdata_type == kRmtUserdataTypeSnapshot)
    {
        data_set->data_profile.snapshot_count++;
        data_set->data_profile.snapshot_name_count += userdata->size_in_bytes + 1;  // +1 for /0
    }
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

    // Add this resource to the list of created resources, and keep track of the maximum number of concurrent resources.
    created_resources.insert(current_token->resource_create_token.resource_identifier);
    data_set->data_profile.max_concurrent_resources =
        RMT_MAXIMUM(data_set->data_profile.max_concurrent_resources, static_cast<int32_t>(created_resources.size()));

    data_set->data_profile.current_resource_count++;
    data_set->data_profile.total_resource_count++;

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

    // Only remove the resource from list of created resources if it has previously been created.
    if (created_resources.find(current_token->resource_destroy_token.resource_identifier) != created_resources.end())
    {
        created_resources.erase(current_token->resource_destroy_token.resource_identifier);
    }
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

    created_resources.clear();

    // if the heap has something there, then add it.
    while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
    {
        // grab the next token from the heap.
        RmtToken           current_token;
        const RmtErrorCode error_code = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
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

    created_resources.clear();

    data_set->cpu_frequency = data_set->streams[0].cpu_frequency;

    // Create an allocator for the token heap to use for generating unique resource IDs
    size_t size_required = (data_set->data_profile.total_resource_count * sizeof(ResourceIdMapNode)) + sizeof(ResourceIdMapAllocator);

    void* data = calloc(size_required, 1);
    RMT_ASSERT(data != nullptr);
    data_set->resource_id_map_allocator                  = static_cast<ResourceIdMapAllocator*>(data);
    data_set->resource_id_map_allocator->allocation_base = (void*)(static_cast<const uint8_t*>(data) + sizeof(ResourceIdMapAllocator));
    data_set->resource_id_map_allocator->allocation_size = size_required - sizeof(ResourceIdMapAllocator);
    data_set->resource_id_map_allocator->resource_count  = 0;
    data_set->stream_merger.allocator                    = data_set->resource_id_map_allocator;
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
static RmtErrorCode AllocateMemoryForSnapshot(RmtDataSet* data_set, RmtDataSnapshot* out_snapshot, bool enable_aliased_resource_usage_sizes)
{
    // Set a pointer to parent data set.
    out_snapshot->data_set = data_set;

    // Initialize the virtual allocation list.
    const size_t virtual_allocation_buffer_size =
        RmtVirtualAllocationListGetBufferSize(data_set->data_profile.total_virtual_allocation_count, data_set->data_profile.max_concurrent_resources);
    if (virtual_allocation_buffer_size > 0)
    {
        out_snapshot->virtual_allocation_buffer = PerformAllocation(data_set, virtual_allocation_buffer_size, sizeof(uint32_t));
        RMT_ASSERT(out_snapshot->virtual_allocation_buffer);
        RMT_RETURN_ON_ERROR(out_snapshot->virtual_allocation_buffer, kRmtErrorOutOfMemory);
        const RmtErrorCode error_code = RmtVirtualAllocationListInitialize(&out_snapshot->virtual_allocation_list,
                                                                           out_snapshot->virtual_allocation_buffer,
                                                                           virtual_allocation_buffer_size,
                                                                           data_set->data_profile.max_virtual_allocation_count,
                                                                           data_set->data_profile.max_concurrent_resources,
                                                                           data_set->data_profile.total_virtual_allocation_count);
        RMT_ASSERT(error_code == kRmtOk);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }

    // create the resource list.
    const size_t resource_list_buffer_size = RmtResourceListGetBufferSize(data_set->data_profile.max_concurrent_resources);
    if (resource_list_buffer_size > 0)
    {
        out_snapshot->resource_list_buffer = PerformAllocation(data_set, resource_list_buffer_size, sizeof(uint32_t));
        RMT_ASSERT(out_snapshot->resource_list_buffer);
        RMT_RETURN_ON_ERROR(out_snapshot->resource_list_buffer, kRmtErrorOutOfMemory);
        const RmtErrorCode error_code = RmtResourceListInitialize(&out_snapshot->resource_list,
                                                                  out_snapshot->resource_list_buffer,
                                                                  resource_list_buffer_size,
                                                                  &out_snapshot->virtual_allocation_list,
                                                                  data_set->data_profile.max_concurrent_resources,
                                                                  enable_aliased_resource_usage_sizes);
        RMT_ASSERT(error_code == kRmtOk);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }

    // initialize the region stack
    out_snapshot->region_stack_buffer = NULL;
    out_snapshot->region_stack_count  = 0;

    return kRmtOk;
}

void UpdateTotalResourceUsageAliasedSize(RmtResourceList*                                                 resource_list,
                                         RmtMemoryAliasingTimelineAlgorithm::RmtMemoryAliasingCalculator* memory_aliasing_calculator);

// consume next RMT token for snapshot generation.
static RmtErrorCode ProcessTokenForSnapshot(RmtDataSet* data_set, RmtToken* current_token, RmtDataSnapshot* out_snapshot)
{
    RmtErrorCode error_code = kRmtOk;

    RMT_UNUSED(data_set);

    const bool enable_aliased_resource_usage_sizes = out_snapshot->resource_list.enable_aliased_resource_usage_sizes;

    switch (current_token->type)
    {
    case kRmtTokenTypeVirtualFree:
    {
        const RmtVirtualAllocation* virtual_allocation = nullptr;
        RmtErrorCode                result             = RmtVirtualAllocationListGetAllocationForAddress(
            &(out_snapshot->virtual_allocation_list), current_token->virtual_free_token.virtual_address, &virtual_allocation);
        // Remove the virtual allocation if it is being tracked and a virtual allocation could be found.
        if ((result == kRmtOk) && (enable_aliased_resource_usage_sizes))
        {
            // Update memory sizes grouped by resource usage types taking into account overlapped aliased resources.
            RmtMemoryAliasingCalculator* memory_aliasing_calculator = RmtMemoryAliasingCalculatorInstance();
            RMT_ASSERT(memory_aliasing_calculator != nullptr);
            memory_aliasing_calculator->DestroyAllocation(virtual_allocation->allocation_identifier);
            UpdateTotalResourceUsageAliasedSize(&out_snapshot->resource_list, memory_aliasing_calculator);
        }
        error_code = RmtVirtualAllocationListRemoveAllocation(&out_snapshot->virtual_allocation_list, current_token->virtual_free_token.virtual_address);
        RMT_ASSERT(error_code == kRmtOk);
        //RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }
    break;

    case kRmtTokenTypePageTableUpdate:
    {
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
        if (current_token->userdata_token.userdata_type == kRmtUserdataTypeName || current_token->userdata_token.userdata_type == kRmtUserdataTypeName_V2)
        {
            if (!data_set->flags.userdata_processed)
            {
                // Get resource name from token. It'll be the first part of the payload, and null-terminated.
                const char* resource_name = reinterpret_cast<const char*>(current_token->userdata_token.payload_cache);

                error_code = RmtResourceUserdataTrackResourceNameToken(current_token->userdata_token.correlation_identifier,
                                                                       resource_name,
                                                                       current_token->common.timestamp,
                                                                       current_token->userdata_token.time_delay);
                RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
            }

            current_token->userdata_token.payload_cache = nullptr;
        }
        else if (current_token->userdata_token.userdata_type == kRmtUserdataTypeCorrelation)
        {
            if (!data_set->flags.userdata_processed)
            {
                RmtResourceUserdataTrackResourceCorrelationToken(
                    current_token->userdata_token.resource_identifier, current_token->userdata_token.correlation_identifier, current_token->common.timestamp);
            }
        }
        else if (current_token->userdata_token.userdata_type == kRmtUserdataTypeMarkImplicitResource ||
                 current_token->userdata_token.userdata_type == kRmtUserdataTypeMarkImplicitResource_V2)
        {
            if (!data_set->flags.userdata_processed)
            {
                // If the HeapType is missing from the MarkImplicitResource token (traces prior to RMT Spec version 1.9), assume the implicit resource is a buffer or image.
                if (current_token->userdata_token.implicit_resource_type == RmtImplicitResourceType::kRmtImplicitResourceTypeUnused)
                {
                    current_token->userdata_token.implicit_resource_type = RmtImplicitResourceType::kRmtImplicitResourceTypeImplicitResource;
                }
                else
                {
                    data_set->flags.implicit_heap_detection = true;
                }
                RmtResourceUserdataTrackImplicitResourceToken(current_token->userdata_token.resource_identifier,
                                                              current_token->common.timestamp,
                                                              current_token->userdata_token.time_delay,
                                                              current_token->userdata_token.implicit_resource_type);
            }
        }
    }
    break;

    case kRmtTokenTypeMisc:
    {
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
        if (!RmtResourceUserDataIsResourceImplicit(current_token->resource_bind_token.resource_identifier))
        {
            error_code = RmtResourceListAddResourceBind(&out_snapshot->resource_list, &current_token->resource_bind_token, !data_set->flags.userdata_processed);

            if (error_code == kRmtErrorSharedAllocationNotFound)
            {
                // This is not a true error, it just means that we encountered a shareable resource without the matching virtual
                // alloc token.  This is an expected case as that allocation is owned outside the target process, so we'll add
                // the allocation to the list so future resource tokens can find it.
                static const RmtHeapType kDummyHeapPrefLocal[RMT_NUM_HEAP_PREFERENCES] = {
                    kRmtHeapTypeLocal, kRmtHeapTypeLocal, kRmtHeapTypeLocal, kRmtHeapTypeLocal};
                static const RmtHeapType kDummyHeapPrefInvisible[RMT_NUM_HEAP_PREFERENCES] = {
                    kRmtHeapTypeLocal, kRmtHeapTypeLocal, kRmtHeapTypeLocal, kRmtHeapTypeLocal};

                // If there's no invisible memory, this allocation is going to be in the local heap.
                const RmtHeapType* kDummyHeapPref = kDummyHeapPrefInvisible;
                if (data_set->flags.local_heap_only)
                {
                    kDummyHeapPref = kDummyHeapPrefLocal;
                }

                // The byte offset of the token in the data stream is used to uniquely identify this allocation.
                // The offset is used rather than the virtual allocation address in case there are allocations/frees then another allocation with the same base address.
                uint64_t allocation_identifier = current_token->common.offset;
                error_code                     = RmtVirtualAllocationListAddAllocation(&out_snapshot->virtual_allocation_list,
                                                                   current_token->common.timestamp,
                                                                   current_token->resource_bind_token.virtual_address,
                                                                   (int32_t)(current_token->resource_bind_token.size_in_bytes >> 12),
                                                                   kDummyHeapPref,
                                                                   RmtOwnerType::kRmtOwnerTypeClientDriver,
                                                                   allocation_identifier);

                if (enable_aliased_resource_usage_sizes)
                {
                    RmtMemoryAliasingCalculator* memory_aliasing_calculator = RmtMemoryAliasingCalculatorInstance();
                    RMT_ASSERT(memory_aliasing_calculator != nullptr);
                    memory_aliasing_calculator->CreateAllocation(allocation_identifier, current_token->resource_bind_token.size_in_bytes);
                }
            }
            else if (error_code == kRmtErrorResourceAlreadyBound)
            {
                // Handle the case where the resource is already bound to a virtual memory allocation.
                // This can occur for command allocators which can be bound to multiple chunks of virtual
                // address space simultaneously or buffer resources already bound to an allocation.  These
                // resources are implicitly destroyed, created again and bound to a different allocation.
                // Heap resources may also need to re-bind if a larger size is required.
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
                    resource_create_token.resource_type       = matching_resource->resource_type;
                    memcpy(&resource_create_token.common, &current_token->common, sizeof(RmtTokenCommon));

                    switch (matching_resource->resource_type)
                    {
                    case kRmtResourceTypeCommandAllocator:
                        memcpy(&resource_create_token.command_allocator, &matching_resource->command_allocator, sizeof(RmtResourceDescriptionCommandAllocator));
                        break;

                    case kRmtResourceTypeBuffer:
                        memcpy(&resource_create_token.buffer, &matching_resource->buffer, sizeof(RmtResourceDescriptionBuffer));
                        break;

                    case kRmtResourceTypeHeap:
                        memcpy(&resource_create_token.heap, &matching_resource->heap, sizeof(RmtResourceDescriptionHeap));
                        break;

                    default:
                        // Unexpected resource type.
                        RMT_ASSERT_FAIL("Re-binding is only supported for buffer, heap and command allocator resource types");
                        break;
                    }

                    // Create the resource.  Since the resource already exists, the Create operation will implicitly destroy it first.
                    error_code = RmtResourceListAddResourceCreate(&out_snapshot->resource_list, &resource_create_token);
                    RMT_ASSERT(error_code == kRmtOk);

                    if (!(current_token->resource_bind_token.is_system_memory && current_token->resource_bind_token.virtual_address == 0))
                    {
                        // Re-bind the resource to its new virtual memory allocation.
                        error_code = RmtResourceListAddResourceBind(
                            &out_snapshot->resource_list, &current_token->resource_bind_token, !data_set->flags.userdata_processed);
                        RMT_ASSERT(error_code == kRmtOk);
                    }
                }
            }

            RMT_ASSERT(error_code == kRmtOk);
            // RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        }
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
        // The byte offset of the token in the data stream is used to uniquely identify this allocation.
        // The offset is used rather than the virtual allocation address in case there are allocations/frees
        // and then another allocation is made with the same base address.
        uint64_t allocation_identifier = current_token->common.offset;

        error_code = RmtVirtualAllocationListAddAllocation(&out_snapshot->virtual_allocation_list,
                                                           current_token->common.timestamp,
                                                           current_token->virtual_allocate_token.virtual_address,
                                                           (int32_t)(current_token->virtual_allocate_token.size_in_bytes >> 12),
                                                           current_token->virtual_allocate_token.preference,
                                                           current_token->virtual_allocate_token.owner_type,
                                                           allocation_identifier);
        RMT_ASSERT(error_code == kRmtOk);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

        if (enable_aliased_resource_usage_sizes)
        {
            // Track virtual allocation for aliased resource size calculation.
            RmtMemoryAliasingCalculator* memory_aliasing_calculator = RmtMemoryAliasingCalculatorInstance();
            RMT_ASSERT(memory_aliasing_calculator != nullptr);
            memory_aliasing_calculator->CreateAllocation(allocation_identifier, current_token->virtual_allocate_token.size_in_bytes);
        }
    }
    break;

    case kRmtTokenTypeResourceUpdate:
    {
        if (!RmtResourceUserDataIsResourceImplicit(current_token->resource_update_token.resource_identifier))
        {
            // Attempt to match the Reource Update token to a previously created resource.
            // If a resource is found, update the usage flags.
            RmtResourceIdentifier id = current_token->resource_update_token.resource_identifier;
            if (unique_resource_id_lookup_map.find(id) != unique_resource_id_lookup_map.end())
            {
                RmtResource* resource = nullptr;
                error_code =
                    RmtResourceListGetResourceByResourceId(&out_snapshot->resource_list, unique_resource_id_lookup_map[id], (const RmtResource**)&resource);
                RMT_ASSERT(error_code == kRmtOk);
                if (resource != nullptr)
                {
                    const RmtResourceUsageType old_usage_type = RmtResourceGetUsageType(resource);
                    resource->buffer.usage_flags              = static_cast<uint32_t>(current_token->resource_update_token.after);
                    const RmtResourceUsageType new_usage_type = RmtResourceGetUsageType(resource);

                    // Decrease the resource usage count for the old usage type.
                    out_snapshot->resource_list.resource_usage_count[old_usage_type]--;

                    // Increase the resource usage count for the new usage type.
                    out_snapshot->resource_list.resource_usage_count[new_usage_type]++;

                    if (resource->bound_allocation != nullptr)
                    {
                        // Update the aliased resource usage sizes.
                        if ((enable_aliased_resource_usage_sizes) && (old_usage_type != RmtResourceUsageType::kRmtResourceUsageTypeHeap))
                        {
                            RmtMemoryAliasingCalculator* memory_aliasing_calculator = RmtMemoryAliasingCalculatorInstance();
                            RMT_ASSERT(memory_aliasing_calculator);
                            Allocation* aliased_resource_allocation =
                                memory_aliasing_calculator->FindAllocation(resource->bound_allocation->allocation_identifier);

                            if (aliased_resource_allocation != nullptr)
                            {
                                aliased_resource_allocation->DestroyResource(
                                    resource->address - resource->bound_allocation->base_address, resource->size_in_bytes, old_usage_type);
                                UpdateTotalResourceUsageAliasedSize(&(out_snapshot->resource_list), memory_aliasing_calculator);

                                aliased_resource_allocation->CreateResource(
                                    resource->address - resource->bound_allocation->base_address, resource->size_in_bytes, new_usage_type);
                                UpdateTotalResourceUsageAliasedSize(&(out_snapshot->resource_list), memory_aliasing_calculator);
                            }
                        }
                    }
                }
            }
        }
    }
    break;

    case kRmtTokenTypeResourceCreate:
    {
        if (!RmtResourceUserDataIsResourceImplicit(current_token->resource_create_token.resource_identifier))
        {
            error_code = RmtResourceListAddResourceCreate(&out_snapshot->resource_list, &current_token->resource_create_token);

            if (!data_set->flags.userdata_processed)
            {
                error_code = RmtResourceUserdataTrackResourceCreateToken(current_token->resource_create_token.original_resource_identifier,
                                                                         current_token->resource_create_token.resource_identifier,
                                                                         current_token->resource_create_token.resource_type,
                                                                         current_token->common.timestamp);
            }
            else
            {
                error_code = RmtResourceUserdataUpdateResourceName(
                    &out_snapshot->resource_list, current_token->resource_create_token.resource_identifier, out_snapshot->timestamp);
            }

            RMT_ASSERT(error_code == kRmtOk);
            // Note: The 32 bit driver resource ID may be reused.
            // In this case, the lookup map will be updated by replacing the old internal resource ID with
            // the one for this ResourceCreate token.
            unique_resource_id_lookup_map[current_token->resource_create_token.original_resource_identifier] =
                current_token->resource_create_token.resource_identifier;
        }

        //RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    }
    break;

    case kRmtTokenTypeResourceDestroy:
    {
        if (!RmtResourceUserDataIsResourceImplicit(current_token->resource_destroy_token.resource_identifier))
        {
            RmtResourceUserdataTrackResourceDestroyToken(current_token->resource_destroy_token.resource_identifier,
                                                         current_token->resource_destroy_token.common.timestamp);

            error_code = RmtResourceListAddResourceDestroy(&out_snapshot->resource_list, &current_token->resource_destroy_token);
            //RMT_ASSERT(error_code == kRmtOk);

            //RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
        }
    }
    break;

    default:
        break;
    }

    return kRmtOk;
}

// helper function that mirrors the .bak file to the original.
static RmtErrorCode CommitTemporaryFileEdits(RmtDataSet* data_set, bool remove_temporary)
{
    // The retry flag is set by the error reporter callback in response to a failed operation.
    bool retry = false;

    RMT_ASSERT(data_set);
    if (data_set->flags.read_only)
    {
        return kRmtOk;
    }

    RmtErrorCode result = kRmtErrorFileAccessFailed;

    if (data_set->file_handle != nullptr)
    {
        fflush((FILE*)data_set->file_handle);
        fclose((FILE*)data_set->file_handle);
        data_set->file_handle = NULL;
    }
    else if (data_set->flags.is_rdf_trace)
    {
        result = RmtRdfStreamClose();
    }

    if (remove_temporary)
    {
        do
        {
            const bool success = MoveTraceFile(data_set->temporary_file_path, data_set->file_path);

            if (success)
            {
                result = kRmtOk;
                retry  = false;
                break;
            }
            else
            {
                // If an error reporter callback exists, the response will indicate whether the failed operation should be tried again.
                if (data_set->error_report_func != nullptr)
                {
                    RmtErrorResponseCode response_code = RmtErrorResponseCode::RmtErrorResponseCodeNone;
                    data_set->error_report_func(data_set, kRmtErrorFileAccessFailed, &response_code);
                    retry = (response_code == RmtErrorResponseCode::RmtErrorResponseCodeRetry) ? true : false;
                }
            }
        } while (retry);
    }
    else
    {
        do
        {
            // For a mirror without remove, we need to recopy the temp
            bool success = MoveTraceFile(data_set->temporary_file_path, data_set->file_path);

            if (success == false)
            {
                // Failed to move backup trace file to original trace file.
                // The backup file is left for the user in case they want to recover any saved snapshots.

                // If an error reporter callback exists, the response will indicate whether the failed operation should be tried again.
                if (data_set->error_report_func != nullptr)
                {
                    RmtErrorResponseCode response_code = RmtErrorResponseCode::RmtErrorResponseCodeNone;
                    data_set->error_report_func(data_set, kRmtErrorFileAccessFailed, &response_code);
                    retry = response_code == RmtErrorResponseCode::RmtErrorResponseCodeRetry ? true : false;
                    if (response_code == RmtErrorResponseCode::RmtErrorResponseCodeIgnore)
                    {
                        data_set->flags.read_only = true;
                    }
                }
            }
            else
            {
                retry = false;
                do
                {
                    success = CopyTraceFile(data_set->file_path, data_set->temporary_file_path);
                    RMT_ASSERT(success);
                    if (success == false)
                    {
                        // If an error reporter callback exists, the response will indicate whether the failed operation should be tried again.
                        if (data_set->error_report_func != nullptr)
                        {
                            RmtErrorResponseCode response_code = RmtErrorResponseCode::RmtErrorResponseCodeNone;
                            data_set->error_report_func(data_set, kRmtErrorFileAccessFailed, &response_code);
                            retry = response_code == RmtErrorResponseCode::RmtErrorResponseCodeRetry ? true : false;
                            if (response_code == RmtErrorResponseCode::RmtErrorResponseCodeIgnore)
                            {
                                data_set->flags.read_only = true;
                            }
                        }
                    }
                } while (retry);
            }
        } while (retry);

        // The temporary file is removed when the trace is closed.
        // Only re-open the trace file if it is still in use (i.e., the remove_temporary flag is false).
        if (remove_temporary == false)
        {
            if (data_set->flags.is_rdf_trace)
            {
                // Re-open an RDF trace file.
                result = RmtRdfStreamOpen(data_set->temporary_file_path, data_set->flags.read_only);
            }
            else
            {
                // Re-open a legacy trace file.
                data_set->file_handle        = NULL;
                const bool    shareable_file = data_set->flags.read_only;
                const errno_t error_no       = OpenFile((FILE**)&data_set->file_handle, data_set->temporary_file_path, "rb+", true, shareable_file);

                RMT_ASSERT(data_set->file_handle);
                RMT_ASSERT(error_no == 0);

                if (error_no == 0)
                {
                    result = kRmtOk;
                }
            }
        }
    }

    return result;
}

// initialize the data set by reading the header chunks, and setting up the streams.
RmtErrorCode RmtDataSetInitialize(const char* path, RmtDataSet* data_set)
{
    RMT_ASSERT(path);
    RMT_ASSERT(data_set);
    RMT_RETURN_ON_ERROR(path, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);
    RmtErrorCode error_code = kRmtOk;

    // Initialize the Driver Overrides string.
    data_set->driver_overrides_json_text = nullptr;

    // copy the path
    const size_t path_length = strlen(path);
    memcpy(data_set->file_path, path, RMT_MINIMUM(RMT_MAXIMUM_FILE_PATH, path_length));
    memcpy(data_set->temporary_file_path, path, RMT_MINIMUM(RMT_MAXIMUM_FILE_PATH, path_length));

    data_set->file_handle                   = NULL;
    data_set->flags.read_only               = false;
    data_set->flags.is_rdf_trace            = false;
    data_set->flags.implicit_heap_detection = false;
    data_set->active_gpu                    = 0;
    data_set->error_report_func             = nullptr;

    errno_t error_no;
    bool    file_transfer_result = false;
    if (IsFileReadOnly(path) || IsCrashDumpFile(path))
    {
        data_set->flags.read_only = true;
    }
    else
    {
        strcat_s(data_set->temporary_file_path, sizeof(data_set->temporary_file_path), ".bak");
#ifdef _LINUX
        if (RmtTraceLoaderIsTraceAlreadyInUse(data_set->temporary_file_path))
        {
            data_set->flags.read_only = true;
        }
        else
        {
            // copy the entire input file to a temporary.
            file_transfer_result = CopyTraceFile(data_set->file_path, data_set->temporary_file_path);
        }
#else
        // copy the entire input file to a temporary.
        file_transfer_result = CopyTraceFile(data_set->file_path, data_set->temporary_file_path);
#endif  // #ifdef _LINUX
    }

    const char* file_access_mode = "rb+";
    const char* trace_file       = data_set->temporary_file_path;

    bool shareable_file = false;
    // The shareable_file flag is used to indicate whether the trace should be opened in exclusive mode
    // or shared mode.  The first instance of RMV opening a trace file will make a backup copy (.bak extension)
    // and open in exclusive mode.  Snapshot modifications are committed to the original .rmv file.  Subsequent
    // instances of RMV opening the same trace file will first try to open the .bak file with write access, but
    // will fail since the file is opened in exclusive mode by the first instance.  The fallback is to open the
    // original .rmv file in shared mode with the read only flag set (i.e., snapshot modifications are not saved).
    // Since the .rmv file for subsequent instances of RMV are opened in shared mode, the first instance is still
    // able to commit snapshot modifications.

    // If the trace file is a standard RMV trace and it doesn't have the read-only attribute set, do an additional check here
    // to see if another instance of RMV already has the file open (this would be the .bak file).
    if (!data_set->flags.read_only)
    {
        // Determine if the back up file or original file should be opened. If the backup file can't be opened with write privileges
        // (because another RMV instance already has opened it in exclusive access mode), set the read only flag and attempt to open
        // the original file in shared, read only mode.
        error_no = OpenFile((FILE**)&data_set->file_handle, trace_file, file_access_mode, true, shareable_file);
        if ((data_set->file_handle == nullptr) || error_no != 0)
        {
            // Set the read only flag so that opening the original file will be attempted.
            data_set->flags.read_only = true;

            // Set the shareable file flag so that the first instance will still be able to update snapshot changes.
            shareable_file = true;
        }
    }

    if (data_set->flags.read_only)
    {
        // File is read-only.  Attempt to just read the original rmv trace file.
        file_access_mode = "rb";
        trace_file       = data_set->file_path;
        error_no         = OpenFile((FILE**)&data_set->file_handle, trace_file, file_access_mode, true, shareable_file);
        if ((data_set->file_handle == nullptr) || error_no != 0)
        {
            error_code = kRmtErrorFileNotOpen;
        }
    }
    else if (!file_transfer_result)
    {
        // If the trace wasn't opened in read only mode and copying to the backup file failed then report an error.
        error_code = kRmtErrorFileAccessFailed;
    }

    if (data_set->file_handle != nullptr)
    {
        // Close the trace file so that it can be opened in RDF format.
        fclose(data_set->file_handle);
        data_set->file_handle = nullptr;
    }

    if (error_code == kRmtOk)
    {
        // Attempt to open the file in RDF format.  Open the original file in read only mode or the backup file in read/write mode.
        error_code = RmtRdfFileParserLoadRdf(trace_file, data_set);

        if (error_code != kRmtOk)
        {
            // Loading as an RDF file failed, attempt to open the trace file using the legacy format.
            error_no = OpenFile((FILE**)&data_set->file_handle, trace_file, file_access_mode, true, shareable_file);
            if ((data_set->file_handle != nullptr) && (error_no == 0))
            {
                error_code = kRmtOk;
            }
            else
            {
                error_code = kRmtErrorFileNotOpen;
            }

            if (error_code == kRmtOk)
            {
                // Get the size of the file.
                const size_t current_stream_offset = ftell((FILE*)data_set->file_handle);
                if (fseek((FILE*)data_set->file_handle, 0L, SEEK_END) != 0)
                {
                    RMT_ASSERT(errno == 0);
                }
                data_set->file_size_in_bytes = ftell((FILE*)data_set->file_handle);

                if (fseek((FILE*)data_set->file_handle, (int32_t)current_stream_offset, SEEK_SET))
                {
                    RMT_ASSERT(errno == 0);
                }

                // Check that the file is large enough to at least contain the RMT file header.
                if (data_set->file_size_in_bytes < sizeof(RmtFileHeader))
                {
                    error_code = kRmtErrorFileNotOpen;
                }
            }

            if (error_code == kRmtOk)
            {
                // Parse legacy trace.
                error_code = ParseChunks(data_set);
                if (error_code == kRmtOk)
                {
                    RmtLegacySnapshotWriter* snapshot_writer = new RmtLegacySnapshotWriter(data_set);
                    data_set->snapshot_writer_handle         = reinterpret_cast<RmtSnapshotWriterHandle*>(snapshot_writer);
                }
            }
        }

        // Vega and older GPUs are no longer supported.
        if (error_code == kRmtOk && data_set->system_info.pcie_family_id < kFamilyNavi)
        {
            error_code = kRmtErrorTraceFileNotSupported;
        }

        if (error_code == kRmtOk)
        {
            CheckForCpuHostApertureSupport(data_set);
            CheckForSAMSupport(data_set);
            data_set->flags.local_heap_only = data_set->flags.sam_enabled || data_set->flags.cpu_host_aperture_enabled;

            // Construct the data profile for subsequent data parsing.
            data_set->flags.contains_correlation_tokens = false;
            error_code                                  = BuildDataProfile(data_set);
            RMT_ASSERT(error_code == kRmtOk);
        }
    }

    if (error_code != kRmtOk)
    {
        // An error occurred.  Do final cleanup.
        if ((FILE*)data_set->file_handle != nullptr)
        {
            fclose((FILE*)data_set->file_handle);
            data_set->file_handle = NULL;
        }

        DestroySnapshotWriter(data_set);
        data_set->flags.is_rdf_trace = false;

        if (!data_set->flags.read_only)
        {
            DeleteTemporaryFile(trace_file);
        }
        else
        {
            data_set->flags.read_only = false;
        }
    }

    return error_code;
}

RmtErrorCode RmtDataSetSetErrorReporter(RmtDataSet* data_set, const RmtDataSetErrorReportFunc reporter_function)
{
    RMT_ASSERT(data_set != nullptr);
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);

    data_set->error_report_func = reporter_function;

    return kRmtOk;
}

// destroy the data set.
RmtErrorCode RmtDataSetDestroy(RmtDataSet* data_set)
{
    data_set->flags.userdata_processed = false;

    if (data_set->file_handle != nullptr)
    {
        // Flush writes and close the handle.
        fflush((FILE*)data_set->file_handle);
        fclose((FILE*)data_set->file_handle);
        data_set->file_handle = NULL;
    }

    CommitTemporaryFileEdits(data_set, true);

    if (data_set->flags.is_rdf_trace)
    {
        RmtRdfFileParserDestroyAllDataStreams();
        RmtRdfStreamClose();
        DestroySnapshotWriter(data_set);
        data_set->flags.is_rdf_trace = false;
    }
    data_set->stream_count = 0;

    data_set->file_handle = NULL;

    // Delete the array of unbound memory regions for all virtual allocations.
    for (int32_t i = 0; i < data_set->virtual_allocation_list.allocation_count; i++)
    {
        delete[] data_set->virtual_allocation_list.allocation_details[i].unbound_memory_regions;
        data_set->virtual_allocation_list.allocation_details[i].unbound_memory_region_count = 0;
    }
    data_set->virtual_allocation_list.allocation_count = 0;

    // Delete the Driver Overrides data if it exists.
    delete data_set->driver_overrides_json_text;
    data_set->driver_overrides_json_text = nullptr;

    free(data_set->resource_id_map_allocator);

    RmtResourceUserDataCleanup();
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
        // For Resource Usage Virtual Size timeline type, aliased sizing should be enabled
        // (disabled for all other timeline types).
        RMT_ASSERT(current_snapshot->resource_list.enable_aliased_resource_usage_sizes);

        for (int32_t current_resource_index = 0; current_resource_index < kRmtResourceUsageTypeCount; ++current_resource_index)
        {
            const uint64_t resource_size_for_usage_type = current_snapshot->resource_list.total_resource_usage_aliased_size[current_resource_index];

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

    // Reset the cancel flag.
    data_set->flags.cancel_background_task_flag = false;

    // Allocate temporary snapshot.
    RmtDataSnapshot* temp_snapshot = (RmtDataSnapshot*)PerformAllocation(data_set, sizeof(RmtDataSnapshot), alignof(RmtDataSnapshot));
    RMT_ASSERT(temp_snapshot);
    RMT_RETURN_ON_ERROR(temp_snapshot, kRmtErrorOutOfMemory);
    RmtErrorCode error_code =
        AllocateMemoryForSnapshot(data_set, temp_snapshot, timeline_type == RmtDataTimelineType::kRmtDataTimelineTypeResourceUsageVirtualSize);
    RMT_ASSERT(error_code == kRmtOk);
    if (error_code != kRmtOk)
    {
        PerformFree(data_set, temp_snapshot);
        return error_code;
    }

    temp_snapshot->maximum_physical_memory_in_bytes = RmtDataSetGetTotalVideoMemoryInBytes(data_set);

    // initialize the page table.
    error_code = RmtPageTableInitialize(&temp_snapshot->page_table, data_set->segment_info, data_set->segment_info_count, data_set->target_process_id);
    RMT_ASSERT(error_code == kRmtOk);

    // initialize the process map.
    error_code = RmtProcessMapInitialize(&temp_snapshot->process_map);
    RMT_ASSERT(error_code == kRmtOk);

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

    RmtStreamMergerReset(&data_set->stream_merger, data_set->file_handle);

    // For each timeline generated, clear the driver resource ID to internal resource ID
    // lookup map.  As ResourceCreate tokens are processed, the mapping to driver resource IDs
    // will be updated to reflect the current state.
    unique_resource_id_lookup_map.clear();

    // if the heap has something there, then add it.
    int32_t last_value_index = -1;
    while (!RmtStreamMergerIsEmpty(&data_set->stream_merger) && !RmtDataSetIsBackgroundTaskCancelled(data_set))
    {
        // grab the next token from the heap.
        RmtToken current_token = {};
        error_code             = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
        RMT_ASSERT(error_code == kRmtOk);
        if (error_code != kRmtOk)
        {
            PerformFree(data_set, temp_snapshot);
            return error_code;
        }

        // Update the temporary snapshot with the RMT token.
        error_code = ProcessTokenForSnapshot(data_set, &current_token, temp_snapshot);
        RMT_ASSERT(error_code == kRmtOk);
        if (error_code != kRmtOk)
        {
            PerformFree(data_set, temp_snapshot);
            return error_code;
        }

        // set the timestamp for the current snapshot
        temp_snapshot->timestamp = current_token.common.timestamp;

        // Generate whatever series values we need for current timeline type from the snapshot.
        last_value_index = UpdateSeriesValuesFromCurrentSnapshot(temp_snapshot, timeline_type, last_value_index, out_timeline);
    }

    if (!data_set->flags.userdata_processed)
    {
        RmtResourceUserdataProcessEvents(data_set->flags.contains_correlation_tokens);
        data_set->flags.userdata_processed = true;
    }

    // clean up temporary structures we allocated to construct the timeline.
    if (timeline_type == RmtDataTimelineType::kRmtDataTimelineTypeResourceUsageVirtualSize)
    {
        RmtMemoryAliasingCalculatorCleanup();
    }
    RmtDataSnapshotDestroy(temp_snapshot);
    PerformFree(data_set, temp_snapshot);
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

        // Increment the non-heap count since the heap has been converted to a buffer resource.
        current_virtual_allocation->non_heap_resource_count++;
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

// A structure that holds the start and end offsets for a region of memory.
struct RegionOffsets
{
    uint64_t start_offset;
    uint64_t end_offset;
};

// Merge overlapped resources into memory regions.
static RmtErrorCode MergeResourceMemoryRegions(const RmtVirtualAllocation* virtual_allocation, std::vector<RmtMemoryRegion>& out_bound_regions)
{
    RMT_ASSERT(virtual_allocation != nullptr);
    RMT_RETURN_ON_ERROR(virtual_allocation != nullptr, kRmtErrorInvalidPointer);

    out_bound_regions.clear();
    if (virtual_allocation->non_heap_resource_count == 0)
    {
        return kRmtOk;
    }

    // Populate the memory region list from the resources bound to this virtual allocation.
    std::vector<RegionOffsets> bound_memory_regions;
    for (int32_t current_resource_index = 0; current_resource_index < virtual_allocation->resource_count; ++current_resource_index)
    {
        const RmtGpuAddress allocation_base_address = virtual_allocation->base_address;
        const RmtResource*  current_resource        = virtual_allocation->resources[current_resource_index];

        // Skip over Heap type resources.
        if (current_resource->resource_type != RmtResourceType::kRmtResourceTypeHeap)
        {
            bound_memory_regions.push_back(RegionOffsets{current_resource->address - allocation_base_address,
                                                         (current_resource->address - allocation_base_address) + current_resource->size_in_bytes});
        }
    }

    // Sort the bound memory regions by starting offsets.
    std::sort(
        bound_memory_regions.begin(), bound_memory_regions.end(), [](RegionOffsets& lhs, RegionOffsets& rhs) { return lhs.start_offset < rhs.start_offset; });

    // Process the bound memory regions, looking for gaps between the regions.  Combine the regions if they overlap.
    std::vector<RegionOffsets>::iterator next_bound_memory_iterator  = bound_memory_regions.begin();
    RegionOffsets                        current_bound_memory_region = *(next_bound_memory_iterator);
    next_bound_memory_iterator++;
    while (next_bound_memory_iterator != bound_memory_regions.end())
    {
        if (current_bound_memory_region.end_offset > next_bound_memory_iterator->start_offset)
        {
            // Extend the current memory region so that it is merged with the next memory region.
            current_bound_memory_region.end_offset = RMT_MAXIMUM(current_bound_memory_region.end_offset, next_bound_memory_iterator->end_offset);
        }
        else
        {
            // There is a break between the current bound memory region and the next one.  Add this memory region to the output vector.
            out_bound_regions.push_back(
                RmtMemoryRegion{current_bound_memory_region.start_offset, current_bound_memory_region.end_offset - current_bound_memory_region.start_offset});
            current_bound_memory_region = *(next_bound_memory_iterator);
        }
        next_bound_memory_iterator++;
    }

    // Add the last bound memory region.
    out_bound_regions.push_back(
        RmtMemoryRegion{current_bound_memory_region.start_offset, current_bound_memory_region.end_offset - current_bound_memory_region.start_offset});

    return kRmtOk;
}

// add unbound resources to the virtual allocation, there should be one of these for every gap in the VA
// address space.
static RmtErrorCode SnapshotGeneratorAddUnboundResources(RmtDataSnapshot* snapshot)
{
    RMT_ASSERT(snapshot);

    for (int32_t current_virtual_allocation_index = 0; current_virtual_allocation_index < snapshot->virtual_allocation_list.allocation_count;
         ++current_virtual_allocation_index)
    {
        std::vector<RmtMemoryRegion> bound_regions;
        RmtVirtualAllocation*        virtual_allocation = &snapshot->virtual_allocation_list.allocation_details[current_virtual_allocation_index];

        // Merge aliased resources into a list of ranges.
        MergeResourceMemoryRegions(virtual_allocation, bound_regions);

        // Use the list of bound resource memory regions to find the unbound gaps in the virtual allocation.
        std::vector<RmtMemoryRegion> unbound_regions;  ///< The list of unbound memory regions.
        uint64_t                     allocation_size_in_bytes = RmtGetAllocationSizeInBytes(virtual_allocation->size_in_4kb_page,
                                                                        kRmtPageSize4Kb);  ///< The virtual allocation size in bytes.
        if (bound_regions.size() < 1)
        {
            // Create an unbound region covering the entire virtual allocation.
            RmtMemoryRegion unbound_region;
            unbound_region.offset = 0;
            unbound_region.size   = allocation_size_in_bytes;
            unbound_regions.push_back(unbound_region);
        }
        else
        {
            // Find the memory region gaps.

            size_t          last_bound_region_index = bound_regions.size() - 1;  ///< The index of the last bound memory region.
            RmtMemoryRegion previous_bound_region({bound_regions[0].offset, bound_regions[0].size});

            if (previous_bound_region.offset > 0)
            {
                // Create an unbound region before the first bound region.
                RmtMemoryRegion unbound_region;
                unbound_region.offset = 0;
                unbound_region.size   = previous_bound_region.offset;
                unbound_regions.push_back(unbound_region);
            }

            for (size_t i = 1; i < bound_regions.size(); i++)
            {
                const RmtMemoryRegion current_bound_region             = bound_regions[i];
                size_t                previous_bound_region_end_offset = previous_bound_region.offset + previous_bound_region.size;
                RmtMemoryRegion       unbound_region;
                unbound_region.offset = previous_bound_region_end_offset;
                unbound_region.size   = current_bound_region.offset - previous_bound_region_end_offset;
                unbound_regions.push_back(unbound_region);
                previous_bound_region = current_bound_region;
            }

            const RmtMemoryRegion last_bound_region = bound_regions[last_bound_region_index];
            if ((last_bound_region_index == 0) && (last_bound_region.offset > 0))
            {
                // Add an unbound region from the start of the virtual allocation to the first bound region.
                RmtMemoryRegion unbound_region;
                unbound_region.offset = 0;
                unbound_region.size   = last_bound_region.offset;
                unbound_regions.push_back(unbound_region);
            }

            if ((last_bound_region.offset + last_bound_region.size) < allocation_size_in_bytes)
            {
                // Create an unbound region between the end of the last bound region and the end of the virtual allocation.
                RmtMemoryRegion unbound_region;
                unbound_region.offset = last_bound_region.offset + last_bound_region.size;
                unbound_region.size   = allocation_size_in_bytes - unbound_region.offset;
                unbound_regions.push_back(unbound_region);
            }
        }

        // Update the list of unbound memory regions for the virtual allocation object.
        size_t unbound_region_count                     = unbound_regions.size();
        virtual_allocation->unbound_memory_region_count = 0;
        if (unbound_region_count > 0)
        {
            virtual_allocation->unbound_memory_regions = new RmtMemoryRegion[unbound_region_count];
            for (size_t i = 0; i < unbound_region_count; i++)
            {
                if (unbound_regions[i].size > 0)
                {
                    virtual_allocation->unbound_memory_regions[virtual_allocation->unbound_memory_region_count] = unbound_regions[i];
                    virtual_allocation->unbound_memory_region_count++;
                }
            }
        }
    }

    return kRmtOk;
}

// Calculate the size after aliasing for each resource.
static RmtErrorCode SnapshotGeneratorCalculateAliasedResourceSizes(RmtDataSnapshot* snapshot)
{
    uint64_t resource_usage_mask = kRmtResourceUsageTypeBitMaskAll;
    return RmtVirtualAllocationListUpdateAliasedResourceSizes(&(snapshot->virtual_allocation_list), &snapshot->resource_list, resource_usage_mask);
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

/// @brief Update the names of virtual allocations if a named heap resource is bound to it.
///
/// @param [in/out]  out_snapshot                   A pointer to the snapshot to be updated.
///
/// @retval
/// kRmtOk                                  The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                 The operation failed because <c><i>out_snapshot</i></c> was <c><i>NULL</i></c>.
///
static RmtErrorCode SnapshotGeneratorUpdateNamedHeaps(RmtDataSnapshot* out_snapshot)
{
    RMT_RETURN_ON_ERROR(out_snapshot, kRmtErrorInvalidPointer);

    const int32_t allocation_count = out_snapshot->virtual_allocation_list.allocation_count;
    for (int32_t allocation_index = 0; allocation_index < allocation_count; allocation_index++)
    {
        // Get a reference to the allocation object so that its name can be updated.
        RmtVirtualAllocation& allocation          = out_snapshot->virtual_allocation_list.allocation_details[allocation_index];
        const int             heap_resource_count = allocation.resource_count - allocation.non_heap_resource_count;
        RMT_ASSERT(heap_resource_count <= 1);

        const RmtResource* first_heap_resource          = nullptr;
        const RmtResource* first_non_heap_resource      = nullptr;
        const char*        first_heap_resource_name     = nullptr;
        const char*        first_non_heap_resource_name = nullptr;

        for (int resource_index = 0; resource_index < allocation.resource_count; resource_index++)
        {
            const RmtResource* resource = allocation.resources[resource_index];
            if (resource->resource_type == RmtResourceType::kRmtResourceTypeHeap)
            {
                first_heap_resource               = resource;
                RmtErrorCode resource_name_result = RmtResourceUserdataGetResourceNameAtTimestamp(
                    first_heap_resource->identifier, resource->create_time, out_snapshot->timestamp, &first_heap_resource_name);
                if ((resource_name_result == kRmtOk) && (first_non_heap_resource != nullptr))
                {
                    break;
                }
            }
            else
            {
                first_non_heap_resource           = resource;
                RmtErrorCode resource_name_result = RmtResourceUserdataGetResourceNameAtTimestamp(
                    first_non_heap_resource->identifier, first_non_heap_resource->create_time, out_snapshot->timestamp, &first_non_heap_resource_name);
                if ((resource_name_result == kRmtOk) && (first_heap_resource != nullptr))
                {
                    break;
                }
            }
        }

        if (heap_resource_count == 1 && first_heap_resource_name != nullptr)
        {
            allocation.name = first_heap_resource_name;
        }
        else if (allocation.non_heap_resource_count == 1 && first_non_heap_resource_name != NULL && first_non_heap_resource->address == allocation.base_address)
        {
            allocation.name = first_non_heap_resource_name;
        }
        else
        {
            allocation.name = nullptr;
        }
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
    RmtErrorCode error_code = AllocateMemoryForSnapshot(data_set, out_snapshot, false);

    out_snapshot->maximum_physical_memory_in_bytes = RmtDataSetGetTotalVideoMemoryInBytes(data_set);

    // initialize the page table.
    error_code = RmtPageTableInitialize(
        &out_snapshot->page_table, out_snapshot->data_set->segment_info, out_snapshot->data_set->segment_info_count, out_snapshot->data_set->target_process_id);
    RMT_ASSERT(error_code == kRmtOk);

    // Reset the RMT stream parsers ready to load the data.
    RmtStreamMergerReset(&data_set->stream_merger, data_set->file_handle);

    // process all the tokens
    while (!RmtStreamMergerIsEmpty(&data_set->stream_merger))
    {
        // grab the next token from the heap.
        RmtToken current_token;
        error_code = RmtStreamMergerAdvance(&data_set->stream_merger, data_set->flags.local_heap_only, &current_token);
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

    if (!data_set->flags.implicit_heap_detection)
    {
        // If the heap_type flag is missing from MarkImplicitResource tokens (as is the case with older traces),
        // convert solitary heaps in an allocation into buffers.
        SnapshotGeneratorConvertHeapsToBuffers(out_snapshot);
    }

    SnapshotGeneratorAddResourcePointers(out_snapshot);
    SnapshotGeneratorCompactVirtualAllocations(out_snapshot);
    SnapshotGeneratorAddUnboundResources(out_snapshot);
    SnapshotGeneratorCalculateAliasedResourceSizes(out_snapshot);
    SnapshotGeneratorCalculateSummary(out_snapshot);
    SnapshotGeneratorCalculateCommitType(out_snapshot);
    SnapshotGeneratorAllocateRegionStack(out_snapshot);
    SnapshotGeneratorCalculateSnapshotPointSummary(out_snapshot, snapshot_point);
    SnapshotGeneratorUpdateNamedHeaps(out_snapshot);

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

    if (!data_set->flags.read_only)
    {
        // Add the minimum timestamp to the snapshot timestamp so that rebase on load works.
        const uint64_t timestamp_with_offset = timestamp + data_set->stream_merger.minimum_start_timestamp;

        // Update the snapshots in the file using which ever trace file format has been loaded.
        SnapshotWriterFromHandle(data_set->snapshot_writer_handle)->Add(name, timestamp_with_offset, static_cast<int16_t>(snapshot_index));
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

    return CommitTemporaryFileEdits(data_set, false);
}

// guts of removing a snapshot without destroying the cached object, lets this code be shared with rename.
static void RemoveSnapshot(RmtDataSet* data_set, const int32_t snapshot_index, RmtDataSnapshot* open_snapshot)
{
    // Clear the snapshot name.  This marks it for deletion.
    data_set->snapshots[snapshot_index].name[0] = '\0';

    if (!data_set->flags.read_only)
    {
        // Update the snapshots in file using which ever trace file format has been loaded.
        SnapshotWriterFromHandle(data_set->snapshot_writer_handle)->Remove(static_cast<int16_t>(snapshot_index));
    }

    // remove the snapshot from the list of snapshot points in the dataset.
    const int32_t last_snapshot_index = data_set->snapshot_count - 1;
    memcpy(&data_set->snapshots[snapshot_index], &data_set->snapshots[last_snapshot_index], sizeof(RmtSnapshotPoint));

    // fix up the snapshot point in the open snapshot (if it needs moving).
    if (open_snapshot)
    {
        if (open_snapshot->snapshot_point == &data_set->snapshots[last_snapshot_index])
        {
            open_snapshot->snapshot_point = &data_set->snapshots[snapshot_index];
        }
    }

    data_set->snapshot_count--;
}

// remove a snapshot from the data set.
RmtErrorCode RmtDataSetRemoveSnapshot(RmtDataSet* data_set, const int32_t snapshot_index, RmtDataSnapshot* open_snapshot)
{
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(snapshot_index < data_set->snapshot_count, kRmtErrorIndexOutOfRange);

    RmtSnapshotPoint* snapshot_point = &data_set->snapshots[snapshot_index];
    RmtDataSnapshotDestroy(snapshot_point->cached_snapshot);

    RemoveSnapshot(data_set, snapshot_index, open_snapshot);

    return CommitTemporaryFileEdits(data_set, false);
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
    RemoveSnapshot(data_set, snapshot_index, nullptr);

    return CommitTemporaryFileEdits(data_set, false);
}

int32_t RmtDataSetGetSeriesIndexForTimestamp(RmtDataSet* data_set, uint64_t timestamp)
{
    RMT_UNUSED(data_set);

    return (int32_t)(timestamp / 3000);
}

uint64_t RmtDataSetGetTotalVideoMemoryInBytes(const RmtDataSet* data_set)
{
    return data_set->segment_info[kRmtHeapTypeLocal].size + data_set->segment_info[kRmtHeapTypeInvisible].size;
}

void RmtDataSetCancelBackgroundTask(RmtDataSet* data_set)
{
    RMT_ASSERT(data_set != nullptr);
    data_set->flags.cancel_background_task_flag = true;
}

bool RmtDataSetIsBackgroundTaskCancelled(const RmtDataSet* data_set)
{
    RMT_ASSERT(data_set != nullptr);

    bool result = false;

    if (data_set->flags.cancel_background_task_flag)
    {
        result = true;
    }

    return result;
}

RmtErrorCode RmtDataSetCopyDriverOverridesString(RmtDataSet* data_set, const char* driver_overrides_string, size_t length)
{
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(driver_overrides_string, kRmtErrorInvalidPointer);
    RmtErrorCode result = kRmtOk;

    delete data_set->driver_overrides_json_text;

    if ((driver_overrides_string == nullptr) || (length == 0))
    {
        data_set->driver_overrides_json_text = nullptr;
    }
    else
    {
        data_set->driver_overrides_json_text = new (std::nothrow) char[length + 1];
        if (data_set->driver_overrides_json_text != nullptr)
        {
            memcpy(data_set->driver_overrides_json_text, driver_overrides_string, length);
            data_set->driver_overrides_json_text[length] = '\0';
        }
        else
        {
            result = kRmtErrorOutOfMemory;
        }
    }

    return result;
}

char* RmtDataSetGetDriverOverridesString(const RmtDataSet* data_set)
{
    RMT_RETURN_ON_ERROR(data_set, nullptr);
    return data_set->driver_overrides_json_text;
}
