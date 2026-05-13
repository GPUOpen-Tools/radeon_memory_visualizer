//=============================================================================
// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test timestamps header.
//=============================================================================

#ifndef RMV_BACKEND_TEST_RMV_TEST_TIMESTAMPS_H_
#define RMV_BACKEND_TEST_RMV_TEST_TIMESTAMPS_H_

#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"

#include "log.h"

namespace backend_test
{
    class RMVTestTimestamps
    {
    public:
        /// @brief Constructor.
        RMVTestTimestamps();

        /// @brief Destructor.
        ~RMVTestTimestamps();

        /// @brief Run test cases for timestamps.
        ///
        /// @param [in] snapshot_index The index of the snapshot being tested.
        /// @param [in] data_set       The data set containing the trace file data.
        /// @param [in] snapshot       The snapshot to use for the timestamp tests.
        /// @param [in] log            The log to write any output to.
        ///
        /// @return true if the tests passed, false if not.
        bool RunTests(int32_t snapshot_index, const RmtDataSet& data_set, const RmtDataSnapshot& snapshot, const Log& log);

    private:
        /// @brief Make sure any embedded snapshots have timestamps < max timestamp.
        ///
        /// @param [in]  data_set The data set containing the trace file data.
        /// @param [in]  log      The log to write any output to.
        /// @param [out] result   The test result, passed in from previous tests. This is modified to be false if this test fails.
        void TestSnapshotTimestamps(const RmtDataSet& data_set, const Log& log, bool* result);

        /// @brief Make sure all allocations have timestamps < max timestamp.
        ///
        /// @param [in]  data_set The data set containing the trace file data.
        /// @param [in]  snapshot The snapshot to use for the timestamp tests.
        /// @param [in]  log      The log to write any output to.
        /// @param [out] result   The test result, passed in from previous tests. This is modified to be false if this test fails.
        void TestAllocationTimestamps(const RmtDataSet& data_set, const RmtDataSnapshot& snapshot, const Log& log, bool* result);
    };
}  // namespace backend_test

#endif  //  RMV_BACKEND_TEST_RMV_TEST_TIMESTAMPS_H_
