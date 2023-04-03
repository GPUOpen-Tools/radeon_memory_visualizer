//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the pane manager.
//=============================================================================

#ifndef RMV_MANAGERS_PANE_MANAGER_H_
#define RMV_MANAGERS_PANE_MANAGER_H_

#include <vector>

#include "rmt_data_snapshot.h"

#include "views/base_pane.h"
#include "views/compare_pane.h"

namespace rmv
{
    /// @brief An enum of all the elements in the tab menu.
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

    /// @brief An enum of all the panes in the start menu.
    enum StartPanes
    {
        kStartPaneWelcome,
        kStartPaneRecentTraces,
        kStartPaneAbout,

        kStartPaneCount,
    };

    /// @brief An enum of all the panes in the timeline menu.
    enum TimelinePanes
    {
        kTimelinePaneGenerateSnapshot,
        kTimelinePaneDeviceConfiguration,

        kTimelinePaneCount,
    };

    /// @brief An enum of all the panes in the snapshots menu.
    enum SnapshotPanes
    {
        kSnapshotPaneHeapOverview,
        kSnapshotPaneResourceOverview,
        kSnapshotPaneAllocationOverview,
        kSnapshotPaneResourceList,
        kSnapshotPaneAllocationExplorer,
        kSnapshotPaneResourceDetails,

        kSnapshotPaneCount,
    };

    /// @brief An enum of all the panes in the compare menu.
    enum ComparePanes
    {
        kComparePaneSnapshotDelta,
        kComparePaneMemoryLeakFinder,

        kComparePaneCount,
    };

    /// @brief An enum of all the panes in the settings menu.
    enum SettingsPanes
    {
        kSettingsPaneGeneral,
        kSettingsPaneThemesAndColors,
        kSettingsPaneKeyboardShortcuts,

        kSettingsPaneCount,
    };

    /// @brief Used to control and track user navigation.
    struct NavLocation
    {
        int32_t main_tab_index;     ///< Main tab index.
        int32_t start_list_row;     ///< Start list row.
        int32_t timeline_list_row;  ///< Timeline list row.
        int32_t snapshot_list_row;  ///< Snapshot list row.
        int32_t compare_list_row;   ///< Compare list row.
        int32_t settings_list_row;  ///< Settings list row.
    };

    static const int32_t kPaneShift = 16;
    static const int32_t kPaneMask  = 0xffff;

    /// @brief An enum of all the panes in RMV.
    ///
    /// The Pane ID is constructed from the main tab index and its index within its sub-tab or sub-menu.
    /// The main tab index is encoded in the top 16 bits of the Pane ID, and the lower 16 bits are used
    /// for the sub-menu index.
    /// This makes converting between Pane ID and main tab/menu index trivial.
    enum RMVPaneId
    {
        kPaneIdStartWelcome                = (kMainPaneStart << kPaneShift) | kStartPaneWelcome,
        kPaneIdStartRecentTraces           = (kMainPaneStart << kPaneShift) | kStartPaneRecentTraces,
        kPaneIdStartAbout                  = (kMainPaneStart << kPaneShift) | kStartPaneAbout,
        kPaneIdTimelineGenerateSnapshot    = (kMainPaneTimeline << kPaneShift) | kTimelinePaneGenerateSnapshot,
        kPaneIdTimelineDeviceConfiguration = (kMainPaneTimeline << kPaneShift) | kTimelinePaneDeviceConfiguration,
        kPaneIdSnapshotHeapOverview        = (kMainPaneSnapshot << kPaneShift) | kSnapshotPaneHeapOverview,
        kPaneIdSnapshotResourceOverview    = (kMainPaneSnapshot << kPaneShift) | kSnapshotPaneResourceOverview,
        kPaneIdSnapshotAllocationOverview  = (kMainPaneSnapshot << kPaneShift) | kSnapshotPaneAllocationOverview,
        kPaneIdSnapshotResourceList        = (kMainPaneSnapshot << kPaneShift) | kSnapshotPaneResourceList,
        kPaneIdSnapshotAllocationExplorer  = (kMainPaneSnapshot << kPaneShift) | kSnapshotPaneAllocationExplorer,
        kPaneIdSnapshotResourceDetails     = (kMainPaneSnapshot << kPaneShift) | kSnapshotPaneResourceDetails,
        kPaneIdCompareSnapshotDelta        = (kMainPaneCompare << kPaneShift) | kComparePaneSnapshotDelta,
        kPaneIdCompareMemoryLeakFinder     = (kMainPaneCompare << kPaneShift) | kComparePaneMemoryLeakFinder,
        kPaneIdSettingsGeneral             = (kMainPaneSettings << kPaneShift) | kSettingsPaneGeneral,
        kPaneIdSettingsThemesAndColors     = (kMainPaneSettings << kPaneShift) | kSettingsPaneThemesAndColors,
        kPaneIdSettingsKeyboardShortcuts   = (kMainPaneSettings << kPaneShift) | kSettingsPaneKeyboardShortcuts,
    };

    /// @brief Hotkeys.
    static const int kGotoGenerateSnapshotPane    = Qt::Key_F;
    static const int kGotoDeviceConfigurationPane = Qt::Key_G;
    static const int kGotoHeapOverviewPane        = Qt::Key_Q;
    static const int kGotoResourceOverviewPane    = Qt::Key_W;
    static const int kGotoAllocationOverviewPane  = Qt::Key_E;
    static const int kGotoResourceListPane        = Qt::Key_T;
    static const int kGotoAllocationExplorerPane  = Qt::Key_Y;
    static const int kGotoResourceHistoryPane     = Qt::Key_U;
    static const int kGotoSnapshotDeltaPane       = Qt::Key_A;
    static const int kGotoMemoryLeakFinderPane    = Qt::Key_S;
    static const int kGotoWelcomePane             = Qt::Key_X;
    static const int kGotoRecentSnapshotsPane     = Qt::Key_C;
    static const int kGotoAboutPane               = Qt::Key_V;
    static const int kGotoGeneralSettingsPane     = Qt::Key_B;
    static const int kGotoThemesAndColorsPane     = Qt::Key_N;
    static const int kGotoKeyboardShortcutsPane   = Qt::Key_M;
    static const int kKeyNavBackwardBackspace     = Qt::Key_Backspace;
    static const int kKeyNavBackwardArrow         = Qt::Key_Left;
    static const int kKeyNavForwardArrow          = Qt::Key_Right;
    static const int kKeyNavUpArrow               = Qt::Key_Up;
    static const int kKeyNavDownArrow             = Qt::Key_Down;

    /// @brief Class to manage the panes and navigating between them.
    class PaneManager
    {
    public:
        /// @brief Constructor.
        PaneManager();

        /// @brief Destructor.
        ~PaneManager();

        /// @brief Take our navigation locations to starting state.
        ///
        /// @return The reset Navigation location.
        const NavLocation& ResetNavigation();

        /// @brief Navigate to a specific pane.
        ///
        /// @param [in] pane the pane to jump to.
        ///
        /// @return The Navigation location.
        const NavLocation* SetupNextPane(RMVPaneId pane);

        /// @brief Work out current pane from app state.
        ///
        /// Called every time there's a pane switch.
        ///
        /// @return The new current pane.
        RMVPaneId UpdateCurrentPane();

        /// @brief Was the SNAPSHOT tab clicked on?
        ///
        /// Snapshots are selected from the timeline and only loaded on a transition to a snapshot pane.
        /// It is up to the calling function to load the snapshot.
        ///
        /// @return true if loading is required, false if not.
        bool ClickedSnapshotTab() const;

        /// @brief Was the COMPARE tab clicked on?
        ///
        /// Snapshots are selected from the timeline and only loaded on a transition to a compare pane.
        /// It is up to the calling function to load the snapshot.
        ///
        /// @return true if loading is required, false if not.
        bool ClickedCompareTab() const;

        /// @brief Store main tab index and update current pane.
        ///
        /// @param [in] tab_index The tab index.
        void UpdateMainTabIndex(const int tab_index);

        /// @brief Store start list row and update current pane.
        ///
        /// @param [in] row The row index.
        void UpdateStartListRow(const int row);

        /// @brief Store timeline list row and update current pane.
        ///
        /// @param [in] row The row index.
        void UpdateTimelineListRow(const int row);

        /// @brief Store snapshot list row and update current pane.
        ///
        /// @param [in] row The row index.
        void UpdateSnapshotListRow(const int row);

        /// @brief Store compare list row and update current pane.
        ///
        /// @param row [in] The row index.
        void UpdateCompareListRow(const int row);

        /// @brief Store settings list row and update current pane.
        ///
        /// @param [in] row The row index.
        void UpdateSettingsListRow(const int row);

        /// @brief Get the main pane group from the pane.
        ///
        /// @param [in] pane The pane to check.
        ///
        /// @return The main pane.
        MainPanes GetMainPaneFromPane(RMVPaneId pane) const;

        /// @brief Get the current pane.
        ///
        /// @return The current pane.
        RMVPaneId GetCurrentPane() const;

        /// @brief Get the previous pane.
        ///
        /// @return The previous pane.
        RMVPaneId GetPreviousPane() const;

        /// @brief Add a pane to the group.
        ///
        /// @param [in] pane The pane to add.
        void AddPane(BasePane* pane);

        /// @brief Add a compare pane to the group.
        ///
        /// @param [in] pane The pane to add.
        void AddComparePane(ComparePane* pane);

        /// @brief Call OnTraceClose() for all panes.
        void OnTraceClose();

        /// @brief Call Reset() for all panes.
        void Reset();

        /// @brief Call ChangeColoring() for all panes.
        void ChangeColoring();

        /// @brief Call OpenSnapshot() for all panes.
        ///
        /// @param [in] snapshot The snapshot to open.
        void OpenSnapshot(RmtDataSnapshot* snapshot);

        /// @brief Call SwitchTimeUnits() for all panes.
        void SwitchTimeUnits();

        /// @brief Update all the compare panes.
        void UpdateCompares();

    private:
        NavLocation               nav_location_;   ///< Track current list and tab locations.
        RMVPaneId                 current_pane_;   ///< Track current pane that is open.
        RMVPaneId                 previous_pane_;  ///< Track previous pane that was open.
        std::vector<BasePane*>    panes_;          ///< The group of panes to send messages to.
        std::vector<ComparePane*> compare_panes_;  ///< The group of compare panes to send messages to.
    };
}  // namespace rmv

#endif  // RMV_MANAGERS_PANE_MANAGER_H_
