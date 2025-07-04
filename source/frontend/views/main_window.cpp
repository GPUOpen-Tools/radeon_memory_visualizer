//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the main window.
//=============================================================================

#include "views/main_window.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMimeData>
#include <QScreen>

#include "qt_common/custom_widgets/driver_overrides_model.h"
#include "qt_common/utils/common_definitions.h"
#include "qt_common/utils/qt_util.h"

#include "rmt_assert.h"

#include "managers/load_animation_manager.h"
#include "managers/message_manager.h"
#include "managers/navigation_manager.h"
#include "managers/pane_manager.h"
#include "managers/snapshot_manager.h"
#include "managers/trace_manager.h"
#include "settings/rmv_geometry_settings.h"
#include "settings/rmv_settings.h"
#include "util/constants.h"
#include "util/time_util.h"
#include "util/version.h"
#include "util/widget_util.h"
#include "views/compare/compare_start_pane.h"
#include "views/compare/memory_leak_finder_pane.h"
#include "views/compare/snapshot_delta_pane.h"
#include "views/settings/keyboard_shortcuts_pane.h"
#include "views/settings/settings_pane.h"
#include "views/settings/themes_and_colors_pane.h"
#include "views/snapshot/allocation_explorer_pane.h"
#include "views/snapshot/allocation_overview_pane.h"
#include "views/snapshot/heap_overview_pane.h"
#include "views/snapshot/resource_details_pane.h"
#include "views/snapshot/resource_list_pane.h"
#include "views/snapshot/resource_overview_pane.h"
#include "views/snapshot/snapshot_start_pane.h"
#include "views/start/about_pane.h"
#include "views/start/recent_traces_pane.h"
#include "views/start/welcome_pane.h"
#include "views/timeline/device_configuration_pane.h"
#include "views/timeline/timeline_pane.h"

using namespace driver_overrides;

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

    ColorThemeType color_mode = static_cast<ColorThemeType>(rmv::RMVSettings::Get().GetColorTheme());

    if (color_mode == kColorThemeTypeCount)
    {
        color_mode = QtCommon::QtUtils::DetectOsSetting();
    }

    QtCommon::QtUtils::ColorTheme::Get().SetColorTheme(static_cast<ColorThemeType>(color_mode));

    qApp->setPalette(QtCommon::QtUtils::ColorTheme::Get().GetCurrentPalette());

    // Load application stylesheet.
    QFile style_sheet(rmv::resource::kStylesheet);

    if (style_sheet.open(QFile::ReadOnly))
    {
        QString app_stylesheet = style_sheet.readAll();

        if (color_mode == kColorThemeTypeDark)
        {
            QFile dark_style_sheet(rmv::resource::kDarkStylesheet);
            if (dark_style_sheet.open(QFile::ReadOnly))
            {
                app_stylesheet.append(dark_style_sheet.readAll());
            }
        }
        else
        {
            QFile light_style_sheet(rmv::resource::kLightStylesheet);
            if (light_style_sheet.open(QFile::ReadOnly))
            {
                app_stylesheet.append(light_style_sheet.readAll());
            }
        }

        qApp->setStyleSheet(app_stylesheet);
    }

    ui_->setupUi(this);

    // Initialize the Driver Overrides model.
    DriverOverridesModel::GetInstance()->SetApplicationDetails(rmv::kRmvApplicationFileTypeString);

    // Set up the links for the Driver Overrides notification banner.
    connect(
        ui_->driver_overrides_notification_banner_, &DriverOverridesNotificationBanner::ShowDetailsClicked, this, &MainWindow::OpenDriverOverridesDetailsLink);

    connect(ui_->driver_overrides_notification_banner_,
            &DriverOverridesNotificationBanner::DontShowAgainRequested,
            this,
            &MainWindow::DontShowDriverOverridesNotification);

    setWindowTitle(GetTitleBarString());
    setWindowIcon(QIcon(":/Resources/assets/rmv_icon_32x32.png"));
    setAcceptDrops(true);

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
    rmv::LoadAnimationManager::Get().Initialize(ui_->main_tab_widget_);

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
    connect(&rmv::TraceManager::Get(), &rmv::TraceManager::TraceOpenFailed, this, &MainWindow::SetupRecentTracesMenu);

    connect(&rmv::SnapshotManager::Get(), &rmv::SnapshotManager::SnapshotOpened, this, &MainWindow::OpenSnapshotPane);
    connect(&rmv::SnapshotManager::Get(), &rmv::SnapshotManager::CompareSnapshotsOpened, this, &MainWindow::OpenComparePane);

    connect(&rmv::SnapshotManager::Get(), &rmv::SnapshotManager::SnapshotLoaded, this, &MainWindow::ShowSnapshotPane);
    connect(&rmv::SnapshotManager::Get(), &rmv::SnapshotManager::CompareSnapshotsLoaded, this, &MainWindow::ShowComparePane);

    connect(themes_and_colors_pane, &ThemesAndColorsPane::RefreshedColors, this, &MainWindow::BroadcastChangeColoring);

    connect(&rmv::MessageManager::Get(), &rmv::MessageManager::ChangeActionsRequested, this, &MainWindow::EnableActions);
}

MainWindow::~MainWindow()
{
    delete open_trace_action_;
    delete close_trace_action_;
    delete exit_action_;
    delete help_action_;
    delete about_action_;

    for (auto action : recent_trace_actions_)
    {
        delete action;
    }

    for (auto action : navigation_actions_)
    {
        delete action;
    }

    delete ui_;
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
    // Set the mouse cursor to pointing hand cursor for all of the tabs.
    QList<QTabBar*> tab_bar = ui_->main_tab_widget_->findChildren<QTabBar*>();
    foreach (QTabBar* item, tab_bar)
    {
        if (item != nullptr)
        {
            item->setCursor(Qt::PointingHandCursor);
            item->setContentsMargins(10, 0, 10, 0);
        }
    }

    // Set the tab that divides the left and right justified tabs.
    ui_->main_tab_widget_->SetSpacerIndex(rmv::kMainPaneSpacer);

    // Adjust spacing around the navigation bar so that it appears centered on the tab bar.
    navigation_bar_.layout()->setContentsMargins(15, 0, 35, 0);

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

void MainWindow::SetupHotkeyNavAction(int key, int pane)
{
    QAction* action = new QAction(this);
    action->setShortcut(key | Qt::ALT);
    navigation_actions_.push_back(action);

    this->addAction(action);
    connect(action, &QAction::triggered, [=]() { ViewPane(pane); });
}

void MainWindow::CreateActions()
{
    SetupHotkeyNavAction(rmv::kGotoWelcomePane, rmv::kPaneIdStartWelcome);
    SetupHotkeyNavAction(rmv::kGotoRecentSnapshotsPane, rmv::kPaneIdStartRecentTraces);
    SetupHotkeyNavAction(rmv::kGotoAboutPane, rmv::kPaneIdStartAbout);
    SetupHotkeyNavAction(rmv::kGotoGenerateSnapshotPane, rmv::kPaneIdTimelineGenerateSnapshot);
    SetupHotkeyNavAction(rmv::kGotoDeviceConfigurationPane, rmv::kPaneIdTimelineDeviceConfiguration);
    SetupHotkeyNavAction(rmv::kGotoHeapOverviewPane, rmv::kPaneIdSnapshotHeapOverview);
    SetupHotkeyNavAction(rmv::kGotoResourceOverviewPane, rmv::kPaneIdSnapshotResourceOverview);
    SetupHotkeyNavAction(rmv::kGotoAllocationOverviewPane, rmv::kPaneIdSnapshotAllocationOverview);
    SetupHotkeyNavAction(rmv::kGotoResourceListPane, rmv::kPaneIdSnapshotResourceList);
    SetupHotkeyNavAction(rmv::kGotoResourceHistoryPane, rmv::kPaneIdSnapshotResourceDetails);
    SetupHotkeyNavAction(rmv::kGotoAllocationExplorerPane, rmv::kPaneIdSnapshotAllocationExplorer);
    SetupHotkeyNavAction(rmv::kGotoSnapshotDeltaPane, rmv::kPaneIdCompareSnapshotDelta);
    SetupHotkeyNavAction(rmv::kGotoMemoryLeakFinderPane, rmv::kPaneIdCompareMemoryLeakFinder);
    SetupHotkeyNavAction(rmv::kGotoGeneralSettingsPane, rmv::kPaneIdSettingsGeneral);
    SetupHotkeyNavAction(rmv::kGotoThemesAndColorsPane, rmv::kPaneIdSettingsThemesAndColors);
    SetupHotkeyNavAction(rmv::kGotoKeyboardShortcutsPane, rmv::kPaneIdSettingsKeyboardShortcuts);

    // NOTE: Actions here pass in the parent widget when constructed. It is the responsibility of the parent widget
    // to make sure they are deleted.

    // Set up forward/backward navigation.
    QAction* shortcut = new QAction(this);
    shortcut->setShortcut(QKeySequence(Qt::ALT | rmv::kKeyNavForwardArrow));
    navigation_actions_.push_back(shortcut);

    connect(shortcut, &QAction::triggered, &rmv::NavigationManager::Get(), &rmv::NavigationManager::NavigateForward);
    this->addAction(shortcut);

    shortcut = new QAction(this);
    shortcut->setShortcut(QKeySequence(Qt::ALT | rmv::kKeyNavBackwardArrow));
    navigation_actions_.push_back(shortcut);

    connect(shortcut, &QAction::triggered, &rmv::NavigationManager::Get(), &rmv::NavigationManager::NavigateBack);
    this->addAction(shortcut);

    shortcut = new QAction(this);
    shortcut->setShortcut(rmv::kKeyNavBackwardBackspace);
    navigation_actions_.push_back(shortcut);

    connect(shortcut, &QAction::triggered, &rmv::NavigationManager::Get(), &rmv::NavigationManager::NavigateBack);
    this->addAction(shortcut);

    // Set up time unit cycling.
    shortcut = new QAction(this);
    shortcut->setShortcut(Qt::CTRL | Qt::Key_T);
    navigation_actions_.push_back(shortcut);

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
    exit_action_->setShortcut(Qt::ALT | Qt::Key_F4);
    connect(exit_action_, &QAction::triggered, this, &MainWindow::CloseRmv);

    for (int i = 0; i < kMaxSubmenuSnapshots; i++)
    {
        recent_trace_actions_.push_back(new QAction("", this));
        recent_trace_connections_.push_back(QMetaObject::Connection());
    }

    help_action_ = new QAction(tr("Help"), this);
    help_action_->setShortcut(Qt::CTRL | Qt::Key_F1);
    connect(help_action_, &QAction::triggered, this, &MainWindow::OpenHelp);

    about_action_ = new QAction(tr("About Radeon Memory Visualizer"), this);
    about_action_->setShortcut(Qt::CTRL | Qt::Key_F2);
    connect(about_action_, &QAction::triggered, this, &MainWindow::OpenAboutPane);
}

void MainWindow::EnableActions(const bool enable)
{
    close_trace_action_->setEnabled(enable);
    for (auto* action : actions())
    {
        action->setEnabled(enable);
    }
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
    const QVector<RecentFileData>& files = rmv::RMVSettings::Get().RecentFiles();

    for (int i = 0; i < recent_trace_connections_.size(); i++)
    {
        disconnect(recent_trace_connections_[i]);
    }

    recent_traces_menu_->clear();

    const int num_items = (files.size() > kMaxSubmenuSnapshots) ? kMaxSubmenuSnapshots : files.size();

    if (num_items == 0)
    {
        recent_traces_menu_->setEnabled(false);
    }
    else
    {
        recent_traces_menu_->setEnabled(true);
        for (int i = 0; i < num_items; i++)
        {
            recent_trace_actions_[i]->setText(files[i].path);

            recent_traces_menu_->addAction(recent_trace_actions_[i]);

            recent_trace_connections_[i] = connect(recent_trace_actions_[i], &QAction::triggered, [=]() { LoadTrace(files[i].path); });
        }
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

    help_menu_ = menuBar()->addMenu(tr("Help"));
    help_menu_->addAction(help_action_);
    help_menu_->addAction(about_action_);

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

    rmv::TraceManager& trace_manager = rmv::TraceManager::Get();
    if (trace_manager.DataSetValid())
    {
        const RmtDataSet* data_set = trace_manager.GetDataSet();
        if (data_set->driver_overrides_json_text != nullptr)
        {
            DriverOverridesModel::GetInstance()->ImportFromJsonText(RmtDataSetGetDriverOverridesString(data_set));
        }
        else
        {
            // Remove any old settings from the Driver Overrides model.
            DriverOverridesModel::GetInstance()->Reset();
        }
    }
}

void MainWindow::OpenDriverOverridesDetailsLink()
{
    // Open the driver experiments details link.
    ViewPane(rmv::kPaneIdTimelineDeviceConfiguration);
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

    // Remove any old settings from the Driver Overrides model.
    DriverOverridesModel::GetInstance()->Reset();
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
            const uint32_t num_urls = event->mimeData()->urls().size();

            for (uint32_t i = 0; i < num_urls; i++)
            {
                const QString potential_trace_path = event->mimeData()->urls().at(i).toLocalFile();

                // Check if the file is valid while dragging it into the window to show the symbol of interdiction for invalid files.
                if (rmv_util::TraceValidToLoad(potential_trace_path) == true)
                {
                    event->setDropAction(Qt::LinkAction);
                    event->accept();
                }
            }
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

            if (rmv_util::TraceValidToLoad(potential_trace_path) == true)
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
        ui_->snapshot_combo_box_->ClearItems();

        int32_t selected_snapshot_point_index = 0;
        for (int32_t current_snapshot_point_index = 0; current_snapshot_point_index < RmtTraceLoaderGetSnapshotCount(); ++current_snapshot_point_index)
        {
            const RmtSnapshotPoint* current_snapshot_point = RmtTraceLoaderGetSnapshotPoint(current_snapshot_point_index);
            QString name_string = QString(current_snapshot_point->name) + " (" + rmv::time_util::ClockToTimeUnit(current_snapshot_point->timestamp) + ")";
            if (current_snapshot_point == selected_snapshot_point)
            {
                selected_snapshot_point_index = current_snapshot_point_index;
            }
            ui_->snapshot_combo_box_->AddItem(name_string, (qulonglong)current_snapshot_point);
        }

        if (RmtTraceLoaderGetSnapshotCount() > 1)
        {
            ui_->main_tab_widget_->setTabEnabled(rmv::kMainPaneSnapshot, true);
            ui_->main_tab_widget_->setTabEnabled(rmv::kMainPaneCompare, true);
        }

        // Search the list of items in the combo box that has the open snapshot pointer.
        ui_->snapshot_combo_box_->blockSignals(true);
        ui_->snapshot_combo_box_->SetSelectedRow(selected_snapshot_point_index);
        ui_->snapshot_combo_box_->blockSignals(false);
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
    RMT_ASSERT(snapshot != nullptr);
    if (snapshot != nullptr)
    {
        pane_manager_.OpenSnapshot(snapshot);
        ui_->snapshot_start_stack_->setCurrentIndex(rmv::kSnapshotIndexPopulatedPane);

        UpdateTitlebar();
        UpdateSnapshotCombobox(snapshot->snapshot_point);
        ResizeNavigationLists();

        qApp->restoreOverrideCursor();
    }
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

void MainWindow::DontShowDriverOverridesNotification()
{
    rmv::RMVSettings::Get().SetDriverOverridesAllowNotifications(false);
}
