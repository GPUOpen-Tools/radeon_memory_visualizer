//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the main window.
//=============================================================================

#include "views/main_window.h"

#include <QDesktopWidget>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMimeData>
#include <QScreen>

#include "qt_common/utils/common_definitions.h"
#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "managers/load_animation_manager.h"
#include "managers/message_manager.h"
#include "managers/navigation_manager.h"
#include "managers/pane_manager.h"
#include "managers/snapshot_manager.h"
#include "managers/trace_manager.h"
#include "views/compare/snapshot_delta_pane.h"
#include "views/compare/memory_leak_finder_pane.h"
#include "views/compare/compare_start_pane.h"
#include "views/settings/settings_pane.h"
#include "views/settings/themes_and_colors_pane.h"
#include "views/settings/keyboard_shortcuts_pane.h"
#include "views/snapshot/allocation_explorer_pane.h"
#include "views/snapshot/resource_list_pane.h"
#include "views/snapshot/resource_details_pane.h"
#include "views/snapshot/resource_overview_pane.h"
#include "views/snapshot/allocation_overview_pane.h"
#include "views/snapshot/heap_overview_pane.h"
#include "views/snapshot/snapshot_start_pane.h"
#include "views/start/about_pane.h"
#include "views/start/recent_traces_pane.h"
#include "views/start/welcome_pane.h"
#include "views/timeline/timeline_pane.h"
#include "views/timeline/device_configuration_pane.h"
#include "settings/rmv_settings.h"
#include "settings/rmv_geometry_settings.h"
#include "util/time_util.h"
#include "util/version.h"
#include "util/widget_util.h"

// Indices for the COMPARE stacked widget. These need to match the widget order
// in the .ui file.
enum SnapshotStackIndex
{
    kCompareNotLoaded = 0,  // Snapshots not loaded.
    kCompareEmpty     = 1,  // Snapshots contain no allocations or resources.
    kCompareOk        = 2,  // Snapshots loaded and contain useful data.
};

// The maximum number of snapshots to list in the recent traces list.
static const int kMaxSubmenuSnapshots = 10;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
    , file_menu_(nullptr)
    , open_trace_action_(nullptr)
    , close_trace_action_(nullptr)
    , exit_action_(nullptr)
    , help_action_(nullptr)
    , about_action_(nullptr)
    , help_menu_(nullptr)
    , recent_traces_menu_(nullptr)
    , navigation_bar_(this)
{
    const bool loaded_settings = rmv::RMVSettings::Get().LoadSettings();

    ui_->setupUi(this);

    setWindowTitle(GetTitleBarString());
    setWindowIcon(QIcon(":/Resources/assets/rmv_icon_32x32.png"));
    setAcceptDrops(true);

    // Set white background for this pane.
    rmv::widget_util::SetWidgetBackgroundColor(this, Qt::white);

    // Setup window sizes and settings.
    SetupWindowRects(loaded_settings);

    // NOTE: Widgets need creating in the order they are to appear in the UI.
    CreatePane<WelcomePane>(ui_->start_stack_);
    RecentTracesPane* recent_traces_pane = CreatePane<RecentTracesPane>(ui_->start_stack_);
    CreatePane<AboutPane>(ui_->start_stack_);
    timeline_pane_ = CreatePane<TimelinePane>(ui_->timeline_stack_);
    CreatePane<DeviceConfigurationPane>(ui_->timeline_stack_);
    CreatePane<HeapOverviewPane>(ui_->snapshot_stack_);
    CreatePane<ResourceOverviewPane>(ui_->snapshot_stack_);
    CreatePane<AllocationOverviewPane>(ui_->snapshot_stack_);
    CreatePane<ResourceListPane>(ui_->snapshot_stack_);
    CreatePane<AllocationExplorerPane>(ui_->snapshot_stack_);
    resource_details_pane_ = CreatePane<ResourceDetailsPane>(ui_->snapshot_stack_);
    CreateComparePane<SnapshotDeltaPane>(ui_->compare_stack_);
    CreateComparePane<MemoryLeakFinderPane>(ui_->compare_stack_);
    CreatePane<SettingsPane>(ui_->settings_stack_);
    ThemesAndColorsPane* themes_and_colors_pane = CreatePane<ThemesAndColorsPane>(ui_->settings_stack_);
    CreatePane<KeyboardShortcutsPane>(ui_->settings_stack_);

    ui_->compare_snapshots_empty_->SetEmptyTitleText();

    ui_->main_tab_widget_->setTabEnabled(rmv::kMainPaneTimeline, false);
    ui_->main_tab_widget_->setTabEnabled(rmv::kMainPaneSnapshot, false);
    ui_->main_tab_widget_->setTabEnabled(rmv::kMainPaneCompare, false);

    SetupTabBar();
    CreateActions();
    CreateMenus();
    rmv::LoadAnimationManager::Get().Initialize(ui_->main_tab_widget_, file_menu_);

    ResetUI();

    rmv::widget_util::InitSingleSelectComboBox(this, ui_->snapshot_combo_box_, "Snapshot", false);
    ui_->snapshot_combo_box_->SetListAboveButton(true);
    connect(ui_->snapshot_combo_box_, &ArrowIconComboBox::SelectionChanged, this, &MainWindow::OpenSelectedSnapshot);

    ViewPane(rmv::kPaneIdStartWelcome);

    connect(&navigation_bar_.BackButton(), &QPushButton::clicked, &rmv::NavigationManager::Get(), &rmv::NavigationManager::NavigateBack);
    connect(&navigation_bar_.ForwardButton(), &QPushButton::clicked, &rmv::NavigationManager::Get(), &rmv::NavigationManager::NavigateForward);
    connect(&rmv::NavigationManager::Get(), &rmv::NavigationManager::EnableBackNavButton, &navigation_bar_, &NavigationBar::EnableBackButton);
    connect(&rmv::NavigationManager::Get(), &rmv::NavigationManager::EnableForwardNavButton, &navigation_bar_, &NavigationBar::EnableForwardButton);

    // Update the navigation and pane switching when the UI panes are changed.
    connect(ui_->start_list_, &QListWidget::currentRowChanged, this, &MainWindow::UpdateStartListRow);
    connect(ui_->timeline_list_, &QListWidget::currentRowChanged, this, &MainWindow::UpdateTimelineListRow);
    connect(ui_->snapshot_list_, &QListWidget::currentRowChanged, this, &MainWindow::UpdateSnapshotListRow);
    connect(ui_->compare_list_, &QListWidget::currentRowChanged, this, &MainWindow::UpdateCompareListRow);
    connect(ui_->settings_list_, &QListWidget::currentRowChanged, this, &MainWindow::UpdateSettingsListRow);
    connect(ui_->main_tab_widget_, &QTabWidget::currentChanged, this, &MainWindow::UpdateMainTabIndex);

    // Set up the stack widget signals so they know which pane in their stack to switch to.
    connect(ui_->start_list_, &QListWidget::currentRowChanged, ui_->start_stack_, &QStackedWidget::setCurrentIndex);
    connect(ui_->timeline_list_, &QListWidget::currentRowChanged, ui_->timeline_stack_, &QStackedWidget::setCurrentIndex);
    connect(ui_->snapshot_list_, &QListWidget::currentRowChanged, ui_->snapshot_stack_, &QStackedWidget::setCurrentIndex);
    connect(ui_->compare_list_, &QListWidget::currentRowChanged, ui_->compare_stack_, &QStackedWidget::setCurrentIndex);
    connect(ui_->settings_list_, &QListWidget::currentRowChanged, ui_->settings_stack_, &QStackedWidget::setCurrentIndex);

    connect(&rmv::MessageManager::Get(), &rmv::MessageManager::OpenTraceFileMenuClicked, this, &MainWindow::OpenTraceFromFileMenu);
    connect(recent_traces_pane, &RecentTracesPane::RecentFileDeleted, this, &MainWindow::SetupRecentTracesMenu);

    connect(&rmv::MessageManager::Get(), &rmv::MessageManager::TitleBarChanged, this, &MainWindow::UpdateTitlebar);

    connect(&rmv::MessageManager::Get(), &rmv::MessageManager::PaneSwitchRequested, this, &MainWindow::ViewPane);
    connect(&rmv::NavigationManager::Get(), &rmv::NavigationManager::NavigateButtonClicked, this, &MainWindow::SetupNextPane);

    connect(&rmv::TraceManager::Get(), &rmv::TraceManager::TraceOpened, this, &MainWindow::OpenTrace);
    connect(&rmv::TraceManager::Get(), &rmv::TraceManager::TraceClosed, this, &MainWindow::CloseTrace);

    connect(&rmv::SnapshotManager::Get(), &rmv::SnapshotManager::SnapshotOpened, this, &MainWindow::OpenSnapshotPane);
    connect(&rmv::SnapshotManager::Get(), &rmv::SnapshotManager::CompareSnapshotsOpened, this, &MainWindow::OpenComparePane);

    connect(&rmv::SnapshotManager::Get(), &rmv::SnapshotManager::SnapshotLoaded, this, &MainWindow::ShowSnapshotPane);
    connect(&rmv::SnapshotManager::Get(), &rmv::SnapshotManager::CompareSnapshotsLoaded, this, &MainWindow::ShowComparePane);

    connect(themes_and_colors_pane, &ThemesAndColorsPane::RefreshedColors, this, &MainWindow::BroadcastChangeColoring);

    // Connect to ScalingManager for notifications.
    connect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &MainWindow::OnScaleFactorChanged);
}

MainWindow::~MainWindow()
{
    disconnect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &MainWindow::OnScaleFactorChanged);

    delete ui_;
}

void MainWindow::OnScaleFactorChanged()
{
    ResizeNavigationLists();
}

void MainWindow::ResizeNavigationLists()
{
    // Update the widths of the navigation lists so they are all the same width.
    // Since these are each independent widgets, the MainWindow needs to keep them in sync.
    // At some resolutions and DPI scales the text/combobox on the bottom of the snapshot pane
    // may define the desired widest width, so take those into consideration as well, but their
    // width doesn't need to be updated (they can be narrower than the snapshotList).
    int widest_width = std::max<int>(ui_->start_list_->sizeHint().width(), ui_->timeline_list_->sizeHint().width());
    widest_width     = std::max<int>(widest_width, ui_->snapshot_list_->sizeHint().width());
    widest_width     = std::max<int>(widest_width, ui_->snapshot_label_->sizeHint().width());
    widest_width     = std::max<int>(widest_width, ui_->snapshot_combo_box_->sizeHint().width());
    widest_width     = std::max<int>(widest_width, ui_->compare_list_->sizeHint().width());
    widest_width     = std::max<int>(widest_width, ui_->settings_list_->sizeHint().width());

    // Also use 1/12th of the MainWindow as a minimum width for the navigation list.
    int minimum_width = this->width() / 12;
    widest_width      = std::max<int>(widest_width, minimum_width);

    ui_->start_list_->setFixedWidth(widest_width);
    ui_->timeline_list_->setFixedWidth(widest_width);
    ui_->snapshot_list_->setFixedWidth(widest_width);
    ui_->compare_list_->setFixedWidth(widest_width);
    ui_->settings_list_->setFixedWidth(widest_width);
}

void MainWindow::SetupTabBar()
{
    // Set grey background for main tab bar background.
    rmv::widget_util::SetWidgetBackgroundColor(ui_->main_tab_widget_, QColor(51, 51, 51));

    // Set the mouse cursor to pointing hand cursor for all of the tabs.
    QList<QTabBar*> tab_bar = ui_->main_tab_widget_->findChildren<QTabBar*>();
    foreach (QTabBar* item, tab_bar)
    {
        if (item != nullptr)
        {
            item->setCursor(Qt::PointingHandCursor);
            item->setContentsMargins(10, 10, 10, 10);
        }
    }

    // Set the tab that divides the left and right justified tabs.
    ui_->main_tab_widget_->SetSpacerIndex(rmv::kMainPaneSpacer);

    // Adjust spacing around the navigation bar so that it appears centered on the tab bar.
    navigation_bar_.layout()->setContentsMargins(15, 3, 35, 2);

    // Setup navigation browser toolbar on the main tab bar.
    ui_->main_tab_widget_->SetTabTool(rmv::kMainPaneNavigation, &navigation_bar_);
}

void MainWindow::SetupWindowRects(bool loaded_settings)
{
    QList<QScreen*> screens = QGuiApplication::screens();
    QRect           geometry;

    Q_ASSERT(!screens.empty());

    if (!screens.empty())
    {
        geometry = screens.front()->availableGeometry();
    }

    if (loaded_settings)
    {
        rmv::RMVGeometrySettings::Restore(this);
    }
    else
    {
        // Move main window to default position if settings file was not loaded.
        const QPoint available_desktop_top_left = geometry.topLeft();
        move(available_desktop_top_left.x() + rmv::kDesktopMargin, available_desktop_top_left.y() + rmv::kDesktopMargin);

        const int width  = geometry.width() * 0.66f;
        const int height = geometry.height() * 0.66f;
        resize(width, height);
    }

#if RMV_DEBUG_WINDOW
    const int desktop_width  = (rmv::kDesktopAvailableWidthPercentage / 100.0f) * geometry.width();
    const int desktop_height = (rmv::kDesktopAvailableHeightPercentage / 100.0f) * geometry.height();

    const int dbg_width  = desktop_width * (rmv::kDebugWindowDesktopWidthPercentage / 100.0f);
    const int dbg_height = desktop_height * (rmv::kDebugWindowDesktopHeightPercentage / 100.f);

    const int dbg_loc_x = rmv::kDesktopMargin + geometry.left();
    const int dbg_loc_y = (desktop_height - dbg_height - rmv::kDesktopMargin) + geometry.top();

    // Place debug out window.
    debug_window_.move(dbg_loc_x, dbg_loc_y);
    debug_window_.resize(dbg_width, dbg_height);
    debug_window_.show();

#endif  // RMV_DEBUG_WINDOW
}

void MainWindow::SetupHotkeyNavAction(QSignalMapper* mapper, int key, int pane)
{
    QAction* action = new QAction(this);
    action->setShortcut(key | Qt::ALT);

    connect(action, SIGNAL(triggered()), mapper, SLOT(map()));
    this->addAction(action);
    mapper->setMapping(action, pane);
}

void MainWindow::CreateActions()
{
    QSignalMapper* signal_mapper = new QSignalMapper(this);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoWelcomePane, rmv::kPaneIdStartWelcome);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoRecentSnapshotsPane, rmv::kPaneIdStartRecentTraces);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoGenerateSnapshotPane, rmv::kPaneIdTimelineGenerateSnapshot);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoDeviceConfigurationPane, rmv::kPaneIdTimelineDeviceConfiguration);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoHeapOverviewPane, rmv::kPaneIdSnapshotHeapOverview);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoResourceOverviewPane, rmv::kPaneIdSnapshotResourceOverview);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoAllocationOverviewPane, rmv::kPaneIdSnapshotAllocationOverview);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoResourceListPane, rmv::kPaneIdSnapshotResourceList);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoResourceHistoryPane, rmv::kPaneIdSnapshotResourceDetails);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoAllocationExplorerPane, rmv::kPaneIdSnapshotAllocationExplorer);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoSnapshotDeltaPane, rmv::kPaneIdCompareSnapshotDelta);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoMemoryLeakFinderPane, rmv::kPaneIdCompareMemoryLeakFinder);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoKeyboardShortcutsPane, rmv::kPaneIdSettingsKeyboardShortcuts);
    connect(signal_mapper, SIGNAL(mapped(int)), this, SLOT(ViewPane(int)));

    // Set up forward/backward navigation.
    QAction* shortcut = new QAction(this);
    shortcut->setShortcut(Qt::ALT | rmv::kKeyNavForwardArrow);

    connect(shortcut, &QAction::triggered, &rmv::NavigationManager::Get(), &rmv::NavigationManager::NavigateForward);
    this->addAction(shortcut);

    shortcut = new QAction(this);
    shortcut->setShortcut(Qt::ALT | rmv::kKeyNavBackwardArrow);

    connect(shortcut, &QAction::triggered, &rmv::NavigationManager::Get(), &rmv::NavigationManager::NavigateBack);
    this->addAction(shortcut);

    shortcut = new QAction(this);
    shortcut->setShortcut(rmv::kKeyNavBackwardBackspace);

    connect(shortcut, &QAction::triggered, &rmv::NavigationManager::Get(), &rmv::NavigationManager::NavigateBack);
    this->addAction(shortcut);

    // Set up time unit cycling.
    shortcut = new QAction(this);
    shortcut->setShortcut(Qt::CTRL | Qt::Key_T);

    connect(shortcut, &QAction::triggered, this, &MainWindow::CycleTimeUnits);
    this->addAction(shortcut);

    open_trace_action_ = new QAction(tr("Open trace"), this);
    open_trace_action_->setShortcut(Qt::CTRL | Qt::Key_O);
    connect(open_trace_action_, &QAction::triggered, this, &MainWindow::OpenTraceFromFileMenu);

    close_trace_action_ = new QAction(tr("Close trace"), this);
    close_trace_action_->setShortcut(Qt::CTRL | Qt::Key_F4);
    connect(close_trace_action_, &QAction::triggered, this, &MainWindow::CloseTrace);
    close_trace_action_->setDisabled(true);

    exit_action_ = new QAction(tr("Exit"), this);
    connect(exit_action_, &QAction::triggered, this, &MainWindow::CloseRmv);

    for (int i = 0; i < kMaxSubmenuSnapshots; i++)
    {
        recent_trace_actions_.push_back(new QAction("", this));
        recent_trace_mappers_.push_back(new QSignalMapper());
    }

    help_action_ = new QAction(tr("Help"), this);
    connect(help_action_, &QAction::triggered, this, &MainWindow::OpenHelp);

    about_action_ = new QAction(tr("About Radeon Memory Visualizer"), this);
    connect(about_action_, &QAction::triggered, this, &MainWindow::OpenAboutPane);
}

void MainWindow::CycleTimeUnits()
{
    rmv::RMVSettings::Get().CycleTimeUnits();
    pane_manager_.SwitchTimeUnits();
    UpdateSnapshotCombobox(rmv::SnapshotManager::Get().GetSelectedSnapshotPoint());
    if (pane_manager_.GetMainPaneFromPane(pane_manager_.GetCurrentPane()) == rmv::kMainPaneSnapshot)
    {
        ResizeNavigationLists();
    }
    update();
}

void MainWindow::SetupRecentTracesMenu()
{
    for (int i = 0; i < recent_trace_mappers_.size(); i++)
    {
        disconnect(recent_trace_actions_[i], SIGNAL(triggered(bool)), recent_trace_mappers_[i], SLOT(map()));
        disconnect(recent_trace_mappers_[i], SIGNAL(mapped(QString)), this, SLOT(LoadTrace(QString)));
    }

    recent_traces_menu_->clear();

    const QVector<RecentFileData>& files = rmv::RMVSettings::Get().RecentFiles();

    const int num_items = (files.size() > kMaxSubmenuSnapshots) ? kMaxSubmenuSnapshots : files.size();

    for (int i = 0; i < num_items; i++)
    {
        recent_trace_actions_[i]->setText(files[i].path);

        recent_traces_menu_->addAction(recent_trace_actions_[i]);

        connect(recent_trace_actions_[i], SIGNAL(triggered(bool)), recent_trace_mappers_[i], SLOT(map()));

        recent_trace_mappers_[i]->setMapping(recent_trace_actions_[i], files[i].path);

        connect(recent_trace_mappers_[i], SIGNAL(mapped(QString)), this, SLOT(LoadTrace(QString)));
    }

    emit rmv::MessageManager::Get().RecentFileListChanged();
}

void MainWindow::CreateMenus()
{
    file_menu_          = menuBar()->addMenu(tr("File"));
    recent_traces_menu_ = new QMenu("Recent traces");

    file_menu_->addAction(open_trace_action_);
    file_menu_->addAction(close_trace_action_);
    file_menu_->addSeparator();
    file_menu_->addMenu(recent_traces_menu_);
    file_menu_->addSeparator();
    file_menu_->addAction(exit_action_);

    file_menu_ = menuBar()->addMenu(tr("Help"));
    file_menu_->addAction(help_action_);
    file_menu_->addAction(about_action_);

    SetupRecentTracesMenu();
}

void MainWindow::LoadTrace(const QString& trace_file)
{
    rmv::TraceManager::Get().LoadTrace(trace_file);
}

void MainWindow::OpenTrace()
{
    close_trace_action_->setDisabled(false);
    timeline_pane_->OnTraceLoad();

    ui_->main_tab_widget_->setTabEnabled(rmv::kMainPaneTimeline, true);
    ui_->main_tab_widget_->setTabEnabled(rmv::kMainPaneSnapshot, true);
    ui_->main_tab_widget_->setTabEnabled(rmv::kMainPaneCompare, true);

    ViewPane(rmv::kPaneIdTimelineGenerateSnapshot);

    SetupRecentTracesMenu();

    UpdateTitlebar();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    rmv::RMVSettings::Get().SetWindowSize(event->size().width(), event->size().height());

    ResizeNavigationLists();

    rmv::LoadAnimationManager::Get().ResizeAnimation();
}

void MainWindow::moveEvent(QMoveEvent* event)
{
    QMainWindow::moveEvent(event);

    rmv::RMVSettings::Get().SetWindowPos(geometry().x(), geometry().y());
}

void MainWindow::OpenTraceFromFileMenu()
{
    QString file_name = QFileDialog::getOpenFileName(this, "Open file", rmv::RMVSettings::Get().GetLastFileOpenLocation(), rmv::text::kFileOpenFileTypes);

    if (!file_name.isNull())
    {
        rmv::TraceManager::Get().LoadTrace(file_name);
    }
}

void MainWindow::CloseTrace()
{
    rmv::TraceManager::Get().ClearTrace();

    pane_manager_.OnTraceClose();

    ResetUI();
    rmv::NavigationManager::Get().Reset();
    close_trace_action_->setDisabled(true);
    UpdateTitlebar();
    setWindowTitle(GetTitleBarString());
}

void MainWindow::ResetUI()
{
    // Default to first tab.
    const rmv::NavLocation& nav_location = pane_manager_.ResetNavigation();

    ui_->main_tab_widget_->setCurrentIndex(nav_location.main_tab_index);
    ui_->start_list_->setCurrentRow(nav_location.start_list_row);
    ui_->timeline_list_->setCurrentRow(nav_location.timeline_list_row);
    ui_->snapshot_list_->setCurrentRow(nav_location.snapshot_list_row);
    ui_->compare_list_->setCurrentRow(nav_location.compare_list_row);
    ui_->settings_list_->setCurrentRow(nav_location.settings_list_row);

    ui_->snapshot_start_stack_->setCurrentIndex(rmv::kSnapshotIndexEmptyPane);
    ui_->compare_start_stack_->setCurrentIndex(kCompareNotLoaded);

    ui_->main_tab_widget_->setTabEnabled(rmv::kMainPaneTimeline, false);
    ui_->main_tab_widget_->setTabEnabled(rmv::kMainPaneSnapshot, false);
    ui_->main_tab_widget_->setTabEnabled(rmv::kMainPaneCompare, false);

    UpdateTitlebar();
    pane_manager_.Reset();
}

void MainWindow::OpenHelp()
{
    // Get the file info.
    QFileInfo file_info(QCoreApplication::applicationDirPath() + rmv::text::kRmvHelpFile);

    // Check to see if the file is not a directory and that it exists.
    if (file_info.isFile() && file_info.exists())
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + rmv::text::kRmvHelpFile));
    }
    else
    {
        // The selected help file is missing on the disk so display a message box stating so.
        const QString text = rmv::text::kMissingRmvHelpFile + QCoreApplication::applicationDirPath() + rmv::text::kRmvHelpFile;
        QtCommon::QtUtils::ShowMessageBox(this, QMessageBox::Ok, QMessageBox::Critical, rmv::text::kMissingRmvHelpFile, text);
    }
}

void MainWindow::OpenAboutPane()
{
    ViewPane(rmv::kPaneIdStartAbout);
}

void MainWindow::CloseRmv()
{
    CloseTrace();
#if RMV_DEBUG_WINDOW
    debug_window_.close();
#endif  // RMV_DEBUG_WINDOW

    close();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event);

    rmv::RMVGeometrySettings::Save(this);

    CloseRmv();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event != nullptr)
    {
        if (event->mimeData()->hasUrls())
        {
            event->setDropAction(Qt::LinkAction);
            event->accept();
        }
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    if (event != nullptr)
    {
        const uint32_t num_urls = event->mimeData()->urls().size();

        for (uint32_t i = 0; i < num_urls; i++)
        {
            const QString potential_trace_path = event->mimeData()->urls().at(i).toLocalFile();

            if (rmv::TraceManager::Get().TraceValidToLoad(potential_trace_path) == true)
            {
                rmv::TraceManager::Get().LoadTrace(potential_trace_path);
            }
        }
    }
}

rmv::RMVPaneId MainWindow::SetupNextPane(rmv::RMVPaneId pane)
{
    const rmv::NavLocation* const nav_location = pane_manager_.SetupNextPane(pane);
    if (nav_location == nullptr)
    {
        return pane;
    }

    // These things emit signals.
    ui_->start_list_->setCurrentRow(nav_location->start_list_row);
    ui_->timeline_list_->setCurrentRow(nav_location->timeline_list_row);
    ui_->snapshot_list_->setCurrentRow(nav_location->snapshot_list_row);
    ui_->compare_list_->setCurrentRow(nav_location->compare_list_row);
    ui_->settings_list_->setCurrentRow(nav_location->settings_list_row);
    ui_->main_tab_widget_->setCurrentIndex(nav_location->main_tab_index);

    rmv::RMVPaneId new_pane = pane_manager_.UpdateCurrentPane();
    return (new_pane);
}

void MainWindow::ViewPane(int pane)
{
    rmv::RMVPaneId current_pane = SetupNextPane(static_cast<rmv::RMVPaneId>(pane));
    Q_ASSERT(pane == current_pane);
    rmv::NavigationManager::Get().RecordNavigationEventPaneSwitch(current_pane);
}

void MainWindow::UpdateSnapshotCombobox(RmtSnapshotPoint* selected_snapshot_point)
{
    rmv::TraceManager& trace_manager = rmv::TraceManager::Get();

    if (trace_manager.DataSetValid())
    {
        const RmtDataSet* data_set = trace_manager.GetDataSet();
        ui_->snapshot_combo_box_->ClearItems();

        int32_t selected_snapshot_point_index = 0;
        for (int32_t current_snapshot_point_index = 0; current_snapshot_point_index < data_set->snapshot_count; ++current_snapshot_point_index)
        {
            const RmtSnapshotPoint* current_snapshot_point = &data_set->snapshots[current_snapshot_point_index];
            QString name_string = QString(current_snapshot_point->name) + " (" + rmv::time_util::ClockToTimeUnit(current_snapshot_point->timestamp) + ")";
            if (current_snapshot_point == selected_snapshot_point)
            {
                selected_snapshot_point_index = current_snapshot_point_index;
            }
            ui_->snapshot_combo_box_->AddItem(name_string, (qulonglong)current_snapshot_point);
        }

        if (data_set->snapshot_count > 1)
        {
            ui_->main_tab_widget_->setTabEnabled(rmv::kMainPaneSnapshot, true);
            ui_->main_tab_widget_->setTabEnabled(rmv::kMainPaneCompare, true);
        }

        // Search the list of items in the combo box that has the open snapshot pointer.
        ui_->snapshot_combo_box_->SetSelectedRow(selected_snapshot_point_index);
    }
}

void MainWindow::OpenSelectedSnapshot()
{
    // Get the RmtSnapshotPoint pointer from the current list item.
    const int32_t current_row_index = ui_->snapshot_combo_box_->CurrentRow();
    if (current_row_index >= 0)
    {
        const QVariant    item_data              = ui_->snapshot_combo_box_->ItemData(current_row_index, Qt::UserRole);
        const uintptr_t   snapshot_point_address = item_data.toULongLong();
        RmtSnapshotPoint* snapshot_point         = (RmtSnapshotPoint*)snapshot_point_address;
        Q_ASSERT(snapshot_point);

        // Do not attempt to re-open the currently open snapshot.
        RmtDataSnapshot* current_snapshot = rmv::SnapshotManager::Get().GetOpenSnapshot();
        if (current_snapshot != nullptr && current_snapshot->snapshot_point != snapshot_point)
        {
            rmv::SnapshotManager::Get().SetSelectedSnapshotPoint(snapshot_point);

            // If switching snapshot on the resource details pane, navigate to the heap overview
            // pane since the selected resource will not be valid.
            if (pane_manager_.GetCurrentPane() == rmv::kPaneIdSnapshotResourceDetails)
            {
                emit rmv::MessageManager::Get().PaneSwitchRequested(rmv::kPaneIdSnapshotHeapOverview);
            }

            rmv::SnapshotManager::Get().SetSelectedResource(0);
            OpenSnapshot();
        }
    }
}

void MainWindow::UpdateStartListRow(const int row)
{
    pane_manager_.UpdateStartListRow(row);
}

void MainWindow::UpdateTimelineListRow(const int row)
{
    pane_manager_.UpdateTimelineListRow(row);
}

void MainWindow::UpdateSnapshotListRow(const int row)
{
    pane_manager_.UpdateSnapshotListRow(row);
}

void MainWindow::UpdateCompareListRow(const int row)
{
    pane_manager_.UpdateCompareListRow(row);
}

void MainWindow::UpdateSettingsListRow(const int row)
{
    pane_manager_.UpdateSettingsListRow(row);
}

void MainWindow::UpdateMainTabIndex(const int tab_index)
{
    pane_manager_.UpdateMainTabIndex(tab_index);

    if (pane_manager_.ClickedSnapshotTab())
    {
        if (rmv::SnapshotManager::Get().LoadSnapshotRequired())
        {
            OpenSnapshot();
        }
        else
        {
            ShowSnapshotPane();
        }
    }

    else if (pane_manager_.ClickedCompareTab())
    {
        if (rmv::SnapshotManager::Get().LoadCompareSnapshotsRequired())
        {
            OpenCompareSnapshots();

            // Reset to the snapshot delta pane since new snapshots were chosen.
            emit rmv::MessageManager::Get().PaneSwitchRequested(rmv::kPaneIdCompareSnapshotDelta);
        }
        else
        {
            ShowComparePane();
        }
    }
}

void MainWindow::OpenSnapshotPane(RmtResourceIdentifier resource_identifier)
{
    // User has requested to view a snapshot so transition to appropriate snapshot pane.
    // If the snapshot is selected from the timeline pane or from the memory leak finder pane,
    // then navigate to the heap overview pane.
    // The actual tab transition will start the snapshot load and display, which duplicates
    // the same behavior had the user just clicked on the SNAPSHOT tab directly.
    rmv::SnapshotManager::Get().SetSelectedResource(resource_identifier);

    rmv::RMVPaneId current_pane = pane_manager_.GetCurrentPane();
    if (current_pane == rmv::kPaneIdTimelineGenerateSnapshot)
    {
        emit rmv::MessageManager::Get().PaneSwitchRequested(rmv::kPaneIdSnapshotHeapOverview);
    }
    else if (current_pane == rmv::kPaneIdCompareMemoryLeakFinder)
    {
        emit rmv::MessageManager::Get().PaneSwitchRequested(rmv::kPaneIdSnapshotResourceDetails);
    }
}

void MainWindow::OpenComparePane()
{
    // User has requested to view a snapshot comparison so transition to the snapshot delta pane.
    // The actual tab transition will start the snapshot load and display, which duplicates
    // the same behavior had the user just clicked on the COMPARE tab directly.
    rmv::RMVPaneId current_pane = pane_manager_.GetCurrentPane();
    if (current_pane == rmv::kPaneIdTimelineGenerateSnapshot)
    {
        emit rmv::MessageManager::Get().PaneSwitchRequested(rmv::kPaneIdCompareSnapshotDelta);
    }
}

void MainWindow::OpenSnapshot()
{
    RmtSnapshotPoint* snapshot_point = rmv::SnapshotManager::Get().GetSelectedSnapshotPoint();
    if (!snapshot_point)
    {
        // Disable snapshot window.
        ui_->snapshot_start_stack_->setCurrentIndex(rmv::kSnapshotIndexEmptyPane);
    }
    else
    {
        qApp->setOverrideCursor(Qt::BusyCursor);
        // Generate the snapshot via a worker thread if it isn't cached. If it is cached,
        // then update the active snapshot.
        rmv::SnapshotManager::Get().GenerateSnapshot(rmv::TraceManager::Get().GetDataSet(), snapshot_point);
    }
}

void MainWindow::ShowSnapshotPane()
{
    rmv::SnapshotManager& snapshot_manager = rmv::SnapshotManager::Get();

    if (snapshot_manager.ResetSelectedResource() == true)
    {
        // The switch to resource details pane will cause the pane to be shown. However, the
        // snapshot may not be loaded yet since loading is done on a tab switch.
        // Force a reshow of the resource details pane now that the snapshot is loaded.
        resource_details_pane_->LoadResourceTimeline();
    }

    RmtDataSnapshot* snapshot = snapshot_manager.GetOpenSnapshot();

    pane_manager_.OpenSnapshot(snapshot);
    ui_->snapshot_start_stack_->setCurrentIndex(rmv::kSnapshotIndexPopulatedPane);

    UpdateTitlebar();
    UpdateSnapshotCombobox(snapshot->snapshot_point);
    ResizeNavigationLists();

    qApp->restoreOverrideCursor();
}

void MainWindow::OpenCompareSnapshots()
{
    RmtSnapshotPoint* snapshot_base = rmv::SnapshotManager::Get().GetSelectedCompareSnapshotPointBase();
    RmtSnapshotPoint* snapshot_diff = rmv::SnapshotManager::Get().GetSelectedCompareSnapshotPointDiff();

    if (snapshot_base == nullptr || snapshot_diff == nullptr)
    {
        // Disable compare window.
        ui_->compare_start_stack_->setCurrentIndex(kCompareNotLoaded);
    }
    else
    {
        qApp->setOverrideCursor(Qt::BusyCursor);
        // Generate the snapshot via a worker thread if it isn't cached. If it is cached,
        // then update the active snapshot.
        rmv::SnapshotManager::Get().GenerateComparison(rmv::TraceManager::Get().GetDataSet(), snapshot_base, snapshot_diff);
    }
}

void MainWindow::ShowComparePane()
{
    if (rmv::SnapshotManager::Get().LoadedCompareSnapshotsValid())
    {
        ui_->compare_start_stack_->setCurrentIndex(kCompareOk);
        pane_manager_.UpdateCompares();
        UpdateTitlebar();
    }
    else
    {
        ui_->compare_start_stack_->setCurrentIndex(kCompareEmpty);
    }
    qApp->restoreOverrideCursor();
}

QString MainWindow::GetTitleBarString() const
{
    QString title = RMV_APP_NAME RMV_BUILD_SUFFIX;

    title.append(" - ");
    QString version_string = QString("V%1").arg(RMV_VERSION_STRING);

    title.append(version_string);

    return title;
}

void MainWindow::UpdateTitlebar()
{
    QString title = "";

    const rmv::TraceManager&    trace_manager    = rmv::TraceManager::Get();
    const rmv::SnapshotManager& snapshot_manager = rmv::SnapshotManager::Get();

    if (trace_manager.DataSetValid())
    {
        const QString& file_name = trace_manager.GetTracePath();

        if (pane_manager_.GetMainPaneFromPane(pane_manager_.GetCurrentPane()) == rmv::kMainPaneSnapshot)
        {
            const char* snapshot_name = snapshot_manager.GetOpenSnapshotName();
            if (snapshot_name != nullptr)
            {
                title.append(QString(snapshot_name));
                title.append(" - ");
            }
        }
        if (pane_manager_.GetMainPaneFromPane(pane_manager_.GetCurrentPane()) == rmv::kMainPaneCompare)
        {
            const char* base_name = snapshot_manager.GetCompareSnapshotName(rmv::kSnapshotCompareBase);
            const char* diff_name = snapshot_manager.GetCompareSnapshotName(rmv::kSnapshotCompareDiff);
            if (base_name != nullptr && diff_name != nullptr)
            {
                title.append(QString(base_name) + " vs. " + QString(diff_name));
                title.append(" - ");
            }
        }

        title.append(file_name);
        title.append(" - ");
    }

    title.append(GetTitleBarString());

    setWindowTitle(title);
}

void MainWindow::BroadcastChangeColoring()
{
    pane_manager_.ChangeColoring();

    // Also change coloring on the empty snapshot and compare panes.
    ui_->snapshot_page_1_->ChangeColoring();
    ui_->compare_snapshots_not_loaded_->ChangeColoring();
    ui_->compare_snapshots_empty_->ChangeColoring();
}
