//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Implementation for back/fwd navigation manager
//=============================================================================

#include "views/navigation_manager.h"

#include <QDebug>

#include "qt_common/custom_widgets/navigation_bar.h"

#include "rmt_assert.h"
#include "rmt_data_set.h"

#include "models/message_manager.h"

namespace rmv
{
    NavigationManager::NavigationManager()
        : navigation_history_location_(0)
        , current_pane_(kPaneStartWelcome)
    {
        Reset();
    }

    NavigationManager::~NavigationManager()
    {
    }

    NavigationManager& NavigationManager::Get()
    {
        static NavigationManager instance;

        return instance;
    }

    void NavigationManager::RecordNavigationEventPaneSwitch(RMVPane pane)
    {
        const NavEvent& curr_event = navigation_history_[navigation_history_location_];

        if (curr_event.type == kNavigationTypePaneSwitch)
        {
            // Prevent dupes
            if (curr_event.pane != pane)
            {
                AddNewPaneSwitch(pane);
            }
        }
        else
        {
            AddNewPaneSwitch(pane);
        }
    }

    void NavigationManager::Reset()
    {
        navigation_history_.clear();

        NavEvent first_event = {};
        first_event.type     = kNavigationTypePaneSwitch;
        first_event.pane     = kPaneStartWelcome;

        navigation_history_.push_back(first_event);
        navigation_history_location_ = 0;

        current_pane_ = kPaneStartWelcome;

        emit EnableBackNavButton(false);
        emit EnableForwardNavButton(false);
    }

    void NavigationManager::UpdateCurrentPane(RMVPane pane)
    {
        current_pane_ = pane;
    }

    void NavigationManager::ReplayNavigationEvent(const NavEvent& event)
    {
        emit MessageManager::Get().NavigateToPaneUnrecorded(event.pane);
    }

    NavigationManager::NavEvent NavigationManager::FindPrevNavigationEvent()
    {
        NavEvent out  = navigation_history_[navigation_history_location_];
        NavEvent curr = navigation_history_[navigation_history_location_];

        if (navigation_history_location_ > 0)
        {
            int32_t  idx  = navigation_history_location_ - 1;
            NavEvent prev = navigation_history_[idx];

            out                          = prev;
            navigation_history_location_ = idx;

            if ((prev.type == kNavigationTypePaneSwitch) && (curr.pane == prev.pane))
            {
                idx--;
                if (idx > 0)
                {
                    NavEvent prev_prev           = navigation_history_[idx];
                    out                          = prev_prev;
                    navigation_history_location_ = idx;
                }
            }
        }

        return out;
    }

    NavigationManager::NavEvent NavigationManager::FindNextNavigationEvent()
    {
        NavEvent out = navigation_history_[navigation_history_location_];

        const int32_t nav_limit = navigation_history_.size() - 1;

        if (navigation_history_location_ < nav_limit)
        {
            int32_t  idx  = navigation_history_location_ + 1;
            NavEvent next = navigation_history_[idx];

            out                          = next;
            navigation_history_location_ = idx;
        }

        return out;
    }

    void NavigationManager::NavigateBack()
    {
        if (navigation_history_location_ > 0)
        {
            RMT_ASSERT(navigation_history_.size() > 1);

            NavEvent prev_event = FindPrevNavigationEvent();

            ReplayNavigationEvent(prev_event);

            emit EnableForwardNavButton(true);
        }

        if (navigation_history_location_ <= 0)
        {
            emit EnableBackNavButton(false);
        }
    }

    void NavigationManager::NavigateForward()
    {
        const int32_t nav_limit = navigation_history_.size() - 1;

        if (navigation_history_location_ < nav_limit)
        {
            NavEvent next_event = FindNextNavigationEvent();

            ReplayNavigationEvent(next_event);

            emit EnableBackNavButton(true);
        }

        if (navigation_history_location_ >= nav_limit)
        {
            emit EnableForwardNavButton(false);
        }
    }

    void NavigationManager::DiscardObsoleteNavHistory()
    {
        for (int32_t i = navigation_history_.size() - 1; i > 0; i--)
        {
            if (i > navigation_history_location_)
            {
                navigation_history_.pop_back();

                emit EnableForwardNavButton(false);
            }
        }
    }

    void NavigationManager::AddNewEvent(const NavEvent& event)
    {
        navigation_history_.push_back(event);
        navigation_history_location_++;

        emit EnableBackNavButton(true);
    }

    void NavigationManager::AddNewPaneSwitch(RMVPane pane)
    {
        DiscardObsoleteNavHistory();

        NavEvent new_event = {};
        new_event.type     = kNavigationTypePaneSwitch;
        new_event.pane     = pane;

        AddNewEvent(new_event);
    }

    const QString NavigationManager::NavigationEventString(const NavEvent& event) const
    {
        QString out = "";

        if (event.type == kNavigationTypePaneSwitch)
        {
            out += GetPaneString(event.pane);
        }

        return out;
    }

    void NavigationManager::PrintHistory() const
    {
        QString out = "";

        const uint32_t nav_history_size = navigation_history_.size();

        for (uint32_t i = 0; i < nav_history_size; i++)
        {
            const NavEvent& curr_event = navigation_history_[i];

            out += "[" + QString::number(i) + "]=" + NavigationEventString(curr_event);

            if (i < nav_history_size - 1)
            {
                out += " | ";
            }
        }

        qDebug() << out;
    }

    QString NavigationManager::GetPaneString(RMVPane pane) const
    {
        QString out = "";

        switch (pane)
        {
        case kPaneStartWelcome:
            out = "Welcome";
            break;
        case kPaneStartRecentTraces:
            out = "Recent traces";
            break;
        case kPaneStartAbout:
            out = "About";
            break;
        case kPaneTimelineGenerateSnapshot:
            out = "Generate snapshot";
            break;
        case kPaneTimelineDeviceConfiguration:
            out = "Device configuration";
            break;
        case kPaneSnapshotResourceOverview:
            out = "Resource overview";
            break;
        case kPaneSnapshotAllocationOverview:
            out = "Allocation overview";
            break;
        case kPaneSnapshotResourceList:
            out = "Resource list";
            break;
        case kPaneSnapshotResourceDetails:
            out = "Resource details";
            break;
        case kPaneSnapshotAllocationExplorer:
            out = "Allocation explorer";
            break;
        case kPaneSnapshotHeapOverview:
            out = "Heap overview";
            break;
        case kPaneCompareSnapshotDelta:
            out = "Snapshot delta";
            break;
        case kPaneCompareMemoryLeakFinder:
            out = "Memory leak finder";
            break;
        case kPaneSettingsGeneral:
            out = "General";
            break;
        case kPaneSettingsThemesAndColors:
            out = "Themes and colors";
            break;
        case kPaneSettingsKeyboardShortcuts:
            out = "Keyboard shortcuts";
            break;
        default:
            break;
        }

        return out;
    }
}  // namespace rmv