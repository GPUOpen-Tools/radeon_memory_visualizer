//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Structures and functions for working with a data set.
//=============================================================================

#ifndef RMV_BACKEND_RMT_DATA_SET_H_
#define RMV_BACKEND_RMT_DATA_SET_H_

#include "rmt_configuration.h"
#include "rmt_data_profile.h"
#include "rmt_data_timeline.h"
#include "rmt_file_format.h"
#include "rmt_parser.h"
#include "rmt_physical_allocation_list.h"
#include "rmt_process_map.h"
#include "rmt_process_start_info.h"
#include "rmt_rdf_system_info.h"
#include "rmt_resource_userdata.h"
#include "rmt_segment_info.h"
#include "rmt_token_heap.h"
#include "rmt_types.h"
#include "rmt_virtual_allocation_list.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

typedef struct RmtDataSnapshot    RmtDataSnapshot;
typedef struct RmtResource        RmtResource;
typedef struct RmtResourceHistory RmtResourceHistory;
typedef void*                     RmtSnapshotWriterHandle;  ///< The type definition for the snapshot writer handle.

/// Callback function prototype for allocating memory.
typedef void* (*RmtDataSetAllocationFunc)(size_t size_in_bytes, size_t alignment);

/// Callback function prototype for freeing memory.
typedef void (*RmtDataSetFreeFunc)(void* buffer);

/// A structure encapsulating a single snapshot point.
typedef struct RmtSnapshotPoint
{
    char             name[RMT_MAXIMUM_NAME_LENGTH];  ///< The name of the snapshot.
    uint64_t         timestamp;                      ///< The point at which the snapshot was taken.
    uint64_t         file_offset;                    ///< The file offset for snapshot management.
    RmtDataSnapshot* cached_snapshot;                ///< A pointer to a <c><i>RmtDataSnapshot</i></c> that has been created for this snapshot point.
    int32_t          virtual_allocations;
    int32_t          resource_count;
    uint64_t         total_virtual_memory;
    uint64_t         bound_virtual_memory;
    uint64_t         unbound_virtual_memory;
    uint64_t         committed_memory[kRmtHeapTypeCount];
    uint16_t         chunk_index;  ///< The index of the snapshot data chunk in the RDF file (not used for legacy traces).
} RmtSnapshotPoint;

// Various flags used by the dataset.
struct RmtDataSetFlags
{
    bool read_only : 1;                    ///< Whether the dataset is loaded as read-only.
    bool sam_enabled : 1;                  ///< Whether the dataset is SAM (smart access memory) enabled.
    bool is_rdf_trace : 1;                 ///< Whether the dataset is generated from an RDF file.
    bool userdata_processed : 1;           ///< Whether the userdata tokens have been processed yet.
    bool contains_correlation_tokens : 1;  ///< Whether the dataset contains any correlation tokens.
    bool cancel_background_task_flag : 1;  ///< If true, indicates a background task has been cancelled.
};

/// A structure encapsulating a single RMT dataset.
typedef struct RmtDataSet
{
    char   file_path[RMT_MAXIMUM_FILE_PATH];            ///< The file path to the file being worked with.
    char   temporary_file_path[RMT_MAXIMUM_FILE_PATH];  ///< The file path to the safe temporary file being worked with.
    FILE*  file_handle;                                 ///< The handle to the RMT file (operates on the temporary).
    size_t file_size_in_bytes;                          ///< The size of the file pointed to by <c><i>fileHandle</i></c> in bytes.
    time_t create_time;                                 ///< The time the trace was created.

    RmtDataSetAllocationFunc allocate_func;  ///< Allocate memory function pointer.
    RmtDataSetFreeFunc       free_func;      ///< Free memory function pointer.

    RmtParser       streams[RMT_MAXIMUM_STREAMS];  ///< An <c><i>RmtParser</i></c> structure for each stream in the file.
    int32_t         stream_count;                  ///< The number of RMT streams in the file.
    RmtStreamMerger stream_merger;                 ///< Token heap.

    RmtRdfSystemInfo system_info;  ///< The system information.

    RmtSegmentInfo segment_info[RMT_MAXIMUM_SEGMENTS];  ///< An array of segment information.
    int32_t        segment_info_count;                  ///< The number of segments.

    RmtProcessStartInfo process_start_info[RMT_MAXIMUM_PROCESS_COUNT];  ///< An array of process start information.
    int32_t             process_start_info_count;  ///< The number of <c><i>RmtProcessStartInfo</i></c> structures in <c><i>processStartInfo</i></c>.
    RmtProcessMap       process_map;               ///< A map of processes seen in the RMT file.

    RmtSnapshotPoint snapshots[RMT_MAXIMUM_SNAPSHOT_POINTS];  ///< An array of all snapshots in the data set.
    int32_t          snapshot_count;                          ///< The number of snapshots used.

    RmtDataProfile data_profile;  ///< The data profile which is populated in the 1st pass of the parser.

    uint64_t maximum_timestamp;  ///< The maximum timestamp seen in this data set.
    uint32_t cpu_frequency;      ///< The CPU frequency (in clock ticks per second) of the machine where the RMT data was captured.
    uint64_t target_process_id;  ///< The target process ID that was traced.

    RmtVirtualAllocationList  virtual_allocation_list;   ///< Temporary virtual allocation list.
    RmtPhysicalAllocationList physical_allocation_list;  ///< Temporary physical allocation list.

    ResourceIdMapAllocator* resource_id_map_allocator;  ///< Allocator buffer/struct used to do lookup of unique resource ID.

    uint32_t                active_gpu;              ///< The active GPU used by the application process that was captured.
    RmtSnapshotWriterHandle snapshot_writer_handle;  ///< The object responsible for writing snapshots to the trace file.

    struct RmtDataSetFlags flags;  ///< The dataset flags, described above.

} RmtDataSet;

/// Initialize the RMT data set from a file path.
///
/// In order to avoid accidental corruption of the file being opened. The RMT backend
/// will make a temporary copy of the file with which to work. Modifications are done
/// to this copy, and then calls to <c><i>rmtDataSetDestroy</i></c> will commit those
/// edits back to the original by way of a file rename.
///
/// The reason for this implementation choice is because changes to the file system
/// metadata are atomic, where as changes to the contents of the file are not. This
/// means if a crash of the application (or wider system) where to happen during a
/// change to the file, then the RMV file may be rendered corrupted and unusable. If
/// we instead copy the file on load, and work in the temporary copy, and then, using
/// only metadata edits (i.e.: rename and delete), place that back where the orignial
/// file was. Then even if the system were to crash, the integrity of the original
/// RMV file is always preserved.
///
/// @param [in]  path                                       A pointer to a string containing the path to the RMT file that we would like to load to initialize the data set.
/// @param [in]  data_set                                   A pointer to a <c><i>RmtDataSet</i></c> structure that will contain the data set.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>data_set</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtDataSetInitialize(const char* path, RmtDataSet* data_set);

/// Destroy the data set.
///
/// @param [in]  data_set                       A pointer to a <c><i>RmtDataSet</i></c> structure that will contain the data set.
///
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>data_set</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtDataSetDestroy(RmtDataSet* data_set);

/// Generate a timeline from the data set.
///
/// @param [in]  data_set                                   A pointer to a <c><i>RmtDataSet</i></c> structure to used to generate the timeline.
/// @param [in]  timeline_type                              The type of timeline to generate.
/// @param [out] out_timeline                               The address of a <c><i>RmtDataTimeline</i></c> structure to populate.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>data_set</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorOutOfMemory                        The operation failed due as memory could not be allocated to create the timeline.
RmtErrorCode RmtDataSetGenerateTimeline(RmtDataSet* data_set, RmtDataTimelineType timeline_type, RmtDataTimeline* out_timeline);

/// Genereate a snapshot from a data set at a specific time.
///
/// @param [in]  data_set                                   A pointer to a <c><i>RmtDataSet</i></c> structure to used to generate the timeline.
/// @param [in]  timestamp                                  The timestamp to generate the snapshot for.
/// @param [in]  name                                       The name of the snapshot.
/// @param [out] out_snapshot                               The address of a <c><i>RmtDataSnapshot</i></c> structure to populate.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>data_set</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorOutOfMemory                        The operation failed due as memory could not be allocated to create the snapshot.
RmtErrorCode RmtDataSetGenerateSnapshot(RmtDataSet* data_set, RmtSnapshotPoint* snapshot_point, RmtDataSnapshot* out_snapshot);

/// Find a segment from a physical address.
///
/// @param [in]  data_set                                   A pointer to a <c><i>RmtDataSet</i></c> structure.
/// @param [in]  physical_address                           The physical address to find the parent segment for.
/// @param [out] out_segment_info                           The address of a pointer to a <c><i>RmtSegmentInfo</i></c> structure. This will be set to <c><i>NULL</i></c> if the segment wasn't found.
///
/// @retval
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>data_set</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorNoAllocationFound                  The operation failed due to <c><i>physicalAddress</i></c> not being present in the segment info.
RmtErrorCode RmtDataSetGetSegmentForPhysicalAddress(const RmtDataSet* data_set, RmtGpuAddress physical_address, const RmtSegmentInfo** out_segment_info);

/// Get the time corresponding to the given number of cpu clock cycles
///
/// @param [in]  data_set                                   A pointer to a <c><i>RmtDataSet</i></c> structure.
/// @param [in]  clk                                        The number of clock cycles
/// @param [out] out_cpu_timestamp                          The time, in nanoseconds, corresponding to the number of clock cycles
///
/// @returns
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>data_set</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorTimestampOutOfBounds               The operation failed due to the CPU clock being invalid.
RmtErrorCode RmtDataSetGetCpuClockTimestamp(const RmtDataSet* data_set, uint64_t clk, double* out_cpu_timestamp);

/// Get whether the CPU clock timestamp is valid
///
/// @param [in]  data_set                                   A pointer to a <c><i>RmtDataSet</i></c> structure.
///
/// @returns
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>data_set</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorTimestampOutOfBounds               The operation failed due to the CPU clock being invalid.
RmtErrorCode RmtDataSetGetCpuClockTimestampValid(const RmtDataSet* data_set);

/// Add a new snapshot to the file.
///
/// @param [in]  data_set                                   A pointer to a <c><i>RmtDataSet</i></c> structure.
/// @param [in]  name                                       The name of the snapshot.
/// @param [in]  timestamp                                  The time at which the snapshot was created.
/// @param [out] out_snapshot_point                         The address of a pointer to a <c><i>RmtSnapshotPoint</i></c> structure.
///
/// @returns
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>data_set</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtDataSetAddSnapshot(RmtDataSet* data_set, const char* name, uint64_t timestamp, RmtSnapshotPoint** out_snapshot_point);

/// Remove a new snapshot to the file.
///
/// Using this function may change the order of snapshot points in <c><i>data_set</i></c>. If you
/// have code that is relying on this order by use of index, then you should make sure you update
/// those indices after a call to this function.
///
/// @param [in]  data_set                                   A pointer to a <c><i>RmtDataSet</i></c> structure.
/// @param [in]  snapshot_index                             The index of the snapshot to delete.
/// @param [in]  open_snapshot                              A pointer to the open snapshot (or nullptr if no open snapshot)
///
/// @returns
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>data_set</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtDataSetRemoveSnapshot(RmtDataSet* data_set, const int32_t snapshot_index, RmtDataSnapshot* open_snapshot);

/// Rename an existing snapshot in the file.
///
/// @param [in]  data_set                                   A pointer to a <c><i>RmtDataSet</i></c> structure.
/// @param [in]  snapshot_index                             The index of the snapshot to rename.
/// @param [in]  name                                       The name of the snapshot.
///
/// @returns
/// kRmtOk                                      The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                     The operation failed due to <c><i>data_set</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtDataSetRenameSnapshot(RmtDataSet* data_set, const int32_t snapshot_index, const char* name);

/// Get the index in level-0 of a series for a specified timestamp.
///
/// @param [in]  data_set                                   A pointer to a <c><i>RmtDataSet</i></c> structure.
/// @param [in]  timestamp                                  The timestamp to convert.
///
/// @returns
/// The series index.
int32_t RmtDataSetGetSeriesIndexForTimestamp(RmtDataSet* data_set, uint64_t timestamp);

/// Get the total video memory for the specified data set.
///
/// @param [in]  data_set                                   A pointer to a <c><i>RmtDataSet</i></c> structure.
///
/// @returns                                                The amount of video memory, in bytes.

uint64_t RmtDataSetGetTotalVideoMemoryInBytes(const RmtDataSet* data_set);

/// Check the cancel flag for the datasets background task.
///
/// @param [in]  data_set                                   A pointer to a <c><i>RmtDataSet</i></c> structure.
///
/// @returns                                                True if the cancel flag is set, otherwise false.
bool RmtDataSetIsBackgroundTaskCancelled(const RmtDataSet* data_set);

/// Set the cancel flag for a dataset background task.
///
/// @param [in]  data_set                                   A pointer to a <c><i>RmtDataSet</i></c> structure.
///
void RmtDataSetCancelBackgroundTask(RmtDataSet* data_set);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_DATA_SET_H_
