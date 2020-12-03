//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header for back/fwd navigation manager
//=============================================================================

#ifndef RMV_VIEWS_NAVIGATION_MANAGER_H_
#define RMV_VIEWS_NAVIGATION_MANAGER_H_

#include <QWidget>
#include <QList>
#include <QModelIndex>

#include "util/definitions.h"
#include "views/pane_manager.h"

namespace rmv
{
    /// Class that handles back and forward navigation.
    class NavigationManager : public QWidget
    {
        Q_OBJECT

    public:
        /// NavigationManager instance get function.
        /// \return a reference to the NavigationManager instance.
        static NavigationManager& Get();

        /// Record a pane switch event.
        /// \param pane The new pane.
        void RecordNavigationEventPaneSwitch(RMVPane pane);

        /// Go back to starting state.
        void Reset();

        /// Update the current pane.
        /// \param pane The current pane.
        void UpdateCurrentPane(RMVPane pane);

    signals:
        /// Signal to enable the navigation back button.
        void EnableBackNavButton(bool enable);

        /// Signal to enable the navigation forward button.
        void EnableForwardNavButton(bool enable);

    public slots:
        /// Go forward.
        void NavigateForward();

        /// Go back.
        void NavigateBack();

    private:
        /// Enum of navigation types.
        enum NavType
        {
            kNavigationTypeUndefined,
            kNavigationTypePaneSwitch,

            kNavigationTypeCount,
        };

        /// Struct to hold navigation event data.
        struct NavEvent
        {
            NavType type;  ///< Navigation type.
            RMVPane pane;  ///< Destination pane.
        };

        /// Constructor.
        explicit NavigationManager();

        /// Destructor.
        virtual ~NavigationManager();

        /// Remove history if user went back towards the middle and then somewhere new.
        void DiscardObsoleteNavHistory();

        /// Replay a previous navigation event.
        /// \param event The navigation event.
        void AddNewEvent(const NavEvent& event);

        /// Helper function to print a Navigation event.
        /// \param event the navigation event.
        /// \return string version of the navigation event.
        const QString NavigationEventString(const NavEvent& event) const;

        /// Register a new pane switch.
        /// \param pane The new pane.
        void AddNewPaneSwitch(RMVPane pane);

        /// Replay a previous navigation event.
        /// \param event The navigation event.
        void ReplayNavigationEvent(const NavEvent& event);

        /// Intelligently find the next navigation event.
        /// \return The next navigation event.
        NavEvent FindNextNavigationEvent();

        /// Intelligently find the previous navigation event.
        /// \return The previous navigation event.
        NavEvent FindPrevNavigationEvent();

        /// Helper function to print the backing structure.
        void PrintHistory() const;

        /// Helper function to convert RmvPane enum to string.
        /// \param pane the pane.
        /// \return string version of pane name.
        QString GetPaneString(RMVPane pane) const;

        QList<NavEvent> navigation_history_;           ///< Track user navigation.
        int32_t         navigation_history_location_;  ///< Current location in navigation history.
        RMVPane         current_pane_;                 ///< Navigation manager is aware of the current pane.
    };
}  // namespace rmv

#endif  // RMV_VIEWS_NAVIGATION_MANAGER_H_
