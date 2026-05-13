//=============================================================================
// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test cases header.
//=============================================================================

#ifndef RMV_BACKEND_TEST_RMV_TEST_CASES_H_
#define RMV_BACKEND_TEST_RMV_TEST_CASES_H_

#include <string>
#include <vector>

#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"

#include "log.h"

class RMVTestCases
{
public:
    /// @brief Accessor for singleton instance.
    ///
    /// @return The singleton instance.
    static RMVTestCases* Get();

    /// @brief Initialize anything needed for testing.
    ///
    /// Load in the trace and parse it.
    ///
    /// @param [in] trace_file_name            The input RMV trace file.
    /// @param [in] expected_results_file_name The json file holding the expected results (or empty if no results).
    /// @param [in] add_snapshot               If no snapshots in the trace file and this is true, add a snapshot.
    /// @param [in] resource_identifier        An optional resource ID used for tests (not used if 0).
    /// @param [in] virtual_address            An optional virtual address used for tests (not used if 0).
    ///
    /// @return true if trace file loaded and parsed correctly, false otherwise.
    bool Initialize(const std::string&    trace_file_name,
                    const std::string&    expected_results_file_name,
                    bool                  add_snapshot,
                    RmtResourceIdentifier resource_identifier,
                    RmtGpuAddress         virtual_address);

    /// @brief Add a message to the log.
    ///
    /// @param [in] log_message The text to add to the log.
    void Log(const char* log_message, ...);

    /// @brief Clean up after testing.
    void Shutdown();

    /// @brief Test the SystemInfo struct and validate that sensible numbers are returned.
    ///
    /// Rather than test for 0, values below a threshold are rejected.
    ///
    /// @return true if the tests passed, false if not.
    bool TestSystemInfo();

    /// @brief Test the amount of requested and committed memory for each heap.
    ///
    /// Ensure that committed memory is <= requested memory.
    ///
    /// @return true if the tests passed, false if not.
    bool TestSegmentInfo();

    /// @brief Test the allocations.
    ///
    /// @return true if the tests passed, false if not.
    bool TestAllocations();

    /// @brief Test the resources.
    ///
    /// @return true if the tests passed, false if not.
    bool TestResources();

    /// @brief Test the timestamps.
    ///
    /// @return true if the tests passed, false if not.
    bool TestTimestamps();

private:
    /// @brief Constructor.
    RMVTestCases();

    /// @brief Destructor.
    ~RMVTestCases();

    static RMVTestCases*          instance_;  ///< The singleton instance.
    RmtDataSet                    data_set_;  ///< The dataset read from file.
    RmtDataTimeline               timeline_;  ///< The timeline.
    std::vector<RmtDataSnapshot*> snapshot_;  ///< The snapshot.

    backend_test::Log log_;  ///< The log file.
};

#endif  //  RMV_BACKEND_TEST_RMV_TEST_CASES_H_
