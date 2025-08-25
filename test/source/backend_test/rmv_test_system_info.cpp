//=============================================================================
// Copyright (c) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test system info implementation.
//=============================================================================

#include <string.h>  // For strstr.
#include <string>

#include "rmt_print.h"

#include "rmv_test_system_info.h"

namespace backend_test
{
    RMVTestSystemInfo::RMVTestSystemInfo()
    {
    }

    RMVTestSystemInfo::~RMVTestSystemInfo()
    {
    }

    bool RMVTestSystemInfo::RunTests(const RmtRdfSystemInfo& system_info, bool is_rdf, const Log& log)
    {
        bool result = true;

        log.Write("TestSystemInfo");
        if (is_rdf)
        {
            log.Write(" CPU Name: %s", system_info.cpu_name);
        }

        TestName(system_info, log, &result);
        TestMemoryType(system_info, log, &result);

        TextUintValue("maximum engine clock", "Mhz", system_info.maximum_engine_clock, 100, log, &result);
        TextUintValue("maximum memory clock", "Mhz", system_info.maximum_memory_clock, 100, log, &result);
#ifndef _LINUX
        // Disable min clock test on Linux for now.
        TextUintValue("minimum engine clock", "Mhz", system_info.minimum_engine_clock, 50, log, &result);
        TextUintValue("minimum memory clock", "Mhz", system_info.minimum_memory_clock, 50, log, &result);
#endif  // _LINUX
        TextUintValue("memory bus width", "bits", system_info.memory_bus_width, 8, log, &result);
        TextUintValue("memory bandwidth", "GB/s", system_info.memory_bandwidth / 1000, 25, log, &result);

        if (is_rdf)
        {
            log.Write(" Driver packaging version: %s", system_info.driver_packaging_version_name);
            log.Write(" Driver software version: %s", system_info.driver_software_version_name);
        }

        return result;
    }

    void RMVTestSystemInfo::TextUintValue(const char* text, const char* units, uint32_t value, uint32_t limit, const Log& log, bool* result)
    {
        if (value >= limit)
        {
            log.Write(" %s: %d %s", text, value, units);
        }
        else
        {
            log.Write(" ERROR: %s reported as %d %s", text, value, units);
            *result = false;
        }
    }

    void RMVTestSystemInfo::TestName(const RmtRdfSystemInfo& system_info, const Log& log, bool* result)
    {
        char lower_name[RMT_MAX_ADAPTER_NAME_LENGTH];

        for (int i = 0; i < RMT_MAX_ADAPTER_NAME_LENGTH; i++)
        {
            lower_name[i] = static_cast<char>(tolower(static_cast<char>(system_info.name[i])));
        }

        if (strstr(lower_name, "unknown") == nullptr)
        {
            log.Write(" GPU name: %s", system_info.name);
        }
        else
        {
            log.Write(" ERROR: name '%s' is invalid", system_info.name);
            *result = false;
        }
    }

    void RMVTestSystemInfo::TestMemoryType(const RmtRdfSystemInfo& system_info, const Log& log, bool* result)
    {
        if (system_info.video_memory_type_name[0] != '\0')
        {
            log.Write(" memory type: %s", system_info.video_memory_type_name);
        }
        else
        {
            log.Write(" ERROR: memory type is null");
            *result = false;
        }
    }

}  // namespace backend_test
