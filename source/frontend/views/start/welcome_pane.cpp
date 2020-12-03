//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of RMV welcome pane.
//=============================================================================

#include "views/start/welcome_pane.h"

#include <QPalette>
#include <QUrl>
#include <QDesktopServices>
#include <QFileInfo>

#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "models/message_manager.h"
#include "settings/rmv_settings.h"
#include "util/version.h"
#include "util/widget_util.h"
#include "views/main_window.h"

static const int kMaxRecentFilesToShow = 8;

WelcomePane::WelcomePane(MainWindow* parent)
    : BasePane(parent)
    , ui_(new Ui::WelcomePane)
    , main_window_(parent)
{
    ui_->setupUi(this);

    constexpr int id = qRegisterMetaType<RmtErrorCode>();
    Q_UNUSED(id);

    // Set white background for this pane
    rmv::widget_util::SetWidgetBackgroundColor(this, Qt::white);

    SetupFileList();

    // Set up the buttons
    InitButton(ui_->open_rmv_trace_button_);
    InitButton(ui_->see_more_recent_files_button_);
    InitButton(ui_->open_getting_started_button_);
    InitButton(ui_->open_rmv_help_button_);

    ui_->quick_link_gpu_open_->SetTitle("GPUOpen website");
    ui_->quick_link_gpu_open_->SetDescLineOne("Check out the latest development blogs, performance tips & tricks ");
    ui_->quick_link_gpu_open_->SetDescLineTwo("and open source releases.");

    ui_->quick_link_profiler_->SetTitle("Explore Radeon GPU Profiler");
    ui_->quick_link_profiler_->SetDescLineOne("Find performance bottlenecks and fine tune your application");
    ui_->quick_link_profiler_->SetDescLineTwo("using Radeon GPU Profiler. Available right now at GPUOpen.");

    ui_->quick_link_analyzer_->SetTitle("Explore Radeon GPU Analyzer");
    ui_->quick_link_analyzer_->SetDescLineOne("Dig into the disassembly, resource utilization and register liveness of");
    ui_->quick_link_analyzer_->SetDescLineTwo("your shaders using RGA. Available right now at GPUOpen.");

    ui_->quick_link_sample_trace_->SetTitle("Sample trace");
    ui_->quick_link_sample_trace_->SetDescLineOne("Still got your training wheels on? Check out a sample trace to see");
    ui_->quick_link_sample_trace_->SetDescLineTwo("what we can do!");

    // Connect buttons to slots
    connect(ui_->open_rmv_trace_button_, &QPushButton::clicked, main_window_, &MainWindow::OpenTrace);
    connect(ui_->see_more_recent_files_button_, &QPushButton::clicked, main_window_, &MainWindow::OpenRecentTracesPane);
    connect(ui_->open_getting_started_button_, &QPushButton::clicked, this, &WelcomePane::OpenTraceHelp);
    connect(ui_->open_rmv_help_button_, &QPushButton::clicked, this, &WelcomePane::OpenRmvHelp);
    connect(ui_->quick_link_gpu_open_, &QPushButton::clicked, this, &WelcomePane::OpenGPUOpenURL);
    connect(ui_->quick_link_profiler_, &QPushButton::clicked, this, &WelcomePane::OpenRGPURL);
    connect(ui_->quick_link_analyzer_, &QPushButton::clicked, this, &WelcomePane::OpenRGAURL);
    connect(ui_->quick_link_sample_trace_, &QPushButton::clicked, this, &WelcomePane::OpenSampleTrace);
    connect(this, &WelcomePane::FileListChanged, this, &WelcomePane::SetupFileList, Qt::QueuedConnection);

    // Notifications are always hidden by default, and will be displayed if new notifications become available.
    ui_->notifications_label_->setVisible(false);
    ui_->notify_update_available_button_->setVisible(false);

    if (RMVSettings::Get().GetCheckForUpdatesOnStartup())
    {
        UpdateCheck::ThreadController* background_thread =
            new UpdateCheck::ThreadController(this, RMV_MAJOR_VERSION, RMV_MINOR_VERSION, RMV_BUGFIX_NUMBER, RMV_BUILD_NUMBER);

        // Get notified when the check for updates has completed.
        // There is not a way in the UI to cancel this thread, so no reason to connect to its CheckForUpdatesCancelled callback.
        connect(background_thread, &UpdateCheck::ThreadController::CheckForUpdatesComplete, this, &WelcomePane::NotifyOfNewVersion);

        background_thread->StartCheckForUpdates(rmv::kRmvUpdateCheckUrl, rmv::kRmvUpdateCheckAssetName);
    }
}

WelcomePane::~WelcomePane()
{
}

void WelcomePane::SetupFileList()
{
    const QVector<RecentFileData>& files = RMVSettings::Get().RecentFiles();

    // Clear any previous recent trace widgets
    for (RecentTraceMiniWidget* widget : trace_widgets_)
    {
        delete widget;
    }

    trace_widgets_.clear();

    // Create a widget for each recent file
    int max_files_to_show                                    = 0;
    files.size() > kMaxRecentFilesToShow ? max_files_to_show = kMaxRecentFilesToShow : max_files_to_show = files.size();
    for (int i = 0; i < max_files_to_show; i++)
    {
        RecentTraceMiniWidget* trace_widget = new RecentTraceMiniWidget(ui_->recent_traces_wrapper_);
        trace_widgets_.push_back(trace_widget);

        // Set up the widget
        trace_widget->SetFile(files[i]);
        trace_widget->show();

        // Trigger a trace open when the trace widget is clicked
        connect(trace_widget, &RecentTraceMiniWidget::clicked, &MessageManager::Get(), &MessageManager::OpenTrace);

        ui_->recent_traces_wrapper_->layout()->addWidget(trace_widget);
    }

    if (files.size() > kMaxRecentFilesToShow)
    {
        ui_->see_more_recent_files_button_->show();
    }
    else
    {
        ui_->see_more_recent_files_button_->hide();
    }
}

void WelcomePane::InitButton(ScaledPushButton*& button)
{
    // Init the button
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(kLinkButtonStylesheet);
}

void WelcomePane::OpenHtmlFile(const QString& html_file)
{
    // Get the file info
    QFileInfo file_info(QCoreApplication::applicationDirPath() + html_file);

    // Check to see if the file is not a directory and that it exists
    if (file_info.isFile() && file_info.exists())
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + html_file));
    }
    else
    {
        // The selected html file is missing on the disk so display a message box stating so
        const QString text = rmv::text::kMissingRmvHelpFile + html_file;
        QtCommon::QtUtils::ShowMessageBox(this, QMessageBox::Ok, QMessageBox::Critical, rmv::text::kMissingRmvHelpFile, text);
    }
}

void WelcomePane::OpenRmvHelp()
{
    OpenHtmlFile(rmv::text::kRmvHelpFile);
}

void WelcomePane::OpenTraceHelp()
{
    OpenHtmlFile(rmv::text::kTraceHelpFile);
}

void WelcomePane::OpenGPUOpenURL()
{
    QDesktopServices::openUrl(QUrl("http://gpuopen.com"));
}

void WelcomePane::OpenRGPURL()
{
    QDesktopServices::openUrl(QUrl("https://gpuopen.com/gaming-product/radeon-gpu-profiler-rgp/"));
}

void WelcomePane::OpenRGAURL()
{
    QDesktopServices::openUrl(QUrl("http://gpuopen.com/gaming-product/radeon-gpu-analyzer-rga/"));
}

void WelcomePane::OpenSampleTrace()
{
    MessageManager::Get().OpenTrace(QCoreApplication::applicationDirPath() + rmv::text::kSampleTraceLocation);
}

void WelcomePane::NotifyOfNewVersion(UpdateCheck::ThreadController* thread, const UpdateCheck::Results& update_check_results)
{
    if (update_check_results.was_check_successful && update_check_results.update_info.is_update_available && !update_check_results.update_info.releases.empty())
    {
        ui_->notifications_label_->setVisible(true);
        ui_->notify_update_available_button_->setVisible(true);
        ui_->notify_update_available_button_->SetTitle("New Version Available!");
        ui_->notify_update_available_button_->SetDescLineOne(update_check_results.update_info.releases[0].title.c_str());
        ui_->notify_update_available_button_->SetDescLineTwo("Click here for more information.");

        // This dialog will get deleted when the WelcomePane is deleted.
        UpdateCheckResultsDialog* results_dialog = new UpdateCheckResultsDialog(this);
        results_dialog->setWindowFlags((results_dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint) | Qt::MSWindowsFixedSizeDialogHint);
        results_dialog->setFixedSize(ScalingManager::Get().Scaled(rmv::kUpdatesResultsDialogWidth),
                                     ScalingManager::Get().Scaled(rmv::kUpdatesResultsDialogHeight));
        results_dialog->SetShowTags(false);
        results_dialog->SetResults(update_check_results);

        // Connect the button so that the when it is clicked, the dialog is shown.
        // This is why the dialog should not be deleted earlier - it could get opened any time.
        connect(ui_->notify_update_available_button_, &QPushButton::clicked, results_dialog, &QDialog::show);
    }

    // Delete the thread so that it no longer exists in the background.
    if (thread != nullptr)
    {
        delete thread;
    }
}
