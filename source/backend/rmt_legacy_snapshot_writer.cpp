//=============================================================================
// Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the class that writes legacy trace snapshot data.
//=============================================================================

#include "rmt_legacy_snapshot_writer.h"

#include "rmt_assert.h"
#include "rmt_file_format.h"
#include "rmt_snapshot_writer.h"
#include "rmt_data_set.h"
#include "rmt_error.h"

#include <stddef.h>  // For offsetof macro.

#ifndef _WIN32
#include "linux/safe_crt.h"
#include <fstream>
#include <unistd.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

RmtLegacySnapshotWriter::RmtLegacySnapshotWriter(RmtDataSet* data_set)
    : RmtSnapshotWriter(data_set)
{
}

RmtLegacySnapshotWriter::~RmtLegacySnapshotWriter()
{
}

// Append a chunk to the trace file for a new snapshot.
RmtErrorCode RmtLegacySnapshotWriter::Add(const char* name, uint64_t timestamp, const uint16_t new_snapshot_index) const
{
    RMT_ASSERT(data_set_);

    // Jump to the end of the file, and write a new snapshot out.
    fseek((FILE*)data_set_->file_handle, 0L, SEEK_END);

    // Add the header.
    RmtFileChunkHeader chunk_header;
    chunk_header.chunk_identifier.chunk_info.chunk_type  = kRmtFileChunkTypeSnapshotInfo;
    chunk_header.chunk_identifier.chunk_info.chunk_index = 0;
    chunk_header.chunk_identifier.chunk_info.reserved    = 0;
    chunk_header.version_major                           = 1;
    chunk_header.version_minor                           = 0;
    chunk_header.padding                                 = 0;
    const int32_t name_length                            = (int32_t)RMT_MINIMUM(strlen(name), RMT_MAXIMUM_NAME_LENGTH);
    chunk_header.size_in_bytes                           = sizeof(RmtFileChunkHeader) + sizeof(RmtFileChunkSnapshotInfo) + name_length;

    size_t write_size = fwrite(&chunk_header, 1, sizeof(RmtFileChunkHeader), (FILE*)data_set_->file_handle);
    RMT_ASSERT(write_size == sizeof(RmtFileChunkHeader));

    // Add the snapshot payload
    RmtFileChunkSnapshotInfo snapshot_info_chunk;
    snapshot_info_chunk.name_length_in_bytes             = name_length;
    snapshot_info_chunk.snapshot_time                    = timestamp;
    data_set_->snapshots[new_snapshot_index].file_offset = ftell((FILE*)data_set_->file_handle);  // Get offset before write to payload
    write_size                                           = fwrite(&snapshot_info_chunk, 1, sizeof(RmtFileChunkSnapshotInfo), (FILE*)data_set_->file_handle);
    RMT_ASSERT(write_size == sizeof(RmtFileChunkSnapshotInfo));

    // Write the name
    write_size = fwrite(name, 1, name_length, (FILE*)data_set_->file_handle);
    RMT_ASSERT(static_cast<int32_t>(write_size) == name_length);

    return kRmtOk;
}

// Updated the snapshot removed snapshot in the trace file.
RmtErrorCode RmtLegacySnapshotWriter::Remove(const uint16_t removed_snapshot_index) const
{
    const uint64_t offset_to_snapshot_chunk = data_set_->snapshots[removed_snapshot_index].file_offset;  // offset to snapshot info chunk.
    fseek((FILE*)data_set_->file_handle, (int32_t)(offset_to_snapshot_chunk + offsetof(RmtFileChunkSnapshotInfo, name_length_in_bytes)), SEEK_SET);
    const uint32_t value = 0;
    fwrite(&value, 1, sizeof(uint32_t), (FILE*)data_set_->file_handle);
    return kRmtOk;
}
