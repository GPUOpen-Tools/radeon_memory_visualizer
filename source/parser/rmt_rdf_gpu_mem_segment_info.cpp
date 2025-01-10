//=============================================================================
// Copyright (c) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the RDF Heap info chunk parser.
//=============================================================================

#include "rmt_rdf_gpu_mem_segment_info.h"

#include "rmt_assert.h"
#include "rmt_format.h"
#include "rmt_types.h"

RmtRdfGpuMemSegmentInfo::RmtRdfGpuMemSegmentInfo()
    : chunk_data_valid_(false)
{
    for (int index = 0; index < RmtHeapType::kRmtHeapTypeCount; index++)
    {
        heap_info_[index] = {};
    }
}

RmtRdfGpuMemSegmentInfo::~RmtRdfGpuMemSegmentInfo()
{
}

bool RmtRdfGpuMemSegmentInfo::LoadChunk(rdfChunkFile* chunk_file)
{
    RMT_ASSERT(chunk_file != nullptr);

    const auto identifier            = RmtRdfGpuMemSegmentInfo::ChunkIdentifier();
    int        result                = kRmtRdfResultFailure;
    int        contains_chunk_result = rdfChunkFileContainsChunk(chunk_file, identifier, 0, &result);
    RMT_ASSERT(contains_chunk_result == rdfResult::rdfResultOk);
    RMT_UNUSED(contains_chunk_result);

    if (result == kRmtRdfResultSuccess)
    {
        std::int64_t chunk_count = 0;
        rdfChunkFileGetChunkCount(chunk_file, identifier, &chunk_count);
        if (chunk_count == 1)
        {
            // For the heaps we don't send a header at this point
            int64_t header_size = 0;
            rdfChunkFileGetChunkHeaderSize(chunk_file, identifier, 0, &header_size);
            if (header_size != 0)
            {
                RMT_ASSERT(!"Header should be empty for heap info");
            }

            int64_t payload_size = 0;
            result               = rdfChunkFileGetChunkDataSize(chunk_file, identifier, 0, &payload_size);

            if ((result == rdfResult::rdfResultOk) && (payload_size > 0))
            {
                result = rdfChunkFileReadChunkData(chunk_file, identifier, 0, heap_info_);
            }

            chunk_data_valid_ = true;
        }
    }

    return result == rdfResult::rdfResultOk;
}

bool RmtRdfGpuMemSegmentInfo::GetChunkData(const unsigned int index, RmtRdfGpuMemSegmentInfo::RmtRdfTraceHeapInfo& out_chunk_data) const
{
    bool result = false;

    RMT_ASSERT((index < RmtHeapType::kRmtHeapTypeCount));

    if ((chunk_data_valid_) && (index < RmtHeapType::kRmtHeapTypeCount))
    {
        out_chunk_data = heap_info_[index];
        result         = chunk_data_valid_;
    }

    return result;
}

const char* RmtRdfGpuMemSegmentInfo::ChunkIdentifier()
{
    return "GpuMemSegment";
}
