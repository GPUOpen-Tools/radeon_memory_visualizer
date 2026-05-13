//=============================================================================
// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test main program.
//=============================================================================

#include <iostream>

#include "rmt_util.h"

#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

#include "rmv_test_cases.h"

int32_t main(int32_t argc, char** argv)
{
    std::string           input_file_path;              // The trace file to load.
    std::string           input_json_results_path;      // The JSON file used to create the trace (optional).
    bool                  add_snapshot        = false;  // Flag for adding a snapshot to the trace if set to true.
    RmtResourceIdentifier resource_identifier = 0;      // An optional resource ID used for tests.
    std::string           virtual_address_text;         // An optional virtual address used for tests.

    Catch::Session session;
    using namespace Catch::clara;

    // Add to Catch's composite command line parser.
    auto cli = Opt(input_file_path, "input_file_path")["--rmv"]("The path to the trace file") |
               Opt(add_snapshot, "true / false")["--snapshot"]("Add a snapshot to the trace file. Optional (default = false)") | session.cli();

    // Now pass the new composite back to Catch so it uses that.
    int test_result = -1;
    session.cli(cli);
    int result = session.applyCommandLine(argc, argv);
    if (result != 0)
    {
        return test_result;
    }

    // Convert virtual address string to an RmtGpuAddress value if present.
    RmtGpuAddress virtual_address = 0;
    if (!virtual_address_text.empty())
    {
        try
        {
            virtual_address = stoll(virtual_address_text, nullptr, 16);
        }

        // catch any exceptions for invalid address.
        catch (const std::invalid_argument& arg)
        {
            RMT_UNUSED(arg);
            Catch::cerr() << "Invalid virtual address specified.";
        }

        catch (const std::out_of_range& arg)
        {
            RMT_UNUSED(arg);
            Catch::cerr() << "Invalid virtual address specified.";
        }
    }

    if (RMVTestCases::Get()->Initialize(input_file_path, input_json_results_path, add_snapshot, resource_identifier, virtual_address) == true)
    {
        test_result = session.run();
        RMVTestCases::Get()->Shutdown();
    }

    // Test result will contain -1 if the tests were not run (some kind of initialization problem)
    // or an integer containing the number of tests that failed.
    return test_result;
}
