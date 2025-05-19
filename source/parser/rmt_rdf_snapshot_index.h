//=============================================================================
// Copyright (c) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Definition of the RFD Snapshot Index chunk.
///
/// This class is responsible for reading the Snapshot Index chunk from the
/// RDF file and storing the data in a vector.  The Snapshot Index chunk contains
/// a list of the active Snapshot Info chunk indices.  Since chunks in the RDF file
/// cannot be deleted or change size, new Snapshot Index chunks are appended when
/// a snapshot is added, deleted or renamed.
//=============================================================================

#ifndef RMV_PARSER_RMT_RDF_SNAPSHOT_INDEX_H_
#define RMV_PARSER_RMT_RDF_SNAPSHOT_INDEX_H_

#include "rdf/rdf/inc/amdrdf.h"

#include "rmt_error.h"

constexpr uint16_t kMaxSnapshotIndex = 1024;

// The class definition of the Snapshot Index chunk.
class RmtRdfSnapshotIndex
{
public:
    // The header for the Snapshot Index chunk.
    struct TraceSnapShotIndexHeader
    {
        uint16_t index_count;  ///< The number of snapshot indices in the payload following the header.
        uint32_t version;      ///< The version number of the Snapshot Index chunk.
    };

public:
    /// Constructor for the Snapshot Index chunk.
    RmtRdfSnapshotIndex();

    /// Destructor for the Snapshot Index chunk.
    ~RmtRdfSnapshotIndex();

    /// Load the last Snapshot Index chunk.
    ///
    /// @param [in]  chunk_file                      A pointer to the chunk file.
    ///
    /// @retval
    /// kRmtOk                                       The operation completed successfully.
    /// @retval
    /// kRmtErrorInvalidPointer                      Invalid <c><i>chunk_file</i></c>.
    /// @retval
    /// kRmtMalformedData                            Failed to retrieve the chunk data from the RDF file.
    /// @retVal
    /// kRmtEof                                      The chunk was not found in the RDF file.
    RmtErrorCode LoadLastChunk(rdfChunkFile* chunk_file);

    /// Retrieves the data loaded from the chunk.
    ///
    /// @param [out]  out_indices                    A pointer to the list of indices for the active Snapshot Info chunks.
    ///
    /// @retval                                      If valid data was loaded, returns true.  Otherwise returns false.
    bool GetChunkData(const std::vector<uint16_t>** out_indices) const;

    /// Return the Snapshot Info chunk identifier.
    ///
    /// @retval                                     A pointer to the character string containing the identifier.
    static const char* ChunkIdentifier();

    /// Return the Version of the Snapshot Index chunk.
    ///
    /// @retval                                     The chunk version.
    static int32_t ChunkVersion();

private:
    std::vector<uint16_t> indices_;              ///< The payload data loaded from the RDF chunk (A list of Snapshot Info chunk indices).
    bool                  is_valid_chunk_data_;  ///< If true, indicates the chunk data is valid.  Otherwise set to false.
};
#endif  // #ifndef RMV_PARSER_RMT_RDF_SNAPSHOT_INDEX_H_
