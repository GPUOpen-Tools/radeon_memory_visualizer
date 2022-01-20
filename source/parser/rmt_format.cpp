//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of functions related to RMT token structures.
//=============================================================================

#include "rmt_format.h"

uint8_t* AllocatePayloadCache(size_t size)
{
    return new uint8_t[size];
}

void DeallocatePayloadCache(uint8_t* payload_cache)
{
    delete[] payload_cache;
}
