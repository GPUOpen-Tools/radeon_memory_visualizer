//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the process map helper functions.
//=============================================================================

#include "rmt_process_map.h"
#include <string.h>  // for memset

RmtErrorCode RmtProcessMapInitialize(RmtProcessMap* process_map)
{
    RMT_RETURN_ON_ERROR(process_map, kRmtErrorInvalidPointer);

    memset(process_map->process_committed_memory, 0, sizeof(process_map->process_committed_memory));
    process_map->process_count = 0;
    return kRmtOk;
}

RmtErrorCode RmtProcessMapAddProcess(RmtProcessMap* process_map, uint64_t process_id)
{
    RMT_RETURN_ON_ERROR(process_map, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR((process_map->process_count + 1) < RMT_MAXIMUM_PROCESS_COUNT, kRmtErrorOutOfMemory);

    // if the process is already in the map we're done.
    if (RmtProcessMapContainsProcessId(process_map, process_id))
    {
        return kRmtOk;
    }

    // set into the next slot if not.
    process_map->process_identifiers[process_map->process_count++] = process_id;
    return kRmtOk;
}

bool RmtProcessMapContainsProcessId(const RmtProcessMap* process_map, uint64_t process_id)
{
    RMT_RETURN_ON_ERROR(process_map, false);

    for (int32_t current_process_index = 0; current_process_index < process_map->process_count; ++current_process_index)
    {
        if (process_map->process_identifiers[current_process_index] == process_id)
        {
            return true;
        }
    }

    return false;
}

RmtErrorCode RmtProcessMapGetIndexFromProcessId(const RmtProcessMap* process_map, uint64_t process_id, int32_t* out_index)
{
    RMT_RETURN_ON_ERROR(process_map, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_index, kRmtErrorInvalidPointer);

    for (int32_t current_process_index = 0; current_process_index < process_map->process_count; ++current_process_index)
    {
        if (process_map->process_identifiers[current_process_index] == process_id)
        {
            *out_index = current_process_index;
            return kRmtOk;
        }
    }

    *out_index = -1;
    return kRmtErrorIndexOutOfRange;
}

uint64_t RmtProcessMapGetCommittedMemoryForProcessId(const RmtProcessMap* process_map, uint64_t process_id)
{
    RMT_RETURN_ON_ERROR(process_map, 0);

    int32_t            index      = -1;
    const RmtErrorCode error_code = RmtProcessMapGetIndexFromProcessId(process_map, process_id, &index);
    if (error_code == kRmtOk)
    {
        return process_map->process_committed_memory[index];
    }

    return 0;
}

RmtErrorCode RmtProcessMapAddCommittedMemoryForProcessId(RmtProcessMap* process_map, uint64_t process_id, uint64_t size_in_bytes)
{
    RMT_RETURN_ON_ERROR(process_map, kRmtErrorInvalidPointer);

    int32_t            index      = -1;
    const RmtErrorCode error_code = RmtProcessMapGetIndexFromProcessId(process_map, process_id, &index);
    if (error_code == kRmtOk)
    {
        process_map->process_committed_memory[index] += size_in_bytes;
    }

    return kRmtOk;
}

RmtErrorCode RmtProcessMapRemoveCommittedMemoryForProcessId(RmtProcessMap* process_map, uint64_t process_id, uint64_t size_in_bytes)
{
    RMT_RETURN_ON_ERROR(process_map, kRmtErrorInvalidPointer);

    int32_t            index      = -1;
    const RmtErrorCode error_code = RmtProcessMapGetIndexFromProcessId(process_map, process_id, &index);
    if (error_code == kRmtOk)
    {
        if (size_in_bytes < process_map->process_committed_memory[index])
        {
            process_map->process_committed_memory[index] -= size_in_bytes;
        }
        else
        {
            process_map->process_committed_memory[index] = 0;
        }
    }

    return kRmtOk;
}
