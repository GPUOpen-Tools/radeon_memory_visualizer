//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of functions for working with the RMT file format.
//=============================================================================

#include "rmt_file_format.h"
#include "rmt_util.h"
#include <stdio.h>  // for fread

RmtErrorCode RmtFileParserCreateFromHandle(RmtFileParser* file_parser, FILE* file_handle)
{
    RMT_RETURN_ON_ERROR(file_parser, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(file_handle, kRmtErrorFileNotOpen);

    // set up the parser.
    file_parser->next_chunk_offset = 0;
    file_parser->file_handle       = file_handle;

    // reset the file for read.
    fseek(file_handle, 0L, SEEK_END);
    file_parser->file_size = ftell(file_handle);
    fseek(file_handle, 0, SEEK_SET);
    if (file_parser->file_size == 0U)
    {
        return kRmtErrorFileNotOpen;
    }

    // read the header in, if we didn't get enough from the file than error out.
    const size_t read_size = fread(&file_parser->header, 1, sizeof(RmtFileHeader), file_handle);
    RMT_RETURN_ON_ERROR(read_size == sizeof(RmtFileHeader), kRmtErrorMalformedData);

    // set the next chunk offset
    file_parser->next_chunk_offset = file_parser->header.chunk_offset;

    // validate that the file contains the magic number
    RMT_RETURN_ON_ERROR(file_parser->header.magic_number == RMT_FILE_MAGIC_NUMBER, kRmtErrorMalformedData);

    return kRmtOk;
}

RmtErrorCode RmtFileParserParseNextChunk(RmtFileParser* file_parser, RmtFileChunkHeader** parsed_chunk)
{
    RMT_RETURN_ON_ERROR(file_parser, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(parsed_chunk, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR((size_t)file_parser->next_chunk_offset < file_parser->file_size, kRmtEndOfFile);
    *parsed_chunk = NULL;

    // read the chunk header in from the file.
    fseek(file_parser->file_handle, file_parser->next_chunk_offset, 0);
    const size_t read_size = fread(&file_parser->current_chunk, 1, sizeof(RmtFileChunkHeader), file_parser->file_handle);
    RMT_RETURN_ON_ERROR(read_size == sizeof(RmtFileChunkHeader), kRmtErrorMalformedData);

    // It is possible to get stuck in loops by malformed data in the file since nextChunkOffset is updated
    // by nextChunk->sizeInButes. Harden for that outside where we validate other chunk data.
    file_parser->next_chunk_offset += file_parser->current_chunk.size_in_bytes;
    *parsed_chunk = &file_parser->current_chunk;

    return kRmtOk;
}

RmtErrorCode RmtFileParserIsChunkSupported(const RmtFileChunkHeader* header)
{
    RMT_UNUSED(header);

    return kRmtOk;
}

RmtErrorCode RmtFileParserIsFileSupported(const RmtFileHeader* header)
{
    RMT_UNUSED(header);

    return kRmtOk;
}
