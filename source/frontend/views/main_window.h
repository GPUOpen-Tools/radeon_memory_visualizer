//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RMV main window.
//=============================================================================

#ifndef RMV_VIEWS_MAIN_WINDOW_H_
#define RMV_VIEWS_MAIN_WINDOW_H_

#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QSignalMapper>

#include "qt_common/custom_widgets/file_loading_widget.h"
#include "qt_common/custom_widgets/navigation_bar.h"
#include "qt_common/custom_widgets/navigation_list_widget.h"

#include "rmt_data_set.h"
#include "rmt_resource_list.h"
#include "rmt_job_system.h"

#include "models/trace_manager.h"
#include "util/definitions.h"
#include "views/debug_window.h"
#include "views/pane_manager.h"

#include "views/start/welcome_pane.h"
#include "views/start/recent_traces_pane.h"

class TimelinePane;

class SnapshotDeltaPane;
class MemoryLeakFinderPane;
class AllocationDeltaPane;

class SettingsPane;

namespace Ui
{
    class MainWindow;
}

namespace rmv
{
    class ThreadController;
}  // namespace rmv

/// Support for RMV main window.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The window's parent.
    explicit MainWindow(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~MainWindow();

    /// Overridden window resize event. Handle what happens when a user resizes the
    /// main window.
    /// \param event the resize event object.
    virtual void resizeEvent(QResizeEvent* event);

    /// Overridden window move event. Handle what happens when a user moves the
    /// main window.
    /// \param event the move event object.
    virtual void moveEvent(QMoveEvent* event);

    /// Handle what happens when X button is pressed.
    /// \param event the close event.
    virtual void closeEvent(QCloseEvent* event);

    /// Reset any UI elements that need resetting when a new trace file is loaded.
    /// This include references to models or event indices that rely on backend
    /// data.
    void ResetUI();

    /// Initialize the job queue used globally throughout the application.
    static void InitializeJobQueue();

    /// Get the job queue.
    /// \return The job queue.
    static RmtJobQueue* GetJobQueue();

    /// Destroy the job queue.
    static void DestroyJobQueue();

    /// Called when an animation needs to be loaded onto a window.
    /// \param parent The parent window.
    /// \param height_offset The offset from the top of the parent widget.
    void StartAnimation(QWidget* parent, int height_offset);

    /// Called when trace file changed to stop animation.
    void StopAnimation();

    /// Called when trace file finished loading.
    void TraceLoadComplete();

public slots:
    /// Called when selecting a recent trace from the recent traces list.
    /// \param trace_file The trace file to load, including full path.
    void LoadTrace(const QString& trace_file);

    /// Close an RMV trace file.
    void CloseTrace();

    /// Open an RMV trace file.
    /// Present the user with a file selection dialog box and load the trace that
    /// the user chooses.
    void OpenTrace();

    /// Open recent traces pane.
    void OpenRecentTracesPane();

    /// Populate recent files menu/list.
    void SetupRecentTracesMenu();

private slots:
    /// Open RMV help file.
    /// Present the user with help regarding RMV.
    void OpenHelp();

    /// Display RMV about information.
    /// Present the user with information about RMV.
    void OpenAboutPane();

    /// Open a snapshot.
    /// \param snapshot_point The snapshot to open.
    void OpenSnapshot(RmtSnapshotPoint* snapshot_point);

    /// Open 2 snapshots for comparison.
    /// \param snapshot_base The base snapshot for comparison.
    /// \param snapshot_diff The snapshot to compare against the base snapshot..
    void CompareSnapshot(RmtSnapshotPoint* snapshot_base, RmtSnapshotPoint* snapshot_diff);

    /// Open currently selected snapshot from the combo box below the snapshot tab
    /// menu.
    void OpenSelectedSnapshot();

    /// Cycle the time units and update the main window to ensure everything is
    /// redrawn to reflect the new time units.
    void CycleTimeUnits();

    /// Update the title bar based on app state.
    void UpdateTitlebar();

    /// Update all comparison panes.
    void UpdateCompares();

    /// Update UI coloring on all panes. Also manually update coloring on the start panes
    /// since these are pre-generated in the UI file and not created dynamically
    void BroadcastChangeColoring();

    /// Let all panes know they should reset themselves.
    void BroadcastReset();

    /// Let all panes know they should update themselves.
    void BroadcastOnTraceClose();

    /// Resize UI elements when the DPI scale factor changes.
    void OnScaleFactorChanged();

    /// Handle what happens when an item in the start tab list is clicked on.
    /// \param row The row in the list selected.
    void UpdateStartListRow(const int row);

    /// Handle what happens when an item in the timeline tab list is clicked on.
    /// \param row The row in the list selected.
    void UpdateTimelineListRow(const int row);

    /// Handle what happens when an item in the snapshot tab list is clicked on.
    /// \param row The row in the list selected.
    void UpdateSnapshotListRow(const int row);

    /// Handle what happens when an item in the compare tab list is clicked on.
    /// \param row The row in the list selected.
    void UpdateCompareListRow(const int row);

    /// Handle what happens when an item in the settings tab list is clicked on.
    /// \param row The row in the list selected.
    void UpdateSettingsListRow(const int row);

    /// Handle what happens when an item on the tab bar is clicked on.
    /// \param row The row in the list selected.
    void UpdateMainTabIndex(const int row);

    /// Navigate to a specific pane.
    /// \param pane The pane to jump to.
    void ViewPane(int pane);

private:
    /// Let all panes know a pane switch happened.
    void BroadcastPaneSwitched();

    /// Handle what happens when RMV is closed.
    void CloseRmv();

    /// Setup navigation bar on the top.
    void SetupTabBar();

    /// Create re-usable actions for our menu bar and possibly other widgets.
    void CreateActions();

    /// Fill in the menu bar with items.
    void CreateMenus();

    /// Update the snapshots combo box on the main snapshots tab.
    /// \param selected_snapshot_point The snapshot point selected that should be shown
    /// in the combo box.
    void UpdateSnapshotCombobox(RmtSnapshotPoint* selected_snapshot_point);

    /// Resize NavigationLists across several tabs of the MainWindow so that they have a consistent
    /// width. The widest width will either be defined by a text item in the navigation list, or
    /// by the text or ArrowIconComboBox at the bottom of the snapshots tab. Since these snapshots are
    /// saved in the trace files, loading a new trace file may also load a snapshot name which is wider
    /// than the any of the navigation item names, so this method should be called whenever a trace is
    /// loaded or when the DPI scale is changed.
    void ResizeNavigationLists();

    /// Setup main/debug window sizes and locations.
    /// \param loaded_settings The bool to indicate if settings should be loaded.
    void SetupWindowRects(bool loaded_settings);

    /// Setup mapping for keyboard binds.
    /// \param mapper signal mapper.
    /// \param key pressed key.
    /// \param pane target pane.
    void SetupHotkeyNavAction(QSignalMapper* mapper, int key, int pane);

    /// Handle a drag enter event.
    /// \param event drag enter event.
    void dragEnterEvent(QDragEnterEvent* event);

    /// Handle a drag-n-drop event.
    /// \param event drop event.
    void dropEvent(QDropEvent* event);

    /// Setup the next pane and navigate to it.
    /// \param pane The pane to jump to.
    rmv::RMVPane SetupNextPane(rmv::RMVPane pane);

    /// Construct title bar content.
    QString GetTitleBarString();

    /// Template function to create a new pane of a certain type
    /// Also saves the base pointer to an array for the pane broadcaster
    template <class PaneType>
    PaneType* CreatePane()
    {
        PaneType* pane = new PaneType(this);
        pane_manager_.AddPane(pane);
        return pane;
    }

    Ui::MainWindow* ui_;  ///< Pointer to the Qt UI design.

#ifdef RMV_DEBUG_WINDOW
    DebugWindow debug_window_;  ///< A supplemental debug window to output custom messages.
#endif

    QMenu*   file_menu_;           ///< File menu control.
    QAction* open_trace_action_;   ///< Action to open an RMV trace.
    QAction* close_trace_action_;  ///< Action to close an RMV trace.
    QAction* exit_action_;         ///< Action to exit RMV.
    QAction* help_action_;         ///< Action to display help.
    QAction* about_action_;        ///< Action to display About Radeon Memory Visualizer.

    QMenu* help_menu_;  ///< Help menu control

    QMenu*                  recent_traces_menu_;    ///< Sub menu containing recently opened files.
    QVector<QSignalMapper*> recent_trace_mappers_;  ///< Map signals to the recent traces when clicked on.
    QVector<QAction*>       recent_trace_actions_;  ///< List of actions for recent traces.

    FileLoadingWidget* file_load_animation_;  ///< Widget to show animation.

    WelcomePane*      welcome_pane_;        ///< Pointer to welcome pane.
    RecentTracesPane* recent_traces_pane_;  ///< Pointer to recent traces pane.
    TimelinePane*     timeline_pane_;       ///< Pointer to timeline pane.

    SnapshotDeltaPane*    snapshot_delta_pane_;      ///< Pointer to snapshot delta pane.
    MemoryLeakFinderPane* memory_leak_finder_pane_;  ///< Pointer to memory leak finder pane.
    SettingsPane*         settings_pane_;            ///< Pointer to settings pane.
    BasePane*             snapshot_start_pane_;      ///< Pointer to snapshot start pane.
    BasePane*             compare_start_pane_;       ///< Pointer to compare start pane.

    NavigationBar    navigation_bar_;  ///< The Back/Forward Navigation buttons added to the main tab bar.
    rmv::PaneManager pane_manager_;    ///< The class responsible for managing the relationships between different panes.

    static RmtJobQueue job_queue_;  ///< The job queue;
};

#endif  // RMV_VIEWS_MAIN_WINDOW_H_
