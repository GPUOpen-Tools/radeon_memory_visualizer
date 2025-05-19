//=============================================================================
// Copyright (c) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of functions for working with the RDF file format.
//=============================================================================

#include "rmt_rdf_file_parser.h"

#include <map>
#include <vector>

#include "rdf/rdf/inc/amdrdf.h"
#include "system_info_utils/source/driver_overrides_reader.h"
#include "system_info_utils/source/system_info_reader.h"

#include "rmt_assert.h"
#include "rmt_error.h"
#include "rmt_format.h"
#include "rmt_print.h"
#include "rmt_rdf_data_stream.h"
#include "rmt_rdf_dd_event_info.h"
#include "rmt_rdf_gpu_mem_segment_info.h"
#include "rmt_rdf_snapshot_index.h"
#include "rmt_rdf_snapshot_info.h"
#include "rmt_rdf_snapshot_writer.h"
#include "rmt_types.h"

#ifndef _WIN32
#include "linux/safe_crt.h"
#endif

// Lookup map for data stream objects.
static std::map<int32_t, RmtRdfDataStream*> data_stream_map;

// The global stream object for the RDF trace file.
static rdfStream* global_data_stream = nullptr;

// Store a snapshot chunk in the data set.
static RmtErrorCode StoreSnapshotToDataSet(rdfChunkFile& chunk_file, RmtRdfSnapshotInfo& snapshot_info_chunk, const uint16_t chunk_index, RmtDataSet& data_set)
{
    RmtErrorCode result = snapshot_info_chunk.LoadChunk(&chunk_file, chunk_index);
    if (result == kRmtOk)
    {
        result = kRmtErrorMalformedData;
        const RmtRdfSnapshotInfo::TraceSnapShot* snapshot_data;
        if (snapshot_info_chunk.GetChunkData(&snapshot_data))
        {
            RMT_ASSERT(snapshot_data->name_length > 0);

            const int16_t snapshot_index = static_cast<int16_t>(data_set.snapshot_count);
            strncpy_s(
                data_set.snapshots[snapshot_index].name, sizeof(data_set.snapshots[snapshot_index].name), snapshot_data->name, snapshot_data->name_length);
            data_set.snapshots[snapshot_index].timestamp   = snapshot_data->snapshot_point;
            data_set.snapshots[snapshot_index].file_offset = 0;  // Not used for RDF.  Reset to 0.
            data_set.snapshots[snapshot_index].chunk_index = chunk_index;
            data_set.snapshot_count++;
            result = kRmtOk;
        }
    }

    return result;
}

// Load active snapshot chunks from the trace file and store them in the data set.
static RmtErrorCode LoadSnapshotChunks(rdfChunkFile* chunk_file, RmtDataSet* data_set)
{
    RmtRdfSnapshotInfo  snapshot_info_chunk;
    RmtRdfSnapshotIndex snapshot_index_chunk;
    RmtErrorCode        result = snapshot_index_chunk.LoadLastChunk(chunk_file);

    if (result == kRmtOk)
    {
        // Loading the last RmvSnapshotIndex chunk succeeded.
        const std::vector<uint16_t>* indices;
        if (snapshot_index_chunk.GetChunkData(&indices))
        {
            // Load each of the Snapshot Info chunks using the indices found in the Snapshot Index chunk and copy to the data set.
            for (uint16_t snapshot_info_chunk_index : *indices)
            {
                if (snapshot_info_chunk_index != kEmptySnapshotIndexChunk)
                {
                    StoreSnapshotToDataSet(*chunk_file, snapshot_info_chunk, snapshot_info_chunk_index, *data_set);
                }
            }
        }
    }
    else
    {
        result = kRmtOk;
        // If an RmvSnapshotIndex chunk is not present than load all of the RmvSnapshotData chunks (the LoadLastChunk() call above returns kRmtEof in this case).
        uint16_t snapshot_info_chunk_count = 0;
        snapshot_info_chunk.GetChunkCount(chunk_file, &snapshot_info_chunk_count);
        for (uint16_t snapshot_info_chunk_index = 0; snapshot_info_chunk_index < snapshot_info_chunk_count; snapshot_info_chunk_index++)
        {
            result = StoreSnapshotToDataSet(*chunk_file, snapshot_info_chunk, snapshot_info_chunk_index, *data_set);
            if (result != kRmtOk)
            {
                break;
            }
        }
    }
    return result;
}

// Load the Driver Overrides chunk.
static RmtErrorCode LoadDriverOverridesChunk(rdfChunkFile* chunk_file, RmtDataSet* data_set)
{
    RMT_RETURN_ON_ERROR(chunk_file != nullptr, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(data_set != nullptr, kRmtErrorInvalidPointer);
    RmtErrorCode result = kRmtErrorMalformedData;

    // Attempt to load Driver Overrides chunk.
    std::string json_text;
    if (driver_overrides_utils::DriverOverridesReader::Parse(chunk_file, json_text))
    {
        result = RmtDataSetCopyDriverOverridesString(data_set, json_text.c_str(), json_text.size());
    }

    return result;
}

// Load the GPU Memory Segment chunk.
static RmtErrorCode LoadSegmentChunk(rdfChunkFile* chunk_file, RmtDataSet* data_set)
{
    RMT_RETURN_ON_ERROR(chunk_file != nullptr, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(data_set != nullptr, kRmtErrorInvalidPointer);

    RmtErrorCode result                = kRmtErrorMalformedData;
    int          rdf_result            = kRmtRdfResultFailure;
    int          contains_chunk_result = rdfChunkFileContainsChunk(chunk_file, RmtRdfGpuMemSegmentInfo::ChunkIdentifier(), 0, &rdf_result);
    RMT_ASSERT(contains_chunk_result == rdfResult::rdfResultOk);
    RMT_UNUSED(contains_chunk_result);

    if (rdf_result == kRmtRdfResultSuccess)
    {
        RmtRdfGpuMemSegmentInfo heap_info;
        const bool              load_success = heap_info.LoadChunk(chunk_file);
        if (load_success)
        {
            int32_t& count = data_set->segment_info_count;
            count          = 0;

            // Fill out heap info.
            for (unsigned int index = 0; index < RmtHeapType::kRmtHeapTypeCount; index++)
            {
                RmtRdfGpuMemSegmentInfo::RmtRdfTraceHeapInfo data = {};
                if (heap_info.GetChunkData(index, data))
                {
                    data_set->segment_info[count].base_address = data.physical_base_address;
                    data_set->segment_info[count].heap_type    = static_cast<RmtHeapType>(data.type);

                    data_set->segment_info[count].index = 0;
                    data_set->segment_info[count].size  = data.size;
                    count++;
                }
                else
                {
                    break;
                }
            }
            result = kRmtOk;
        }
    }

    return result;
}

// Get the RDF Data Stream object from a parser.
RmtRdfDataStream* GetRdfDataStreamFromParser(const RmtParser* rmt_parser)
{
    RMT_RETURN_ON_ERROR(rmt_parser != nullptr, nullptr);

    RmtRdfDataStream* data_stream = nullptr;
    if (data_stream_map.find(rmt_parser->stream_index) != data_stream_map.end())
    {
        data_stream = data_stream_map[rmt_parser->stream_index];
    }

    return data_stream;
}

// Delete all instantiated RDF data streams for a trace.
RmtErrorCode RmtRdfFileParserDestroyAllDataStreams()
{
    for (auto& data_stream : data_stream_map)
    {
        delete data_stream.second;
    }
    data_stream_map.clear();

    return kRmtOk;
}

// Callback function to fill a buffer with the next chunk from a data stream.
static RmtErrorCode RdfDataStreamGetNextChunk(const RmtParser* rmt_parser, const size_t start_offset, void** out_rmt_buffer, size_t* out_rmt_buffer_size)
{
    RMT_RETURN_ON_ERROR(rmt_parser != nullptr, kRmtErrorInvalidPointer);
    RmtRdfDataStream* data_stream = GetRdfDataStreamFromParser(rmt_parser);
    RMT_RETURN_ON_ERROR(data_stream != nullptr, kRmtErrorInvalidPointer);

    RmtErrorCode result = kRmtErrorOutOfMemory;

    // If there are more chunks in the queue, load the next one.
    if (data_stream->GetRemainingUnprocessedChunks() > 0)
    {
        if ((data_stream->LoadNextChunk(start_offset)))
        {
            // If LoadNextChunk() succeeds, update the buffer pointer and buffer size.
            *out_rmt_buffer      = data_stream->GetBuffer();
            *out_rmt_buffer_size = data_stream->GetBufferFillSize();
            result               = kRmtOk;
        }
        else
        {
            *out_rmt_buffer      = nullptr;
            *out_rmt_buffer_size = 0;

            // Report EOF to indicate all data has been parsed.
            result = kRmtEof;
        }
    }

    return result;
}

// Callback function to reset the RDF data stream to the first chunk in the queue.
static RmtErrorCode RdfDataStreamReset(const RmtParser* rmt_parser)
{
    RMT_RETURN_ON_ERROR(rmt_parser, kRmtErrorInvalidPointer);

    RmtRdfDataStream* data_stream = GetRdfDataStreamFromParser(rmt_parser);
    RMT_RETURN_ON_ERROR(data_stream != nullptr, kRmtErrorInvalidPointer);

    data_stream->Reset();
    return kRmtOk;
}

// Group chunks by stream index and add them to the stream's chunk queue.
static RmtErrorCode QueueDataStreamChunks(rdfChunkFile* chunk_file, RmtDataSet* data_set, const char* path)
{
    RMT_RETURN_ON_ERROR(chunk_file, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(path, kRmtErrorInvalidPointer);

    RmtErrorCode result     = kRmtErrorMalformedData;
    const auto   identifier = RmtRdfDataStream::ChunkIdentifier();

    // Delete any existing data streams before parsing the file.
    RmtRdfFileParserDestroyAllDataStreams();
    data_set->stream_count = 0;

    // Make sure the file contains at least one data stream chunk.
    int rdf_result            = kRmtRdfResultFailure;
    int contains_chunk_result = rdfChunkFileContainsChunk(chunk_file, RmtRdfGpuMemSegmentInfo::ChunkIdentifier(), 0, &rdf_result);
    RMT_ASSERT(contains_chunk_result == rdfResult::rdfResultOk);
    RMT_UNUSED(contains_chunk_result);

    if (rdf_result == kRmtRdfResultSuccess)
    {
        std::int64_t chunk_count = 0;
        rdfChunkFileGetChunkCount(chunk_file, identifier, &chunk_count);
        if (chunk_count > 0)
        {
            result = kRmtOk;  // Assume success.

            // Group the data stream chunks by stream index.
            for (uint32_t chunk_index = 0; chunk_index < chunk_count; chunk_index++)
            {
                int64_t header_size = 0;
                rdfChunkFileGetChunkHeaderSize(chunk_file, identifier, chunk_index, &header_size);

                if (header_size > 0)
                {
                    RMT_ASSERT(header_size == sizeof(RmtRdfDataStream::RmtRdfTraceStreamHeader));

                    // Read the chunk's header to determine which stream it belongs to.
                    RmtRdfDataStream::RmtRdfTraceStreamHeader stream_header;
                    if (rdfChunkFileReadChunkHeader(chunk_file, identifier, chunk_index, &stream_header) == rdfResult::rdfResultOk)
                    {
                        int32_t stream_index = stream_header.stream_index;
                        if ((stream_index < 0) || (stream_index >= RMT_MAXIMUM_STREAMS))
                        {
                            // The stream index is out of range.
                            RMT_ASSERT(false);
                            result = kRmtErrorIndexOutOfRange;
                            break;
                        }

                        if (stream_header.total_data_size > 0)
                        {
                            if (data_stream_map.find(stream_index) == data_stream_map.end())
                            {
                                // Create a new data stream object if this stream index hasn't been seen before.
                                RmtRdfDataStream* data_stream = new RmtRdfDataStream(path,
                                                                                     stream_index,
                                                                                     stream_header.process_id,
                                                                                     stream_header.thread_id,
                                                                                     stream_header.rdf_major_version,
                                                                                     stream_header.rdf_minor_version,
                                                                                     &global_data_stream);
                                data_stream_map[stream_index] = data_stream;
                            }

                            // Add the chunk to the appropriate data stream.
                            int64_t chunk_size;
                            rdfChunkFileGetChunkDataSize(chunk_file, identifier, chunk_index, &chunk_size);
                            {
                                data_stream_map[stream_index]->AddChunk(chunk_index, chunk_size);
                            }
                        }
                    }
                }
            }

            // Initialize a parser for each stream and pre-load the first chunk.
            for (auto& data_stream_item : data_stream_map)
            {
                const int32_t     stream_index = data_stream_item.first;
                RmtRdfDataStream* data_stream  = data_stream_item.second;

                // Preload the first chunk for this stream.
                if (data_stream->LoadNextChunk(0) == false)
                {
                    result = kRmtErrorMalformedData;
                    break;
                }

                RmtParser&         parser             = data_set->streams[stream_index];
                const RmtErrorCode parser_init_result = RmtParserInitialize(&parser,
                                                                            nullptr,
                                                                            0,
                                                                            static_cast<uint32_t>(data_stream->GetStreamSize()),
                                                                            data_stream->GetBuffer(),
                                                                            static_cast<uint32_t>(data_stream->GetBufferFillSize()),
                                                                            data_stream->GetMajorVersion(),
                                                                            data_stream->GetMinorVersion(),
                                                                            stream_index,
                                                                            data_stream->GetProcessId(),
                                                                            data_stream->GetThreadId());

                if (parser_init_result != kRmtOk)
                {
                    result = kRmtErrorMalformedData;
                    break;
                }
                else
                {
                    // Set the parser callback functions for RDF data stream processing.
                    RmtParserSetCallbacks(&parser, RdfDataStreamGetNextChunk, RdfDataStreamReset);

                    // Set the target process.
                    if (data_stream->GetProcessId() != 0 && data_set->target_process_id == 0)
                    {
                        data_set->target_process_id = data_stream->GetProcessId();
                    }

                    // Increment the number of streams.
                    data_set->stream_count++;
                }
            }
        }
    }

    if (result != kRmtOk)
    {
        // If any of the steps fail, delete the data streams.
        RmtRdfFileParserDestroyAllDataStreams();
    }

    return result;
}

static RmtErrorCode LoadSystemInfoChunk(rdfChunkFile* chunk_file, RmtDataSet* data_set)
{
    RMT_RETURN_ON_ERROR(chunk_file, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);

    RmtErrorCode                  result      = kRmtEof;
    system_info_utils::SystemInfo system_info = {};

    bool system_info_result = false;
    system_info_result      = system_info_utils::SystemInfoReader::Parse(chunk_file, system_info);

    if (system_info_result)
    {
        const uint32_t& active_gpu = data_set->active_gpu;
        if (active_gpu < system_info.gpus.size())
        {
            data_set->system_info.pcie_family_id   = system_info.gpus[active_gpu].asic.id_info.family;
            data_set->system_info.pcie_revision_id = system_info.gpus[active_gpu].asic.id_info.revision;
            data_set->system_info.device_id        = system_info.gpus[active_gpu].asic.id_info.device;

            data_set->system_info.minimum_engine_clock = static_cast<int32_t>(system_info.gpus[active_gpu].asic.engine_clock_hz.min / 1000000);
            data_set->system_info.maximum_engine_clock = static_cast<int32_t>(system_info.gpus[active_gpu].asic.engine_clock_hz.max / 1000000);

            data_set->system_info.minimum_memory_clock = static_cast<int32_t>(system_info.gpus[active_gpu].memory.mem_clock_hz.min / 1000000);
            data_set->system_info.maximum_memory_clock = static_cast<int32_t>(system_info.gpus[active_gpu].memory.mem_clock_hz.max / 1000000);

            data_set->system_info.memory_bus_width = system_info.gpus[active_gpu].memory.bus_bit_width;
            // Convert memory bandwidth from bytes/sec to MB/s. Conversion is deliberately (1024*1024 = 1048576).
            data_set->system_info.memory_bandwidth            = static_cast<int32_t>(system_info.gpus[active_gpu].memory.bandwidth / 1048576);
            data_set->system_info.memory_operations_per_clock = system_info.gpus[active_gpu].memory.mem_ops_per_clock;

            strncpy_s(data_set->system_info.video_memory_type_name,
                      sizeof(data_set->system_info.video_memory_type_name),
                      system_info.gpus[active_gpu].memory.type.c_str(),
                      sizeof(data_set->system_info.video_memory_type_name) - 1);

            strncpy_s(data_set->system_info.name,
                      sizeof(data_set->system_info.name),
                      system_info.gpus[active_gpu].name.c_str(),
                      sizeof(data_set->system_info.name) - 1);
        }

        // For now, assume CPU 0 is the active one.
        const uint32_t active_cpu = 0;
        if (active_gpu < system_info.cpus.size())
        {
            strncpy_s(data_set->system_info.cpu_name, RMT_MAX_CPU_NAME_LENGTH, system_info.cpus[active_cpu].name.c_str(), RMT_MAX_CPU_NAME_LENGTH - 1);

            data_set->system_info.cpu_max_clock_speed = system_info.cpus[active_cpu].max_clock_speed;
            data_set->system_info.num_physical_cores  = system_info.cpus[active_cpu].num_physical_cores;
            data_set->system_info.num_logical_cores   = system_info.cpus[active_cpu].num_logical_cores;
        }

        data_set->system_info.system_physical_memory_size = system_info.os.memory.physical;

        strncpy_s(data_set->system_info.driver_packaging_version_name,
                  RMT_MAX_DRIVER_PACKAGING_VERSION_NAME_LENGTH,
                  system_info.driver.packaging_version.c_str(),
                  RMT_MAX_DRIVER_PACKAGING_VERSION_NAME_LENGTH - 1);

        strncpy_s(data_set->system_info.driver_software_version_name,
                  RMT_MAX_DRIVER_SOFTWARE_VERSION_NAME_LENGTH,
                  system_info.driver.software_version.c_str(),
                  RMT_MAX_DRIVER_SOFTWARE_VERSION_NAME_LENGTH - 1);

        strncpy_s(data_set->system_info.system_memory_type_name,
                  RMT_MAX_MEMORY_TYPE_NAME_LENGTH,
                  system_info.os.memory.type.c_str(),
                  RMT_MAX_MEMORY_TYPE_NAME_LENGTH - 1);

        strncpy_s(data_set->system_info.os_name, RMT_MAX_OS_NAME_LENGTH, system_info.os.name.c_str(), RMT_MAX_OS_NAME_LENGTH - 1);

        result = kRmtOk;
    }

    return result;
}

RmtErrorCode RmtRdfFileParserLoadRdf(const char* path, RmtDataSet* data_set)
{
    RMT_RETURN_ON_ERROR(path, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(data_set, kRmtErrorInvalidPointer);

    RmtErrorCode error_code = RmtRdfStreamOpen(path, data_set->flags.read_only);

    if (error_code == kRmtOk)
    {
        rdfChunkFile* chunk_file = nullptr;
        error_code               = kRmtErrorMalformedData;
        if ((rdfChunkFileOpenStream(global_data_stream, &chunk_file) == rdfResult::rdfResultOk) && (chunk_file != nullptr))
        {
            // The System Info chunk is currently optional, but flagged with an assert as a warning.
            error_code = LoadSystemInfoChunk(chunk_file, data_set);
            RMT_ASSERT(error_code == kRmtOk);

            // Load the Driver Overrides chunk if it is present.
            error_code = LoadDriverOverridesChunk(chunk_file, data_set);
            RMT_ASSERT(error_code == kRmtOk);

            if (error_code == kRmtOk)
            {
                error_code = LoadSegmentChunk(chunk_file, data_set);
            }

            if (error_code == kRmtOk)
            {
                error_code = QueueDataStreamChunks(chunk_file, data_set, path);
            }

            if (error_code == kRmtOk)
            {
                error_code = LoadSnapshotChunks(chunk_file, data_set);
                RMT_ASSERT(error_code == kRmtOk);
            }

            int chunk_close_result = rdfChunkFileClose(&chunk_file);
            RMT_ASSERT(chunk_close_result == rdfResult::rdfResultOk);
            RMT_UNUSED(chunk_close_result);
        }
    }

    if (error_code == kRmtOk)
    {
        // Initialize the token heap for k-way merging.
        error_code = RmtStreamMergerInitialize(&data_set->stream_merger, data_set->streams, data_set->stream_count, nullptr);
        RMT_ASSERT(error_code == kRmtOk);
        if (error_code == kRmtOk)
        {
            // Set the flag indicating that the file is an RDF trace.
            data_set->flags.is_rdf_trace = true;

            RmtRdfSnapshotWriter* snapshot_writer = new RmtRdfSnapshotWriter(data_set, &global_data_stream);
            data_set->snapshot_writer_handle      = reinterpret_cast<RmtSnapshotWriterHandle*>(snapshot_writer);

            // Rebase any snapshot times to be relative to the minimum timestamp.
            for (int32_t current_snapshot_index = 0; current_snapshot_index < data_set->snapshot_count; ++current_snapshot_index)
            {
                data_set->snapshots[current_snapshot_index].timestamp -= data_set->stream_merger.minimum_start_timestamp;
            }
        }
    }

    if (error_code != kRmtOk)
    {
        // Only close the RDF stream on error.  It is intentionally left open otherwise to prevent other applications from opening the same file with write privileges.
        // Other applications can open the file in read only mode.  When the user closes the trace, the RDF stream is closed by the destroy data set process.
        RmtRdfStreamClose();
    }

    return error_code;
}

/// Open the global RDF stream.
RmtErrorCode RmtRdfStreamOpen(const char* path, const bool read_only)
{
    RMT_RETURN_ON_ERROR(path, kRmtErrorInvalidPointer);
    RMT_ASSERT(global_data_stream == nullptr);

    RmtErrorCode error_code = kRmtErrorMalformedData;

    if (global_data_stream != nullptr)
    {
        RmtRdfStreamClose();
    }

    rdfStreamAccess access_mode = rdfStreamAccess::rdfStreamAccessRead;
    if (!read_only)
    {
        access_mode = rdfStreamAccess::rdfStreamAccessReadWrite;
    }
    rdfStreamFromFileCreateInfo stream_create_info{path, access_mode, rdfFileMode::rdfFileModeOpen, read_only};

    rdfResult rdf_result = rdfResult::rdfResultOk;
    rdf_result           = static_cast<rdfResult>(rdfStreamFromFile(&stream_create_info, &global_data_stream));

    if ((rdf_result == rdfResult::rdfResultOk) && (global_data_stream != nullptr))
    {
        error_code = kRmtOk;
    }

    return error_code;
}

/// Close the global RDF stream.
RmtErrorCode RmtRdfStreamClose()
{
    RmtErrorCode result = kRmtOk;
    if (global_data_stream != nullptr)
    {
        int rdf_result = rdfStreamClose(&global_data_stream);
        RMT_ASSERT(rdf_result == rdfResult::rdfResultOk);
        if (rdf_result != rdfResult::rdfResultOk)
        {
            result = kRmtErrorMalformedData;
        }
        global_data_stream = nullptr;
    }

    return result;
}
