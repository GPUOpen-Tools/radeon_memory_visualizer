//=============================================================================
// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT
/// @author AMD Developer Tools Team
/// @file
/// @brief  Main entry point.
//=============================================================================

#include <stdarg.h>
#ifdef _WIN32
// Suppress windows.h's min/max macros so std::numeric_limits<>::max() in
// binary_security_check.h parses correctly.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "binary_security_check.h"
#endif

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QStyleFactory>

#include "qt_common/custom_widgets/driver_overrides_model.h"
#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "rmt_print.h"

#include "managers/trace_manager.h"
#include "util/rmv_util.h"
#include "views/main_window.h"

/// @brief Handle printing from the backend.
///
/// @param [in] message Incoming message.
void PrintCallback(LogLevel log_level, const char* message)
{
    DebugWindow::DbgMsg(log_level, "%s", message);
}

/// @brief Detect RMV trace if any was specified as command line param.
///
/// @return Empty string if no trace, and full path if valid RMV file.
static QString GetTracePath()
{
    QString out = "";

    if (QCoreApplication::arguments().count() > 1)
    {
        const QString potential_trace_path = QDir::toNativeSeparators(QCoreApplication::arguments().at(1));
        if (rmv_util::TraceValidToLoad(potential_trace_path) == true)
        {
            out = potential_trace_path;
        }
    }

    return out;
}

#if 0
#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"
static RmtDataSet command_line_data_set;
static RmtDataSnapshot command_line_snapshot;
#endif

/// @brief Main entry point.
///
/// @param [in] argc The number of arguments.
/// @param [in] argv An array containing arguments.
int main(int argc, char* argv[])
{
#ifdef _WIN32
    // Restrict DLL search order to the application directory and System32, removing the current
    // working directory and PATH to prevent DLL planting attacks. This must be called before any
    // Qt initialization to ensure Qt plugin dependencies in the application directory still resolve.
    if (!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32))
    {
        // Failed to update DLL search order; abort to avoid running with unsafe search paths.
        MessageBox(NULL,
                   _T("Failed to configure a secure DLL search path.\nThe application will now exit."),
                   _T("Security Warning"),
                   MB_OK | MB_ICONERROR | MB_TOPMOST);
        return -1;
    }

    if (!BinarySecurity::IsInstallationValid())
    {
        return -1;
    }
#endif
    RmtSetPrintingCallback(PrintCallback, true);
#if 0
    // Test feature to dump RMV file to JSON. Need a bunch of error handling.
    if (argc == 4)
    {
        RmtErrorCode error_code = RmtDataSetInitialize(argv[1], &command_line_data_set);
        if (error_code != kRmtOk)
        {
            return 1;
        }

        const uint64_t timestamp = strtoull(argv[2], NULL, 0);
        RMT_UNUSED(timestamp);
        error_code = RmtDataSetGenerateSnapshot(&command_line_data_set, command_line_data_set.maximumTimestamp, "Snapshot 0", &command_line_snapshot);
        if (error_code != kRmtOk)
        {
            return 2;
        }

        error_code = RmtJsonDumpToFile(&command_line_snapshot, argv[3]);
        if (error_code != kRmtOk)
        {
            return 3;
        }

        return 0;
    }
#endif

    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("fusion"));

    MainWindow* window = new (std::nothrow) MainWindow();
    int         result = -1;
    if (window != nullptr)
    {
        window->show();

        // Initialize scaling manager and call ScaleFactorChanged at least once, so that
        // any existing Scaled classes run their initialization as well.
        ScalingManager::Get().Initialize(window);

        rmv::TraceManager::Get().Initialize(window);

        if (!GetTracePath().isEmpty())
        {
            rmv::TraceManager::Get().LoadTrace(GetTracePath());
        }

        result = a.exec();

        driver_overrides::DriverOverridesModel::DestroyInstance();
        delete window;
    }

    return result;
}
