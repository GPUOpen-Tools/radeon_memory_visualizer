//=============================================================================
// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test resource header.
//=============================================================================

#ifndef RMV_BACKEND_TEST_RMV_TEST_RESOURCES_H_
#define RMV_BACKEND_TEST_RMV_TEST_RESOURCES_H_

#include "rmt_data_snapshot.h"

#include "log.h"

namespace backend_test
{
    class RMVTestResources
    {
    public:
        /// @brief Constructor.
        RMVTestResources();

        /// @brief Destructor.
        ~RMVTestResources();

        /// @brief Run all the resource tests.
        ///
        /// @param [in] snapshot_index The index of the snapshot being tested.
        /// @param [in] snapshot       The snapshot to use for the resource tests.
        /// @param [in] log            The log to write any output to.
        ///
        /// @return true if the tests passed, false if not.
        bool RunTests(int32_t snapshot_index, const RmtDataSnapshot& snapshot, const Log& log);

    private:
        /// @brief Test the resource array for basic validity.
        ///
        /// Make sure pointers are non-nullptr and the resource count is sensible.
        ///
        /// @param [in] snapshot The snapshot to use for the resource tests.
        /// @param [in] log      The log to write any output to.
        ///
        /// @return true if the test passed, false if failed.
        bool TestResourceArray(const RmtDataSnapshot& snapshot, const Log& log);

        /// @brief Test the resource creation flags.
        ///
        /// Ensure that they don't use any invalid or reserved flags.
        ///
        /// @param [in]  snapshot The snapshot to use for the resource tests.
        /// @param [in]  log      The log to write any output to.
        /// @param [out] result   The test result, passed in from previous tests. This is modified to be false if this test fails.
        void TestResourceCreateFlags(const RmtDataSnapshot& snapshot, const Log& log, bool* result);

        /// @brief Test the resource usage types.
        ///
        /// Ensure there are no invalid resource usage types.
        ///
        /// @param [in]  snapshot The snapshot to use for the resource tests.
        /// @param [in]  log      The log to write any output to.
        /// @param [out] result   The test result, passed in from previous tests. This is modified to be false if this test fails.
        void TestResourceUsageTypes(const RmtDataSnapshot& snapshot, const Log& log, bool* result);

        /// @brief Test resource sizes and total size for validity.
        ///
        /// @param [in]  snapshot The snapshot to use for the resource tests.
        /// @param [in]  log      The log to write any output to.
        /// @param [out] result   The test result, passed in from previous tests. This is modified to be false if this test fails.
        void TestResourceSizes(const RmtDataSnapshot& snapshot, const Log& log, bool* result);

        /// @brief Test resource timestamps. Make sure they're all less than the global timestamp.
        ///
        /// @param [in]  snapshot The snapshot to use for the resource tests.
        /// @param [in]  log      The log to write any output to.
        /// @param [out] result   The test result, passed in from previous tests. This is modified to be false if this test fails.
        void TestResourceTimestamps(const RmtDataSnapshot& snapshot, const Log& log, bool* result);
    };
}  // namespace backend_test

#endif  //  RMV_BACKEND_TEST_RMV_TEST_RESOURCES_H_
