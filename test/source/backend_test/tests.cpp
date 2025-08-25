//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend catch2 test cases.
//=============================================================================

#include <catch.hpp>

#include "rmv_test_cases.h"

TEST_CASE("TestSystemInfo", "RMVTest")
{
    bool result = RMVTestCases::Get()->TestSystemInfo();
    REQUIRE(result == true);
}

TEST_CASE("TestSegmentInfo", "RMVTest")
{
    bool result = RMVTestCases::Get()->TestSegmentInfo();
    REQUIRE(result == true);
}

TEST_CASE("TestAllocations", "RMVTest")
{
    bool result = RMVTestCases::Get()->TestAllocations();
    REQUIRE(result == true);
}

TEST_CASE("TestResources", "RMVTest")
{
    bool result = RMVTestCases::Get()->TestResources();
    REQUIRE(result == true);
}

TEST_CASE("TestTimestamps", "RMVTest")
{
    bool result = RMVTestCases::Get()->TestTimestamps();
    REQUIRE(result == true);
}
