//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of RMV main window.
//=============================================================================

#include "views/main_window.h"

#include "ui_main_window.h"

#include <map>
#include <functional>

#include <QDesktopWidget>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMimeData>

#include "qt_common/utils/common_definitions.h"
#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "rmt_data_snapshot.h"
#include "rmt_assert.h"

#include "models/snapshot_manager.h"
#include "models/trace_manager.h"
#include "models/message_manager.h"
#include "views/compare/snapshot_delta_pane.h"
#include "views/compare/memory_leak_finder_pane.h"
#include "views/compare/compare_start_pane.h"
#include "views/navigation_manager.h"
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
#include "views/timeline/timeline_pane.h"
#include "views/timeline/device_configuration_pane.h"
#include "settings/rmv_settings.h"
#include "settings/rmv_geometry_settings.h"
#include "util/time_util.h"
#include "util/thread_controller.h"
#include "util/version.h"
#include "util/widget_util.h"

// Indices for the SNAPSHOT and COMPARE stacked widget. Index 1 is displayed if
// a snapshot loaded, otherwise and empty pane (0) is shown
static const int kIndexEmptyPane     = 0;
static const int kIndexPopulatedPane = 1;

// Thread count for the job queue.
static const int32_t kThreadCount = 8;

static const int kMaxSubmenuSnapshots = 10;
RmtJobQueue      MainWindow::job_queue_;

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
    , file_load_animation_(nullptr)
    , navigation_bar_(this)
{
    const bool loaded_settings = RMVSettings::Get().LoadSettings();

    ui_->setupUi(this);

    setWindowTitle(GetTitleBarString());
    setWindowIcon(QIcon(":/Resources/assets/rmv_icon_32x32.png"));
    setAcceptDrops(true);

    // Set white background for this pane
    rmv::widget_util::SetWidgetBackgroundColor(this, Qt::white);

    // Setup window sizes and settings
    SetupWindowRects(loaded_settings);

    welcome_pane_                                = CreatePane<WelcomePane>();
    recent_traces_pane_                          = CreatePane<RecentTracesPane>();
    BasePane* about_pane                         = CreatePane<AboutPane>();
    timeline_pane_                               = CreatePane<TimelinePane>();
    BasePane* resource_overview_pane             = CreatePane<ResourceOverviewPane>();
    BasePane* allocation_overview_pane           = CreatePane<AllocationOverviewPane>();
    BasePane* resource_list_pane                 = CreatePane<ResourceListPane>();
    BasePane* resource_details_pane              = CreatePane<ResourceDetailsPane>();
    BasePane* device_configuration_pane          = CreatePane<DeviceConfigurationPane>();
    BasePane* allocation_explorer_pane           = CreatePane<AllocationExplorerPane>();
    BasePane* heap_overview_pane                 = CreatePane<HeapOverviewPane>();
    snapshot_delta_pane_                         = CreatePane<SnapshotDeltaPane>();
    memory_leak_finder_pane_                     = CreatePane<MemoryLeakFinderPane>();
    settings_pane_                               = CreatePane<SettingsPane>();
    ThemesAndColorsPane* themes_and_colors_pane  = CreatePane<ThemesAndColorsPane>();
    BasePane*            keyboard_shortcuts_pane = CreatePane<KeyboardShortcutsPane>();
    snapshot_start_pane_                         = dynamic_cast<BasePane*>(ui_->snapshot_start_stack_->widget(0));
    compare_start_pane_                          = dynamic_cast<BasePane*>(ui_->compare_start_stack_->widget(0));

    // NOTE: Widgets need adding in the order they are to appear in the UI
    ui_->start_stack_->addWidget(welcome_pane_);
    ui_->start_stack_->addWidget(recent_traces_pane_);
    ui_->start_stack_->addWidget(about_pane);
    ui_->timeline_stack_->addWidget(timeline_pane_);
    ui_->timeline_stack_->addWidget(device_configuration_pane);
    ui_->snapshot_stack_->addWidget(heap_overview_pane);
    ui_->snapshot_stack_->addWidget(resource_overview_pane);
    ui_->snapshot_stack_->addWidget(allocation_overview_pane);
    ui_->snapshot_stack_->addWidget(resource_list_pane);
    ui_->snapshot_stack_->addWidget(allocation_explorer_pane);
    ui_->snapshot_stack_->addWidget(resource_details_pane);
    ui_->compare_stack_->addWidget(snapshot_delta_pane_);
    ui_->compare_stack_->addWidget(memory_leak_finder_pane_);
    ui_->settings_stack_->addWidget(settings_pane_);
    ui_->settings_stack_->addWidget(themes_and_colors_pane);
    ui_->settings_stack_->addWidget(keyboard_shortcuts_pane);

    ui_->main_tab_widget_->setTabEnabled(kMainPaneTimeline, false);
    ui_->main_tab_widget_->setTabEnabled(kMainPaneSnapshot, false);
    ui_->main_tab_widget_->setTabEnabled(kMainPaneCompare, false);

    SetupTabBar();
    CreateActions();
    CreateMenus();

    ResetUI();

    rmv::widget_util::InitSingleSelectComboBox(this, ui_->snapshot_combo_box_, "Snapshot", false);
    ui_->snapshot_combo_box_->SetListAboveButton(true);
    connect(ui_->snapshot_combo_box_, &ArrowIconComboBox::SelectionChanged, this, &MainWindow::OpenSelectedSnapshot);

    ViewPane(rmv::kPaneStartWelcome);

    connect(&navigation_bar_.BackButton(), &QPushButton::clicked, &rmv::NavigationManager::Get(), &rmv::NavigationManager::NavigateBack);
    connect(&navigation_bar_.ForwardButton(), &QPushButton::clicked, &rmv::NavigationManager::Get(), &rmv::NavigationManager::NavigateForward);
    connect(&rmv::NavigationManager::Get(), &rmv::NavigationManager::EnableBackNavButton, &navigation_bar_, &NavigationBar::EnableBackButton);
    connect(&rmv::NavigationManager::Get(), &rmv::NavigationManager::EnableForwardNavButton, &navigation_bar_, &NavigationBar::EnableForwardButton);
    connect(&MessageManager::Get(), &MessageManager::OpenTrace, this, &MainWindow::LoadTrace);

    // Update the navigation and pane switching when the UI panes are changed
    connect(ui_->start_list_, &QListWidget::currentRowChanged, this, &MainWindow::UpdateStartListRow);
    connect(ui_->timeline_list_, &QListWidget::currentRowChanged, this, &MainWindow::UpdateTimelineListRow);
    connect(ui_->snapshot_list_, &QListWidget::currentRowChanged, this, &MainWindow::UpdateSnapshotListRow);
    connect(ui_->compare_list_, &QListWidget::currentRowChanged, this, &MainWindow::UpdateCompareListRow);
    connect(ui_->settings_list_, &QListWidget::currentRowChanged, this, &MainWindow::UpdateSettingsListRow);
    connect(ui_->main_tab_widget_, &QTabWidget::currentChanged, this, &MainWindow::UpdateMainTabIndex);

    // Set up the stack widget signals so they know which pane in their stack to switch to
    connect(ui_->start_list_, &QListWidget::currentRowChanged, ui_->start_stack_, &QStackedWidget::setCurrentIndex);
    connect(ui_->timeline_list_, &QListWidget::currentRowChanged, ui_->timeline_stack_, &QStackedWidget::setCurrentIndex);
    connect(ui_->snapshot_list_, &QListWidget::currentRowChanged, ui_->snapshot_stack_, &QStackedWidget::setCurrentIndex);
    connect(ui_->compare_list_, &QListWidget::currentRowChanged, ui_->compare_stack_, &QStackedWidget::setCurrentIndex);
    connect(ui_->settings_list_, &QListWidget::currentRowChanged, ui_->settings_stack_, &QStackedWidget::setCurrentIndex);

    connect(&MessageManager::Get(), &MessageManager::NavigateToPane, this, &MainWindow::ViewPane);
    connect(&MessageManager::Get(), &MessageManager::NavigateToPaneUnrecorded, this, &MainWindow::SetupNextPane);

    connect(&MessageManager::Get(), &MessageManager::OpenSnapshot, this, &MainWindow::OpenSnapshot);
    connect(&MessageManager::Get(), &MessageManager::CompareSnapshot, this, &MainWindow::CompareSnapshot);
    connect(themes_and_colors_pane, &ThemesAndColorsPane::RefreshedColors, this, &MainWindow::BroadcastChangeColoring);

    // Connect to ScalingManager for notifications
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
    // Set grey background for main tab bar background
    rmv::widget_util::SetWidgetBackgroundColor(ui_->main_tab_widget_, QColor(51, 51, 51));

    // set the mouse cursor to pointing hand cursor for all of the tabs
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
    ui_->main_tab_widget_->SetSpacerIndex(kMainPaneSpacer);

    // Adjust spacing around the navigation bar so that it appears centered on the tab bar.
    navigation_bar_.layout()->setContentsMargins(15, 3, 35, 2);

    // Setup navigation browser toolbar on the main tab bar.
    ui_->main_tab_widget_->SetTabTool(kMainPaneNavigation, &navigation_bar_);
}

void MainWindow::SetupWindowRects(bool loaded_settings)
{
    if (loaded_settings)
    {
        RMVGeometrySettings::Restore(this);
    }
    else
    {
        // Move main window to default position if settings file was not loaded.
        const QRect  geometry                   = QApplication::desktop()->availableGeometry();
        const QPoint available_desktop_top_left = geometry.topLeft();
        move(available_desktop_top_left.x() + rmv::kDesktopMargin, available_desktop_top_left.y() + rmv::kDesktopMargin);

        const int width  = geometry.width() * 0.66f;
        const int height = geometry.height() * 0.66f;
        resize(width, height);
    }

#if RMV_DEBUG_WINDOW
    const int   screen_number = QApplication::desktop()->screenNumber(this);
    const QRect desktop_res   = QApplication::desktop()->availableGeometry(screen_number);

    const int desktop_width  = (rmv::kDesktopAvailableWidthPercentage / 100.0f) * desktop_res.width();
    const int desktop_height = (rmv::kDesktopAvailableHeightPercentage / 100.0f) * desktop_res.height();

    const int dbg_width  = desktop_width * (rmv::kDebugWindowDesktopWidthPercentage / 100.0f);
    const int dbg_height = desktop_height * (rmv::kDebugWindowDesktopHeightPercentage / 100.f);

    const int dbg_loc_x = rmv::kDesktopMargin + desktop_res.left();
    const int dbg_loc_y = (desktop_height - dbg_height - rmv::kDesktopMargin) + desktop_res.top();

    // place debug out window
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
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoWelcomePane, rmv::kPaneStartWelcome);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoRecentSnapshotsPane, rmv::kPaneStartRecentTraces);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoGenerateSnapshotPane, rmv::kPaneTimelineGenerateSnapshot);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoDeviceConfigurationPane, rmv::kPaneTimelineDeviceConfiguration);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoHeapOverviewPane, rmv::kPaneSnapshotHeapOverview);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoResourceOverviewPane, rmv::kPaneSnapshotResourceOverview);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoAllocationOverviewPane, rmv::kPaneSnapshotAllocationOverview);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoResourceListPane, rmv::kPaneSnapshotResourceList);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoResourceHistoryPane, rmv::kPaneSnapshotResourceDetails);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoAllocationExplorerPane, rmv::kPaneSnapshotAllocationExplorer);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoSnapshotDeltaPane, rmv::kPaneCompareSnapshotDelta);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoMemoryLeakFinderPane, rmv::kPaneCompareMemoryLeakFinder);
    SetupHotkeyNavAction(signal_mapper, rmv::kGotoKeyboardShortcutsPane, rmv::kPaneSettingsKeyboardShortcuts);
    connect(signal_mapper, SIGNAL(mapped(int)), this, SLOT(ViewPane(int)));

    // Set up forward/backward navigation
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

    // Set up time unit cycling
    shortcut = new QAction(this);
    shortcut->setShortcut(Qt::CTRL | Qt::Key_T);

    connect(shortcut, &QAction::triggered, this, &MainWindow::CycleTimeUnits);
    this->addAction(shortcut);

    open_trace_action_ = new QAction(tr("Open trace"), this);
    open_trace_action_->setShortcut(Qt::CTRL | Qt::Key_O);
    connect(open_trace_action_, &QAction::triggered, this, &MainWindow::OpenTrace);

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
    settings_pane_->CycleTimeUnits();
    pane_manager_.SwitchTimeUnits();
    UpdateSnapshotCombobox(SnapshotManager::Get().GetSelectedSnapshotPoint());
    if (pane_manager_.GetMainPaneFromPane(pane_manager_.GetCurrentPane()) == kMainPaneSnapshot)
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

    const QVector<RecentFileData>& files = RMVSettings::Get().RecentFiles();

    const int num_items = (files.size() > kMaxSubmenuSnapshots) ? kMaxSubmenuSnapshots : files.size();

    for (int i = 0; i < num_items; i++)
    {
        recent_trace_actions_[i]->setText(files[i].path);

        recent_traces_menu_->addAction(recent_trace_actions_[i]);

        connect(recent_trace_actions_[i], SIGNAL(triggered(bool)), recent_trace_mappers_[i], SLOT(map()));

        recent_trace_mappers_[i]->setMapping(recent_trace_actions_[i], files[i].path);

        connect(recent_trace_mappers_[i], SIGNAL(mapped(QString)), this, SLOT(LoadTrace(QString)));
    }

    emit welcome_pane_->FileListChanged();
    emit recent_traces_pane_->FileListChanged();
}

void MainWindow::OpenRecentTracesPane()
{
    ViewPane(rmv::kPaneStartRecentTraces);
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
    TraceManager& trace_manager = TraceManager::Get();
    if (trace_manager.ReadyToLoadTrace())
    {
        if (trace_manager.TraceValidToLoad(trace_file) == true)
        {
            bool success = trace_manager.LoadTrace(trace_file, false);

            if (success)
            {
                StartAnimation(ui_->main_tab_widget_, ui_->main_tab_widget_->TabHeight());
            }
        }
        else
        {
            // The selected trace file is missing on the disk so display a message box stating so
            const QString text = rmv::text::kOpenRecentTraceStart + trace_file + rmv::text::kOpenRecentTraceEnd;
            QtCommon::QtUtils::ShowMessageBox(this, QMessageBox::Ok, QMessageBox::Critical, rmv::text::kOpenRecentTraceTitle, text);
        }
    }
}

void MainWindow::TraceLoadComplete()
{
    close_trace_action_->setDisabled(false);
    timeline_pane_->OnTraceLoad();

    ui_->main_tab_widget_->setTabEnabled(kMainPaneTimeline, true);
    ui_->main_tab_widget_->setTabEnabled(kMainPaneSnapshot, true);
    ui_->main_tab_widget_->setTabEnabled(kMainPaneCompare, true);

    ViewPane(rmv::kPaneTimelineGenerateSnapshot);

    SetupRecentTracesMenu();

    UpdateTitlebar();
}

void MainWindow::StartAnimation(QWidget* parent, int height_offset)
{
    if (file_load_animation_ == nullptr)
    {
        file_load_animation_ = new FileLoadingWidget(parent);

        // Set overall size of the widget to cover the tab contents.
        int width  = parent->width();
        int height = parent->height() - height_offset;
        file_load_animation_->setGeometry(parent->x(), parent->y() + height_offset, width, height);

        // Set the contents margin so that the animated bars cover a portion of the middle of the screen.
        const int desired_loading_dimension = ScalingManager::Get().Scaled(200);
        int       vertical_margin           = (height - desired_loading_dimension) / 2;
        int       horizontal_margin         = (width - desired_loading_dimension) / 2;
        file_load_animation_->setContentsMargins(horizontal_margin, vertical_margin, horizontal_margin, vertical_margin);

        file_load_animation_->show();
        ui_->main_tab_widget_->setDisabled(true);
        file_menu_->setDisabled(true);

        qApp->setOverrideCursor(Qt::BusyCursor);
    }
}

void MainWindow::StopAnimation()
{
    if (file_load_animation_ != nullptr)
    {
        delete file_load_animation_;
        file_load_animation_ = nullptr;

        ui_->main_tab_widget_->setEnabled(true);
        file_menu_->setEnabled(true);

        qApp->restoreOverrideCursor();
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    RMVSettings::Get().SetWindowSize(event->size().width(), event->size().height());

    ResizeNavigationLists();

    // Update Animation widget on window resize
    if (file_load_animation_ != nullptr)
    {
        // Set overall size of the widget to cover the tab contents.
        double   height_offset = ui_->main_tab_widget_->TabHeight();
        QWidget* parent        = file_load_animation_->parentWidget();
        int      width         = parent->width();
        int      height        = parent->height() - height_offset;
        file_load_animation_->setGeometry(parent->x(), parent->y() + height_offset, width, height);

        // Set the contents margin so that the animated bars only cover a small area in the middle of the screen.
        const int desired_loading_dimension = 200;
        int       vertical_margin           = (height - desired_loading_dimension) / 2;
        int       horizontal_margin         = (width - desired_loading_dimension) / 2;
        file_load_animation_->setContentsMargins(horizontal_margin, vertical_margin, horizontal_margin, vertical_margin);
    }
}

void MainWindow::moveEvent(QMoveEvent* event)
{
    QMainWindow::moveEvent(event);

    RMVSettings::Get().SetWindowPos(geometry().x(), geometry().y());
}

void MainWindow::OpenTrace()
{
    QString file_name = QFileDialog::getOpenFileName(this, "Open file", RMVSettings::Get().GetLastFileOpenLocation(), rmv::text::kFileOpenFileTypes);

    if (!file_name.isNull())
    {
        LoadTrace(file_name);
    }
}

void MainWindow::CloseTrace()
{
    TraceManager::Get().ClearTrace();

    BroadcastOnTraceClose();
    ResetUI();
    rmv::NavigationManager::Get().Reset();
    close_trace_action_->setDisabled(true);
    UpdateTitlebar();
    setWindowTitle(GetTitleBarString());
}

void MainWindow::ResetUI()
{
    // Default to first tab
    const NavLocation& nav_location = pane_manager_.ResetNavigation();

    ui_->main_tab_widget_->setCurrentIndex(nav_location.main_tab_index);
    ui_->start_list_->setCurrentRow(nav_location.start_list_row);
    ui_->timeline_list_->setCurrentRow(nav_location.timeline_list_row);
    ui_->snapshot_list_->setCurrentRow(nav_location.snapshot_list_row);
    ui_->compare_list_->setCurrentRow(nav_location.compare_list_row);
    ui_->settings_list_->setCurrentRow(nav_location.settings_list_row);

    ui_->snapshot_start_stack_->setCurrentIndex(kIndexEmptyPane);
    ui_->compare_start_stack_->setCurrentIndex(kIndexEmptyPane);

    ui_->main_tab_widget_->setTabEnabled(kMainPaneTimeline, false);
    ui_->main_tab_widget_->setTabEnabled(kMainPaneSnapshot, false);
    ui_->main_tab_widget_->setTabEnabled(kMainPaneCompare, false);

    BroadcastReset();
}

void MainWindow::OpenHelp()
{
    // Get the file info
    QFileInfo file_info(QCoreApplication::applicationDirPath() + rmv::text::kRmvHelpFile);

    // Check to see if the file is not a directory and that it exists
    if (file_info.isFile() && file_info.exists())
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + rmv::text::kRmvHelpFile));
    }
    else
    {
        // The selected help file is missing on the disk so display a message box stating so
        const QString text = rmv::text::kMissingRmvHelpFile + QCoreApplication::applicationDirPath() + rmv::text::kRmvHelpFile;
        QtCommon::QtUtils::ShowMessageBox(this, QMessageBox::Ok, QMessageBox::Critical, rmv::text::kMissingRmvHelpFile, text);
    }
}

void MainWindow::OpenAboutPane()
{
    ViewPane(rmv::kPaneStartAbout);
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

    RMVGeometrySettings::Save(this);

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

            if (TraceManager::Get().TraceValidToLoad(potential_trace_path) == true)
            {
                LoadTrace(potential_trace_path);
            }
        }
    }
}

rmv::RMVPane MainWindow::SetupNextPane(rmv::RMVPane pane)
{
    MainPanes           main_pane     = pane_manager_.GetMainPaneFromPane(pane);
    const TraceManager& trace_manager = TraceManager::Get();

    if (main_pane == kMainPaneSnapshot || main_pane == kMainPaneCompare)
    {
        // make sure at least one snapshot is generated before navigating to the
        // snapshot or compare panes
        if (!trace_manager.DataSetValid())
        {
            return pane;
        }

        RmtDataSnapshot* open_snapshot = trace_manager.GetOpenSnapshot();
        if (main_pane == kMainPaneSnapshot && open_snapshot == nullptr)
        {
            return pane;
        }

        if (main_pane == kMainPaneCompare &&
            (trace_manager.GetComparedSnapshot(kSnapshotCompareBase) == nullptr || trace_manager.GetComparedSnapshot(kSnapshotCompareDiff) == nullptr))
        {
            return pane;
        }
    }
    else if (main_pane == kMainPaneTimeline)
    {
        // make sure a trace is loaded before navigating to the timeline pane
        if (!trace_manager.DataSetValid())
        {
            return pane;
        }
    }

    const NavLocation& nav_location = pane_manager_.SetupNextPane(pane);

    // These things emit signals
    ui_->start_list_->setCurrentRow(nav_location.start_list_row);
    ui_->timeline_list_->setCurrentRow(nav_location.timeline_list_row);
    ui_->snapshot_list_->setCurrentRow(nav_location.snapshot_list_row);
    ui_->compare_list_->setCurrentRow(nav_location.compare_list_row);
    ui_->settings_list_->setCurrentRow(nav_location.settings_list_row);
    ui_->main_tab_widget_->setCurrentIndex(nav_location.main_tab_index);

    rmv::RMVPane new_pane = pane_manager_.UpdateCurrentPane();
    return (new_pane);
}

void MainWindow::ViewPane(int pane)
{
    rmv::RMVPane current_pane = SetupNextPane(static_cast<rmv::RMVPane>(pane));
    rmv::NavigationManager::Get().RecordNavigationEventPaneSwitch(current_pane);
}

void MainWindow::UpdateSnapshotCombobox(RmtSnapshotPoint* selected_snapshot_point)
{
    TraceManager& trace_manager = TraceManager::Get();

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
            ui_->main_tab_widget_->setTabEnabled(kMainPaneSnapshot, true);
            ui_->main_tab_widget_->setTabEnabled(kMainPaneCompare, true);
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
        RMT_ASSERT(snapshot_point);

        // Do not attempt to re-open the currently open snapshot.
        RmtDataSnapshot* current_snapshot = TraceManager::Get().GetOpenSnapshot();
        if (current_snapshot != nullptr && current_snapshot->snapshot_point != snapshot_point)
        {
            // if switching snapshot on the resource details pane, navigate to the heap overview
            // pane since the selected resource will not be valid
            if (pane_manager_.GetCurrentPane() == rmv::kPaneSnapshotResourceDetails)
            {
                emit MessageManager::Get().NavigateToPane(rmv::kPaneSnapshotResourceOverview);
            }

            emit MessageManager::Get().SelectSnapshot(snapshot_point);
            emit MessageManager::Get().OpenSnapshot(snapshot_point);
        }
    }
}

void MainWindow::BroadcastOnTraceClose()
{
    pane_manager_.OnTraceClose();
}

void MainWindow::UpdateStartListRow(const int row)
{
    pane_manager_.UpdateStartListRow(row);
    BroadcastPaneSwitched();
}

void MainWindow::UpdateTimelineListRow(const int row)
{
    pane_manager_.UpdateTimelineListRow(row);
    BroadcastPaneSwitched();
}

void MainWindow::UpdateSnapshotListRow(const int row)
{
    pane_manager_.UpdateSnapshotListRow(row);
    BroadcastPaneSwitched();
}

void MainWindow::UpdateCompareListRow(const int row)
{
    pane_manager_.UpdateCompareListRow(row);
    BroadcastPaneSwitched();
}

void MainWindow::UpdateSettingsListRow(const int row)
{
    pane_manager_.UpdateSettingsListRow(row);
    BroadcastPaneSwitched();
}

void MainWindow::UpdateMainTabIndex(const int row)
{
    pane_manager_.UpdateMainTabIndex(row);
    BroadcastPaneSwitched();
}

void MainWindow::BroadcastPaneSwitched()
{
    UpdateSnapshotCombobox(SnapshotManager::Get().GetSelectedSnapshotPoint());

    // Catch any transition to the snapshot tab from any other tab and
    // make sure the snapshot is opened, specifically the case of selecting
    // something in the timeline pane, and then moving to the snapshot view.
    MainPanes previous_main_pane = pane_manager_.GetMainPaneFromPane(pane_manager_.GetPreviousPane());
    MainPanes current_main_pane  = pane_manager_.GetMainPaneFromPane(pane_manager_.GetCurrentPane());
    if (current_main_pane == kMainPaneSnapshot && previous_main_pane != kMainPaneSnapshot)
    {
        // Only open the snapshot if it's different to the one already opened
        RmtSnapshotPoint* snapshot_point = SnapshotManager::Get().GetSelectedSnapshotPoint();
        RmtDataSnapshot*  snapshot       = nullptr;
        if (snapshot_point != nullptr)
        {
            snapshot = snapshot_point->cached_snapshot;
        }
        if (TraceManager::Get().SnapshotAlreadyOpened(snapshot) == false || snapshot == nullptr)
        {
            OpenSnapshot(snapshot_point);
        }
    }
    UpdateTitlebar();
    ResizeNavigationLists();

    pane_manager_.PaneSwitched();
}

void MainWindow::BroadcastReset()
{
    UpdateTitlebar();

    pane_manager_.Reset();
}

void MainWindow::OpenSnapshot(RmtSnapshotPoint* snapshot_point)
{
    if (!snapshot_point)
    {
        // disable snapshot window
        TraceManager::Get().ClearOpenSnapshot();
        ui_->snapshot_start_stack_->setCurrentIndex(kIndexEmptyPane);
        return;
    }

    if (!snapshot_point->cached_snapshot)
    {
        qApp->setOverrideCursor(Qt::BusyCursor);
        // Generate the snapshot via a worker thread. When the worker
        // thread finishes, this function will be called again but this time
        // snapshot_point->cachedSnapshot will contain valid data
        SnapshotManager::Get().GenerateSnapshot(snapshot_point, this, ui_->main_tab_widget_);
    }
    else
    {
        // Snapshot is loaded at this point.
        // deselect any selected resource
        emit MessageManager::Get().ResourceSelected(0);

        TraceManager::Get().SetOpenSnapshot(snapshot_point->cached_snapshot);

        ui_->snapshot_start_stack_->setCurrentIndex(kIndexPopulatedPane);
        UpdateCompares();

        pane_manager_.OpenSnapshot(snapshot_point->cached_snapshot);

        UpdateTitlebar();
        UpdateSnapshotCombobox(snapshot_point);

        // Should only do this jump if we're on the timeline pane as this will catch the case
        // of opening a snapshot while on the timeline pane, and jump to heap overview.
        // It shouldn't do this if we open a snapshot from the snapshot pane itself.
        if (pane_manager_.GetCurrentPane() == rmv::kPaneTimelineGenerateSnapshot)
        {
            emit MessageManager::Get().NavigateToPane(rmv::kPaneSnapshotHeapOverview);
        }
        ResizeNavigationLists();

        qApp->restoreOverrideCursor();
    }
}

void MainWindow::CompareSnapshot(RmtSnapshotPoint* snapshot_base, RmtSnapshotPoint* snapshot_diff)
{
    if (snapshot_base == nullptr || snapshot_diff == nullptr)
    {
        TraceManager::Get().ClearComparedSnapshots();

        // disable compare window
        ui_->compare_start_stack_->setCurrentIndex(kIndexEmptyPane);
        return;
    }

    if (!snapshot_base->cached_snapshot || !snapshot_diff->cached_snapshot)
    {
        qApp->setOverrideCursor(Qt::BusyCursor);
        // Generate the snapshots via a worker thread. When the worker
        // thread finishes, this function will be called again but this time
        // snapshot_point->cachedSnapshot will contain valid data
        SnapshotManager::Get().GenerateComparison(snapshot_base, snapshot_diff, this, ui_->main_tab_widget_);
    }
    else
    {
        TraceManager::Get().SetComparedSnapshot(snapshot_base->cached_snapshot, snapshot_diff->cached_snapshot);

        UpdateCompares();

        UpdateTitlebar();

        emit MessageManager::Get().NavigateToPane(rmv::kPaneCompareSnapshotDelta);

        qApp->restoreOverrideCursor();
    }
}

QString MainWindow::GetTitleBarString()
{
    QString title = RMV_APP_NAME RMV_BUILD_SUFFIX;

    title.append(" - ");
    QString version_string;

    version_string.sprintf("V%s", RMV_VERSION_STRING);

    title.append(version_string);

    return title;
}

void MainWindow::UpdateTitlebar()
{
    QString title = "";

    const TraceManager& trace_manager = TraceManager::Get();

    if (trace_manager.DataSetValid())
    {
        const QString file_name = trace_manager.GetTracePath();

        if (pane_manager_.GetMainPaneFromPane(pane_manager_.GetCurrentPane()) == kMainPaneSnapshot)
        {
            const char* snapshot_name = trace_manager.GetOpenSnapshotName();
            if (snapshot_name != nullptr)
            {
                title.append(QString(snapshot_name));
                title.append(" - ");
            }
        }
        if (pane_manager_.GetMainPaneFromPane(pane_manager_.GetCurrentPane()) == kMainPaneCompare)
        {
            const char* base_name = trace_manager.GetCompareSnapshotName(kSnapshotCompareBase);
            const char* diff_name = trace_manager.GetCompareSnapshotName(kSnapshotCompareDiff);
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

void MainWindow::UpdateCompares()
{
    if (TraceManager::Get().GetComparedSnapshot(kSnapshotCompareDiff) != nullptr)
    {
        ui_->compare_start_stack_->setCurrentIndex(kIndexPopulatedPane);

        snapshot_delta_pane_->Refresh();
        memory_leak_finder_pane_->Refresh();
    }
}

void MainWindow::BroadcastChangeColoring()
{
    pane_manager_.ChangeColoring();
    snapshot_start_pane_->ChangeColoring();
    compare_start_pane_->ChangeColoring();
}

void MainWindow::InitializeJobQueue()
{
    const RmtErrorCode error_code = RmtJobQueueInitialize(&job_queue_, kThreadCount);
    RMT_ASSERT(error_code == RMT_OK);
}

RmtJobQueue* MainWindow::GetJobQueue()
{
    return &job_queue_;
}

void MainWindow::DestroyJobQueue()
{
    RmtJobQueueShutdown(&job_queue_);
}
