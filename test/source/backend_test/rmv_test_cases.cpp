//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test cases implementation.
//=============================================================================

#include <string.h>  // for memset

#include "rmv_test_allocations.h"
#include "rmv_test_cases.h"
#include "rmv_test_resources.h"
#include "rmv_test_segment_info.h"
#include "rmv_test_system_info.h"
#include "rmv_test_timestamps.h"

RMVTestCases* RMVTestCases::instance_ = nullptr;

RMVTestCases::RMVTestCases()
{
    memset((void*)&data_set_, 0, sizeof(data_set_));
    memset((void*)&timeline_, 0, sizeof(timeline_));
}

RMVTestCases::~RMVTestCases()
{
    delete instance_;
}

RMVTestCases* RMVTestCases::Get()
{
    if (instance_ == nullptr)
    {
        instance_ = new RMVTestCases();
    }
    return instance_;
}

bool RMVTestCases::Initialize(const std::string&    trace_file_name,
                              const std::string&    expected_results_file_name,
                              bool                  add_snapshot,
                              RmtResourceIdentifier resource_identifier,
                              RmtGpuAddress         virtual_address)
{
    // Initialize the log file.
    log_.Open(trace_file_name.c_str());

    // Check the rmv file has been specified.
    if (trace_file_name.empty())
    {
        log_.WriteConsole("Error: Missing path to the trace file");
        return false;
    }

    // Initialize the data set.
    RmtErrorCode error_code = RmtDataSetInitialize(trace_file_name.c_str(), &data_set_);
    if (error_code != kRmtOk)
    {
        log_.WriteConsole("Error: Unable to initialize data set (error code  0x%X)", error_code);
        return false;
    }

    // Generate the timeline.
    error_code = RmtDataSetGenerateTimeline(&data_set_, kRmtDataTimelineTypeResourceUsageVirtualSize, &timeline_);
    if (error_code != kRmtOk)
    {
        log_.WriteConsole("Error: Can't generate timeline");
        RmtDataSetDestroy(&data_set_);
        return false;
    }

    // Generate a snapshot.
    if (data_set_.snapshot_count == 0)
    {
        if (add_snapshot == true)
        {
            /// Add snapshot to the trace, about half way through.
            RmtSnapshotPoint* snapshot_point = NULL;
            RmtDataSetAddSnapshot(&data_set_, "Snapshot 0", data_set_.maximum_timestamp / 2, &snapshot_point);
            if (snapshot_point == nullptr)
            {
                Shutdown();
                return false;
            }
        }
        else
        {
            log_.WriteConsole("Error: No snapshot in trace file. Create one manually or use the --snapshot option to generate one automatically.");
            Shutdown();
            return false;
        }
    }

    for (int32_t index = 0; index < data_set_.snapshot_count; index++)
    {
        if (index >= static_cast<int32_t>(snapshot_.size()))
        {
            // Snapshot is added to a snapshot_ vector and deleted in Shutdown().
            RmtDataSnapshot* snapshot = new RmtDataSnapshot;
            snapshot_.push_back(snapshot);
        }

        RmtSnapshotPoint* snapshot_point = &data_set_.snapshots[index];
        error_code                       = RmtDataSetGenerateSnapshot(&data_set_, snapshot_point, snapshot_[index]);

        if (error_code != kRmtOk)
        {
            log_.WriteConsole("Error: Can't generate snapshot from snapshot point %d", index);
            Shutdown();
            return false;
        }
    }

    RMT_UNUSED(expected_results_file_name);
    RMT_UNUSED(resource_identifier);
    RMT_UNUSED(virtual_address);

    // Dump out status of SAM/CPU Host Aperture.
    if (data_set_.flags.cpu_host_aperture_enabled)
    {
        Log("CPU host aperture is enabled");
    }
    else if (data_set_.flags.sam_enabled)
    {
        Log("Smart Access Memory support is enabled");
    }

    return true;
}

void RMVTestCases::Log(const char* log_message, ...)
{
    log_.Write(log_message);
}

void RMVTestCases::Shutdown()
{
    for (size_t index = 0; index < snapshot_.size(); ++index)
    {
        delete snapshot_[index];
    }
    RmtDataTimelineDestroy(&timeline_);
    RmtDataSetDestroy(&data_set_);
}

bool RMVTestCases::TestSystemInfo()
{
    backend_test::RMVTestSystemInfo system_info_test;
    bool                            result = system_info_test.RunTests(data_set_.system_info, data_set_.flags.is_rdf_trace, log_);
    return result;
}

bool RMVTestCases::TestSegmentInfo()
{
    for (int32_t index = 0; index < static_cast<int32_t>(snapshot_.size()); index++)
    {
        backend_test::RMVTestSegmentInfo segment_info_test;
        bool                             result = segment_info_test.RunTests(index, *snapshot_[index], log_);
        if (result == false)
        {
            return result;
        }
    }
    return true;
}

bool RMVTestCases::TestAllocations()
{
    for (int32_t index = 0; index < static_cast<int32_t>(snapshot_.size()); index++)
    {
        backend_test::RMVTestAllocations allocations_test;
        bool                             result = allocations_test.RunTests(index, *snapshot_[index], log_);
        if (result == false)
        {
            return result;
        }
    }
    return true;
}

bool RMVTestCases::TestResources()
{
    for (int32_t index = 0; index < static_cast<int32_t>(snapshot_.size()); index++)
    {
        backend_test::RMVTestResources resources_test;
        bool                           result = resources_test.RunTests(index, *snapshot_[index], log_);
        if (result == false)
        {
            return result;
        }
    }
    return true;
}

bool RMVTestCases::TestTimestamps()
{
    for (int32_t index = 0; index < static_cast<int32_t>(snapshot_.size()); index++)
    {
        backend_test::RMVTestTimestamps timestamps_test;
        bool                            result = timestamps_test.RunTests(index, data_set_, *snapshot_[index], log_);
        if (result == false)
        {
            return result;
        }
    }
    return true;
}
