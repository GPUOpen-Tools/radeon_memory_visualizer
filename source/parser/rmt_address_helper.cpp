//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Game Engineering Group
/// \brief Implementation of helper functions for working with addresses.
//=============================================================================

#include "rmt_address_helper.h"
#include <rmt_assert.h>

uint64_t RmtGetPageSize(RmtPageSize page_size)
{
    switch (page_size)
    {
    case kRmtPageSize4Kb:
        return 4 * 1024;
    case kRmtPageSize64Kb:
        return 64 * 1024;
    case kRmtPageSize256Kb:
        return 256 * 1024;
    case kRmtPageSize1Mb:
        return 1024 * 1024;
    case kRmtPageSize2Mb:
        return 2 * 1024 * 1024;
    default:
        return 0;
    }
}

uint64_t RmtGetAllocationSizeInBytes(uint64_t size_in_pages, RmtPageSize page_size)
{
    const uint64_t page_size_in_bytes = RmtGetPageSize(page_size);
    return size_in_pages * page_size_in_bytes;
}

bool RmtAllocationsOverlap(RmtGpuAddress base_address1, uint64_t size_in_bytes1, RmtGpuAddress base_address2, uint64_t size_in_bytes2)
{
    // Case 1: |---2----| |----1----|
    if (base_address1 > (base_address2 + size_in_bytes2))
    {
        return false;
    }

    // Case 2: |---1----| |----2----|
    if (base_address2 > (base_address1 + size_in_bytes1))
    {
        return false;
    }

    // Case 3: |--------1------------|
    //                   |----2---|
    // Case 4: |-----1-----|
    //                  |-----2-----|
    // Case 5: |-----2-----|
    //                  |-----1------|
    // Case 6: |-----2-------------------|
    //                 |----1----|
    return true;
}
