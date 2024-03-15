//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the main window.
//=============================================================================

#ifndef RMV_VIEWS_MAIN_WINDOW_H_
#define RMV_VIEWS_MAIN_WINDOW_H_

#include <QMainWindow>
#include <QAction>
#include <QMenu>

#include "ui_main_window.h"

#include "qt_common/custom_widgets/navigation_bar.h"
#include "qt_common/custom_widgets/navigation_list_widget.h"

#include "rmt_resource_list.h"

#include "managers/pane_manager.h"
#include "util/definitions.h"
#include "views/debug_window.h"
#include "views/snapshot/resource_details_pane.h"
#include "views/timeline/timeline_pane.h"

/// @brief Support for the main window.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The window's parent.
    explicit MainWindow(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~MainWindow();

    /// @brief Overridden window resize event.
    ///
    /// Handle what happens when a user resizes the main window.
    ///
    /// @param [in] event The resize event object.
    virtual void resizeEvent(QResizeEvent* event);

    /// @brief Overridden window move event.
    ///
    /// Handle what happens when a user moves the main window.
    ///
    /// @param [in] event The move event object.
    virtual void moveEvent(QMoveEvent* event);

    /// @brief Handle what happens when X button is pressed.
    ///
    /// @param [in] event The close event.
    virtual void closeEvent(QCloseEvent* event);

    /// @brief Reset any UI elements that need resetting when a new trace file is loaded.
    ///
    /// This include references to models or event indices that rely on backend data.
    void ResetUI();

    /// @brief Enable or disable shortcut keys.
    ///
    /// @param [in] enable                          If true, enables actions, otherwise disables actions.
    void EnableActions(const bool enable);

public slots:
    /// @brief Called when trace file finished loading.
    void OpenTrace();

    /// @brief Close an RMV trace file.
    void CloseTrace();

    /// @brief Open an RMV trace file via the file menu.
    ///
    /// Present the user with a file selection dialog box and load the trace that
    /// the user chooses.
    void OpenTraceFromFileMenu();

    /// @brief Populate recent files menu/list.
    void SetupRecentTracesMenu();

private slots:
    /// @brief Called when selecting a recent trace from the recent traces list.
    ///
    /// @param [in] trace_file The trace file to load, including full path.
    void LoadTrace(const QString& trace_file);

    /// @brief Open RMV help file.
    ///
    /// Present the user with help regarding RMV.
    void OpenHelp();

    /// @brief Display the about information.
    ///
    /// Present the user with information about RMV.
    void OpenAboutPane();

    /// @brief Called when the user requests loading of a snapshot.
    ///
    /// @param [in] resource_identifier The resource selected when opening the snapshot, for the
    ///  case of selecting a resource from the memory leak pane to see its details.
    void OpenSnapshotPane(RmtResourceIdentifier resource_identifier = 0);

    /// @brief Called when the user requests comparison of 2 snapshots.
    void OpenComparePane();

    /// @brief Show the loaded snapshot. Finalize the UI and flip to the SNAPSHOT tab.
    void ShowSnapshotPane();

    /// @brief Show the comparison snapshots. Finalize the UI and flip to the COMPARE tab.
    void ShowComparePane();

    /// @brief Open currently selected snapshot from the combo box below the snapshot tab menu.
    void OpenSelectedSnapshot();

    /// @brief Cycle the time units and update the main window to ensure everything is
    /// redrawn to reflect the new time units.
    void CycleTimeUnits();

    /// @brief Update the title bar based on app state.
    void UpdateTitlebar();

    /// @brief Update UI coloring on all panes.
    ///
    /// Also manually update coloring on the start panes since these are pre-generated in
    /// the UI file and not created dynamically.
    void BroadcastChangeColoring();

    /// @brief Resize UI elements when the DPI scale factor changes.
    void OnScaleFactorChanged();

    /// @brief Handle what happens when an item in the start tab list is clicked on.
    ///
    /// @param [in] row The row in the list selected.
    void UpdateStartListRow(const int row);

    /// @brief Handle what happens when an item in the timeline tab list is clicked on.
    ///
    /// @param [in] row The row in the list selected.
    void UpdateTimelineListRow(const int row);

    /// @brief Handle what happens when an item in the snapshot tab list is clicked on.
    ///
    /// @param [in] row The row in the list selected.
    void UpdateSnapshotListRow(const int row);

    /// @brief Handle what happens when an item in the compare tab list is clicked on.
    ///
    /// @param [in] row The row in the list selected.
    void UpdateCompareListRow(const int row);

    /// @brief Handle what happens when an item in the settings tab list is clicked on.
    ///
    /// @param [in] row The row in the list selected.
    void UpdateSettingsListRow(const int row);

    /// @brief Handle what happens when an item on the tab bar is clicked on.
    ///
    /// @param [in] tab_index The tab index.
    void UpdateMainTabIndex(const int tab_index);

    /// @brief Navigate to a specific pane.
    ///
    /// @param [in] pane The pane to jump to.
    void ViewPane(int pane);

private:
    /// @brief Open the snapshot the user selected in the UI.
    void OpenSnapshot();

    /// @brief Open the 2 selected snapshots for comparison.
    void OpenCompareSnapshots();

    /// @brief Handle what happens when RMV is closed.
    void CloseRmv();

    /// @brief Setup navigation bar on the top.
    void SetupTabBar();

    /// @brief Create re-usable actions for our menu bar and possibly other widgets.
    void CreateActions();

    /// @brief Fill in the menu bar with items.
    void CreateMenus();

    /// @brief Update the snapshots combo box on the main snapshots tab.
    ///
    /// @param [in] selected_snapshot_point The snapshot point selected that should be shown
    ///  in the combo box.
    void UpdateSnapshotCombobox(RmtSnapshotPoint* selected_snapshot_point);

    /// @brief Resize NavigationLists across several tabs of the MainWindow so that they have a consistent width.
    ///
    /// The widest width will either be defined by a text item in the navigation list, or by the text or
    /// ArrowIconComboBox at the bottom of the snapshots tab. Since these snapshots are saved in the trace
    /// files, loading a new trace file may also load a snapshot name which is wider than the any of the
    /// navigation item names, so this method should be called whenever a trace is loaded or when the DPI
    /// scale is changed.
    void ResizeNavigationLists();

    /// @brief Setup main/debug window sizes and locations.
    ///
    /// @param [in] loaded_settings The bool to indicate if settings should be loaded.
    void SetupWindowRects(bool loaded_settings);

    /// @brief Setup mapping for keyboard binds.
    ///
    /// @param [in] key    The pressed key.
    /// @param [in] pane   The target pane.
    void SetupHotkeyNavAction(int key, int pane);

    /// @brief Handle a drag enter event.
    ///
    /// @param [in] event The drag enter event.
    void dragEnterEvent(QDragEnterEvent* event);

    /// @brief Handle a drag-n-drop event.
    ///
    /// @param [in] event The drop event.
    void dropEvent(QDropEvent* event);

    /// @brief Setup the next pane and navigate to it.
    ///
    /// @param [in] pane The Id of the pane to jump to.
    ///
    /// @return The Id of the pane to navigate to.
    rmv::RMVPaneId SetupNextPane(rmv::RMVPaneId pane);

    /// @brief Construct title bar content.
    QString GetTitleBarString() const;

    /// @brief Template function to add a pane to the pane manager and its parent stacked widget.
    ///
    /// @param [in] pane         The pane to add.
    /// @param [in] widget_stack The stacked widget to add the pane to.
    template <class PaneType>
    void AddPane(PaneType* pane, QStackedWidget* widget_stack)
    {
        pane_manager_.AddPane(pane);
        widget_stack->addWidget(pane);
    }

    /// @brief Template function to create a new pane of a certain type.
    ///
    /// @param [in] widget_stack The stacked widget to add the created pane to.
    ///
    /// @return Pointer to the newly created pane.
    template <class PaneType>
    PaneType* CreatePane(QStackedWidget* widget_stack)
    {
        PaneType* pane = new PaneType(this);
        AddPane(pane, widget_stack);
        return pane;
    }

    /// @brief Template function to create compare pane.
    ///
    /// @param [in] widget_stack The stacked widget to add the created pane to.
    ///
    /// @return Pointer to the newly created pane.
    template <class PaneType>
    PaneType* CreateComparePane(QStackedWidget* widget_stack)
    {
        PaneType* pane = new PaneType(this);
        pane_manager_.AddComparePane(pane);
        AddPane(pane, widget_stack);
        return pane;
    }

    Ui::MainWindow* ui_;  ///< Pointer to the Qt UI design.

#ifdef RMV_DEBUG_WINDOW
    DebugWindow debug_window_;  ///< A supplemental debug window to output custom messages.
#endif

    QMenu*   file_menu_;           ///< File menu control.
    QAction* open_trace_action_;   ///< Action to open a trace file.
    QAction* close_trace_action_;  ///< Action to close a trace file.
    QAction* exit_action_;         ///< Action to exit RMV.
    QAction* help_action_;         ///< Action to display help.
    QAction* about_action_;        ///< Action to display About Radeon Memory Visualizer.

    QMenu* help_menu_;  ///< Help menu control

    QMenu*                           recent_traces_menu_;        ///< Sub menu containing recently opened files.
    QVector<QAction*>                recent_trace_actions_;      ///< List of actions for recent traces.
    QVector<QMetaObject::Connection> recent_trace_connections_;  ///< List of previously connected signals/slots.

    TimelinePane*        timeline_pane_;          ///< Pointer to timeline pane.
    ResourceDetailsPane* resource_details_pane_;  ///< Pointer to resource details pane.

    NavigationBar    navigation_bar_;  ///< The Back/Forward Navigation buttons added to the main tab bar.
    rmv::PaneManager pane_manager_;    ///< The class responsible for managing the relationships between different panes.
};

#endif  // RMV_VIEWS_MAIN_WINDOW_H_
