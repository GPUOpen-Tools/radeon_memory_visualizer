//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Data structures and functions to help construct a map of proc IDs.
//=============================================================================

#ifndef RMV_BACKEND_RMT_PROCESS_MAP_H_
#define RMV_BACKEND_RMT_PROCESS_MAP_H_

#include "rmt_error.h"
#include "rmt_configuration.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// A structure encapsulating a set of process IDs.
typedef struct RmtProcessMap
{
    uint64_t process_identifiers[RMT_MAXIMUM_PROCESS_COUNT];       ///< The 32bit process identifier.
    uint64_t process_committed_memory[RMT_MAXIMUM_PROCESS_COUNT];  ///< The amount of committed memory (in bytes) per process.
    int32_t  process_count;                                        ///< The number of processes.
} RmtProcessMap;

/// Initialize the process map.
///
/// @param [in]  process_map                    A pointer to a <c><i>RmtProcessMap</i></c> structure that will contain the process map.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>process_map</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtProcessMapInitialize(RmtProcessMap* process_map);

/// Add a process ID to the map.
///
/// @param [in]  process_map                    A pointer to a <c><i>RmtProcessMap</i></c> structure that contains the process map.
/// @param [in]  process_id                     The process ID to add.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>process_map</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorOutOfMemory                        The operation failed due to the number of processes in the map being greater than <c><i>RMT_MAXIMUM_PROCESS_COUNT</i></c>.
RmtErrorCode RmtProcessMapAddProcess(RmtProcessMap* process_map, uint64_t process_id);

/// Get the amount of committed memory (in bytes) for a specified process ID.
///
/// @param [in]  process_map                    A pointer to a <c><i>RmtProcessMap</i></c> structure that contains the process map.
/// @param [in]  process_id                     The specified process ID.
///
/// @return
/// The amount of committed memory, in bytes.
uint64_t RmtProcessMapGetCommittedMemoryForProcessId(const RmtProcessMap* process_map, uint64_t process_id);

/// Add some memory (in bytes) for a specified process ID.
///
/// @param [in]  process_map                    A pointer to a <c><i>RmtProcessMap</i></c> structure that contains the process map.
/// @param [in]  process_id                     The specified process ID.
/// @param [in]  size_in_bytes                  The amount of memory to add.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>process_map</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtProcessMapAddCommittedMemoryForProcessId(RmtProcessMap* process_map, uint64_t process_id, uint64_t size_in_bytes);

/// Remove some memory (in bytes) from a specified process ID.
///
/// @param [in]  process_map                    A pointer to a <c><i>RmtProcessMap</i></c> structure that contains the process map.
/// @param [in]  process_id                     The specified process ID.
/// @param [in]  size_in_bytes                  The amount of memory to remove.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>process_map</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtProcessMapRemoveCommittedMemoryForProcessId(RmtProcessMap* process_map, uint64_t process_id, uint64_t size_in_bytes);

/// Query the process map to see if it contains the specified process ID.
///
/// @param [in]  process_map                    A pointer to a <c><i>RmtProcessMap</i></c> structure that contains the process map.
/// @param [in]  process_id                     The specified process ID.
///
/// @returns
/// true if process ID is in the map, false if not.
bool RmtProcessMapContainsProcessId(const RmtProcessMap* process_map, uint64_t process_id);

/// Get the index of a process from a process ID.
///
/// @param [in]  process_map                    A pointer to a <c><i>RmtProcessMap</i></c> structure that contains the process map.
/// @param [in]  process_id                     The specified process ID.
/// @param [out] out_index                      Pointer to an <c><i>int32_t</i></c> to receive the index.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>process_map</i></c> or <c><i>out_index</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtProcessMapGetIndexFromProcessId(const RmtProcessMap* process_map, uint64_t process_id, int32_t* out_index);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_PROCESS_MAP_H_
