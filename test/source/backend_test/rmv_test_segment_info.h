//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test segment info header.
//=============================================================================

#ifndef RMV_TEST_SEGMENT_INFO_H_
#define RMV_TEST_SEGMENT_INFO_H_

#include "rmt_data_snapshot.h"

#include "log.h"

namespace backend_test
{
    class RMVTestSegmentInfo
    {
    public:
        /// @brief Constructor.
        RMVTestSegmentInfo();

        /// @brief Destructor.
        ~RMVTestSegmentInfo();

        /// @brief Run test cases for the segment info.
        ///
        /// @param [in] snapshot_index The index of the snapshot being tested.
        /// @param [in] snapshot       The snapshot to use for the segment tests.
        /// @param [in] log            The log to write any output to.
        ///
        /// @return true if the tests passed, false if not.
        bool RunTests(int32_t snapshot_index, const RmtDataSnapshot& snapshot, const Log& log);

    private:
        /// @brief Test the amount of requested and committed memory for each heap.
        ///
        /// Ensure that committed memory is <= requested memory.
        ///
        /// @param [in] snapshot The snapshot to use for the segment tests.
        /// @param [in] log      The log to write any output to.
        ///
        /// @return true if the test passed, false if not.
        bool TestRequestedAndCommitted(const RmtDataSnapshot& snapshot, const Log& log);

        /// @brief Test the amount of requested and bound memory for each heap.
        ///
        /// Ensure that bound memory is <= requested memory.
        ///
        /// @param [in] snapshot The snapshot to use for the segment tests.
        /// @param [in] log      The log to write any output to.
        ///
        /// @return true if the test passed, false if not.
        bool TestRequestedAndBound(const RmtDataSnapshot& snapshot, const Log& log);
    };
}  // namespace backend_test

#endif  //  RMV_TEST_SEGMENT_INFO_H_
