//==============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the RMV Snapshot Manager.
//==============================================================================

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"

#include "models/message_manager.h"
#include "models/snapshot_manager.h"
#include "models/trace_manager.h"

/// Worker class definition to generate a snapshot on a separate thread.
class SnapshotWorker : public rmv::BackgroundTask
{
public:
    /// Constructor for single snapshot generation.
    /// \param snapshot_point The object containing the snapshot information.
    explicit SnapshotWorker(RmtSnapshotPoint* snapshot_point)
        : BackgroundTask()
    {
        snapshot_point_[kSnapshotCompareBase] = snapshot_point;
        snapshot_point_[kSnapshotCompareDiff] = nullptr;
        snapshot_[kSnapshotCompareBase]       = &snapshot_point->cached_snapshot;
        snapshot_[kSnapshotCompareDiff]       = nullptr;
    }

    /// Constructor for comparison snapshot generation.
    /// \param snapshot_base_point The object containing the snapshot information for the base snapshot.
    /// \param snapshot_diff_point The object containing the snapshot information for the diff snapshot.
    explicit SnapshotWorker(RmtSnapshotPoint* snapshot_base_point, RmtSnapshotPoint* snapshot_diff_point)
        : BackgroundTask()
    {
        snapshot_point_[kSnapshotCompareBase] = snapshot_base_point;
        snapshot_point_[kSnapshotCompareDiff] = snapshot_diff_point;
        snapshot_[kSnapshotCompareBase]       = &snapshot_base_point->cached_snapshot;
        snapshot_[kSnapshotCompareDiff]       = &snapshot_diff_point->cached_snapshot;
    }

    /// Destructor
    ~SnapshotWorker()
    {
    }

    /// Worker thread function.
    virtual void ThreadFunc()
    {
        GenerateSnapshot(kSnapshotCompareBase);
        GenerateSnapshot(kSnapshotCompareDiff);

        if (snapshot_[kSnapshotCompareBase] != nullptr)
        {
            if (snapshot_[kSnapshotCompareDiff] != nullptr)
            {
                emit MessageManager::Get().CompareSnapshot(snapshot_point_[kSnapshotCompareBase], snapshot_point_[kSnapshotCompareDiff]);
            }
            else
            {
                emit MessageManager::Get().OpenSnapshot(snapshot_point_[kSnapshotCompareBase]);
            }
        }
    }

private:
    /// Call the backend function to generate the snapshot. If the snapshot is already
    /// cached, use that instead.
    /// \param index The index of the snapshot point to use (base or diff).
    void GenerateSnapshot(int32_t index)
    {
        if (snapshot_point_[index] != nullptr)
        {
            if (snapshot_point_[index]->cached_snapshot == nullptr)
            {
                RmtDataSnapshot*   new_snapshot = new RmtDataSnapshot();
                RmtDataSet*        data_set     = TraceManager::Get().GetDataSet();
                const RmtErrorCode error_code   = RmtDataSetGenerateSnapshot(data_set, snapshot_point_[index], new_snapshot);
                RMT_ASSERT(error_code == RMT_OK);

                *snapshot_[index] = new_snapshot;
            }
            else
            {
                *snapshot_[index] = snapshot_point_[index]->cached_snapshot;
            }
        }
    }

    RmtSnapshotPoint* snapshot_point_[kSnapshotCompareCount];  ///< The snapshot point the snapshot was taken.
    RmtDataSnapshot** snapshot_[kSnapshotCompareCount];        ///< Pointer to where the snapshot is stored.
};

// Ths single snapshot manager instance.
static SnapshotManager snapshot_manager;

SnapshotManager::SnapshotManager()
    : thread_controller_(nullptr)
    , selected_snapshot_(nullptr)
{
}

SnapshotManager::~SnapshotManager()
{
}

SnapshotManager& SnapshotManager::Get()
{
    return snapshot_manager;
}

void SnapshotManager::GenerateSnapshot(RmtSnapshotPoint* snapshot_point, MainWindow* main_window, QWidget* parent_widget)
{
    RMT_ASSERT(thread_controller_ == nullptr);
    if (thread_controller_ == nullptr)
    {
        // start the processing thread and pass in the worker object. The thread controller will take ownership
        // of the worker and delete it once it's complete. Passes in a pointer to a variable that accepts the
        // generated snapshot ID so this can be saved after the worker thread finishes
        thread_controller_ = new rmv::ThreadController(main_window, parent_widget, new SnapshotWorker(snapshot_point));

        // when the worker thread has finished, a signal will be emitted. Wait for the signal here and update
        // the UI with the newly acquired data from the worker thread
        connect(thread_controller_, &rmv::ThreadController::ThreadFinished, this, &SnapshotManager::GenerateSnapshotCompleted);
    }
}

void SnapshotManager::GenerateComparison(RmtSnapshotPoint* snapshot_base_point,
                                         RmtSnapshotPoint* snapshot_diff_point,
                                         MainWindow*       main_window,
                                         QWidget*          parent_widget)
{
    RMT_ASSERT(thread_controller_ == nullptr);
    if (thread_controller_ == nullptr)
    {
        // start the processing thread and pass in the worker object. The thread controller will take ownership
        // of the worker and delete it once it's complete. Passes in a pointer to a variable that accepts the
        // generated snapshot ID so this can be saved after the worker thread finishes
        thread_controller_ = new rmv::ThreadController(main_window, parent_widget, new SnapshotWorker(snapshot_base_point, snapshot_diff_point));

        // when the worker thread has finished, a signal will be emitted. Wait for the signal here and update
        // the UI with the newly acquired data from the worker thread
        connect(thread_controller_, &rmv::ThreadController::ThreadFinished, this, &SnapshotManager::GenerateSnapshotCompleted);
    }
}

void SnapshotManager::GenerateSnapshotCompleted()
{
    disconnect(thread_controller_, &rmv::ThreadController::ThreadFinished, this, &SnapshotManager::GenerateSnapshotCompleted);
    thread_controller_->deleteLater();
    thread_controller_ = nullptr;
}

RmtSnapshotPoint* SnapshotManager::GetSelectedSnapshotPoint() const
{
    return selected_snapshot_;
}

void SnapshotManager::SetSelectedSnapshotPoint(RmtSnapshotPoint* snapshot_point)
{
    selected_snapshot_ = snapshot_point;
}
