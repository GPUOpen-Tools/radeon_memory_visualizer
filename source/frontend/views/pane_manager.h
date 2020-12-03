//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the pane broadcaster.
/// This class is responsible for sending event messages to all panes.
//=============================================================================

#ifndef RMV_VIEWS_PANE_MANAGER_H_
#define RMV_VIEWS_PANE_MANAGER_H_

#include <vector>
#include <QWidget>

#include "rmt_data_set.h"

#include "base_pane.h"

/// An enum of all the elements in the tab menu.
enum MainPanes
{
    kMainPaneNavigation,
    kMainPaneStart,
    kMainPaneTimeline,
    kMainPaneSnapshot,
    kMainPaneCompare,
    kMainPaneSpacer,
    kMainPaneSettings,

    kMainPaneCount,
};

/// Used to control and track user navigation.
struct NavLocation
{
    int32_t main_tab_index;     ///< Main tab index.
    int32_t start_list_row;     ///< Start list row.
    int32_t timeline_list_row;  ///< Timeline list row.
    int32_t snapshot_list_row;  ///< Snapshot list row.
    int32_t compare_list_row;   ///< Compare list row.
    int32_t settings_list_row;  ///< Settings list row.
};

namespace rmv
{
    /// An enum of all the panes in RMV.
    enum RMVPane
    {
        kPaneStartWelcome,
        kPaneStartRecentTraces,
        kPaneStartAbout,
        kPaneTimelineGenerateSnapshot,
        kPaneTimelineDeviceConfiguration,
        kPaneSnapshotHeapOverview,
        kPaneSnapshotResourceOverview,
        kPaneSnapshotAllocationOverview,
        kPaneSnapshotResourceList,
        kPaneSnapshotAllocationExplorer,
        kPaneSnapshotResourceDetails,
        kPaneCompareSnapshotDelta,
        kPaneCompareMemoryLeakFinder,
        kPaneSettingsGeneral,
        kPaneSettingsThemesAndColors,
        kPaneSettingsKeyboardShortcuts,
    };

    /// Hotkeys.
    static const int kGotoGenerateSnapshotPane    = Qt::Key_F;
    static const int kGotoDeviceConfigurationPane = Qt::Key_G;
    static const int kGotoHeapOverviewPane        = Qt::Key_Q;
    static const int kGotoResourceOverviewPane    = Qt::Key_W;
    static const int kGotoAllocationOverviewPane  = Qt::Key_E;
    static const int kGotoResourceListPane        = Qt::Key_R;
    static const int kGotoAllocationExplorerPane  = Qt::Key_T;
    static const int kGotoResourceHistoryPane     = Qt::Key_Y;
    static const int kGotoSnapshotDeltaPane       = Qt::Key_A;
    static const int kGotoMemoryLeakFinderPane    = Qt::Key_S;
    static const int kGotoWelcomePane             = Qt::Key_Z;
    static const int kGotoRecentSnapshotsPane     = Qt::Key_X;
    static const int kGotoKeyboardShortcutsPane   = Qt::Key_C;
    static const int kKeyNavBackwardBackspace     = Qt::Key_Backspace;
    static const int kKeyNavBackwardArrow         = Qt::Key_Left;
    static const int kKeyNavForwardArrow          = Qt::Key_Right;
    static const int kKeyNavUpArrow               = Qt::Key_Up;
    static const int kKeyNavDownArrow             = Qt::Key_Down;

    /// Class to manage the panes and navigating betweem them.
    class PaneManager : public QObject
    {
        Q_OBJECT

    public:
        /// Constructor.
        PaneManager();

        /// Destructor.
        ~PaneManager();

        /// Take our navigation locations to starting state.
        /// \return The reset Navigation location.
        const NavLocation& ResetNavigation();

        /// Navigate to a specific pane.
        /// \param pane the pane to jump to.
        /// \return The Navigation location.
        const NavLocation& SetupNextPane(int pane);

        /// Work out current pane from app state.
        /// Called every time there's a pane switch.
        /// \return The new current pane.
        RMVPane UpdateCurrentPane();

        /// Store main tab index and update current pane.
        /// \param idx tab index.
        /// \return true if the snapshot pane was selected, false otherwise.
        bool UpdateMainTabIndex(int idx);

        /// Get the main pane group from the pane.
        /// \param pane The pane to check.
        /// \return The main pane.
        MainPanes GetMainPaneFromPane(RMVPane pane);

        /// Get the current pane.
        /// \return The current pane.
        RMVPane GetCurrentPane();

        /// Get the previous pane.
        /// \return The previous pane.
        RMVPane GetPreviousPane();

        /// Add a pane to the group.
        /// \param pane The pane to add.
        void AddPane(BasePane* pane);

        /// Call OnTraceClose() for all panes.
        void OnTraceClose();

        /// Call PaneSwitched() for all panes.
        void PaneSwitched();

        /// Call Reset() for all panes.
        void Reset();

        /// Call ChangeColoring() for all panes.
        void ChangeColoring();

        /// Call OpenSnapshot() for all panes.
        /// \param snapshot The snapshot to open.
        void OpenSnapshot(RmtDataSnapshot* snapshot);

        /// Call SwitchTimeUnits() for all panes.
        void SwitchTimeUnits();

    public slots:
        /// Store start list row and update current pane.
        /// \param row The tab index.
        void UpdateStartListRow(const int row);

        /// Store timeline list row and update current pane.
        /// \param row The row index.
        void UpdateTimelineListRow(const int row);

        /// Store snapshot list row and update current pane.
        /// \param row The tab index.
        void UpdateSnapshotListRow(const int row);

        /// Store compare list row and update current pane.
        /// \param row The tab index.
        void UpdateCompareListRow(const int row);

        /// Store settings list row and update current pane.
        /// \param row The tab index.
        void UpdateSettingsListRow(const int row);

    private:
        NavLocation            nav_location_;   ///< Track current list and tab locations.
        RMVPane                current_pane_;   ///< Track current pane that is open.
        RMVPane                previous_pane_;  ///< Track previous pane that was open.
        std::vector<BasePane*> panes_;          ///< The group of panes to send messages to.
    };
}  // namespace rmv

#endif  // RMV_VIEWS_PANE_MANAGER_H_
