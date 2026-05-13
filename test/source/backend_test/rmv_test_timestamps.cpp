//=============================================================================
// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test timestamps implementation.
//=============================================================================

#include "rmv_test_timestamps.h"

namespace backend_test
{
    static const uint64_t kMaxTimestamp = 864000000000;  // 24 hours in clocks.

    RMVTestTimestamps::RMVTestTimestamps()
    {
    }

    RMVTestTimestamps::~RMVTestTimestamps()
    {
    }

    bool RMVTestTimestamps::RunTests(int32_t snapshot_index, const RmtDataSet& data_set, const RmtDataSnapshot& snapshot, const Log& log)
    {
        log.Write("TestTimestamps (%d):", snapshot_index);
        bool result = true;
        TestSnapshotTimestamps(data_set, log, &result);
        TestAllocationTimestamps(data_set, snapshot, log, &result);
        return result;
    }

    void RMVTestTimestamps::TestSnapshotTimestamps(const RmtDataSet& data_set, const Log& log, bool* result)
    {
        bool    test_result    = true;
        int32_t snapshot_count = data_set.snapshot_count;

        // Make sure snapshot count is valid.
        if (snapshot_count < 0)
        {
            log.Write(" ERROR: negative snapshot count %d", snapshot_count);
            test_result = false;
        }
        else
        {
            log.Write(" Snapshots found: %d", snapshot_count);
        }

        // Check snapshot time is valid.
        for (int32_t index = 0; index < snapshot_count; index++)
        {
            const RmtSnapshotPoint& snapshot = data_set.snapshots[index];
            if (snapshot.timestamp > data_set.maximum_timestamp)
            {
                log.Write(" WARNING: snapshot %d timestamp (%llu) larger than max timestamp (%llu)", index, snapshot.timestamp, data_set.maximum_timestamp);
            }
        }

        // Check global timestamp is valid.
        if (data_set.maximum_timestamp > kMaxTimestamp)
        {
            log.Write(" ERROR: Maximum timestamp too large (%llu)", data_set.maximum_timestamp);
            test_result = false;
        }

        if (test_result == true)
        {
            log.Write(" Snapshot timestamps OK");
        }
        else
        {
            *result = test_result;
        }
    }

    void RMVTestTimestamps::TestAllocationTimestamps(const RmtDataSet& data_set, const RmtDataSnapshot& snapshot, const Log& log, bool* result)
    {
        bool                            test_result             = true;
        const RmtVirtualAllocationList& virtual_allocation_list = snapshot.virtual_allocation_list;
        int32_t                         allocation_count        = virtual_allocation_list.allocation_count;
        for (int32_t index = 0; index < allocation_count; index++)
        {
            const RmtVirtualAllocation& virtual_allocation = virtual_allocation_list.allocation_details[index];

            // Make sure timestamp is less than maximum timestamp.
            if (virtual_allocation.timestamp > data_set.maximum_timestamp)
            {
                log.Write(
                    " ERROR: allocation %d timestamp (%llu) larger than max timestamp (%llu)", index, virtual_allocation.timestamp, data_set.maximum_timestamp);
                test_result = false;
            }
        }

        if (test_result == true)
        {
            log.Write(" Allocation timestamps OK");
        }
        else
        {
            *result = test_result;
        }
    }

}  // namespace backend_test
