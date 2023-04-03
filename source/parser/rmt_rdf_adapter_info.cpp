//=============================================================================
// Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the RFD Adapter Information chunk parser.
//=============================================================================

#include "rmt_rdf_adapter_info.h"

#include "rdf/rdf/inc/amdrdf.h"

RmtRdfAdapterInfo::RmtRdfAdapterInfo()
    : adapter_info_{0}
    , chunk_data_valid_(false)
{
}

RmtRdfAdapterInfo::~RmtRdfAdapterInfo()
{
}

bool RmtRdfAdapterInfo::LoadChunk(rdf::ChunkFile& chunk_file)
{
    bool retVal = false;

    const auto identifier = ChunkIdentifier();

    chunk_data_valid_ = false;
    if (!chunk_file.ContainsChunk(identifier))
    {
        return false;
    }

    const auto chunk_count = chunk_file.GetChunkCount(identifier);
    if (chunk_count != 1)
    {
        return false;
    }

    uint64_t header_size  = chunk_file.GetChunkHeaderSize(identifier);
    uint64_t payload_size = chunk_file.GetChunkDataSize(identifier);

    std::vector<std::uint8_t> header(header_size);
    if (header_size > 0)
    {
        chunk_file.ReadChunkHeaderToBuffer(identifier, header.data());
    }

    if (payload_size > 0)
    {
        chunk_file.ReadChunkDataToBuffer(identifier, &adapter_info_);
        retVal = true;
    }

    chunk_data_valid_ = true;

    return retVal;
}

const RmtRdfAdapterInfo::RmtRdfTraceAdapterInfo& RmtRdfAdapterInfo::GetChunkData() const
{
    return adapter_info_;
}

const char* RmtRdfAdapterInfo::ChunkIdentifier()
{
    return "AdapterInfo";
}
