//==============================================================================
// Copyright (c) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the Snapshot Manager.
//==============================================================================

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"

#include "managers/message_manager.h"
#include "managers/snapshot_manager.h"

#include "util/definitions.h"
#include "util/rmv_util.h"

/// @brief Worker class definition to generate a snapshot on a separate thread.
class SnapshotWorker : public rmv::BackgroundTask
{
public:
    /// @brief Constructor for comparison snapshot generation.
    ///
    /// @param [in] data_set            The data set contain the RMT data.
    /// @param [in] snapshot_base_point The object containing the snapshot information for the base snapshot.
    /// @param [in] snapshot_diff_point The object containing the snapshot information for the diff snapshot.
    explicit SnapshotWorker(RmtDataSet* data_set, RmtSnapshotPoint* snapshot_base_point, RmtSnapshotPoint* snapshot_diff_point)
        : BackgroundTask(false)
    {
        data_set_                                  = data_set;
        snapshot_point_[rmv::kSnapshotCompareBase] = snapshot_base_point;
        snapshot_[rmv::kSnapshotCompareBase]       = &snapshot_base_point->cached_snapshot;

        if (snapshot_diff_point != nullptr)
        {
            snapshot_point_[rmv::kSnapshotCompareDiff] = snapshot_diff_point;
            snapshot_[rmv::kSnapshotCompareDiff]       = &snapshot_diff_point->cached_snapshot;
        }
        else
        {
            snapshot_point_[rmv::kSnapshotCompareDiff] = nullptr;
            snapshot_[rmv::kSnapshotCompareDiff]       = nullptr;
        }
    }

    /// @brief Destructor.
    ~SnapshotWorker()
    {
    }

    /// @brief Worker thread function.
    virtual void ThreadFunc()
    {
        GenerateSnapshot(rmv::kSnapshotCompareBase);
        GenerateSnapshot(rmv::kSnapshotCompareDiff);

        if (snapshot_[rmv::kSnapshotCompareBase] != nullptr)
        {
            if (snapshot_[rmv::kSnapshotCompareDiff] != nullptr)
            {
                RMT_ASSERT((*snapshot_[rmv::kSnapshotCompareBase])->snapshot_point != (*snapshot_[rmv::kSnapshotCompareDiff])->snapshot_point);
                RMT_ASSERT(snapshot_point_[rmv::kSnapshotCompareBase] != snapshot_point_[rmv::kSnapshotCompareDiff]);
                emit rmv::SnapshotManager::Get().SnapshotThreadFinished(snapshot_point_[rmv::kSnapshotCompareBase], snapshot_point_[rmv::kSnapshotCompareDiff]);
            }
            else
            {
                emit rmv::SnapshotManager::Get().SnapshotThreadFinished(snapshot_point_[rmv::kSnapshotCompareBase], nullptr);
            }
        }
    }

private:
    /// @brief Call the backend function to generate the snapshot.
    ///
    /// If the snapshot is already cached, use that instead.
    ///
    /// @param [in] index The index of the snapshot point to use (base or diff).
    void GenerateSnapshot(int32_t index)
    {
        if (snapshot_point_[index] != nullptr)
        {
            if (snapshot_point_[index]->cached_snapshot == nullptr)
            {
                RmtDataSnapshot*   new_snapshot = new RmtDataSnapshot();
                const RmtErrorCode error_code   = RmtDataSetGenerateSnapshot(data_set_, snapshot_point_[index], new_snapshot);
                RMT_ASSERT(error_code == kRmtOk);

                *snapshot_[index] = new_snapshot;
            }
            else
            {
                *snapshot_[index]                   = snapshot_point_[index]->cached_snapshot;
                (*snapshot_[index])->snapshot_point = snapshot_point_[index];
            }
        }
    }

    RmtDataSet*       data_set_;                                    ///< The data set to create the snapshot from.
    RmtSnapshotPoint* snapshot_point_[rmv::kSnapshotCompareCount];  ///< The snapshot point the snapshot was taken.
    RmtDataSnapshot** snapshot_[rmv::kSnapshotCompareCount];        ///< Pointer to where the snapshot is stored.
};

namespace rmv
{
    // The single snapshot manager instance.
    static SnapshotManager snapshot_manager;

    SnapshotManager::SnapshotManager()
        : thread_controller_(nullptr)
        , selected_snapshot_(nullptr)
        , selected_compared_snapshots_{}
        , loaded_snapshot_(nullptr)
        , loaded_compared_snapshots_{}
        , resource_identifier_(0)
    {
    }

    SnapshotManager::~SnapshotManager()
    {
    }

    SnapshotManager& SnapshotManager::Get()
    {
        return snapshot_manager;
    }

    void SnapshotManager::GenerateSnapshot(RmtDataSet* data_set, RmtSnapshotPoint* snapshot_point)
    {
        GenerateComparison(data_set, snapshot_point, nullptr);
    }

    void SnapshotManager::GenerateComparison(RmtDataSet* data_set, RmtSnapshotPoint* snapshot_base_point, RmtSnapshotPoint* snapshot_diff_point)
    {
        bool thread_needed = true;
        if (snapshot_base_point->cached_snapshot)
        {
            if (snapshot_diff_point == nullptr || snapshot_diff_point->cached_snapshot)
            {
                ShowSnapshots(snapshot_base_point, snapshot_diff_point);
                thread_needed = false;
            }
        }

        if (thread_needed)
        {
            RMT_ASSERT(thread_controller_ == nullptr);
            if (thread_controller_ == nullptr)
            {
                // Start the processing thread and pass in the worker object. The thread controller will take ownership
                // of the worker and delete it once it's complete. Passes in a pointer to a variable that accepts the
                // generated snapshot ID so this can be saved after the worker thread finishes.
                thread_controller_ = new rmv::ThreadController(nullptr, new SnapshotWorker(data_set, snapshot_base_point, snapshot_diff_point));

                // When the worker thread has finished, a signal will be emitted. Wait for the signal here and update
                // the UI with the newly acquired data from the worker thread.
                connect(thread_controller_, &rmv::ThreadController::ThreadFinished, this, &SnapshotManager::GenerateSnapshotCompleted);
                connect(this, &rmv::SnapshotManager::SnapshotThreadFinished, this, &SnapshotManager::ShowSnapshots);
            }
        }
    }

    void SnapshotManager::GenerateSnapshotCompleted()
    {
        disconnect(thread_controller_, &rmv::ThreadController::ThreadFinished, this, &SnapshotManager::GenerateSnapshotCompleted);
        disconnect(this, &rmv::SnapshotManager::SnapshotThreadFinished, this, &SnapshotManager::ShowSnapshots);
        thread_controller_->deleteLater();
        thread_controller_ = nullptr;
    }

    void SnapshotManager::RemoveSnapshot(RmtSnapshotPoint* snapshot_point)
    {
        // If the snapshot point has a cached snapshot (i.e.: there's a chance its open, then look at closing it).
        if (snapshot_point->cached_snapshot)
        {
            // If we're about to remove the snapshot that's open, then signal to everyone its about to vanish.
            const RmtDataSnapshot* open_snapshot = GetOpenSnapshot();
            if (open_snapshot == snapshot_point->cached_snapshot && open_snapshot != nullptr)
            {
                SetSelectedSnapshotPoint(nullptr);
                ClearOpenSnapshot();
            }

            if (GetCompareSnapshot(rmv::kSnapshotCompareBase) == snapshot_point->cached_snapshot ||
                GetCompareSnapshot(rmv::kSnapshotCompareDiff) == snapshot_point->cached_snapshot)
            {
                if (GetCompareSnapshot(rmv::kSnapshotCompareBase) != nullptr || GetCompareSnapshot(rmv::kSnapshotCompareDiff) != nullptr)
                {
                    SetSelectedCompareSnapshotPoints(nullptr, nullptr);
                    ClearCompareSnapshots();
                }
            }
        }

        // Deselect the selected snapshot if it's being removed.
        if (snapshot_point == GetSelectedSnapshotPoint())
        {
            SetSelectedSnapshotPoint(nullptr);
        }
    }

    bool SnapshotManager::LoadSnapshotRequired() const
    {
        const RmtSnapshotPoint* snapshot_point = GetSelectedSnapshotPoint();
        const RmtDataSnapshot*  snapshot       = nullptr;
        if (snapshot_point != nullptr)
        {
            snapshot = snapshot_point->cached_snapshot;
        }

        if (snapshot != loaded_snapshot_ || snapshot == nullptr)
        {
            return true;
        }
        return false;
    }

    bool SnapshotManager::LoadCompareSnapshotsRequired() const
    {
        const RmtSnapshotPoint* base_snapshot_point = GetSelectedCompareSnapshotPointBase();
        const RmtSnapshotPoint* diff_snapshot_point = GetSelectedCompareSnapshotPointDiff();
        const RmtDataSnapshot*  base_snapshot       = nullptr;
        const RmtDataSnapshot*  diff_snapshot       = nullptr;
        if (base_snapshot_point != nullptr)
        {
            base_snapshot = base_snapshot_point->cached_snapshot;
        }
        if (diff_snapshot_point != nullptr)
        {
            diff_snapshot = diff_snapshot_point->cached_snapshot;
        }

        if (base_snapshot != loaded_compared_snapshots_[kSnapshotCompareBase] || base_snapshot == nullptr ||
            diff_snapshot != loaded_compared_snapshots_[kSnapshotCompareDiff] || diff_snapshot == nullptr)
        {
            return true;
        }

        return false;
    }

    void SnapshotManager::ShowSnapshots(RmtSnapshotPoint* snapshot_base, RmtSnapshotPoint* snapshot_diff)
    {
        if (snapshot_diff != nullptr)
        {
            SetCompareSnapshot(snapshot_base->cached_snapshot, snapshot_diff->cached_snapshot);
            emit CompareSnapshotsLoaded();
        }
        else
        {
            SetOpenSnapshot(snapshot_base->cached_snapshot);
            CacheResourceData();
            emit SnapshotLoaded();
        }
    }

    RmtSnapshotPoint* SnapshotManager::GetSelectedSnapshotPoint() const
    {
        return selected_snapshot_;
    }

    void SnapshotManager::SetSelectedSnapshotPoint(RmtSnapshotPoint* snapshot_point)
    {
        selected_snapshot_ = snapshot_point;
    }

    RmtSnapshotPoint* SnapshotManager::GetSelectedCompareSnapshotPointBase() const
    {
        return selected_compared_snapshots_[kSnapshotCompareBase];
    }

    RmtSnapshotPoint* SnapshotManager::GetSelectedCompareSnapshotPointDiff() const
    {
        return selected_compared_snapshots_[kSnapshotCompareDiff];
    }

    void SnapshotManager::SetSelectedCompareSnapshotPoints(RmtSnapshotPoint* snapshot_point_base, RmtSnapshotPoint* snapshot_point_diff)
    {
        selected_compared_snapshots_[kSnapshotCompareBase] = snapshot_point_base;
        selected_compared_snapshots_[kSnapshotCompareDiff] = snapshot_point_diff;
    }

    void SnapshotManager::SetCompareSnapshot(RmtDataSnapshot* snapshot_base, RmtDataSnapshot* snapshot_diff)
    {
        loaded_compared_snapshots_[kSnapshotCompareBase] = snapshot_base;
        loaded_compared_snapshots_[kSnapshotCompareDiff] = snapshot_diff;
    }

    void SnapshotManager::SwapCompareSnapshots()
    {
        RmtDataSnapshot* temp                            = loaded_compared_snapshots_[kSnapshotCompareBase];
        loaded_compared_snapshots_[kSnapshotCompareBase] = loaded_compared_snapshots_[kSnapshotCompareDiff];
        loaded_compared_snapshots_[kSnapshotCompareDiff] = temp;
    }

    const char* SnapshotManager::GetCompareSnapshotName(CompareSnapshots index) const
    {
        return GetSnapshotName(loaded_compared_snapshots_[index]);
    }

    RmtDataSnapshot* SnapshotManager::GetCompareSnapshot(CompareSnapshots snapshot_id) const
    {
        return loaded_compared_snapshots_[snapshot_id];
    }

    void SnapshotManager::ClearCompareSnapshots()
    {
        loaded_compared_snapshots_[kSnapshotCompareBase] = nullptr;
        loaded_compared_snapshots_[kSnapshotCompareDiff] = nullptr;
    }

    RmtDataSnapshot* SnapshotManager::GetOpenSnapshot() const
    {
        return loaded_snapshot_;
    }

    void SnapshotManager::ClearOpenSnapshot()
    {
        loaded_snapshot_ = nullptr;
    }

    const char* SnapshotManager::GetOpenSnapshotName() const
    {
        return GetSnapshotName(loaded_snapshot_);
    }

    void SnapshotManager::SetOpenSnapshot(RmtDataSnapshot* snapshot)
    {
        loaded_snapshot_ = snapshot;
    }

    const char* SnapshotManager::GetSnapshotName(const RmtDataSnapshot* snapshot) const
    {
        if (snapshot != nullptr)
        {
            if (snapshot->snapshot_point != nullptr)
            {
                return snapshot->snapshot_point->name;
            }
            else
            {
                return snapshot->name;
            }
        }
        return nullptr;
    }

    void SnapshotManager::CacheResourceData()
    {
        // Snapshot is loaded at this point.
        RmtDataSnapshot* snapshot = rmv::SnapshotManager::Get().GetOpenSnapshot();

        const RmtVirtualAllocationList& allocation_list  = snapshot->virtual_allocation_list;
        const int32_t                   allocation_count = allocation_list.allocation_count;
        alias_model_.Clear();
        if (allocation_count > 0)
        {
            for (int32_t loop = 0; loop < allocation_count; loop++)
            {
                alias_model_.Generate(&allocation_list.allocation_details[loop]);
            }
        }
    }

    const rmv::AliasedResourceModel& SnapshotManager::GetAliasModel() const
    {
        return alias_model_;
    }

    void SnapshotManager::SetSelectedResource(RmtResourceIdentifier resource_identifier)
    {
        resource_identifier_ = resource_identifier;
        emit rmv::MessageManager::Get().ResourceSelected(resource_identifier_);
    }

    bool SnapshotManager::ResetSelectedResource()
    {
        bool result          = resource_identifier_ != 0;
        resource_identifier_ = 0;
        return result;
    }

    bool SnapshotManager::LoadedSnapshotValid()
    {
        if (loaded_snapshot_ != nullptr)
        {
            if (loaded_snapshot_->resource_list.resource_count > 0)
            {
                return true;
            }
        }
        return false;
    }

    bool SnapshotManager::LoadedCompareSnapshotsValid()
    {
        if (loaded_compared_snapshots_[kSnapshotCompareBase] != nullptr && loaded_compared_snapshots_[kSnapshotCompareDiff] != nullptr)
        {
            if (loaded_compared_snapshots_[kSnapshotCompareBase]->resource_list.resource_count > 0 ||
                loaded_compared_snapshots_[kSnapshotCompareDiff]->resource_list.resource_count > 0)
            {
                return true;
            }
        }
        return false;
    }

}  // namespace rmv
