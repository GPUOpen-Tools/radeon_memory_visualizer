//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Main entry point.
//=============================================================================

#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QStyleFactory>
#include <stdarg.h>

#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "rmt_print.h"

#include "managers/trace_manager.h"
#include "views/main_window.h"
#include "util/rmv_util.h"

/// @brief Handle printing from the backend.
///
/// @param [in] message Incoming message.
void PrintCallback(const char* message)
{
    DebugWindow::DbgMsg(message);
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
#ifdef _DEBUG
    RmtSetPrintingCallback(PrintCallback, true);
#endif
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

    // Load application stylesheet.
    QFile style_sheet(rmv::resource::kStylesheet);
    if (style_sheet.open(QFile::ReadOnly))
    {
        a.setStyleSheet(style_sheet.readAll());
    }

    // Force a light theme for now. When we remove all the places that manually set the
    // background color to white and the text to black, and create a custom dark stylesheet
    // for widgets that have been customized in the stylesheet for light theme, we can set
    // the palette to the OS theme.
    a.setPalette(QtCommon::QtUtils::ColorTheme::Get().GetCurrentPalette());

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
        delete window;
    }

    return result;
}
