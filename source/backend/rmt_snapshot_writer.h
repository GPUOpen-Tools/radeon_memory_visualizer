//=============================================================================
// Copyright (c) 2023-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Base class that handles writing snapshot data.
//=============================================================================

#ifndef RMV_BACKEND_RMT_SNAPSHOT_WRITER_H_
#define RMV_BACKEND_RMT_SNAPSHOT_WRITER_H_

#include "rmt_data_set.h"
#include "rmt_error.h"

// The base class that handles writing snapshot data.
class RmtSnapshotWriter
{
public:
    /// Constructor for the RmtSnapshotWriter
    ///
    /// @param [in]  data_set                             A pointer to the <c><i>RmtDataSetdata</i></c> object.
    explicit RmtSnapshotWriter(RmtDataSet* data_set);

    /// Destructor for the RmtSnapshotWriter
    virtual ~RmtSnapshotWriter();

    /// Add a snapshot to a trace file.
    ///
    /// @param [in]  name                                 A pointer to the data set containing the events.
    /// @param [in]  timestamp                            The position on the timeline of the snapshot.
    /// @param [in]  new_snapshot_index                   The index of the snapshot in the data set to be added.
    ///
    /// @retval
    /// kRmtOk                                            The operation completed successfully.
    /// @retval
    /// kRmtErrorMalformedData                            The operation failed.
    /// @retval
    /// kRmtErrorInvalidPointer                           The operation failed because of an invalid pointer.
    virtual RmtErrorCode Add(const char* name, const uint64_t timestamp, const uint16_t new_snapshot_index) const = 0;

    /// Remove a snapshot from a trace file.
    ///
    /// @param [in]  new_snapshot_index                   The index of the snapshot in the data set to be removed.
    ///
    /// @retval
    /// kRmtOk                                            The operation completed successfully.
    /// @retval
    /// kRmtErrorMalformedData                            The operation failed.
    /// @retval
    /// kRmtErrorInvalidPointer                           The operation failed because of an invalid pointer.
    virtual RmtErrorCode Remove(const uint16_t removed_snapshot_index) const = 0;

protected:
    RmtDataSet* data_set_;  ///< A pointer to the RMT data set.
};
#endif  // #ifndef RMV_BACKEND_RMT_SNAPSHOT_WRITER_H_
