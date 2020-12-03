//==============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Class definition for the RMV Snapshot Manager.
//==============================================================================

#ifndef RMV_MODELS_SNAPSHOT_MANAGER_H_
#define RMV_MODELS_SNAPSHOT_MANAGER_H_

#include <QObject>

#include "rmt_data_set.h"

#include "util/thread_controller.h"

enum CompareSnapshots
{
    kSnapshotCompareBase,
    kSnapshotCompareDiff,

    kSnapshotCompareCount
};

/// Class to handle the generation of a single snapshot and snapshots used for comparison.
/// Since snapshot generation can take a few seconds, the generation itself is done on a
/// worker thread while the main UI thread displays a loading animation. This functionality
/// is abstracted away in this class and a couple methods are added to initiate snapshot
/// generation.
class SnapshotManager : public QObject
{
    Q_OBJECT

public:
    /// Constructor.
    explicit SnapshotManager();

    /// Destructor.
    ~SnapshotManager();

    /// Accessor for singleton instance.
    /// \return A reference to the snapshot manager.
    static SnapshotManager& Get();

    /// Create a new snapshot. Run the snapshot generation in a separate thread and use the main thread
    /// to show the loading animation in the cases where the snapshot generation takes a while.
    /// \param snapshot_point The object containing the snapshot information.
    /// \param main_window Pointer to the main window.
    /// \param parent_widget The widget where the loading animation is to be displayed.
    void GenerateSnapshot(RmtSnapshotPoint* snapshot_point, MainWindow* main_window, QWidget* parent_widget);

    /// Create snapshots for comparison. Run the snapshot generation in a separate thread and use the
    /// main thread to show the loading animation in the cases where the snapshot generation takes a while.
    /// \param snapshot_base_point The object containing the snapshot information for the base snapshot.
    /// \param snapshot_diff_point The object containing the snapshot information for the diff snapshot.
    /// \param main_window Pointer to the main window.
    /// \param parent_widget The widget where the loading animation is to be displayed.
    void GenerateComparison(RmtSnapshotPoint* snapshot_base_point, RmtSnapshotPoint* snapshot_diff_point, MainWindow* main_window, QWidget* parent_widget);

    /// Get the snapshot point selected in the UI.
    RmtSnapshotPoint* GetSelectedSnapshotPoint() const;

    /// Set the snapshot point from the UI.
    /// \param snapshot_point The snapshot point selected.
    void SetSelectedSnapshotPoint(RmtSnapshotPoint* snapshot_point);

private slots:
    /// Slot to handle what happens when the snapshot worker thread has finished.
    void GenerateSnapshotCompleted();

private:
    rmv::ThreadController* thread_controller_;  ///< The thread for processing backend data.
    RmtSnapshotPoint*      selected_snapshot_;  ///< The selected snapshot point.
};

#endif  // RMV_MODELS_SNAPSHOT_MANAGER_H_
