//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Declaration for the MessageManager.
///
/// The message manager is used to send messages between panes and allow
/// for broadcasting of UI events. For example, if a resource is selected
/// in one pane, any interested panes can set up a connection for the
/// ResourceSelected signal and respond to it.
/// NOTE: The message manager should be used sparingly; if there is a direct
/// connection possible between a signal and slot, that should be used.
///
//=============================================================================

#ifndef RMV_MANAGERS_MESSAGE_MANAGER_H_
#define RMV_MANAGERS_MESSAGE_MANAGER_H_

#include <QObject>

#include "rmt_types.h"
#include "rmt_virtual_allocation_list.h"

#include "managers/pane_manager.h"

namespace rmv
{
    /// @brief Class that allows communication between any custom QObjects.
    class MessageManager : public QObject
    {
        Q_OBJECT

    public:
        /// @brief Accessor for singleton instance.
        ///
        /// @return A reference to the message manager.
        static MessageManager& Get();

    signals:
        /// @brief Signal to open a trace via a file menu.
        void OpenTraceFileMenuClicked();

        /// @brief Something changed the file list (either a delete or a new file added).
        void RecentFileListChanged();

        /// @brief Signal a resource was selected.
        ///
        /// @param [in] resource_identifier The resource identifier of the resource selected.
        void ResourceSelected(RmtResourceIdentifier resource_identifier);

        /// @brief Signal an unbound resource was selected (pass its allocation).
        ///
        /// @param [in] allocation The allocation containing the unbound resource selected.
        void UnboundResourceSelected(const RmtVirtualAllocation* allocation);

        /// @brief Signal that the title bar has changed and needs updating.
        void TitleBarChanged();

        /// @brief Signal to navigate to a specific pane.
        ///
        /// @param [in] pane The pane to navigate to.
        void PaneSwitchRequested(rmv::RMVPaneId pane);

        /// @brief Signal to requested that snapshots be switched (from the Snapshot delta pane).
        void SwapSnapshotsRequested();

        /// @brief Signal for when the hash values changed.
        void HashesChanged();

        /// @brief Signal to request enabling or disabling UI actions.
        ///
        /// @param [in] enable  If true, actions should be enabled.  Otherwise, actions should be disabled.
        void ChangeActionsRequested(const bool enable);
    };
}  // namespace rmv

#endif  // RMV_MANAGERS_MESSAGE_MANAGER_H_
