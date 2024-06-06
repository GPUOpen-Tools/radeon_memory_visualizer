//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the welcome pane.
//=============================================================================

#include "views/start/welcome_pane.h"

#include <QPalette>
#include <QUrl>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileInfo>

#include "qt_common/utils/qt_util.h"

#include "managers/message_manager.h"
#include "managers/trace_manager.h"
#include "settings/rmv_settings.h"
#include "util/constants.h"
#include "util/version.h"
#include "util/widget_util.h"

static const int kMaxRecentFilesToShow = 8;

WelcomePane::WelcomePane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::WelcomePane)
{
    ui_->setupUi(this);

    constexpr int id = qRegisterMetaType<RmtErrorCode>();
    Q_UNUSED(id);

    // Set white background for this pane.
    rmv::widget_util::SetWidgetBackgroundColor(this, Qt::white);

    SetupFileList();

    // Set up the buttons.
    InitButton(ui_->open_rmv_trace_button_);
    InitButton(ui_->see_more_recent_files_button_);
    InitButton(ui_->open_getting_started_button_);
    InitButton(ui_->open_rmv_help_button_);

    ui_->quick_link_gpu_open_->SetTitle("GPUOpen website");
    ui_->quick_link_gpu_open_->SetDescLineOne("Check out the latest development blogs, performance tips & tricks ");
    ui_->quick_link_gpu_open_->SetDescLineTwo("and open source releases.");

    ui_->quick_link_github_->SetTitle("Encounter a problem or have an idea?");
    ui_->quick_link_github_->SetDescLineOne("To provide feedback or suggestions, or to file a bug, visit our");
    ui_->quick_link_github_->SetDescLineTwo("GitHub page.");

    ui_->quick_link_rgp_->SetTitle("Explore Radeon GPU Profiler");
    ui_->quick_link_rgp_->SetDescLineOne("Find performance bottlenecks and fine tune your application");
    ui_->quick_link_rgp_->SetDescLineTwo("using Radeon GPU Profiler. Available right now at GPUOpen.");

    ui_->quick_link_rga_->SetTitle("Explore Radeon GPU Analyzer");
    ui_->quick_link_rga_->SetDescLineOne("Dig into the disassembly, resource utilization and register liveness of");
    ui_->quick_link_rga_->SetDescLineTwo("your shaders using RGA. Available right now at GPUOpen.");

    ui_->quick_link_rgd_->SetTitle("Explore Radeon GPU Detective");
    ui_->quick_link_rgd_->SetDescLineOne("Investigate GPU crashes, gather your evidence, and probe any page");
    ui_->quick_link_rgd_->SetDescLineTwo("faults! Learn more on GPUOpen.");

    ui_->quick_link_rra_->SetTitle("Explore Radeon Raytracing Analyzer");
    ui_->quick_link_rra_->SetDescLineOne("Assess your acceleration structures and discover your ray traversal");
    ui_->quick_link_rra_->SetDescLineTwo("hotspots! Available right now at GPUOpen.");

    ui_->quick_link_sample_trace_->SetTitle("Sample trace");
    ui_->quick_link_sample_trace_->SetDescLineOne("Still got your training wheels on? Check out a sample trace to see");
    ui_->quick_link_sample_trace_->SetDescLineTwo("what we can do!");

    ui_->quick_link_rdna_performance_->SetTitle("RDNA performance guide");
    ui_->quick_link_rdna_performance_->SetDescLineOne("Learn valuable optimization techniques from this in-depth performance");
    ui_->quick_link_rdna_performance_->SetDescLineTwo("guide full of tidbits, tips and tricks.");

    // Connect buttons to slots.
    connect(ui_->open_rmv_trace_button_, &QPushButton::clicked, [=]() { emit rmv::MessageManager::Get().OpenTraceFileMenuClicked(); });
    connect(ui_->see_more_recent_files_button_, &QPushButton::clicked, [=]() {
        emit rmv::MessageManager::Get().PaneSwitchRequested(rmv::kPaneIdStartRecentTraces);
    });
    connect(ui_->open_getting_started_button_, &QPushButton::clicked, this, &WelcomePane::OpenTraceHelp);
    connect(ui_->open_rmv_help_button_, &QPushButton::clicked, this, &WelcomePane::OpenRmvHelp);
    connect(ui_->quick_link_gpu_open_, &QPushButton::clicked, this, &WelcomePane::OpenGPUOpenURL);
    connect(ui_->quick_link_github_, &QPushButton::clicked, this, &WelcomePane::OpenGitHubURL);
    connect(ui_->quick_link_rgp_, &QPushButton::clicked, this, &WelcomePane::OpenRGPURL);
    connect(ui_->quick_link_rga_, &QPushButton::clicked, this, &WelcomePane::OpenRGAURL);
    connect(ui_->quick_link_rgd_, &QPushButton::clicked, this, &WelcomePane::OpenRGDURL);
    connect(ui_->quick_link_rra_, &QPushButton::clicked, this, &WelcomePane::OpenRRAURL);
    connect(ui_->quick_link_sample_trace_, &QPushButton::clicked, this, &WelcomePane::OpenSampleTrace);
    connect(ui_->quick_link_rdna_performance_, &QPushButton::clicked, this, &WelcomePane::OpenRDNAPerformanceURL);
    connect(&rmv::MessageManager::Get(), &rmv::MessageManager::RecentFileListChanged, this, &WelcomePane::SetupFileList, Qt::QueuedConnection);

    // Notifications are always hidden by default, and will be displayed if new notifications become available.
    ui_->notifications_label_->setVisible(false);
    ui_->notify_update_available_button_->setVisible(false);

    if (rmv::RMVSettings::Get().GetCheckForUpdatesOnStartup())
    {
        UpdateCheck::ThreadController* background_thread =
            new UpdateCheck::ThreadController(this, RMV_MAJOR_VERSION, RMV_MINOR_VERSION, RMV_BUILD_NUMBER, RMV_BUGFIX_NUMBER);

        // Get notified when the check for updates has completed.
        // There is not a way in the UI to cancel this thread, so no reason to connect to its CheckForUpdatesCancelled callback.
        connect(background_thread, &UpdateCheck::ThreadController::CheckForUpdatesComplete, this, &WelcomePane::NotifyOfNewVersion);

        background_thread->StartCheckForUpdates(rmv::kRmvUpdateCheckUrl, rmv::kRmvUpdateCheckAssetName);
    }
}

WelcomePane::~WelcomePane()
{
}

void WelcomePane::showEvent(QShowEvent* event)
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

void WelcomePane::SetupFileList()
{
    const QVector<RecentFileData>& files = rmv::RMVSettings::Get().RecentFiles();

    // Clear any previous recent trace widgets
    for (RecentTraceMiniWidget* widget : trace_widgets_)
    {
        delete widget;
    }

    trace_widgets_.clear();

    // Create a widget for each recent file
    int max_files_to_show = 0;
    files.size() > kMaxRecentFilesToShow ? max_files_to_show = kMaxRecentFilesToShow : max_files_to_show = files.size();
    for (int i = 0; i < max_files_to_show; i++)
    {
        RecentTraceMiniWidget* trace_widget = new RecentTraceMiniWidget(ui_->recent_traces_wrapper_);
        trace_widgets_.push_back(trace_widget);

        // Set up the widget
        trace_widget->SetFile(files[i]);
        trace_widget->show();

        // Trigger a trace open when the trace widget is clicked
        connect(trace_widget, &RecentTraceMiniWidget::clicked, &rmv::TraceManager::Get(), &rmv::TraceManager::LoadTrace);

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

void WelcomePane::InitButton(ScaledPushButton* button)
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
    QDesktopServices::openUrl(rmv::text::kGpuOpenUrl);
}

void WelcomePane::OpenGitHubURL()
{
    QDesktopServices::openUrl(rmv::text::kRmvGithubUrl);
}

void WelcomePane::OpenRGPURL()
{
    QDesktopServices::openUrl(rmv::text::kRgpGpuOpenUrl);
}

void WelcomePane::OpenRGAURL()
{
    QDesktopServices::openUrl(rmv::text::kRgaGpuOpenUrl);
}

void WelcomePane::OpenRGDURL()
{
    QDesktopServices::openUrl(rmv::text::kRgdGpuOpenUrl);
}

void WelcomePane::OpenRRAURL()
{
    QDesktopServices::openUrl(rmv::text::kRraGpuOpenUrl);
}

void WelcomePane::OpenSampleTrace()
{
    rmv::TraceManager::Get().LoadTrace(QCoreApplication::applicationDirPath() + rmv::text::kSampleTraceLocation);
}

void WelcomePane::OpenRDNAPerformanceURL()
{
    QDesktopServices::openUrl(rmv::text::kRdnaPerformanceGpuOpenUrl);
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
        results_dialog->setFixedSize(rmv::kUpdatesResultsDialogWidth,
                                     rmv::kUpdatesResultsDialogHeight);
        results_dialog->SetShowTags(false);
        results_dialog->SetResults(update_check_results);

        QDialogButtonBox* button_box = results_dialog->findChild<QDialogButtonBox*>("button_box_");
        if (button_box != nullptr)
        {
            QPushButton* close_button = button_box->button(QDialogButtonBox::Close);
            if (close_button != nullptr)
            {
                close_button->setCursor(Qt::PointingHandCursor);
            }
        }

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
