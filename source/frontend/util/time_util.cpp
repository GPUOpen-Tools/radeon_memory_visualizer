//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a number of time-related utilities.
//=============================================================================

#include "util/time_util.h"

#include "qt_common/utils/qt_util.h"

#include "rmt_assert.h"
#include "rmt_error.h"
#include "rmt_data_set.h"

#include "managers/trace_manager.h"
#include "settings/rmv_settings.h"

namespace rmv
{
    QString time_util::ClockToTimeUnit(uint64_t clk)
    {
        double time = 0.0;

        TimeUnitType unit_type = RMVSettings::Get().GetUnits();

        if (unit_type == kTimeUnitTypeClk)
        {
            time = clk;
        }
        else
        {
            TraceManager& trace_manager = TraceManager::Get();
            if (trace_manager.DataSetValid())
            {
                // This should only get called if the CPU clock timestamp is valid.
                const RmtDataSet* data_set   = trace_manager.GetDataSet();
                RmtErrorCode      error_code = RmtDataSetGetCpuClockTimestamp(data_set, clk, &time);
                RMT_UNUSED(error_code);
                RMT_ASSERT(error_code == kRmtOk);
            }
        }

        return QtCommon::QtUtils::ClockToTimeUnit(time, unit_type);
    }

    double time_util::TimeToClockRatio()
    {
        double ratio = 1.0;
        if (RMVSettings::Get().GetUnits() != kTimeUnitTypeClk)
        {
            TraceManager& trace_manager = TraceManager::Get();
            if (trace_manager.DataSetValid())
            {
                const RmtDataSet* data_set = trace_manager.GetDataSet();
                RmtDataSetGetCpuClockTimestamp(data_set, 1, &ratio);
            }
            RMT_ASSERT(ratio > 0.0);
        }
        return ratio;
    }
}  // namespace rmv