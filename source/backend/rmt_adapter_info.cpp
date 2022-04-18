//=============================================================================
// Copyright (c) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  The adapter information captured for the target process.
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
    case kRmtAdapterInfoMemoryTypeDdr2:
        return "DDR2";
    case kRmtAdapterInfoMemoryTypeDdr3:
        return "DDR3";
    case kRmtAdapterInfoMemoryTypeDdr4:
        return "DDR4";
    case kRmtAdapterInfoMemoryTypeDdr5:
        return "DDR5";
    case kRmtAdapterInfoMemoryTypeGddr5:
        return "GDDR5";
    case kRmtAdapterInfoMemoryTypeGddr6:
        return "GDDR6";
    case kRmtAdapterInfoMemoryTypeHbm:
        return "HBM";
    case kRmtAdapterInfoMemoryTypeHbm2:
        return "HBM2";
    case kRmtAdapterInfoMemoryTypeHbm3:
        return "HBM3";
    case kRmtAdapterInfoMemoryTypeLpddr4:
        return "LPDDR4";
    case kRmtAdapterInfoMemoryTypeLpddr5:
        return "LPDDR5";

    default:
        return "";
    }
}
