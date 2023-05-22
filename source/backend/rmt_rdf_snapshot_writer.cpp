//=============================================================================
// Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the class that handles writing snapshot data to RDF trace files.
//=============================================================================

#include "rmt_rdf_snapshot_writer.h"

#include "rmt_data_set.h"
#include "rmt_error.h"
#include "rmt_rdf_snapshot_index.h"
#include "rmt_rdf_snapshot_info.h"
#include "rmt_rdf_file_parser.h"
#include "rmt_snapshot_writer.h"

#include "rdf/rdf/inc/amdrdf.h"

#include <string>

#ifndef _WIN32
#include <linux/safe_crt.h>
#endif

RmtRdfSnapshotWriter::RmtRdfSnapshotWriter(RmtDataSet* data_set, rdfStream** stream)
    : RmtSnapshotWriter(data_set)
    , stream_(stream)
{
}

RmtRdfSnapshotWriter::~RmtRdfSnapshotWriter()
{
}

// Append chunks to the trace file for a new snapshot.
RmtErrorCode RmtRdfSnapshotWriter::Add(const char* name, const uint64_t timestamp, const uint16_t new_snapshot_index) const
{
    RMT_RETURN_ON_ERROR(data_set_ != nullptr, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(stream_ != nullptr, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(*stream_ != nullptr, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(name != nullptr, kRmtErrorInvalidPointer);

    RmtErrorCode result = kRmtErrorMalformedData;

    rdfChunkFileWriterCreateInfo chunk_writer_create_info{*stream_, true};
    rdfChunkFileWriter*          chunk_file_writer          = nullptr;
    rdfChunkFile*                snapshot_index_chunk_file  = nullptr;
    int64_t                      snapshot_index_chunk_count = 0;

    rdfResult rdf_result = static_cast<rdfResult>(rdfChunkFileOpenStream(*stream_, &snapshot_index_chunk_file));
    if (rdf_result == rdfResult::rdfResultOk)
    {
        rdf_result =
            static_cast<rdfResult>(rdfChunkFileGetChunkCount(snapshot_index_chunk_file, RmtRdfSnapshotIndex::ChunkIdentifier(), &snapshot_index_chunk_count));
        rdfChunkFileClose(&snapshot_index_chunk_file);
    }

    // Open the snapshot info chunk.
    if (rdf_result == rdfResult::rdfResultOk)
    {
        rdf_result = static_cast<rdfResult>(rdfChunkFileWriterCreate2(&chunk_writer_create_info, &chunk_file_writer));
    }

    if (rdf_result == rdfResult::rdfResultOk)
    {
        // Append snapshot data for the snapshot being added.
        rdfChunkCreateInfo chunk_create_info;
        chunk_create_info.compression = rdfCompression::rdfCompressionNone;
        chunk_create_info.headerSize  = 0;
        chunk_create_info.pHeader     = nullptr;
        chunk_create_info.version     = RmtRdfSnapshotInfo::ChunkVersion();
        strcpy_s(chunk_create_info.identifier, sizeof(chunk_create_info.identifier), RmtRdfSnapshotInfo::ChunkIdentifier());

        // Populate the snapshot data structure for the snapshot info chunk.
        RmtRdfSnapshotInfo::TraceSnapShot snapshot_data;
        snapshot_data.name_length = static_cast<int32_t>(strlen(name));
        strncpy_s(snapshot_data.name, kMaxSnapshotNameLen - 1, name, snapshot_data.name_length);
        snapshot_data.snapshot_point = timestamp;
        snapshot_data.version        = RmtRdfSnapshotInfo::ChunkVersion();

        int chunk_index = 0;  // The index of the chunk being appended to the trace file.

        // Append the snapshot info chunk.
        rdf_result =
            static_cast<rdfResult>(rdfChunkFileWriterWriteChunk(chunk_file_writer, &chunk_create_info, sizeof(snapshot_data), &snapshot_data, &chunk_index));

        // Close the chunk file for the snapshot info.
        rdfChunkFileWriterDestroy(&chunk_file_writer);

        // Update the chunk index for the snapshot in the data set.
        data_set_->snapshots[new_snapshot_index].chunk_index = static_cast<int16_t>(chunk_index);

        // If a snapshot index chunk is already in the RDF file, a new updated one needs to be appended to the file.
        if ((snapshot_index_chunk_count > 0) && (rdf_result == rdfResult::rdfResultOk))
        {
            // build the list of indices for the active snapshots.
            std::vector<int16_t> indices;
            std::string          chunk_indices_string;
            indices.reserve(data_set_->snapshot_count);
            for (int16_t snapshot_index = 0; snapshot_index < data_set_->snapshot_count; snapshot_index++)
            {
                // Skip snapshots with an empty name.
                if (data_set_->snapshots[snapshot_index].name[0] != '\0')
                {
                    indices.push_back(data_set_->snapshots[snapshot_index].chunk_index);
                }
            }

            // Populate the snapshot index chunk header
            RmtRdfSnapshotIndex::TraceSnapShotIndexHeader header;
            header.index_count = static_cast<int16_t>(indices.size());
            header.version     = RmtRdfSnapshotIndex::ChunkVersion();

            // Populate the chunk create info structure for the snapshot index chunk.
            chunk_create_info.compression = rdfCompression::rdfCompressionNone;
            chunk_create_info.headerSize  = sizeof(header);
            chunk_create_info.pHeader     = &header;
            chunk_create_info.version     = RmtRdfSnapshotIndex::ChunkVersion();
            memcpy(chunk_create_info.identifier, RmtRdfSnapshotIndex::ChunkIdentifier(), RDF_IDENTIFIER_SIZE);

            // Open the snapshot index chunk.
            rdf_result = static_cast<rdfResult>(rdfChunkFileWriterCreate2(&chunk_writer_create_info, &chunk_file_writer));

            chunk_index = 0;  // Receives the index of the new snapshot info chunk.

            // Append a new snapshot index chunk.
            rdf_result = static_cast<rdfResult>(
                rdfChunkFileWriterWriteChunk(chunk_file_writer, &chunk_create_info, indices.size() * sizeof(int16_t), indices.data(), &chunk_index));

            // Close the chunk file for the snapshot index.
            rdfChunkFileWriterDestroy(&chunk_file_writer);
        }
    }

    if (rdf_result == rdfResult::rdfResultOk)
    {
        result = kRmtOk;
    }

    return result;
}

// Append an updated snapshot index chunk with the active snapshots.
RmtErrorCode RmtRdfSnapshotWriter::Remove(const uint16_t removed_snapshot_index) const
{
    RMT_UNUSED(removed_snapshot_index);

    RMT_RETURN_ON_ERROR(data_set_ != nullptr, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(stream_ != nullptr, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(*stream_ != nullptr, kRmtErrorInvalidPointer);

    RmtErrorCode result = kRmtErrorMalformedData;

    rdfChunkFileWriterCreateInfo chunk_writer_create_info{*stream_, true};
    rdfChunkFileWriter*          chunk_file = nullptr;
    rdfResult                    rdf_result = static_cast<rdfResult>(rdfChunkFileWriterCreate2(&chunk_writer_create_info, &chunk_file));

    if (rdf_result == rdfResult::rdfResultOk)
    {
        std::vector<int16_t> indices;
        std::string          chunk_indices_string;

        // NOTE: The snapshot count isn't decremented until after the remove operation completes so the count will be one here if the last snapshot is being deleted.
        if (data_set_->snapshot_count > 1)
        {
            indices.reserve(data_set_->snapshot_count);
            for (int16_t snapshot_index = 0; snapshot_index < data_set_->snapshot_count; snapshot_index++)
            {
                // Skip snapshots with an empty name.
                if (data_set_->snapshots[snapshot_index].name[0] != '\0')
                {
                    indices.push_back(data_set_->snapshots[snapshot_index].chunk_index);
                }
            }
        }
        else
        {
            indices.push_back(kEmptySnapshotIndexChunk);
        }

        // Populate the header for the snapshot index chunk.
        RmtRdfSnapshotIndex::TraceSnapShotIndexHeader header;
        header.index_count = static_cast<int16_t>(indices.size());
        header.version     = RmtRdfSnapshotIndex::ChunkVersion();

        // Populate the structure to create the snapshot index chunk.
        rdfChunkCreateInfo chunk_create_info;
        chunk_create_info.compression = rdfCompression::rdfCompressionNone;
        chunk_create_info.headerSize  = sizeof(header);
        chunk_create_info.pHeader     = &header;
        chunk_create_info.version     = RmtRdfSnapshotIndex::ChunkVersion();
        memcpy(chunk_create_info.identifier, RmtRdfSnapshotIndex::ChunkIdentifier(), RDF_IDENTIFIER_SIZE);

        // Open the snapshot index chunk.
        rdf_result = static_cast<rdfResult>(rdfChunkFileWriterCreate2(&chunk_writer_create_info, &chunk_file));

        int chunk_index = 0;  // Receives the index of the new snapshot index chunk.

        // Append a new snapshot index chunk.
        rdf_result = static_cast<rdfResult>(
            rdfChunkFileWriterWriteChunk(chunk_file, &chunk_create_info, indices.size() * sizeof(int16_t), indices.data(), &chunk_index));
        if (rdf_result == rdfResult::rdfResultOk)
        {
            result = kRmtOk;
        }

        // Close the chunk file for the snapshot index.
        rdfChunkFileWriterDestroy(&chunk_file);
    }

    return result;
}