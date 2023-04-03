//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Core parsing code for RMT data.
//=============================================================================

#ifndef RMV_PARSER_RMT_PARSER_H_
#define RMV_PARSER_RMT_PARSER_H_

#include "rmt_error.h"
#include <stdio.h>

typedef struct RmtToken RmtToken;

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

// call forward for use in function pointer typedef.
typedef struct RmtParser RmtParser;

/// A callback function that <c><i>rmtParseAdvance</i></c> will call when it runs out of memory.
///
/// The host code can then either provide an additional memory to the parser for it to continue
/// parsing. If the host code returns memory to the parser then it should also return kRmtOk
/// from the callback function. If the host does not wish to provide additional memory (perhaps
/// the end of the buffer has already been reached) then the host code can return
/// <c><i>kRmtErrorOutOfMemory</i></c> from the callback to indicate that there is no more
/// more memory for the parser to consume.
///
/// @param [in] parser                      A pointer to a <c><i>RmtParser</i></c> structure that is requesting additional memory.
/// @param [in] start_offset                The offset in the current buffer where the parser is currently positioned. Any memory returned should include this data.
/// @param [out] out_rmt_buffer             A pointer to the address of the buffer that <c><i>parser</i></c> will be updated to work with.
/// @param [out] out_rmt_buffer_size        A pointer to a <c><i>size_t</i></c> which contains the size of the buffer pointed to by the contents of <c><i>outRmtBuffer</i></c>.
typedef RmtErrorCode (*RmtParserNextChunkCallbackFunc)(const RmtParser* parser, const size_t start_offset, void** out_rmt_buffer, size_t* out_rmt_buffer_size);

/// A callback function that resets the object that manages the data stream buffer.
///
/// @param [in] parser                      A pointer to a <c><i>RmtParser</i></c> structure that is requesting additional memory.
typedef RmtErrorCode (*RmtParserResetDataStreamCallbackFunc)(const RmtParser* parser);

/// A structure representing current position in the RMT parser.
typedef struct RmtParserPosition
{
    uint64_t timestamp;                ///< The last time seen.
    int32_t  stream_start_offset;      ///< The start position in the stream (in bytes).
    int32_t  stream_current_offset;    ///< The current offset (in bytes) into the stream.
    int32_t  seen_timestamp;           ///< Flag indicating if we've seen a timestamp packet in the buffer yet.
    int32_t  file_buffer_actual_size;  ///< The size of the file buffer.
    int32_t  file_buffer_offset;       ///< The offset into the file buffer.
} RmtParserPosition;

/// A structure encapsulating the RMT format parser state.
typedef struct RmtParser
{
    uint64_t start_timestamp;    ///< The timestamp considered to the be the start of the trace, specified in RMT clocks.
    uint64_t current_timestamp;  ///< The current time in RMT clocks.
    int32_t  seen_timestamp;     ///< Set to non-zero if we have seen a <c><i>kRmtTokenTypeTimestamp</i></c> while parsing.
    uint32_t cpu_frequency;      ///< The CPU frequency (in clock ticks per second) of the machine where the RMT data was captured.

    RmtParserNextChunkCallbackFunc next_chunk_func;  ///< The function to call to request more memory to parse when we run out of tokens.
    FILE*                          file_handle;      ///< The handle to read the file.

    // offset within the stream
    int32_t stream_current_offset;  ///< The current offset into <c><i>rmtBuffer</i></c>.
    int32_t stream_start_offset;    ///< The starting offset into <c><i>rmtBuffer</i></c>.
    size_t  stream_size;            ///< The max length to read from this stream.

    // local buffering from file.
    void*   file_buffer;              ///< Buffer to contain reads of data from the file.
    int32_t file_buffer_size;         ///< The size of the file buffer.
    int32_t file_buffer_offset;       ///< The current offset into the file buffer.
    int32_t file_buffer_actual_size;  ///< The actual size of the dat in the file buffer.

    int32_t  major_version;         ///< The major version of the RMT format.
    int32_t  minor_version;         ///< The minor version of the RMT format.
    uint64_t thread_id;             ///< The thread ID of the CPU thread in the target application where the RMT data was collected from.
    uint64_t process_id;            ///< The process ID of the target application where the RMT data was collected from.
    int32_t  stream_index;          ///< The index of the RMT stream within the RMT file.
    bool     buffer_refill_needed;  ///< A flag that indicates all data in the buffer has been parsed and the next chunk needs to be loaded.
    RmtParserResetDataStreamCallbackFunc reset_data_stream_func;  ///< The function called to reset the object that manages the data stream buffer.

} RmtParser;

/// Initialize the RMT parser structure.
///
/// @param [in] rmt_parser                  A pointer to a <c><i>RmtParser</i></c> structure.
/// @param [in] file_handle                 A pointer to the data set file handle.
/// @param [in] file_offset                 The offset into the file specified by file_handle where the file chunk is to be found.
/// @param [in] stream_size                 The size of the file chunk.
/// @param [in] file_buffer                 A pointer to the read buffer.
/// @param [in] file_buffer_size            The size of the read buffer.
/// @param [in] major_version               The major version number of the <c><i>RmtFileChunkHeader</i></c>.
/// @param [in] minor_version               The minor version number of the <c><i>RmtFileChunkHeader</i></c>.
/// @param [in] stream_index                The index of the stream.
/// @param [in] process_id                  The process ID corresponding to the stream.
/// @param [in] thread_id                   The thread ID corresponding to the stream.
///
/// @retval
/// kRmtOk                              The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer             The operation failed because <c><i>rmt_parser</i></c> or <c><i>file_handle</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorInvalidSize                The operation failed because the stream_size is invalid.
RmtErrorCode RmtParserInitialize(RmtParser* rmt_parser,
                                 FILE*      file_handle,
                                 int32_t    file_offset,
                                 int32_t    stream_size,
                                 void*      file_buffer,
                                 int32_t    file_buffer_size,
                                 int32_t    major_version,
                                 int32_t    minor_version,
                                 int32_t    stream_index,
                                 uint64_t   process_id,
                                 uint64_t   thread_id);

/// Set the callback functions that the <c><i>RmtParser</i></c> structure uses to manage the data stream buffer.
///
/// @param [in] rmt_parser                    A pointer to an <c><i>RmtParser</i></c> structure.
/// @param [in] next_chunk_callback           A pointer to the function that fills the buffer with the next data stream chunk (set to <c><i>NULL</i></c> to disable).
/// @param [in] reset_data_stream_callback    A pointer to the function that resets the buffer pointers for the object that manages the data stream (set to <c><i>NULL</i></c> to disable).
///
/// @retval
/// kRmtOk                              The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer             The operation failed because <c><i>rmt_parser</i></c> is <c><i>NULL</i></c>.
RmtErrorCode RmtParserSetCallbacks(RmtParser*                           rmt_parser,
                                   RmtParserNextChunkCallbackFunc       next_chunk_callback,
                                   RmtParserResetDataStreamCallbackFunc reset_data_stream_callback);

/// Advance the RMT parser forward by a single token.
///
/// @param [in]  rmt_parser                 A pointer to a <c><i>RmtParser</i></c> structure.
/// @param [out] out_token                  A pointer to a <c><i>RmtToken</i></c> structure.
/// @param [in]  out_parser_position        A pointer to a <c><i>RmtParserPosition</i></c> structure.
///
/// @retval
/// kRmtOk                              The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer             The operation failed because <c><i>rmt_parser</i></c> or <c><i>out_parser_position</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtParserAdvance(RmtParser* rmt_parser, RmtToken* out_token, RmtParserPosition* out_parser_position);

/// Check if the RMT parser has finished.
///
/// @param [in] rmt_parser                  A pointer to a <c><i>RmtParser</i></c> structure.
///
/// @returns
/// true if the parser has finished, false if not.
bool RmtParserIsCompleted(RmtParser* rmt_parser);

/// Set the current position of the RMT buffer on the parser.
///
/// @param [in]  rmt_parser                 A pointer to a <c><i>RmtParser</i></c> structure.
/// @param [in]  parser_position            A pointer to a <c><i>RmtParserPosition</i></c> structure.
///
/// @retval
/// kRmtOk                              The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer             The operation failed because <c><i>rmt_parser</i></c> or <c><i>parser_position</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtParserSetPosition(RmtParser* rmt_parser, const RmtParserPosition* parser_position);

/// Reset the RMT parser.
///
/// @param [in] rmt_parser                  A pointer to a <c><i>RmtParser</i></c> structure.
/// @param [in] file_handle                 The file handle for the memory trace.
///
/// @retval
/// kRmtOk                              The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer             The operation failed because <c><i>rmt_parser</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtParserReset(RmtParser* rmt_parser, FILE* file_handle);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_PARSER_RMT_PARSER_
