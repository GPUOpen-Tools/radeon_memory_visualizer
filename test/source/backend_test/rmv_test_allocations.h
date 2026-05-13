//=============================================================================
// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test allocation header.
//=============================================================================

#ifndef RMV_BACKEND_TEST_RMV_TEST_ALLOCATIONS_H_
#define RMV_BACKEND_TEST_RMV_TEST_ALLOCATIONS_H_

#include "rmt_data_snapshot.h"

#include "log.h"

namespace backend_test
{
    class RMVTestAllocations
    {
    public:
        /// @brief Constructor.
        RMVTestAllocations();

        /// @brief Destructor.
        ~RMVTestAllocations();

        /// @brief Run test cases for resources.
        ///
        /// @param [in] snapshot_index The index of the snapshot being tested.
        /// @param [in] snapshot       The snapshot to use for the allocation tests.
        /// @param [in] log            The log to write any output to.
        ///
        /// @return true if the tests passed, false if not.
        bool RunTests(int32_t snapshot_index, const RmtDataSnapshot& snapshot, const Log& log);

    private:
        /// @brief Test the virtual allocation array for basic validity.
        ///
        /// Make sure pointers are non-nullptr and the resource count is sensible.
        ///
        /// @param [in] snapshot The snapshot to use for the allocation tests.
        /// @param [in] log      The log to write any output to.
        ///
        /// @return true if the test passed, false if failed.
        bool TestVirtualAllocationArray(const RmtDataSnapshot& snapshot, const Log& log);

        /// @brief Test the virtual allocation sizes and total size for validity.
        ///
        /// @param [in]  snapshot The snapshot to use for the allocation tests.
        /// @param [in]  log      The log to write any output to.
        /// @param [out] result   The test result, passed in from previous tests. This is modified to be false if this test fails.
        void TestVirtualAllocationSizes(const RmtDataSnapshot& snapshot, const Log& log, bool* result);

        /// @brief Test the virtual allocation bindings.
        ///
        /// For each resource in the allocation, ensure that its bound allocation points to this allocation.
        ///
        /// @param [in]  snapshot The snapshot to use for the allocation tests.
        /// @param [in]  log      The log to write any output to.
        /// @param [out] result   The test result, passed in from previous tests. This is modified to be false if this test fails.
        void TestVirtualAllocationBindings(const RmtDataSnapshot& snapshot, const Log& log, bool* result);

        /// @brief Test the allocation guid in the back end.
        ///
        /// This has been changed to be the same as the index of the allocation in the allocation list.
        /// This test will fail if the value of the guid is changed.
        ///
        /// @param [in]  snapshot The snapshot to use for the allocation tests.
        /// @param [in]  log      The log to write any output to.
        /// @param [out] result   The test result, passed in from previous tests. This is modified to be false if this test fails.
        void TestVirtualAllocationGuids(const RmtDataSnapshot& snapshot, const Log& log, bool* result);

        /// @brief Test the total size of resources in each allocation.
        ///
        /// Will take into account aliased resources. Compares the result calculated by the test kit against a value calculated
        /// in the backend and make sure they are the same. Any differences may indicate problems with the implementation.
        ///
        /// @param [in]  snapshot The snapshot to use for the allocation tests.
        /// @param [in]  log      The log to write any output to.
        /// @param [out] result   The test result, passed in from previous tests. This is modified to be false if this test fails.
        void TestVirtualAllocationResourceSizes(const RmtDataSnapshot& snapshot, const Log& log, bool* result);

        /// @brief Get the amount of memory used by resources in the allocation.
        ///
        /// @param [in]  virtual_allocation  The virtual allocation to process.
        /// @param [in]  log                 The log to write any output to.
        /// @param [in]  dump_to_log         If true, dump out debug information to the log file.
        ///
        /// @return The total memory used by resources, in bytes.
        uint64_t GetTotalResourceSizeInAllocation(const RmtVirtualAllocation* virtual_allocation, const Log& log, bool dump_to_log);
    };
}  // namespace backend_test

#endif  //  RMV_BACKEND_TEST_RMV_TEST_ALLOCATIONS_H_
