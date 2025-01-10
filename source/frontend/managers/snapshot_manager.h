//==============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Class definition for the Snapshot Manager.
//==============================================================================

#ifndef RMV_MANAGERS_SNAPSHOT_MANAGER_H_
#define RMV_MANAGERS_SNAPSHOT_MANAGER_H_

#include <QObject>

#include "rmt_data_set.h"

#include "models/aliased_resource_model.h"
#include "util/definitions.h"
#include "util/thread_controller.h"

namespace rmv
{
    /// @brief Enum for the comparison snapshot types.
    enum CompareSnapshots
    {
        kSnapshotCompareBase,
        kSnapshotCompareDiff,

        kSnapshotCompareCount
    };

    /// @brief Class to handle the management of snapshot monitoring and loading.
    ///
    /// Since snapshot generation can take a few seconds, the generation itself is done on a
    /// worker thread while the main UI thread displays a loading animation. This functionality
    /// is abstracted away in this class and a couple methods are added to initiate snapshot
    /// generation. Additionally, the loading of snapshots is deferred until they are actually
    /// viewed, either by manually clicking on the SNAPSHOT or COMPARE tabs or selecting snapshots
    /// by double clicking or selecting via a context menu.
    class SnapshotManager : public QObject
    {
        Q_OBJECT

    public:
        /// @brief Constructor.
        explicit SnapshotManager();

        /// @brief Destructor.
        ~SnapshotManager();

        /// @brief Accessor for singleton instance.
        ///
        /// @return A reference to the snapshot manager.
        static SnapshotManager& Get();

        /// @brief Create a new snapshot.
        ///
        /// Run the snapshot generation in a separate thread and use the main thread
        /// to show the loading animation in the cases where the snapshot generation takes a while.
        ///
        /// @param [in] data_set       The data set to create the snapshot from.
        /// @param [in] snapshot_point The object containing the snapshot information.
        void GenerateSnapshot(RmtDataSet* data_set, RmtSnapshotPoint* snapshot_point);

        /// @brief Create snapshots for comparison.
        ///
        /// Run the snapshot generation in a separate thread and use the
        /// main thread to show the loading animation in the cases where the snapshot generation takes a while.
        ///
        /// @param [in] data_set            The data set to create the snapshots from.
        /// @param [in] snapshot_base_point The object containing the snapshot information for the base snapshot.
        /// @param [in] snapshot_diff_point The object containing the snapshot information for the diff snapshot.
        void GenerateComparison(RmtDataSet* data_set, RmtSnapshotPoint* snapshot_base_point, RmtSnapshotPoint* snapshot_diff_point);

        /// @brief Remove a snapshot.
        ///
        /// Make sure the removed snapshot is deselected and notify the rest of the UI.
        ///
        /// @param [in] snapshot_point The snapshot to be removed.
        void RemoveSnapshot(RmtSnapshotPoint* snapshot_point);

        /// @brief Does a snapshot need loading?
        ///
        /// @return true if snapshot needs loading, false if not.
        bool LoadSnapshotRequired() const;

        /// @brief Do any of the compare snapshots need loading?
        ///
        /// @return true if a snapshot needs loading, false if not.
        bool LoadCompareSnapshotsRequired() const;

        /// @brief Get the snapshot point selected in the UI.
        ///
        /// @return The selected snapshot point.
        RmtSnapshotPoint* GetSelectedSnapshotPoint() const;

        /// @brief Set the snapshot point selected in the UI.
        ///
        /// @param [in] snapshot_point The snapshot point selected.
        void SetSelectedSnapshotPoint(RmtSnapshotPoint* snapshot_point);

        /// @brief Get the snapshot point for the snapshot used as the base when comparing snapshots.
        ///
        /// @return The base snapshot point.
        RmtSnapshotPoint* GetSelectedCompareSnapshotPointBase() const;

        /// @brief Get the snapshot point for the snapshot used as the diff when comparing snapshots.
        ///
        /// @return The diff snapshot point.
        RmtSnapshotPoint* GetSelectedCompareSnapshotPointDiff() const;

        /// @brief If 2 items are selected in the snapshot table, set their snapshot points so they can be compared.
        ///
        /// The snapshot selected last in the table will be the diff snapshot which will be compared with the
        /// snapshot selected first.
        ///
        /// @param [in] snapshot_point_base The snapshot point of the first table item selected.
        /// @param [in] snapshot_point_diff The snapshot point of the last table item selected.
        void SetSelectedCompareSnapshotPoints(RmtSnapshotPoint* snapshot_point_base, RmtSnapshotPoint* snapshot_point_diff);

        /// @brief Get a pointer to a comparison snapshot.
        ///
        /// @param [in] snapshot_id The snapshot id, either base or diff.
        ///
        /// @return The snapshot.
        RmtDataSnapshot* GetCompareSnapshot(CompareSnapshots snapshot_id) const;

        /// @brief Get the snapshot name from a snapshot. Prefer the name from the snapshot point.
        ///
        /// If that doesn't exist, use the name from the snapshot itself.
        ///
        /// @param [in] index The compare snapshot index.
        ///
        /// @return The snapshot name.
        const char* GetCompareSnapshotName(CompareSnapshots index) const;

        /// @brief Clear the comparison snapshots.
        void ClearCompareSnapshots();

        /// @brief Swap the comparison snapshots.
        void SwapCompareSnapshots();

        /// @brief Get the snapshot name from a snapshot. Prefer the name from the snapshot point.
        ///
        /// If that doesn't exist, use the name from the snapshot itself.
        ///
        /// @return The snapshot name.
        const char* GetOpenSnapshotName() const;

        /// @brief Get a pointer to the opened snapshot.
        ///
        /// @return The opened snapshot.
        RmtDataSnapshot* GetOpenSnapshot() const;

        /// @brief Clear the opened snapshot.
        void ClearOpenSnapshot();

        /// @brief Get the model responsible for managing resource aliasing.
        ///
        /// @return The alias model.
        const rmv::AliasedResourceModel& GetAliasModel() const;

        /// @brief Set the selected resource when opening a snapshot.
        ///
        /// Used for the case of selecting a resource from the memory leak pane to see
        /// its details.
        ///
        /// @param [in] resource_identifier The resource selected.
        void SetSelectedResource(RmtResourceIdentifier resource_identifier);

        /// @brief Reset the selected resource after a snapshot has been loaded.
        ///
        /// @return true if there is a resource selected already (in the case where
        /// transition to the resource details pane is needed).
        bool ResetSelectedResource();

        /// @brief Is the currently loaded snapshot valid?
        ///
        /// Make sure the loaded snapshot is actually loaded and has valid data ie it
        /// contains allocations and resources.
        ///
        /// @return true if snapshot is valid, false if not.
        bool LoadedSnapshotValid();

        /// @brief Are the currently loaded snapshots for comparison valid?
        ///
        /// Make sure the loaded snapshots for comparison are actually loaded
        /// and they both contain valid data ie they both contain allocations
        /// and resources. At least one of the snapshots needs to contain
        /// valid data.
        ///
        /// @return true if snapshot is valid, false if not.
        bool LoadedCompareSnapshotsValid();

    signals:
        /// @brief Signal for when a snapshot marker was clicked on.
        ///
        /// @param [in] snapshot_point The snapshot point of the snapshot selected.
        void SnapshotMarkerSelected(RmtSnapshotPoint* snapshot_point);

        /// @brief Signal for when a snapshot was opened.
        ///
        /// @param [in] resource_identifier The resource selected when opening the snapshot, for the
        /// case of selecting a resource from the memory leak pane to see its details.
        void SnapshotOpened(RmtResourceIdentifier resource_identifier = 0);

        /// @brief Signal for when 2 snapshots are to be compared.
        void CompareSnapshotsOpened();

        /// @brief Signal for when a snapshot was loaded.
        void SnapshotLoaded();

        /// @brief Signal for when the compare snaphots have been loaded.
        void CompareSnapshotsLoaded();

        /// @brief Signal for when the loading thread has completed.
        ///
        /// @param snapshot_base The snapshot point of the first snapshot to open (or the single snapshot to view).
        /// @param snapshot_diff The snapshot point of the second snapshot to open (can be nullptr).
        void SnapshotThreadFinished(RmtSnapshotPoint* snapshot_base, RmtSnapshotPoint* snapshot_diff);

    private slots:
        /// @brief Slot to handle what happens when the snapshot worker thread has finished.
        void GenerateSnapshotCompleted();

        /// @brief Prepare to show the loaded snapshots.
        ///
        /// @param snapshot_base The snapshot point of the first snapshot to open (or the single snapshot to view).
        /// @param snapshot_diff The snapshot point of the second snapshot to open (can be nullptr).
        void ShowSnapshots(RmtSnapshotPoint* snapshot_base, RmtSnapshotPoint* snapshot_diff);

    private:
        /// @brief Cache any resource data for the currently active snapshot.
        void CacheResourceData();

        /// @brief Set the value of the opened snapshot.
        ///
        /// @param snapshot The opened snapshot.
        void SetOpenSnapshot(RmtDataSnapshot* snapshot);

        /// @brief Update the currently active compare snapshots.
        ///
        /// @param snapshot_base The base snapshot to compare.
        /// @param snapshot_diff The snapshot to compare with the base snapshot.
        void SetCompareSnapshot(RmtDataSnapshot* snapshot_base, RmtDataSnapshot* snapshot_diff);

        /// @brief Get the snapshot name from a snapshot. Prefer the name from the snapshot point.
        ///
        /// If that doesn't exist, use the name from the snapshot itself.
        ///
        /// @param snapshot The snapshot.
        ///
        /// @return The snapshot name.
        const char* GetSnapshotName(const RmtDataSnapshot* snapshot) const;

        rmv::ThreadController*    thread_controller_;                                   ///< The thread for processing backend data.
        RmtSnapshotPoint*         selected_snapshot_;                                   ///< The snapshot point selected in the snapshot table.
        RmtSnapshotPoint*         selected_compared_snapshots_[kSnapshotCompareCount];  ///< The compare snapshots selected in the snapshot table.
        RmtDataSnapshot*          loaded_snapshot_;                                     ///< A pointer to the currently opened (loaded) snapshot.
        RmtDataSnapshot*          loaded_compared_snapshots_[kSnapshotCompareCount];    ///< Pointers to the currently opened (loaded) snapshots being compared.
        rmv::AliasedResourceModel alias_model_;                                         ///< The model used for showing aliased resources.
        RmtResourceIdentifier     resource_identifier_;                                 ///< The resource to select when opening the snapshot.
    };
}  // namespace rmv

#endif  // RMV_MANAGERS_SNAPSHOT_MANAGER_H_
