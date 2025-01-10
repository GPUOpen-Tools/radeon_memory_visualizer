//=============================================================================
// Copyright (c) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the RFD Data Stream chunk parser.
//=============================================================================

#include "rmt_rdf_data_stream.h"

#include "rmt_assert.h"
#include "rmt_format.h"
#include "rmt_print.h"

// Default padding used for initial chunk buffer size.
static int32_t kBufferPadding = 128;

RmtRdfDataStream::RmtRdfDataStream(const std::string& trace_file_path,
                                   const uint32_t     stream_index,
                                   const uint32_t     process_id,
                                   const uint32_t     thread_id,
                                   const uint16_t     rdf_major_version,
                                   const uint16_t     rdf_minor_version,
                                   rdfStream**        stream)
    : current_buffer_fill_size_(0)
    , next_chunk_number_(0)
    , stream_size_(0)
    , trace_file_path_(trace_file_path)
    , stream_index_(stream_index)
    , process_id_(process_id)
    , thread_id_(thread_id)
    , rdf_major_version_(rdf_major_version)
    , rdf_minor_version_(rdf_minor_version)
    , stream_(stream)
{
}

RmtRdfDataStream::~RmtRdfDataStream()
{
}

const char* RmtRdfDataStream::ChunkIdentifier()
{
    return "RmtData";
}

// Reset the list of chunks in the queue.
void RmtRdfDataStream::Reset()
{
    next_chunk_number_ = 0;
    chunk_buffer_.clear();
}

// Adds a chunk to the list of chunks contained in the Data Stream.
void RmtRdfDataStream::AddChunk(const int32_t chunk_index, const size_t chunk_size)
{
    // Store the chunk info in the chunk info structure.
    RmtRdfChunkInfo chunk_info;
    chunk_info.chunk_index = chunk_index;
    chunk_info.chunk_size  = chunk_size;

    // Add the chunk info to the list.
    chunk_info_list_.push_back(chunk_info);
    next_chunk_number_ = 0;

    // Adjust the size of the stream.
    stream_size_ += chunk_info.chunk_size;

    // Adjust the buffer to fit the largest chunk added plus some padding for unprocessed tokens.
    // If a smaller chunk is added, the resize() will not affect the current buffer size.
    chunk_buffer_.resize(chunk_size + kBufferPadding);
}

// Load the next chunk from the queued list of chunks.
// Any data in the current buffer that hasn't been processed (indicated by the offset parameter), is moved to the
// front of the buffer before loading the next data chunk.
bool RmtRdfDataStream::LoadNextChunk(const size_t offset)
{
    bool    ret_val               = false;
    int64_t remaining_buffer_size = 0;

    if (next_chunk_number_ > 0)
    {
        if (GetBufferFillSize() > offset)
        {
            // Calculate how much unprocessed data is left in the current buffer.
            remaining_buffer_size = GetBufferFillSize() - offset;

            // Move the remaining data to the front of the new buffer.  This handles the case where
            // a partial token was left at the end of the buffer.  The data loaded to the new buffer is appended
            // after the data left in the previous buffer so that the full data for the unprocessed tokan can be parsed.
            memcpy(chunk_buffer_.data(), chunk_buffer_.data() + offset, remaining_buffer_size);
        }
    }

    if (next_chunk_number_ < chunk_info_list_.size())
    {
        uint64_t payload_size = chunk_info_list_[next_chunk_number_].chunk_size;
        if (payload_size > 0)
        {
            // Adjust the buffer size to be the number of remaining bytes plus the size of the new chunk.
            chunk_buffer_.resize(remaining_buffer_size + payload_size);
            current_buffer_fill_size_ = remaining_buffer_size + payload_size;

            // Add the new chunk to the buffer.
            if (*stream_ != nullptr)
            {
                rdfChunkFile* chunk_file = nullptr;
                if ((rdfChunkFileOpenStream(*stream_, &chunk_file) == rdfResult::rdfResultOk) && (chunk_file != nullptr))
                {
                    const char* identifier = ChunkIdentifier();
                    if (rdfChunkFileReadChunkData(
                            chunk_file, identifier, chunk_info_list_[next_chunk_number_].chunk_index, chunk_buffer_.data() + remaining_buffer_size) ==
                        rdfResult::rdfResultOk)
                    {
                        // Point to the next chunk info item in the list.
                        next_chunk_number_++;
                        ret_val = true;
                    }
                    int chunk_close_result = rdfChunkFileClose(&chunk_file);
                    RMT_ASSERT(chunk_close_result == rdfResult::rdfResultOk);
                    RMT_UNUSED(chunk_close_result);
                }
            }
        }
    }
    else
    {
        // There are no more chunks to load.
        // Adjust the size of the buffer to be only the number of remaining bytes that haven't been processed (if there are any).
        chunk_buffer_.resize(remaining_buffer_size);
        current_buffer_fill_size_ = remaining_buffer_size;
        if (remaining_buffer_size > 0)
        {
            // There are still some bytes remaining in the buffer so return true.
            ret_val = true;
        }
    }

    return ret_val;
}

uint8_t* RmtRdfDataStream::GetBuffer()
{
    return chunk_buffer_.data();
}

int16_t RmtRdfDataStream::GetMajorVersion() const
{
    return rdf_major_version_;
}

int16_t RmtRdfDataStream::GetMinorVersion() const
{
    return rdf_minor_version_;
}

int32_t RmtRdfDataStream::GetProcessId() const
{
    return process_id_;
}

int32_t RmtRdfDataStream::GetThreadId() const
{
    return thread_id_;
}

size_t RmtRdfDataStream::GetStreamSize() const
{
    return stream_size_;
}

size_t RmtRdfDataStream::GetRemainingUnprocessedChunks() const
{
    const size_t size = chunk_info_list_.size() - next_chunk_number_;
    return size;
}

const char* RmtRdfDataStream::GetTraceFilePath() const
{
    return trace_file_path_.c_str();
}

size_t RmtRdfDataStream::GetBufferFillSize() const
{
    return current_buffer_fill_size_;
}
