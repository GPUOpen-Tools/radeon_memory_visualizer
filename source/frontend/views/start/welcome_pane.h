//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RMV welcome pane.
//=============================================================================

#ifndef RMV_VIEWS_START_WELCOME_PANE_H_
#define RMV_VIEWS_START_WELCOME_PANE_H_

#include "ui_welcome_pane.h"

#include <QVector>
#include <QScrollArea>
#include <QVBoxLayout>

#include "qt_common/custom_widgets/quick_link_button_widget.h"
#include "qt_common/custom_widgets/recent_trace_mini_widget.h"
#include "qt_common/custom_widgets/scaled_push_button.h"
#include "update_check_api/source/update_check_results_dialog.h"

#include "views/base_pane.h"

class MainWindow;

/// Support for RMV recently opened traces pane.
class WelcomePane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit WelcomePane(MainWindow* parent);

    /// Destructor.
    virtual ~WelcomePane();

signals:
    void FileListChanged();

private slots:
    /// Setup the list of recent files. Called whenever the number of recent files
    /// changes or whenever the list needs updating.
    void SetupFileList();

    /// Open RMV help file.
    /// Present the user with help regarding RMV.
    void OpenRmvHelp();

    /// Open trace help file.
    /// Present the user with help about how to capture a trace with the panel.
    void OpenTraceHelp();

    /// Open a URL to GPUOpen.
    void OpenGPUOpenURL();

    /// Open a URL to RGP.
    void OpenRGPURL();

    /// Open a URL to RGA.
    void OpenRGAURL();

    /// Open sample trace.
    void OpenSampleTrace();

    /// Notify the user that a new version is available.
    /// \param thread The background thread that checked for updates.
    /// \param update_check_results The results of the check for updates.
    void NotifyOfNewVersion(UpdateCheck::ThreadController* thread, const UpdateCheck::Results& update_check_results);

private:
    /// Open an HTML file.
    /// \\param html_file the file to open.
    void OpenHtmlFile(const QString& html_file);

    /// Initialize the button.
    /// \param button Pointer to the push button.
    void InitButton(ScaledPushButton*& button);

    Ui::WelcomePane*                ui_;             ///< Pointer to the Qt UI design.
    MainWindow*                     main_window_;    ///< Reference to the mainwindow (parent).
    QVector<RecentTraceMiniWidget*> trace_widgets_;  ///< Array of trace widgets.
};

#endif  // RMV_VIEWS_START_WELCOME_PANE_H_
