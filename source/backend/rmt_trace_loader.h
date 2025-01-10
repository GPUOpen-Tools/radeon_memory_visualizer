//=============================================================================
// Copyright (c) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Trace loader structures and functions.
//=============================================================================

#ifndef RMV_BACKEND_RMT_TRACE_LOADER_H_
#define RMV_BACKEND_RMT_TRACE_LOADER_H_

#include "rmt_data_set.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// @brief Get a pointer to the internal data set.
///
/// @return Pointer to the currently loaded data set.
RmtDataSet* RmtTraceLoaderGetDataSet();

/// @brief Get a pointer to the internal timeline.
///
/// @return Pointer to the internal timeline.
RmtDataTimeline* RmtTraceLoaderGetTimeline();

/// @brief Is the data set valid?
///
/// @return true if the data set is valid, false if not.
bool RmtTraceLoaderDataSetValid();

/// @brief Load a trace.
///
/// @param[in] trace_file_name  The trace file to load.
/// @param[in] reporter_function  The callback function used to report errors.
///
/// @return an RmtErrorCode indicating whether the trace loaded correctly.
RmtErrorCode RmtTraceLoaderTraceLoad(const char* trace_file_name, RmtDataSetErrorReportFunc reporter_function = nullptr);

/// @brief Clear out the trace data.
void RmtTraceLoaderClearTrace();

/// @brief Get a snapshot point from the loaded trace.
///
/// @param[in] index  The snapshot index.
///
/// @return The snapshot point.
RmtSnapshotPoint* RmtTraceLoaderGetSnapshotPoint(int32_t index);

/// @brief Get the number of snapshots in the trace.
///
/// @return The number of snapshots.
int32_t RmtTraceLoaderGetSnapshotCount();

#ifdef _LINUX
/// @brief Determine if a trace file is already opened by another process.
///
/// @param[in] trace_file_path  The full path of the trace file.
///
/// @return Determine if a trace file is already opened by another process.
bool RmtTraceLoaderIsTraceAlreadyInUse(const char* trace_file_path);
#endif  // #ifdef _LINUX

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_TRACE_LOADER_H_
