//=============================================================================
// Copyright (c) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the RFD Snapshot Info chunk. Loads the chunk data into a buffer.
//=============================================================================

#include "rmt_rdf_snapshot_index.h"

#include "rmt_assert.h"

#include "rdf/rdf/inc/amdrdf.h"

#include <vector>
#include <string>

#ifndef _WIN32
#include <linux/safe_crt.h>
#endif

// The identifier for the Snapshot Index Chunk.
constexpr const char* kSnapshotIndexChunkId = "RmvSnapshotIndex";

// Version of the Snapshot Info Chunk.
const int kSnapshotIndexChunkVersion = 1;

RmtRdfSnapshotIndex::RmtRdfSnapshotIndex()
    : is_valid_chunk_data_(false)
{
}

RmtRdfSnapshotIndex::~RmtRdfSnapshotIndex()
{
}

// Load the last Snapshot Index chunk.
// Note: Only the last chunk index is valid.  All others should be ignored.
RmtErrorCode RmtRdfSnapshotIndex::LoadLastChunk(rdfChunkFile* chunk_file)
{
    RMT_RETURN_ON_ERROR(chunk_file != nullptr, kRmtErrorInvalidPointer);

    RMT_ASSERT(chunk_file != nullptr);

    RmtErrorCode result                = kRmtErrorMalformedData;
    const auto   identifier            = RmtRdfSnapshotIndex::ChunkIdentifier();
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
            int chunk_index = static_cast<int>(chunk_count - 1);

            // Load the header for the Snapshot Index chunk.
            RmtRdfSnapshotIndex::TraceSnapShotIndexHeader header;
            rdfChunkFileReadChunkHeader(chunk_file, identifier, chunk_index, &header);

            // Adjust the size of the indices to match the index count in the Snapshot Index chunk header.
            indices_.resize(header.index_count);

            // Load the indices from the RDF file.
            rdf_result = rdfChunkFileReadChunkData(chunk_file, kSnapshotIndexChunkId, chunk_index, indices_.data());

            if (rdf_result == rdfResult::rdfResultOk)
            {
                result               = kRmtOk;
                is_valid_chunk_data_ = true;
            }
        }
    }

    return result;
}

// Retrieve the data loaded from the chunk.
bool RmtRdfSnapshotIndex::GetChunkData(const std::vector<uint16_t>** out_indices) const
{
    if (is_valid_chunk_data_)
    {
        *out_indices = &indices_;
    }
    return is_valid_chunk_data_;
}

const char* RmtRdfSnapshotIndex::ChunkIdentifier()
{
    return kSnapshotIndexChunkId;
}

int32_t RmtRdfSnapshotIndex::ChunkVersion()
{
    return kSnapshotIndexChunkVersion;
}
