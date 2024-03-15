//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for rmv_util which holds useful utility functions.
//=============================================================================

#ifndef RMV_UTIL_RMV_UTIL_H_
#define RMV_UTIL_RMV_UTIL_H_

#include <vector>

#include "rmt_virtual_allocation_list.h"
#include "util/definitions.h"

/// @brief The state of a snapshot in the timeline pane.
enum SnapshotState
{
    kSnapshotStateNone,
    kSnapshotStateViewed,
    kSnapshotStateCompared,

    kSnapshotStateCount,
};

/// @brief The state of a comparison between 2 snapshot paramters.
enum DeltaChange
{
    kDeltaChangeIncrease,
    kDeltaChangeDecrease,
    kDeltaChangeNone,

    kDeltaChangeCount,
};

namespace rmv_util
{
    /// @brief Get file path to the log/settings file.
    ///
    /// Find the 'Temp' folder on the local OS and create a subfolder (on linux, create .RMV folder).
    ///
    /// @return The location of the settings and log file.
    QString GetFileLocation();

    /// @brief Return whether a trace may be loaded.
    ///
    /// @param [in] trace_path The path to the trace.
    ///
    /// @return true if we may attempt an actual trace load, false otherwise.
    bool TraceValidToLoad(const QString& trace_path);

    /// @brief Get the text color that works best displayed on top of a given background color.
    ///
    /// Make the light color off-white so it can be seen against the white background.
    ///
    /// @param [in] background_color     The color that the text is to be displayed on top of.
    /// @param [in] has_white_background Is the text drawn to a mainly white background?
    ///  This will be the case for small objects that need coloring where the text
    ///  extends onto the background. If this is the case and the text color needs to
    ///  be light to contrast against a darker color, use a light gray rather than white.
    ///
    /// @return The text color, either black or white/light gray.
    QColor GetTextColorForBackground(const QColor& background_color, bool has_white_background = false);

    /// @brief Get the color needed for a snapshot state.
    ///
    /// @param [in] state The snapshot state.
    ///
    /// @return a QColor assigned to the state.
    QColor GetSnapshotStateColor(SnapshotState state);

    /// @brief Get the color needed for a delta change.
    ///
    /// @param [in] delta The type of delta.
    ///
    /// @return a QColor assigned to the delta type.
    QColor GetDeltaChangeColor(DeltaChange delta);

    /// @brief Calculate the logarithmically scaled value given a step value.
    ///
    /// @param [in]  step_value                     An unscaled whole number used to calculate a scaled threshold value.
    /// @param [in]  max_steps                      The maximum number of steps in the range.
    ///
    /// @return The calculated value.
    uint64_t CalculateSizeThresholdFromStepValue(const uint32_t step_value, const uint32_t max_steps);

    /// @brief Retrieves the name of a virtual allocation or a string containing the base address in hexadecimal form.
    ///
    /// @param [in] virtual_allocation              A pointer to the virtual allocation object.
    ///
    /// @return The virtual allocation name string.
    QString GetVirtualAllocationName(const RmtVirtualAllocation* virtual_allocation);

};  // namespace rmv_util

#endif  // RMV_UTIL_RMV_UTIL_H_
