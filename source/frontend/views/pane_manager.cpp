//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation for the pane broadcaster.
/// This class is responsible for sending event messages to all panes.
//=============================================================================

#include "views/navigation_manager.h"
#include "views/pane_manager.h"

namespace rmv
{
    // An enum of all the panes in the start menu.
    enum StartPanes
    {
        kStartPaneWelcome,
        kStartPaneRecentTraces,
        kStartPaneAbout,

        kStartPaneCount,
    };

    // An enum of all the panes in the timeline menu.
    enum TimelinePanes
    {
        kTimelinePaneGenerateSnapshot,
        kTimelinePaneDeviceConfiguration,

        kTimelinePaneCount,
    };

    // An enum of all the panes in the snapshots menu.
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

    // An enum of all the panes in the compare menu.
    enum ComparePanes
    {
        kComparePaneSnapshotDelta,
        kComparePaneMemoryLeakFinder,

        kComparePaneCount,
    };

    // An enum of all the panes in the settings menu.
    enum SettingsPanes
    {
        kSettingsPaneGeneral,
        kSettingsPaneThemesAndColors,
        kSettingsPaneKeyboardShortcuts,

        kSettingsPaneCount,
    };

    static const int32_t kShift = 16;
    static const int32_t kMask  = 0xffff;

    // A map of pane ID to the pane components. The 2 components share an integer with each
    // component using 16 bits.
    static const std::map<RMVPane, int32_t> kPaneToComponentsMap = {
        {kPaneStartWelcome, (kMainPaneStart << kShift) | kStartPaneWelcome},
        {kPaneStartRecentTraces, (kMainPaneStart << kShift) | kStartPaneRecentTraces},
        {kPaneStartAbout, (kMainPaneStart << kShift) | kStartPaneAbout},

        {kPaneTimelineGenerateSnapshot, (kMainPaneTimeline << kShift) | kTimelinePaneGenerateSnapshot},
        {kPaneTimelineDeviceConfiguration, (kMainPaneTimeline << kShift) | kTimelinePaneDeviceConfiguration},

        {kPaneSnapshotHeapOverview, (kMainPaneSnapshot << kShift) | kSnapshotPaneHeapOverview},
        {kPaneSnapshotResourceOverview, (kMainPaneSnapshot << kShift) | kSnapshotPaneResourceOverview},
        {kPaneSnapshotAllocationOverview, (kMainPaneSnapshot << kShift) | kSnapshotPaneAllocationOverview},
        {kPaneSnapshotResourceList, (kMainPaneSnapshot << kShift) | kSnapshotPaneResourceList},
        {kPaneSnapshotResourceDetails, (kMainPaneSnapshot << kShift) | kSnapshotPaneResourceDetails},
        {kPaneSnapshotAllocationExplorer, (kMainPaneSnapshot << kShift) | kSnapshotPaneAllocationExplorer},

        {kPaneCompareSnapshotDelta, (kMainPaneCompare << kShift) | kComparePaneSnapshotDelta},
        {kPaneCompareMemoryLeakFinder, (kMainPaneCompare << kShift) | kComparePaneMemoryLeakFinder},

        {kPaneSettingsGeneral, (kMainPaneSettings << kShift) | kSettingsPaneGeneral},
        {kPaneSettingsThemesAndColors, (kMainPaneSettings << kShift) | kSettingsPaneThemesAndColors},
        {kPaneSettingsKeyboardShortcuts, (kMainPaneSettings << kShift) | kSettingsPaneKeyboardShortcuts},
    };

    // This is the inverse of the map above and is generated in code.
    static std::map<int32_t, RMVPane> components_to_pane_map;

    PaneManager::PaneManager()
        : nav_location_{}
        , current_pane_(kPaneStartWelcome)
        , previous_pane_(kPaneStartWelcome)
    {
        ResetNavigation();
        for (auto iter = kPaneToComponentsMap.begin(); iter != kPaneToComponentsMap.end(); ++iter)
        {
            components_to_pane_map.insert(std::make_pair((*iter).second, (*iter).first));
        }
    }

    PaneManager::~PaneManager()
    {
        for (auto it = panes_.begin(); it != panes_.end(); ++it)
        {
            if ((*it) != nullptr)
            {
                delete (*it);
            }
        }
    }

    const NavLocation& PaneManager::ResetNavigation()
    {
        nav_location_.main_tab_index    = kMainPaneStart;
        nav_location_.start_list_row    = kStartPaneWelcome;
        nav_location_.timeline_list_row = kTimelinePaneGenerateSnapshot;
        nav_location_.snapshot_list_row = kSnapshotPaneHeapOverview;
        nav_location_.compare_list_row  = kComparePaneSnapshotDelta;
        nav_location_.settings_list_row = kSettingsPaneGeneral;

        return nav_location_;
    }

    MainPanes PaneManager::GetMainPaneFromPane(RMVPane pane)
    {
        auto iter = kPaneToComponentsMap.find(static_cast<RMVPane>(pane));
        if (iter != kPaneToComponentsMap.end())
        {
            return static_cast<MainPanes>(iter->second >> kShift);
        }
        return kMainPaneStart;
    }

    RMVPane PaneManager::GetCurrentPane()
    {
        return current_pane_;
    }

    RMVPane PaneManager::GetPreviousPane()
    {
        return previous_pane_;
    }

    const NavLocation& PaneManager::SetupNextPane(int pane)
    {
        auto iter = kPaneToComponentsMap.find(static_cast<RMVPane>(pane));
        if (iter != kPaneToComponentsMap.end())
        {
            int32_t pane_info            = iter->second;
            int32_t main_tab_index       = pane_info >> kShift;
            int32_t list_row             = pane_info & kMask;
            nav_location_.main_tab_index = main_tab_index;

            switch (main_tab_index)
            {
            case kMainPaneStart:
                nav_location_.start_list_row = list_row;
                break;

            case kMainPaneTimeline:
                nav_location_.timeline_list_row = list_row;
                break;

            case kMainPaneSnapshot:
                nav_location_.snapshot_list_row = list_row;
                break;

            case kMainPaneCompare:
                nav_location_.compare_list_row = list_row;
                break;

            case kMainPaneSettings:
                nav_location_.settings_list_row = list_row;
                break;

            default:
                break;
            }
        }

        return nav_location_;
    }

    RMVPane PaneManager::UpdateCurrentPane()
    {
        // create the combined component
        int32_t component = (nav_location_.main_tab_index << kShift);

        switch (nav_location_.main_tab_index)
        {
        case kMainPaneStart:
            component |= nav_location_.start_list_row;
            break;

        case kMainPaneTimeline:
            component |= nav_location_.timeline_list_row;
            break;

        case kMainPaneSnapshot:
            component |= nav_location_.snapshot_list_row;
            break;

        case kMainPaneCompare:
            component |= nav_location_.compare_list_row;
            break;

        case kMainPaneSettings:
            component |= nav_location_.settings_list_row;
            break;

        default:
            break;
        }

        previous_pane_ = current_pane_;
        auto iter      = components_to_pane_map.find(component);
        if (iter != components_to_pane_map.end())
        {
            current_pane_ = (*iter).second;
        }

        NavigationManager::Get().UpdateCurrentPane(current_pane_);
        return current_pane_;
    }

    bool PaneManager::UpdateMainTabIndex(int idx)
    {
        bool result = false;

        if ((idx >= kMainPaneStart) && (idx < kMainPaneCount))
        {
            nav_location_.main_tab_index = idx;

            // if the snapshot pane is clicked on, make sure the currently viewed snapshot is selected
            // and ensure the combo box is populated correctly
            if (idx == kMainPaneSnapshot)
            {
                result = true;
            }

            UpdateCurrentPane();

            NavigationManager::Get().RecordNavigationEventPaneSwitch(current_pane_);
        }
        return result;
    }

    void PaneManager::UpdateStartListRow(const int row)
    {
        if (row < kStartPaneCount)
        {
            nav_location_.start_list_row = row;

            UpdateCurrentPane();

            NavigationManager::Get().RecordNavigationEventPaneSwitch(current_pane_);
        }
    }

    void PaneManager::UpdateTimelineListRow(const int row)
    {
        if (row < kTimelinePaneCount)
        {
            nav_location_.timeline_list_row = row;

            UpdateCurrentPane();

            NavigationManager::Get().RecordNavigationEventPaneSwitch(current_pane_);
        }
    }

    void PaneManager::UpdateSnapshotListRow(const int row)
    {
        if (row < kSnapshotPaneCount)
        {
            nav_location_.snapshot_list_row = row;

            UpdateCurrentPane();

            NavigationManager::Get().RecordNavigationEventPaneSwitch(current_pane_);
        }
    }

    void PaneManager::UpdateCompareListRow(const int row)
    {
        if (row < kComparePaneCount)
        {
            nav_location_.compare_list_row = row;

            UpdateCurrentPane();

            NavigationManager::Get().RecordNavigationEventPaneSwitch(current_pane_);
        }
    }

    void PaneManager::UpdateSettingsListRow(const int row)
    {
        if (row < kSettingsPaneCount)
        {
            nav_location_.settings_list_row = row;

            UpdateCurrentPane();

            NavigationManager::Get().RecordNavigationEventPaneSwitch(current_pane_);
        }
    }

    void PaneManager::AddPane(BasePane* pane)
    {
        panes_.push_back(pane);
    }

    void PaneManager::OnTraceClose()
    {
        for (auto it = panes_.begin(); it != panes_.end(); ++it)
        {
            if ((*it) != nullptr)
            {
                (*it)->OnTraceClose();
            }
        }
    }

    void PaneManager::PaneSwitched()
    {
        for (auto it = panes_.begin(); it != panes_.end(); ++it)
        {
            if ((*it) != nullptr)
            {
                (*it)->PaneSwitched();
            }
        }
    }

    void PaneManager::Reset()
    {
        for (auto it = panes_.begin(); it != panes_.end(); ++it)
        {
            if ((*it) != nullptr)
            {
                (*it)->Reset();
            }
        }
    }

    void PaneManager::ChangeColoring()
    {
        for (auto it = panes_.begin(); it != panes_.end(); ++it)
        {
            if ((*it) != nullptr)
            {
                (*it)->ChangeColoring();
            }
        }
    }

    void PaneManager::OpenSnapshot(RmtDataSnapshot* snapshot)
    {
        for (auto it = panes_.begin(); it != panes_.end(); ++it)
        {
            if ((*it) != nullptr)
            {
                (*it)->OpenSnapshot(snapshot);
            }
        }
    }

    void PaneManager::SwitchTimeUnits()
    {
        for (auto it = panes_.begin(); it != panes_.end(); ++it)
        {
            if ((*it) != nullptr)
            {
                (*it)->SwitchTimeUnits();
            }
        }
    }

}  // namespace rmv
