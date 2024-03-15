//=============================================================================
// Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of the RFD File parser.
//=============================================================================

#ifndef RMV_PARSER_RMT_RDF_FILE_PARSER_H_
#define RMV_PARSER_RMT_RDF_FILE_PARSER_H_

#include "rmt_data_set.h"
#include "rmt_error.h"
#include "rmt_types.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// Loads an RDF memory trace file and populates the data set.
///
/// @param [in]     path                        A path to the memory trace file to be loaded.
/// @param [in,out] out_data_set                A pointer to the data set populated with the trace file information.
///
/// @retval
/// kRmtOk                                      The trace file loaded successfully.
/// @retval
/// kRmtErrorMalformedData                      The file contains unexpected data.
/// @retVal
/// kRmtErrorInvalidSize                        The trace file is missing data.
///
RmtErrorCode RmtRdfFileParserLoadRdf(const char* path, RmtDataSet* out_data_set);

/// Delete all RDF data stream objects created while parsing the RDF file.
///
/// @retval
/// kRmtOk                                      The data streams were successfully deleted.
///
RmtErrorCode RmtRdfFileParserDestroyAllDataStreams();

/// Open the global RDF stream.
///
/// @param [in] path                            A path to the memory trace file to be loaded.
/// @param [in] read_only                       Open the file in read only mode if true, otherwise enable write access.
///
/// @retval
/// kRmtOk                                      The RDF stream was successfully opened.
/// @retval
/// kRmtErrorMalformedData                      The file contains unexpected data.
///
RmtErrorCode RmtRdfStreamOpen(const char* path, const bool read_only);

/// Close the global RDF stream.
///
/// @retval
/// kRmtOk                                      The RDF stream was successfully closed.
///
RmtErrorCode RmtRdfStreamClose();

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_PARSER_RMT_RDF_FILE_PARSER_H_
