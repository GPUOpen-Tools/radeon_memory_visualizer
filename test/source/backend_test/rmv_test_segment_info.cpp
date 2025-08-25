//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test segment info implementation.
//=============================================================================

#include "rmt_print.h"
#include "rmt_util.h"

#include "rmv_test_segment_info.h"

#ifndef _WIN32
#include "linux/safe_crt.h"
#endif

namespace backend_test
{
    RMVTestSegmentInfo::RMVTestSegmentInfo()
    {
    }

    RMVTestSegmentInfo::~RMVTestSegmentInfo()
    {
    }

    bool RMVTestSegmentInfo::RunTests(int32_t snapshot_index, const RmtDataSnapshot& snapshot, const Log& log)
    {
        log.Write("TestSegmentInfo (%d):", snapshot_index);
        bool result = TestRequestedAndBound(snapshot, log);
        return result;
    }

    bool RMVTestSegmentInfo::TestRequestedAndBound(const RmtDataSnapshot& snapshot, const Log& log)
    {
        RmtSegmentStatus segment_status;
        bool             result = true;

        for (int heap = 0; heap < kRmtHeapTypeNone; heap++)
        {
            RmtDataSnapshotGetSegmentStatus(&snapshot, static_cast<RmtHeapType>(heap), &segment_status);

            char info_string[1024];
            sprintf_s(info_string,
                      1024,
                      " heap %s: requested: %llu, bound: %llu, committed: %llu",
                      RmtGetHeapTypeNameFromHeapType(static_cast<RmtHeapType>(heap)),
                      segment_status.total_virtual_memory_requested,
                      segment_status.total_bound_virtual_memory,
                      segment_status.total_physical_mapped_by_process);

            if (segment_status.total_virtual_memory_requested >= segment_status.total_bound_virtual_memory)
            {
                log.Write(info_string);
            }
            else
            {
                log.Write(" ERROR:%s", info_string);
                result = false;
            }
        }

        return result;
    }

    bool RMVTestSegmentInfo::TestRequestedAndCommitted(const RmtDataSnapshot& snapshot, const Log& log)
    {
        RmtSegmentStatus segment_status;
        bool             result = true;

        for (int heap = 0; heap < kRmtHeapTypeNone; heap++)
        {
            RmtDataSnapshotGetSegmentStatus(&snapshot, static_cast<RmtHeapType>(heap), &segment_status);
            if (segment_status.total_virtual_memory_requested < segment_status.total_physical_mapped_by_process)
            {
                log.Write(" ERROR: heap %s: requested: %llu, committed: %llu",
                          RmtGetHeapTypeNameFromHeapType(static_cast<RmtHeapType>(heap)),
                          segment_status.total_virtual_memory_requested,
                          segment_status.total_physical_mapped_by_process);
                result = false;
            }
        }

        return result;
    }
}  // namespace backend_test
