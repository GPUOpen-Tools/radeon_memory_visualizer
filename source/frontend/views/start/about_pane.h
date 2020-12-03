//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RMV system information about pane.
//=============================================================================

#ifndef RMV_VIEWS_START_ABOUT_PANE_H_
#define RMV_VIEWS_START_ABOUT_PANE_H_

#include "ui_about_pane.h"

#include <QLabel>
#include <QDialog>

#include "qt_common/custom_widgets/scaled_push_button.h"
#include "update_check_api/source/update_check_results_dialog.h"

#include "views/base_pane.h"

class MainWindow;

/// Support for RMV system information about pane.
class AboutPane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit AboutPane(MainWindow* parent);

    /// Destructor.
    virtual ~AboutPane();

private slots:
    /// Open RMV help file.
    /// Present the user with help regarding RMV.
    void OpenRmvHelp();

    /// Open trace help file.
    /// Present the user with help about how to capture a trace with the panel.
    void OpenTraceHelp();

    /// Open RMV help file.
    /// Present the user with help regarding RMV.
    void OpenRmvLicense();
    /// Perform a check for updates.
    /// Runs a background thread that goes online to look for updates.
    void CheckForUpdates();

    /// Callback after a check for updates has returned.
    /// Displays a modal dialog box with the update information or error message.
    /// \param thread The background thread that was checking for updates.
    /// \param update_check_results The results of the check for updates.
    void CheckForUpdatesCompleted(UpdateCheck::ThreadController* thread, const UpdateCheck::Results& update_check_results);

    /// Callback for when check for updates has returned due to being cancelled.
    /// Restores the UI to allow checking for updates again.
    /// \param thread The background thread that was checking for updates.
    void CheckForUpdatesCancelled(UpdateCheck::ThreadController* thread);

private:
    /// Open an HTML file.
    /// \param html_file the file to open.
    void OpenHtmlFile(const QString& html_file);

    /// Initialize the button.
    /// \param button Pointer to the push button.
    void InitButton(ScaledPushButton*& button);

    Ui::AboutPane* ui_;  ///< Pointer to the Qt UI design.

    /// A dialog that is displayed while the check for updates is in-progress.
    /// Closing this dialog will signal the check for updates to be cancelled.
    /// It will close automatically after the check for updates completes.
    QDialog* check_for_updates_pending_dialog_;

    /// The label on the check for updates pending dialog.
    QLabel* check_for_updates_dialog_label_;

    /// This class creates and interacts with the background thread that
    /// performs the check for updates. We need to store a member variable
    /// so that we can cancel the thread if needed. The thread will emit a
    /// signal when the check for updates has either been cancelled or after
    /// it has completed.
    UpdateCheck::ThreadController* check_for_updates_thread_;
};

#endif  // RMV_VIEWS_START_ABOUT_PANE_H_
