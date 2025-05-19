//=============================================================================
// Copyright (c) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Definition of the RFD Snapshot Info chunk.
///
/// This class is responsible for reading the Snapshot Info chunk from the
/// RDF file and storing the data in a structure.  Since chunks in the RDF file
/// cannot be deleted or change size, new chunks are appended when a snapshot
/// is renamed.  If a snapshot is deleted, the snapshot chunk is left in the RDF
/// file and a new Snapshot Index chunk is appended which has the index of the
/// deleted snapshot removed.
//=============================================================================

#ifndef RMV_PARSER_RMT_RDF_SNAPSHOT_INFO_H_
#define RMV_PARSER_RMT_RDF_SNAPSHOT_INFO_H_

#include "rdf/rdf/inc/amdrdf.h"

#include "rmt_error.h"

constexpr uint32_t kMaxSnapshotNameLen = 128;  // Snapshot name length including null terminator.

// The class definition of the Snapshot Info Chunk.
class RmtRdfSnapshotInfo
{
public:
    // The payload data of the chunk.
    struct TraceSnapShot
    {
        char     name[kMaxSnapshotNameLen];  ///< The name of the snapshot.
        uint64_t snapshot_point;             ///< 64bit timestamp of the snapshot.
        uint32_t name_length;                ///< The size in bytes of the snapshot name.
        uint32_t version;                    ///< The version of the Snapshot Info Chunk.
    };

public:
    /// Constructor for the Snapshot Info chunk.
    RmtRdfSnapshotInfo();

    /// Destructor for the Snapshot Info chunk.
    ~RmtRdfSnapshotInfo();

    /// Load Snapshot Info data from the RDF file.
    ///
    /// @param [in]  chunk_file                      A pointer to the chunk file.
    /// @param [in]  chunk_index                     The index of the chunk to load.
    ///
    /// @retval
    /// kRmtOk                                       The operation completed successfully.
    /// @retval
    /// kRmtErrorInvalidPointer                      Invalid <c><i>chunk_file</i></c>.
    /// @retval
    /// kRmtMalformedData                            Failed to retrieve the chunk data from the RDF file.
    /// @retVal
    /// kRmtEof                                      The chunk was not found in the RDF file.
    RmtErrorCode LoadChunk(rdfChunkFile* chunk_file, int chunk_index);

    /// Retrieves a pointer to the snapshot info loaded from the chunk.
    ///
    /// @param [out] out_snapshot_info               A pointer to the chunk data which was loaded.
    ///
    /// @retval                                      Returns true if the chunk data is valid, otherwise returns false.
    bool GetChunkData(const RmtRdfSnapshotInfo::TraceSnapShot** out_snapshot_info) const;

    /// Retrieves number of snapshot info chunks in the trace.
    ///
    /// @param [in]  chunk_file                      A pointer to the chunk file.
    /// @param [in]  out_chunk_count                 A pointer to the variable that receives the chunk count.
    ///
    /// @retval
    /// kRmtOk                                       The operation completed successfully.
    /// kRmtErrorInvalidPointer                      Invalid </i></c>chunk_file</i></c> or </i></c>out_chunk_count</i></c> pointer.
    /// kRmtMalformedData                            Failed to retrieve the number of chunks in the RDF file.
    RmtErrorCode GetChunkCount(rdfChunkFile* chunk_file, uint16_t* out_chunk_count);

    /// Return the Snapshot Info chunk identifier.
    ///
    /// @retval                                     A pointer to the character string containing the identifier.
    static const char* ChunkIdentifier();

    /// Return the Version of the Snapshot Info chunk.
    ///
    /// @retval                                     The chunk version.
    static int32_t ChunkVersion();

private:
    TraceSnapShot snapshot_data_;        ///< The payload data loaded from the RDF chunk.
    bool          is_valid_chunk_data_;  ///< If true, indicates the chunk data is valid.  Otherwise set to false.
};
#endif  // #ifndef RMV_PARSER_RMT_RDF_SNAPSHOT_INFO_H_
