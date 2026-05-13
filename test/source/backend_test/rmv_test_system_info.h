//=============================================================================
// Copyright (c) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test system info header.
//=============================================================================

#ifndef RMV_BACKEND_TEST_RMV_TEST_SYSTEM_INFO_H_
#define RMV_BACKEND_TEST_RMV_TEST_SYSTEM_INFO_H_

#include "rmt_data_set.h"
#include "rmt_rdf_system_info.h"

#include "log.h"

namespace backend_test
{
    class RMVTestSystemInfo
    {
    public:
        /// @brief Constructor.
        RMVTestSystemInfo();

        /// @brief Destructor.
        ~RMVTestSystemInfo();

        /// @brief Test the system_info data and validate that sensible numbers are returned.
        ///
        /// @param [in] system_info    The system info object to test.
        /// @param [in] is_rdf         Is this a new style RDF file.
        /// @param [in] log            The log to write any output to.
        ///
        /// @return true if the tests passed, false if not.
        bool RunTests(const RmtRdfSystemInfo& system_info, bool is_rdf, const Log& log);

    private:
        /// @brief Test an unsigned integer value.
        ///
        /// @param [in]  text   The text describing the parameter being tested.
        /// @param [in]  units  The text describing the units of the parameter being tested.
        /// @param [in]  value  The value of the parameter being tested.
        /// @param [in]  limit  The lower limit of the value being tested.
        /// @param [in]  log    The log to write any output to.
        /// @param [out] result If the test fails, contains the test failure result.
        void TextUintValue(const char* text, const char* units, uint32_t value, uint32_t limit, const Log& log, bool* result);

        /// @brief Make sure the GPU name string is valid.
        ///
        /// Check it doesn't contain the word 'unknown'.
        ///
        /// @param [in]  system_info  The system info object to test.
        /// @param [in]  log          The log to write any output to.
        /// @param [out] result       If the test fails, contains the test failure result.
        void TestName(const RmtRdfSystemInfo& system_info, const Log& log, bool* result);

        /// @brief Make sure the memory type is valid.
        ///
        /// @param [in]  system_info  The system info object to test.
        /// @param [in]  log          The log to write any output to.
        /// @param [out] result       If the test fails, contains the test failure result.
        void TestMemoryType(const RmtRdfSystemInfo& system_info, const Log& log, bool* result);
    };
}  // namespace backend_test

#endif  //  RMV_BACKEND_TEST_RMV_TEST_SYSTEM_INFO_H_
