//=============================================================================
// Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of the RDF DD Event Information chunk parser.
//=============================================================================

#ifndef RMV_PARSER_RMT_RDF_DD_EVENT_INFO_H_
#define RMV_PARSER_RMT_RDF_DD_EVENT_INFO_H_

#include "rdf/rdf/inc/amdrdf.h"

/// Load timestamp info from the DD Event chunk header.
/// @param [in]  chunk_file                      The <c><i>rdfChunkFile</i></c> object pointer to load from.
/// @param [out] out_timestamp                   The timestamp from the DDEvent chunk header.
/// @param [out] out_frequency                   The frequency from the DDEvent chunk header.
///
/// @retval                                     If successful, returns true.  Otherwise returns false.
///
bool RmtRdfLoadTimestampData(rdfChunkFile* chunk_file, uint64_t& out_timestamp, uint32_t& out_frequency);

#endif  // #ifndef  RMV_PARSER_RMT_RDF_DD_EVENT_INFO_H_
