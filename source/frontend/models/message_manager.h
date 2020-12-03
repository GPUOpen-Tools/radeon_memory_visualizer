//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Declaration for the MessageManager
//=============================================================================

#ifndef RMV_MODELS_MESSAGE_MANAGER_H_
#define RMV_MODELS_MESSAGE_MANAGER_H_

#include <QObject>

#include "rmt_data_set.h"
#include "rmt_resource_list.h"

#include "views/pane_manager.h"

/// Class that allows communication between any custom QObjects.
class MessageManager : public QObject
{
    Q_OBJECT

public:
    /// Accessor for singleton instance.
    /// \return A reference to the message manager.
    static MessageManager& Get();

signals:
    /// Signal to open a RMV trace.
    /// \param trace_file The name of the trace file to open.
    void OpenTrace(QString trace_file);

    /// Signal for when a snapshot was opened.
    /// \param snapshot_point The snapshot point of the snapshot to open.
    void OpenSnapshot(RmtSnapshotPoint* snapshot_point);

    /// Signal for when a snapshot was to be compared.
    /// \param snapshot_base The snapshot point of the first snapshot to open.
    /// \param snapshot_diff The snapshot point of the second snapshot to open.
    void CompareSnapshot(RmtSnapshotPoint* snapshot_base, RmtSnapshotPoint* snapshot_diff);

    /// Signal for when a snapshot was clicked.
    /// \param snapshot_point The snapshot point of the snapshot selected.
    void SelectSnapshot(RmtSnapshotPoint* snapshot_point);

    /// Signal a resource was selected.
    /// \param resource_identifier The resource identifier of the resource selected.
    void ResourceSelected(RmtResourceIdentifier resource_identifier);

    /// Signal an unbound resource was selected (pass its allocation).
    /// \param allocation The allocation containing the unbound resource selected.
    void UnboundResourceSelected(const RmtVirtualAllocation* allocation);

    /// Signal a new snapshot point was created.
    /// \param snapshot_point The snapshot point of the snapshot added.
    void SnapshotAdded(RmtSnapshotPoint* snapshot_point);

    /// Signal a snapshot point was renamed.
    /// \param snapshot_point The snapshot point of the snapshot renamed.
    void SnapshotRenamed(RmtSnapshotPoint* snapshot_point);

    /// Signal a snapshot point was deleted.
    /// \param snapshot_point The snapshot point of the snapshot deleted.
    void SnapshotDeleted(RmtSnapshotPoint* snapshot_point);

    /// Signal to navigate to a specific pane.
    /// \param pane The pane to navigate to.
    void NavigateToPane(rmv::RMVPane pane);

    /// Signal to navigate to a specific pane without going through nav manager.
    /// \param pane The pane to navigate to.
    void NavigateToPaneUnrecorded(rmv::RMVPane pane);

    /// Signal for when the hash values changed.
    void UpdateHashes();
};

#endif  // RMV_MODELS_MESSAGE_MANAGER_H_
