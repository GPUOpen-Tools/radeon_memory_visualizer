//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of RMV system information about pane.
//=============================================================================

#include "views/start/about_pane.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QUrl>

#include "qt_common/utils/qt_util.h"

#include "util/constants.h"
#include "util/version.h"
#include "util/widget_util.h"

AboutPane::AboutPane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::AboutPane)
    , check_for_updates_pending_dialog_(nullptr)
    , check_for_updates_dialog_label_(nullptr)
    , check_for_updates_thread_(nullptr)
{
    ui_->setupUi(this);

    // Set label text for version information.
    ui_->label_version_data_->setText(RMV_VERSION_STRING);
    ui_->label_build_data_->setText(QString::number(RMV_BUILD_NUMBER));
    ui_->label_build_date_data_->setText(RMV_BUILD_DATE_STRING);

    QString copyright(RMV_COPYRIGHT_STRING);
    copyright.replace("(C)", u8"\u00A9");
    ui_->label_copyright_->setText(copyright);

    InitButton(ui_->open_getting_started_button_);
    InitButton(ui_->open_rmv_help_button_);
    InitButton(ui_->read_license_button_);
    InitButton(ui_->check_for_updates_button_);

    // Hookup buttons.
    connect(ui_->open_getting_started_button_, &QPushButton::clicked, this, &AboutPane::OpenTraceHelp);
    connect(ui_->open_rmv_help_button_, &QPushButton::clicked, this, &AboutPane::OpenRmvHelp);
    connect(ui_->read_license_button_, &QPushButton::clicked, this, &AboutPane::OpenRmvLicense);
    connect(ui_->check_for_updates_button_, &QPushButton::clicked, this, &AboutPane::CheckForUpdates);
}

AboutPane::~AboutPane()
{
}

void AboutPane::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);

    // Get the file info.
    QFileInfo file_info(QCoreApplication::applicationDirPath() + rmv::text::kTraceHelpFile);

    // Check to see if the file is not a directory and that it exists.
    if (file_info.isFile() && file_info.exists())
    {
        ui_->open_getting_started_button_->show();
    }
    else
    {
        ui_->open_getting_started_button_->hide();
    }
}

void AboutPane::InitButton(ScaledPushButton* button)
{
    button->setCursor(Qt::PointingHandCursor);
    button->SetLinkStyleSheet();
}

void AboutPane::OpenHtmlFile(const QString& html_file)
{
    // Get the file info.
    QFileInfo file_info(QCoreApplication::applicationDirPath() + html_file);

    // Check to see if the file is not a directory and that it exists.
    if (file_info.isFile() && file_info.exists())
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + html_file));
    }
    else
    {
        // The selected html file is missing on the disk so display a message box stating so.
        const QString text = rmv::text::kMissingRmvHelpFile + html_file;
        QtCommon::QtUtils::ShowMessageBox(this, QMessageBox::Ok, QMessageBox::Critical, rmv::text::kMissingRmvHelpFile, text);
    }
}

void AboutPane::OpenRmvLicense()
{
    OpenHtmlFile(rmv::text::kRmvLicenseFile);
}

void AboutPane::OpenRmvHelp()
{
    OpenHtmlFile(rmv::text::kRmvHelpFile);
}

void AboutPane::OpenTraceHelp()
{
    OpenHtmlFile(rmv::text::kTraceHelpFile);
}

void AboutPane::CheckForUpdates()
{
    // Don't allow checking for updates if there is already one in progress.
    if (check_for_updates_thread_ == nullptr)
    {
        check_for_updates_thread_ = new UpdateCheck::ThreadController(this, RMV_MAJOR_VERSION, RMV_MINOR_VERSION, RMV_BUILD_NUMBER, RMV_BUGFIX_NUMBER);

        // Build dialog to display and allow user to cancel the check if desired.
        if (check_for_updates_pending_dialog_ == nullptr)
        {
            check_for_updates_pending_dialog_ = new QDialog(this);
            check_for_updates_pending_dialog_->setWindowTitle(RMV_APP_NAME RMV_BUILD_SUFFIX);
            check_for_updates_pending_dialog_->setWindowFlags((check_for_updates_pending_dialog_->windowFlags() & ~Qt::WindowContextHelpButtonHint) |
                                                              Qt::MSWindowsFixedSizeDialogHint);
            check_for_updates_pending_dialog_->setFixedWidth(rmv::kUpdatesPendingDialogWidth);
            check_for_updates_pending_dialog_->setFixedHeight(rmv::kUpdatesPendingDialogHeight);

            QVBoxLayout* pLayout = new QVBoxLayout();
            check_for_updates_pending_dialog_->setLayout(pLayout);
            check_for_updates_dialog_label_ = new QLabel(rmv::kRmvUpdateCheckCheckingForUpdates);
            check_for_updates_pending_dialog_->layout()->addWidget(check_for_updates_dialog_label_);
            check_for_updates_pending_dialog_->layout()->addItem(new QSpacerItem(5, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));

            // Add Cancel button to cancel the check for updates.
            QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Cancel, check_for_updates_pending_dialog_);
            button_box->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
            check_for_updates_pending_dialog_->layout()->addWidget(button_box);

            // If the cancel button is pressed, signal the dialog to reject, which is similar to closing it.
            connect(button_box, &QDialogButtonBox::rejected, check_for_updates_pending_dialog_, &QDialog::reject);
        }

        // Cancel the check for updates if the dialog is closed.
        connect(check_for_updates_pending_dialog_, &QDialog::rejected, check_for_updates_thread_, &UpdateCheck::ThreadController::CancelCheckForUpdates);

        // Get notified when the check for updates has completed or was cancelled.
        connect(check_for_updates_thread_, &UpdateCheck::ThreadController::CheckForUpdatesComplete, this, &AboutPane::CheckForUpdatesCompleted);
        connect(check_for_updates_thread_, &UpdateCheck::ThreadController::CheckForUpdatesCancelled, this, &AboutPane::CheckForUpdatesCancelled);

        // Signal the check for updates to start.
        check_for_updates_thread_->StartCheckForUpdates(rmv::kRmvUpdateCheckUrl, rmv::kRmvUpdateCheckAssetName);

        // Show the WaitCursor on the check for updates button to suggest to users that it is in-progress.
        ui_->check_for_updates_button_->setCursor(Qt::WaitCursor);

        // Display the dialog.
        check_for_updates_pending_dialog_->show();
    }
}

void AboutPane::CheckForUpdatesCompleted(UpdateCheck::ThreadController* thread, const UpdateCheck::Results& update_check_results)
{
    if (update_check_results.was_check_successful && !update_check_results.update_info.is_update_available)
    {
        // Update the existing dialog to report that there are no updates available.
        check_for_updates_dialog_label_->setText(rmv::kRmvUpdateCheckNoUpdatesAvailable);
        check_for_updates_pending_dialog_->update();
    }
    else
    {
        check_for_updates_pending_dialog_->close();

        UpdateCheckResultsDialog* results_dialog = new UpdateCheckResultsDialog(this);
        if (results_dialog != nullptr)
        {
            results_dialog->setAttribute(Qt::WA_DeleteOnClose);
            results_dialog->setWindowFlags(results_dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);

            results_dialog->setModal(true);
            results_dialog->setFixedWidth(rmv::kUpdatesResultsDialogWidth);
            results_dialog->setFixedHeight(rmv::kUpdatesResultsDialogHeight);
            results_dialog->SetShowTags(false);

            QDialogButtonBox* button_box = results_dialog->findChild<QDialogButtonBox*>("button_box_");
            if (button_box != nullptr)
            {
                QPushButton* close_button = button_box->button(QDialogButtonBox::Close);
                if (close_button != nullptr)
                {
                    close_button->setCursor(Qt::PointingHandCursor);
                }
            }

            results_dialog->SetResults(update_check_results);
            results_dialog->show();
        }
    }

    // Restore pointing hand cursor.
    ui_->check_for_updates_button_->setCursor(Qt::PointingHandCursor);

    // Delete the previous thread since it is no longer useful.
    if (check_for_updates_thread_ == thread)
    {
        if (check_for_updates_thread_ != nullptr)
        {
            delete check_for_updates_thread_;
            check_for_updates_thread_ = nullptr;
        }
    }
}

void AboutPane::CheckForUpdatesCancelled(UpdateCheck::ThreadController* thread)
{
    // Restore pointing hand cursor.
    ui_->check_for_updates_button_->setCursor(Qt::PointingHandCursor);

    // Delete the previous thread since it is no longer useful.
    if (check_for_updates_thread_ == thread)
    {
        if (check_for_updates_thread_ != nullptr)
        {
            delete check_for_updates_thread_;
            check_for_updates_thread_ = nullptr;
        }
    }
}
