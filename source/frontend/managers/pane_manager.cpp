//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the pane manager.
//=============================================================================

#include "managers/pane_manager.h"

#include "managers/navigation_manager.h"
#include "managers/trace_manager.h"

namespace rmv
{
    PaneManager::PaneManager()
        : nav_location_{}
        , current_pane_(kPaneIdStartWelcome)
        , previous_pane_(kPaneIdStartWelcome)
    {
        ResetNavigation();
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

    MainPanes PaneManager::GetMainPaneFromPane(RMVPaneId pane) const
    {
        return static_cast<MainPanes>(pane >> kPaneShift);
    }

    RMVPaneId PaneManager::GetCurrentPane() const
    {
        return current_pane_;
    }

    RMVPaneId PaneManager::GetPreviousPane() const
    {
        return previous_pane_;
    }

    const NavLocation* PaneManager::SetupNextPane(RMVPaneId pane)
    {
        rmv::MainPanes main_pane = GetMainPaneFromPane(pane);

        if (main_pane == rmv::kMainPaneSnapshot || main_pane == rmv::kMainPaneCompare || main_pane == rmv::kMainPaneTimeline)
        {
            // Make sure a trace is loaded before navigating.
            if (!rmv::TraceManager::Get().DataSetValid())
            {
                return nullptr;
            }
        }

        int32_t main_tab_index       = pane >> kPaneShift;
        int32_t list_row             = pane & kPaneMask;
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

        return &nav_location_;
    }

    RMVPaneId PaneManager::UpdateCurrentPane()
    {
        // Create the combined component.
        int32_t current_pane = (nav_location_.main_tab_index << kPaneShift);

        switch (nav_location_.main_tab_index)
        {
        case kMainPaneStart:
            current_pane |= nav_location_.start_list_row;
            break;

        case kMainPaneTimeline:
            current_pane |= nav_location_.timeline_list_row;
            break;

        case kMainPaneSnapshot:
            current_pane |= nav_location_.snapshot_list_row;
            break;

        case kMainPaneCompare:
            current_pane |= nav_location_.compare_list_row;
            break;

        case kMainPaneSettings:
            current_pane |= nav_location_.settings_list_row;
            break;

        default:
            break;
        }

        // Only update the current pane if it's changed.
        if (current_pane != current_pane_)
        {
            previous_pane_ = current_pane_;
            current_pane_  = static_cast<RMVPaneId>(current_pane);
        }

        return current_pane_;
    }

    bool PaneManager::ClickedSnapshotTab() const
    {
        // Catch any transition to the snapshot tab from any other tab and
        // make sure the snapshot is opened, specifically the case of selecting
        // something in the timeline pane, and then moving to the snapshot view.
        const MainPanes current_main_pane  = GetMainPaneFromPane(GetCurrentPane());
        const MainPanes previous_main_pane = GetMainPaneFromPane(GetPreviousPane());
        if (current_main_pane == kMainPaneSnapshot && previous_main_pane != kMainPaneSnapshot)
        {
            return true;
        }
        return false;
    }

    bool PaneManager::ClickedCompareTab() const
    {
        // Catch any transition to the compare tab from any other tab and
        // make sure the snapshots are opened, specifically the case of selecting
        // something in the timeline pane, and then moving to the compare view.
        const MainPanes current_main_pane  = GetMainPaneFromPane(GetCurrentPane());
        const MainPanes previous_main_pane = GetMainPaneFromPane(GetPreviousPane());
        if (current_main_pane == kMainPaneCompare && previous_main_pane != kMainPaneCompare)
        {
            return true;
        }
        return false;
    }

    void PaneManager::UpdateMainTabIndex(const int tab_index)
    {
        if ((tab_index >= kMainPaneStart) && (tab_index < kMainPaneCount))
        {
            nav_location_.main_tab_index = tab_index;
            UpdateCurrentPane();
            NavigationManager::Get().RecordNavigationEventPaneSwitch(current_pane_);
        }
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

    void PaneManager::AddComparePane(ComparePane* pane)
    {
        compare_panes_.push_back(pane);
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

    void PaneManager::UpdateCompares()
    {
        for (auto it = compare_panes_.begin(); it != compare_panes_.end(); ++it)
        {
            if ((*it) != nullptr)
            {
                (*it)->Refresh();
            }
        }
    }

}  // namespace rmv
