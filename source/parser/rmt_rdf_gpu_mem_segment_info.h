//=============================================================================
// Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of the RDF GPU Memory Segment (heap) Information chunk parser.
//=============================================================================

#ifndef RMV_PARSER_RMT_RDF_GPU_MEM_SEGMENT_INFO_H_
#define RMV_PARSER_RMT_RDF_GPU_MEM_SEGMENT_INFO_H_

#include "rmt_types.h"

#include "rdf/rdf/inc/amdrdf.h"

/// The Parser for the RFD GPU Memory Segment Info Chunk.
class RmtRdfGpuMemSegmentInfo
{
public:
    /// Constructor.
    RmtRdfGpuMemSegmentInfo();

    /// Destructor.
    ~RmtRdfGpuMemSegmentInfo();

    /// Load an RFD Heap Info chunk.
    /// @param [in] chunk_file                      The <c><i>rdfChunkFile</i></c> structure pointer to load from.
    ///
    /// @retval                                     If successful, returns true.  Otherwise returns false.
    ///
    bool LoadChunk(rdfChunkFile* chunk_file);

    /// Return the Heap Info chunk identifier.
    ///
    /// @retval                                     A pointer to the character string containing the identifier.
    ///
    static const char* ChunkIdentifier();

    /// The Chunk Data.
    struct RmtRdfTraceHeapInfo
    {
        RmtHeapType type;                   ///< The heap type.
        uint64_t    physical_base_address;  ///< The base address of the segment.
        uint64_t    size;                   ///< The size of the segment (in bytes).
    };

    /// Retrieve the heap info chunk data.  Note: LoadChunk() must be called first.
    ///
    /// @param [in]     index                       The heap index of the data to retrieve.
    /// @param [out]    out_chunk_data              A reference to the chunk data.
    ///
    /// @retval                                     If the chunk data is valid, true is returned.  Otherwise, returns false.
    ///
    bool GetChunkData(const unsigned int index, RmtRdfGpuMemSegmentInfo::RmtRdfTraceHeapInfo& out_chunk_data) const;

private:
    RmtRdfTraceHeapInfo heap_info_[RmtHeapType::kRmtHeapTypeCount];  ///< The array of heap information structures.
    bool                chunk_data_valid_;                           ///< If true, indicates the chunk data is valid.  Otherwise set to false.
};
#endif  // #ifndef  RMV_PARSER_RMT_RDF_GPU_MEM_SEGMENT_INFO_H_
