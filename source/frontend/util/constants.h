//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Intended to hold globally-known definitions.
//=============================================================================

#ifndef RMV_UTIL_CONSTANTS_H_
#define RMV_UTIL_CONSTANTS_H_

#include "rmt_constants.h"

#include <QColor>
#include <QUrl>

namespace rmv
{
    static const QString kRmvExecutableBaseFilename = "RadeonMemoryVisualizer";

#ifdef _DEBUG
    static const QString kRmvExecutableDebugIdentifier = "-d";
#endif

    // Checking for updates.
    static const QString kRmvUpdateCheckAssetName          = "RMV-Updates.json";
    static const QString kRmvUpdateCheckCheckingForUpdates = "Checking for updates...";
    static const QString kRmvUpdateCheckNoUpdatesAvailable = "Checking for updates... done.\n\nNo updates available.";

#ifdef _DEBUG
    static const QString kRmvUpdateCheckUrl = ".";
#else
    static const QString kRmvUpdateCheckUrl = "https://api.github.com/repos/GPUOpen-Tools/radeon_memory_visualizer/releases/latest";
#endif

    static const int kUpdatesPendingDialogWidth  = 400;
    static const int kUpdatesPendingDialogHeight = 150;
    static const int kUpdatesResultsDialogWidth  = 400;
    static const int kUpdatesResultsDialogHeight = 300;

    // Rendering things.
    static const int kHoverDarkenColor = 125;

    // Memory subscription colors.
    static const QColor kUnderSubscribedColor   = QColor(0, 175, 80);
    static const QColor kOverSubscribedColor    = QColor(255, 0, 0);
    static const QColor kCloseToSubscribedColor = QColor(226, 124, 48);

    // Widget lengths and heights.
    static const int kSearchBoxWidth        = 200;
    static const int kDoubleSliderWidth     = 200;
    static const int kDoubleSliderHeight    = 20;
    static const int kColoredLegendsHeight  = 20;
    static const int kAllocationHeight      = 100;
    static const int kHeapComboBoxWidth     = 140;
    static const int kResourceComboBoxWidth = 140;
    static const int kSizeSliderRange       = 25;

    // Control window sizes.
    static const int   kDesktopMargin                      = 25;
    static const float kDesktopAvailableWidthPercentage    = 99.0F;
    static const float kDesktopAvailableHeightPercentage   = 95.0F;
    static const float kDebugWindowDesktopWidthPercentage  = 66.0F;
    static const float kDebugWindowDesktopHeightPercentage = 25.0F;

    // App colors.
    static const QColor kCheckboxEnableColor = QColor(0, 122, 217);

    // Indices for the SNAPSHOT and COMPARE stacked widget. Index 1 is displayed if
    // a snapshot loaded, otherwise an empty pane (0) is shown.
    static const int kSnapshotIndexEmptyPane     = 0;
    static const int kSnapshotIndexPopulatedPane = 1;

    namespace text
    {
        // Generic error message.
        static const QString kOpenTraceErrorTitle = "Error";
        static const QString kOpenTraceErrorText  = "The trace file, %1, failed to load.";

        // Delete recent file pop up dialog.
        static const QString kDeleteRecentTraceTitle        = "Error";
        static const QString kDeleteRecentTraceText         = "\n\nDo you want to remove it from the recent files list?";

        static const QString kRecentTraceAlreadyOpenedTitle = "Warning";
        static const QString kRecentTraceAlreadyOpenedText =
            "Opening the trace file as read-only (snapshot edits will not be saved). The RMV file is either read only or has been opened in another instance "
            "of RMV";

        // Specific error messages (appended to generic error message).
        static const QString kOpenTraceOutOfVirtualGPUMemory =
            "\n\nThe trace file contains allocations requesting more than 64GB of GPU memory. Please reduce the video memory consumption.";

        static const QString kOpenTraceFileNotSupported =
            "\n\nThe trace file is no longer supported. Please load it using an older version of the Radeon Memory Visualizer.";

        // Open recent trace missing pop up dialog.
        static const QString kOpenRecentTraceTitle = "Trace not opened";
        static const QString kOpenRecentTraceStart = "Trace \"";
        static const QString kOpenRecentTraceEnd   = "\"  does not exist!";

        // Allocation overview pane sort modes.
        static const QString kSortByAllocationId       = "Sort by allocation id";
        static const QString kSortByAllocationSize     = "Sort by allocation size";
        static const QString kSortByAllocationAge      = "Sort by allocation age";
        static const QString kSortByResourceCount      = "Sort by resource count";
        static const QString kSortByFragmentationScore = "Sort by fragmentation score";

        // Sort directions.
        static const QString kSortAscending  = "Ascending";
        static const QString kSortDescending = "Descending";

        // Other text strings.
        static const QString kPreferredHeap = "Preferred heap";
        static const QString kActualHeap    = "Actual heap";
        static const QString kResourceUsage = "Resource usage";

        // Time units.
        static const QString kSettingsUnitsClocks       = "Clocks";
        static const QString kSettingsUnitsMilliseconds = "Milliseconds";
        static const QString kSettingsUnitsSeconds      = "Seconds";
        static const QString kSettingsUnitsMinutes      = "Minutes";
        static const QString kSettingsUnitsHours        = "Hours";

        // Help file locations for trace and RMV.
        static const QString kTraceHelpFile       = "/help/rdp/index.html";
        static const QString kRmvHelpFile         = "/help/rmv/index.html";
        static const QString kRmvLicenseFile      = "/LICENSE.txt";
        static const QString kSampleTraceLocation = "/samples/sample_trace" + QString(kRMVTraceFileExtension);

        static const QString kFileOpenFileTypes = "All supported files (*" + QString(kRMVTraceFileExtension) + " *" + QString(kRGDTraceFileExtension) +
                                                  ") ;; RMV trace files (*" + QString(kRMVTraceFileExtension) + ") ;; RGD crash dump files (*" +
                                                  kRGDTraceFileExtension + ")";

        static const QString kMissingRmvTrace    = "Missing RMV sample trace: ";
        static const QString kMissingRmvHelpFile = "Missing RMV help file: ";

        // External links.
        static const QUrl kGpuOpenUrl                = QUrl("https://gpuopen.com");
        static const QUrl kRmvGithubUrl              = QUrl("https://github.com/GPUOpen-Tools/radeon_memory_visualizer");
        static const QUrl kRgpGpuOpenUrl             = QUrl("https://gpuopen.com/rgp/");
        static const QUrl kRgaGpuOpenUrl             = QUrl("https://gpuopen.com/rga/");
        static const QUrl kRgdGpuOpenUrl             = QUrl("https://gpuopen.com/radeon-gpu-detective/");
        static const QUrl kRraGpuOpenUrl             = QUrl("https://gpuopen.com/rra/");
        static const QUrl kRdnaPerformanceGpuOpenUrl = QUrl("https://gpuopen.com/performance/");
    }  // namespace text

    namespace resource
    {
        // Stylesheet resource.
        static const QString kStylesheet = ":/Resources/stylesheet.qss";

        // Zoom in/out svg resources.
        static const QString kZoomInEnabled   = ":/Resources/assets/zoom_in.svg";
        static const QString kZoomOutEnabled  = ":/Resources/assets/zoom_out.svg";
        static const QString kZoomInDisabled  = ":/Resources/assets/zoom_in_disabled.svg";
        static const QString kZoomOutDisabled = ":/Resources/assets/zoom_out_disabled.svg";

        static const QString kZoomToSelectionEnabled  = ":/Resources/assets/zoom_to_selection.svg";
        static const QString kZoomResetEnabled        = ":/Resources/assets/zoom_reset.svg";
        static const QString kZoomToSelectionDisabled = ":/Resources/assets/zoom_to_selection_disabled.svg";
        static const QString kZoomResetDisabled       = ":/Resources/assets/zoom_reset_disabled.svg";

    }  // namespace resource
}  // namespace rmv

#endif  // RMV_UTIL_CONSTANTS_H_
