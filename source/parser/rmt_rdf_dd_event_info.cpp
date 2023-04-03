//=============================================================================
// Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the RDF DD Event Information chunk parser.
//=============================================================================

#include "rmt_rdf_dd_event_info.h"

#include "rmt_assert.h"
#include "rmt_error.h"
#include "rmt_format.h"

#include "rdf/rdf/inc/amdrdf.h"

// The chunk identifier.
static const char* kDDEventChunkIdentifier = "DDEvent";

// The chunk data header.
struct DDEventProviderHeader
{
    uint16_t versionMajor           = 0;  ///< Major version number of the event provider, indicating the events data format.
    uint16_t versionMinor           = 0;  ///< Minor version number of the event provider, indicating the events data format.
    uint32_t reserved               = 0;  ///< reserved.
    uint32_t providerId             = 0;  ///< Number uniquely identifying an event provider.
    uint32_t timeUnit               = 0;  ///< Time unit indicates the precision of timestamp delta.
    uint64_t baseTimestamp          = 0;  ///< First timestamp counter before any other events. Used to calibrate timing of all subquent events.
    uint64_t baseTimestampFrequency = 0;  ///< The frequency of counter, in counts per second.
};

bool RmtRdfLoadTimestampData(rdfChunkFile* chunk_file, uint64_t& out_timestamp, uint32_t& out_frequency)
{
    RMT_RETURN_ON_ERROR(chunk_file, false);

    int rdf_result     = kRmtRdfResultFailure;
    int rdf_return_val = rdfResultError;

    int result = rdfChunkFileContainsChunk(chunk_file, kDDEventChunkIdentifier, 0, &rdf_result);
    RMT_ASSERT(result == rdfResult::rdfResultOk);
    RMT_UNUSED(result);

    if (rdf_result == kRmtRdfResultSuccess)
    {
        std::int64_t chunk_count = 0;
        rdfChunkFileGetChunkCount(chunk_file, kDDEventChunkIdentifier, &chunk_count);
        if (chunk_count > 0)
        {
            DDEventProviderHeader header;
            rdf_return_val = rdfChunkFileReadChunkHeader(chunk_file, kDDEventChunkIdentifier, 0, &header);
            if (rdf_return_val == rdfResult::rdfResultOk)
            {
                out_timestamp = header.baseTimestamp;
                RMT_ASSERT(header.baseTimestampFrequency <= UINT32_MAX);
                out_frequency = static_cast<uint32_t>(header.baseTimestampFrequency);
            }
        }
    }

    return rdf_return_val == rdfResult::rdfResultOk;
}
