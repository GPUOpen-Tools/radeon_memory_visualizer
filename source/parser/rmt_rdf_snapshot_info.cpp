//=============================================================================
// Copyright (c) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the RFD Snapshot Info chunk. Loads the chunk data into a buffer.
//=============================================================================

#include "rmt_rdf_snapshot_info.h"

#include <string>
#include <vector>

#include "rdf/rdf/inc/amdrdf.h"

#ifndef _WIN32
#include "linux/safe_crt.h"
#endif

#include "rmt_assert.h"

// The identifier for the Snapshot Info Chunk.
constexpr const char* kSnapshotDataChunkId = "RmvSnapshotData";

// Version of the Snapshot Info Chunk.
const int kSnapshotDataChunkVersion = 1;

RmtRdfSnapshotInfo::RmtRdfSnapshotInfo()
    : snapshot_data_{}
    , is_valid_chunk_data_(false)
{
}

RmtRdfSnapshotInfo::~RmtRdfSnapshotInfo()
{
}

// Load the Snapshot info chunk.
RmtErrorCode RmtRdfSnapshotInfo::LoadChunk(rdfChunkFile* chunk_file, int chunk_index)
{
    RMT_RETURN_ON_ERROR(chunk_file != nullptr, kRmtErrorInvalidPointer);

    RMT_ASSERT(chunk_file != nullptr);

    RmtErrorCode result                = kRmtErrorMalformedData;
    const auto   identifier            = RmtRdfSnapshotInfo::ChunkIdentifier();
    int          contains_chunk_result = 0;
    int          rdf_result            = rdfChunkFileContainsChunk(chunk_file, identifier, 0, &contains_chunk_result);
    RMT_ASSERT(rdf_result == rdfResult::rdfResultOk);
    RMT_RETURN_ON_ERROR(rdf_result == rdfResult::rdfResultOk, kRmtErrorMalformedData);

    result = kRmtEof;
    if (contains_chunk_result != 0)
    {
        result                   = kRmtEof;
        std::int64_t chunk_count = 0;
        rdfChunkFileGetChunkCount(chunk_file, identifier, &chunk_count);
        if (chunk_count > 0)
        {
            int64_t payload_size = 0;
            rdf_result           = rdfChunkFileGetChunkDataSize(chunk_file, identifier, 0, &payload_size);

            if ((rdf_result == rdfResult::rdfResultOk) && (payload_size > 0))
            {
                rdf_result = rdfChunkFileReadChunkData(chunk_file, identifier, chunk_index, &snapshot_data_);
            }

            if (rdf_result == rdfResult::rdfResultOk)
            {
                result = kRmtOk;
            }
        }
    }

    if (result == kRmtOk)
    {
        is_valid_chunk_data_ = true;
    }

    return result;
}

// Retrieve the number of RmvSnapshotData chunks in the RDF file.
RmtErrorCode RmtRdfSnapshotInfo::GetChunkCount(rdfChunkFile* chunk_file, uint16_t* chunk_count)
{
    RMT_RETURN_ON_ERROR(chunk_file != nullptr, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(chunk_count != nullptr, kRmtErrorInvalidPointer);
    RmtErrorCode result = kRmtErrorMalformedData;

    std::int64_t rdf_chunk_count = 0;
    const int    rdf_result      = rdfChunkFileGetChunkCount(chunk_file, ChunkIdentifier(), &rdf_chunk_count);
    if (rdf_result == rdfResult::rdfResultOk)
    {
        *chunk_count = static_cast<uint16_t>(rdf_chunk_count);
        result       = kRmtOk;
    }

    return result;
}

// Retrieve the data loaded from the chunk.
bool RmtRdfSnapshotInfo::GetChunkData(const RmtRdfSnapshotInfo::TraceSnapShot** out_chunk_data) const
{
    if (is_valid_chunk_data_)
    {
        *out_chunk_data = &snapshot_data_;
    }
    return is_valid_chunk_data_;
}

const char* RmtRdfSnapshotInfo::ChunkIdentifier()
{
    return kSnapshotDataChunkId;
}

int32_t RmtRdfSnapshotInfo::ChunkVersion()
{
    return kSnapshotDataChunkVersion;
}
