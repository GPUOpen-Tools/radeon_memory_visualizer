//=============================================================================
// Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Definition of the RFD Data Stream chunk parser.
//=============================================================================

#ifndef RMV_PARSER_RMT_RDF_DATA_STREAM_H_
#define RMV_PARSER_RMT_RDF_DATA_STREAM_H_

#include "rdf/rdf/inc/amdrdf.h"

#include <queue>
#include <vector>

/// Chunk information structure.
typedef struct RmtRdfChunkInfo
{
    int32_t chunk_index;  ///< The chunk index.
    size_t  chunk_size;   ///< The number of bytes in the chunk.
} RmtRdfChunckData;

/// The class that manages the loading of chunks for the Data Stream.
class RmtRdfDataStream
{
public:
    /// Constructor for the RmtRdfDataStream class.
    /// @param [in] trace_file_path              The trace file path.
    /// @param [in] stream_index                 The index of the stream.
    /// @param [in] process_id                   The process ID of the Data Stream.
    /// @param [in] thread_id                    The thread ID of the Data Stream.
    /// @param [in] rdf_major_version            The major version of the Data Stream.
    /// @param [in] rdf_minor_version            The minor version of the Data Stream.
    /// @param [in] stream                       A pointer to the RDF trace file stream.
    RmtRdfDataStream(const std::string& trace_file_path,
                     const uint32_t     stream_index,
                     const uint32_t     process_id,
                     const uint32_t     thread_id,
                     const uint16_t     rdf_major_version,
                     const uint16_t     rdf_minor_version,
                     rdfStream**        stream);

    /// Destructor.
    ~RmtRdfDataStream();

    /// Reset buffer and chunks loaded.
    ///
    void Reset();

    /// Load the next chunk of the RFD Data Stream.
    /// @param [in] offset                       The number of bytes from the start of the current buffer that has been processed.
    ///
    /// @retval                                  If successful, returns true.  Otherwise returns false.
    ///
    bool LoadNextChunk(const size_t offset);

    /// Retrieve a pointer to the chunk buffer.
    ///
    /// @retval                                  A pointer to the chunk buffer.
    ///
    uint8_t* GetBuffer();

    /// Return the Data Stream chunk identifier.
    ///
    /// @retval                                  A pointer to the character string containing the identifier.
    ///
    static const char* ChunkIdentifier();

    /// Add a chunk to the queue of chunks for the Data Stream.
    ///
    /// @param [in] chunk_index                  The index of the chunk to add.
    /// @param [in] chunk_size                   The number of bytes in the chunk to add.
    ///
    void AddChunk(const int32_t chunk_index, const size_t chunk_size);

    /// Retrieve the number of bytes loaded into the buffer.
    ///
    /// @retval                                  The buffer fill size.
    ///
    size_t GetBufferFillSize() const;

    /// Retrieve the total number of bytes in the stream.
    ///
    /// @retval                                  The stream size.
    ///
    size_t GetStreamSize() const;

    /// Retrieve the number of unprocessed chunks remaining to be loaded.
    ///
    /// @retval                                  The remaining chunk count.
    ///
    size_t GetRemainingUnprocessedChunks() const;

    /// Retrieve the file name of the RDF trace file.
    ///
    /// @retval                                  The trace file path.
    ///
    const char* GetTraceFilePath() const;

    /// Retrieve the Data Stream major version number.
    ///
    /// @retval                                  The trace major version number.
    ///
    int16_t GetMajorVersion() const;

    /// Retrieve the Data Stream minor version number.
    ///
    /// @retval                                  The trace minor version number.
    ///
    int16_t GetMinorVersion() const;

    /// Retrieve the process ID of the stream.
    ///
    /// @retval                                  The process ID.
    ///
    int32_t GetProcessId() const;

    /// Retrieve the thread ID of the stream.
    ///
    /// @retval                                  The thread ID.
    ///
    int32_t GetThreadId() const;

    /// The stream header chunk data format.
    typedef struct RmtRdfTraceStreamHeader
    {
        uint32_t process_id;         ///< The process ID that generated this RMT data. If unknown program to 0.
        uint32_t thread_id;          ///< The CPU thread ID of the thread in the application that generated this RMT data.
        size_t   total_data_size;    ///< The payload size in bytes.
        uint32_t stream_index;       ///< The index for this stream.
        uint16_t rdf_major_version;  ///< The major version number for this stream.
        uint16_t rdf_minor_version;  ///< The minor version number for this stream.
    } RmtRdfTraceStreamHeader;

private:
    std::vector<std::uint8_t>    chunk_buffer_;              ///< The RMT Token stream buffer for a chunk.
    size_t                       current_buffer_fill_size_;  ///< The number of bytes loaded into the current buffer.
    std::vector<RmtRdfChunkInfo> chunk_info_list_;           ///< The list of queued chunks.
    size_t                       next_chunk_number_;         ///< The next chunk in the chunk_info_list_ to load.
    size_t                       stream_size_;               ///< The total number of bytes in the Data Stream.
    const std::string            trace_file_path_;           ///< The full path of the trace file.
    const uint32_t               stream_index_;              ///< The index of this stream.
    const uint32_t               process_id_;                ///< The process ID that generated this RMT data. If unknown program to 0.
    const uint32_t               thread_id_;                 ///< The CPU thread ID of the thread in the application that generated this RMT data.
    const uint16_t               rdf_major_version_;         ///< The major version number for this stream.
    const uint16_t               rdf_minor_version_;         ///< The minor version number for this stream.
    rdfStream**                  stream_;                    ///< A pointer to the RDF stream.
};
#endif  // #ifndef RMV_PARSER_RMT_RDF_DATA_STREAM_H_
