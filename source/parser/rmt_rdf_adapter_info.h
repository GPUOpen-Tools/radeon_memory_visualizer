//=============================================================================
// Copyright (c) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of the RFD Adapter Information chunk parser
//=============================================================================

#ifndef RMV_PARSER_RMT_RDF_ADAPTER_INFO_H_
#define RMV_PARSER_RMT_RDF_ADAPTER_INFO_H_

#include "rdf/rdf/inc/amdrdf.h"

/// The Parser for the Adapter Info Chunk.
class RmtRdfAdapterInfo
{
public:
    /// @brief Constructor.
    RmtRdfAdapterInfo();

    /// @brief Destructor.
    ~RmtRdfAdapterInfo();

    /// Load an RFD Adapter Info chunk.
    /// @param [in] chunk_file                      The <c><i>rdf::ChunkFile</i></c> object to load from.
    ///
    /// @retval                                     If successful, returns true.  Otherwise returns false.
    ///
    bool LoadChunk(rdf::ChunkFile& chunk_file);

    /// Return the Adapter Info chunk identifier.
    ///
    /// @retval                                     A pointer to the character string containing the identifier.
    ///
    static const char* ChunkIdentifier();

    /// @brief The adapter Info chunk data format.
    struct RmtRdfTraceAdapterInfo
    {
        char     name[128];             ///< Name of the gpu
        uint32_t family_id;             ///< PCI Family
        uint32_t revision_id;           ///< PCI Revision
        uint32_t device_id;             ///< PCI Device
        uint32_t min_engine_clock;      ///< Minumum engine clock in Mhz
        uint32_t max_engine_clock;      ///< Maximum engine clock in Mhz
        uint32_t memory_type;           ///< Type of memory
        uint32_t memory_ops_per_clock;  ///< Number of memory operations per clock
        uint32_t memory_bus_width;      ///< Bus width of memory interface in bits
        uint32_t memory_bandwidth;      ///< Bandwidth of memory in MB/s
        uint32_t min_memory_clock;      ///< Minumum memory clock in Mhz
        uint32_t max_memory_clock;      ///< Minumum memory clock in Mhz
    };

    /// Accessor for the adaptor information structure.
    ///
    /// @retval
    /// kRmtOk                                      A reference to the adapter info.
    ///
    const RmtRdfTraceAdapterInfo& GetChunkData() const;

private:
    RmtRdfTraceAdapterInfo adapter_info_;      ///< The data retrieved from the adapter info chunk.
    bool                   chunk_data_valid_;  ///< If true, indicates the chunk data is valid.  Otherwise set to false.
};
#endif  // #ifndef  RMV_PARSER_RMT_RDF_ADAPTER_INFO_H_
