//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Game Engineering Group
/// \brief  The adapter information captured for the target process.
//=============================================================================

#include "rmt_adapter_info.h"
#include "rmt_assert.h"

const char* RmtAdapterInfoGetVideoMemoryType(const RmtAdapterInfo* adapter_info)
{
    RMT_ASSERT(adapter_info);

    switch (adapter_info->memory_type)
    {
    case kRmtAdapterInfoMemoryTypeUnknown:
        return "Unknown";
    case kRmtAdapterInfoMemoryTypeDdR2:
        return "DDR2";
    case kRmtAdapterInfoMemoryTypeDdR3:
        return "DDR3";
    case kRmtAdapterInfoMemoryTypeDdR4:
        return "DDR4";
    case kRmtAdapterInfoMemoryTypeGddR5:
        return "GDDR5";
    case kRmtAdapterInfoMemoryTypeGddR6:
        return "GDDR6";
    case kRmtAdapterInfoMemoryTypeHbm:
        return "HBM";
    case kRmtAdapterInfoMemoryTypeHbm2:
        return "HBM2";
    case kRmtAdapterInfoMemoryTypeHbm3:
        return "HBM3";

    default:
        return "";
    }
}
