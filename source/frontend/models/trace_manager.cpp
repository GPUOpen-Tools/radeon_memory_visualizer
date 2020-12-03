//==============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the RMV Trace Manager.
//==============================================================================

#include "models/trace_manager.h"

#include <QtCore>
#include <QMessageBox>
#include <QByteArray>
#include <vector>

#include "qt_common/utils/qt_util.h"

#ifndef _WIN32
#include <linux/safe_crt.h>
#endif

#include "rmt_assert.h"
#include "rmt_error.h"
#include "rmt_virtual_allocation_list.h"

#include "models/message_manager.h"
#include "settings/rmv_settings.h"
#include "util/definitions.h"

/// Spawns a thread to load a dataset of either VMA or RMV origin
class LoadingThread : public QThread
{
public:
    explicit LoadingThread(const QString& path)
        : path_data(path.toLatin1())
    {
    }

    void run() Q_DECL_OVERRIDE
    {
        const char*               trace_path = path_data.data();
        const TraceLoadReturnCode error_code = TraceManager::Get().TraceLoad(trace_path);
        emit                      TraceManager::Get().TraceLoadComplete(error_code);
    }

private:
    QByteArray path_data;  ///< The path to the trace being loaded
};

/// Pointer to the loading thread object.
static LoadingThread* loading_thread = nullptr;

/// The single instance of the trace manager.
static TraceManager trace_manager;

TraceManager& TraceManager::Get()
{
    return trace_manager;
}

TraceManager::TraceManager(QObject* parent)
    : QObject(parent)
    , open_snapshot_(nullptr)
    , compared_snapshots_{}
    , main_window_(nullptr)
    , resource_thresholds_{}
{
    int id = qRegisterMetaType<TraceLoadReturnCode>();
    Q_UNUSED(id);

    ClearTrace();
}

TraceManager::~TraceManager()
{
}

void TraceManager::Initialize(MainWindow* main_window)
{
    main_window_ = main_window;
}

TraceLoadReturnCode TraceManager::TraceLoad(const char* trace_file_name)
{
    // Load a snapshot for viewing
    active_trace_path_ = QDir::toNativeSeparators(trace_file_name);

    open_snapshot_                            = nullptr;
    compared_snapshots_[kSnapshotCompareBase] = nullptr;
    compared_snapshots_[kSnapshotCompareDiff] = nullptr;

    // Loading regular binary RMV data
    RmtErrorCode error_code = RmtDataSetInitialize(trace_file_name, &data_set_);
    if (error_code != RMT_OK)
    {
        memset(&data_set_, 0, sizeof(RmtDataSet));
        return kTraceLoadReturnFail;
    }

    // create the default timeline for the data set.
    error_code = RmtDataSetGenerateTimeline(&data_set_, kRmtDataTimelineTypeResourceUsageVirtualSize, &timeline_);
    if (error_code != RMT_OK)
    {
        return kTraceLoadReturnFail;
    }

    return kTraceLoadReturnSuccess;
}

void TraceManager::ClearTrace()
{
    if (DataSetValid())
    {
        // clean up any cached snapshots.
        for (int32_t current_snapshot_point_index = 0; current_snapshot_point_index < data_set_.snapshot_count; ++current_snapshot_point_index)
        {
            RmtSnapshotPoint* current_snapshot_point = &data_set_.snapshots[current_snapshot_point_index];
            if (current_snapshot_point->cached_snapshot)
            {
                RmtDataSnapshotDestroy(current_snapshot_point->cached_snapshot);
                delete current_snapshot_point->cached_snapshot;
            }
        }

        RmtDataTimelineDestroy(&timeline_);
        RmtDataSetDestroy(&data_set_);
    }

    compared_snapshots_[kSnapshotCompareBase] = nullptr;
    compared_snapshots_[kSnapshotCompareDiff] = nullptr;
    open_snapshot_                            = nullptr;
    memset(&data_set_, 0, sizeof(RmtDataSet));

    active_trace_path_.clear();
    alias_model_.Clear();
}

bool TraceManager::LoadTrace(const QString& path, bool compare)
{
    bool result = false;

    QFileInfo trace_file(path);

    if (!path.isEmpty() && trace_file.exists())
    {
        // Nothing loaded, so load
        if (!DataSetValid())
        {
            // save the file location for future reference
            RMVSettings::Get().SetLastFileOpenLocation(path);

            // set up callback for when loading thread is done
            connect(&TraceManager::Get(), &TraceManager::TraceLoadComplete, &TraceManager::Get(), &TraceManager::OnTraceLoadComplete);

            loading_thread = new LoadingThread(path);
            loading_thread->start();

            result = true;
        }

        // Load up a supplemental trace for comparison
        else if (DataSetValid() && compare)
        {
            loading_thread = new LoadingThread(path);
            loading_thread->start();

            result = true;
        }

        // Fire up a new instance if desired trace is different than current
        else if (SameTrace(trace_file) == false)
        {
            // Attempt to open a new instance of RMV using the selected trace file as an argument
            const QString rmv_executable = qApp->applicationDirPath() + GetDefaultRmvName();

            // If Rmv executable does not exist, put up a message box
            QFileInfo rmv_file(rmv_executable);
            if (rmv_file.exists())
            {
                QProcess* rmv_process = new QProcess(this);
                if (rmv_process != nullptr)
                {
                    QStringList rmv_args;
                    rmv_args << path;

                    bool process_result = rmv_process->startDetached(rmv_executable, rmv_args);

                    if (!process_result)
                    {
                        // The selected trace file is missing on the disk so display a message box stating so
                        const QString text = rmv::text::kOpenRecentTraceStart + trace_file.fileName() + rmv::text::kOpenRecentTraceEnd;
                        QtCommon::QtUtils::ShowMessageBox(main_window_, QMessageBox::Ok, QMessageBox::Critical, rmv::text::kOpenRecentTraceTitle, text);
                    }
                }
            }
            else
            {
                // If the executable does not exist, put up a message box
                const QString text = rmv_executable + " does not exist";
                QtCommon::QtUtils::ShowMessageBox(main_window_, QMessageBox::Ok, QMessageBox::Critical, rmv::text::kOpenRecentTraceTitle, text);
            }
        }
        else  // reload the same file
        {
            main_window_->CloseTrace();

            // set up callback for when loading thread is done
            connect(&TraceManager::Get(), &TraceManager::TraceLoadComplete, &TraceManager::Get(), &TraceManager::OnTraceLoadComplete);

            loading_thread = new LoadingThread(path);
            loading_thread->start();

            result = true;
        }
    }
    else
    {
        // The selected trace file is missing on the disk so display a message box stating so
        QString text = rmv::text::kOpenRecentTraceStart + trace_file.fileName() + rmv::text::kOpenRecentTraceEnd;
        QtCommon::QtUtils::ShowMessageBox(main_window_, QMessageBox::Ok, QMessageBox::Critical, rmv::text::kOpenRecentTraceTitle, text);
    }

    return result;
}

void TraceManager::OnTraceLoadComplete(TraceLoadReturnCode error_code)
{
    bool remove_from_list = false;

    if (error_code != kTraceLoadReturnSuccess)
    {
        // if the trace file doesn't exist, ask the user if they want to remove it from
        // the recent traces list. This has to be done from the main thread.
        QFileInfo file_info(active_trace_path_);
        QString   text = rmv::text::kDeleteRecentTraceText.arg(file_info.fileName());

        const int ret = QtCommon::QtUtils::ShowMessageBox(
            main_window_, QMessageBox::Yes | QMessageBox::No, QMessageBox::Question, rmv::text::kDeleteRecentTraceTitle, text);

        if (ret == QMessageBox::Yes)
        {
            remove_from_list = true;
        }
    }

    if (DataSetValid())
    {
        const RmtDataSet* data_set = GetDataSet();
        if (data_set->read_only == true)
        {
            // Another instance already has the trace file opened, so pop up an OK dialog box
            const int ret = QtCommon::QtUtils::ShowMessageBox(
                main_window_, QMessageBox::Ok, QMessageBox::Warning, rmv::text::kRecentTraceAlreadyOpenedTitle, rmv::text::kRecentTraceAlreadyOpenedText);
            Q_UNUSED(ret);
        }

        RMVSettings::Get().TraceLoaded(active_trace_path_.toLatin1().data(), data_set, remove_from_list);
        RMVSettings::Get().SaveSettings();

        if (error_code == kTraceLoadReturnSuccess)
        {
            main_window_->TraceLoadComplete();
        }
    }
    main_window_->StopAnimation();

    disconnect(&TraceManager::Get(), &TraceManager::TraceLoadComplete, &TraceManager::Get(), &TraceManager::OnTraceLoadComplete);

    // defer deleting of this object until later, in case the thread is still executing something
    // under the hood and can't be deleted right now. Even though the thread may have finished working,
    // it may still have access to mutexes and deleting right now might be bad.
    loading_thread->deleteLater();
    loading_thread = nullptr;

    if (error_code != kTraceLoadReturnSuccess)
    {
        TraceManager::Get().ClearTrace();
    }
}

bool TraceManager::SameTrace(const QFileInfo& new_trace) const
{
    const QString new_trace_file_path    = QDir::toNativeSeparators(new_trace.absoluteFilePath());
    const QString active_trace_file_path = QDir::toNativeSeparators(active_trace_path_);

    return (new_trace_file_path.compare(active_trace_file_path) == 0);
}

bool TraceManager::ReadyToLoadTrace() const
{
    return (loading_thread == nullptr || loading_thread->isRunning() == false);
}

bool TraceManager::TraceValidToLoad(const QString& trace_path) const
{
    bool may_load = false;

    QFileInfo trace_file(trace_path);
    if (trace_file.exists() && trace_file.isFile())
    {
        may_load = true;
    }

    return may_load;
}

void TraceManager::SetOpenSnapshot(RmtDataSnapshot* snapshot)
{
    memset(resource_thresholds_, 0, sizeof(uint64_t) * (rmv::kSizeSliderRange + 1));

    open_snapshot_ = snapshot;

    std::vector<uint64_t> resource_sizes;

    const RmtResourceList& resource_list  = snapshot->resource_list;
    int                    resource_count = resource_list.resource_count;
    if (resource_count > 0)
    {
        resource_sizes.reserve(resource_count);

        for (int loop = 0; loop < resource_count; loop++)
        {
            const RmtResource& resource = resource_list.resources[loop];
            resource_sizes.push_back(resource.size_in_bytes);
        }
        BuildResourceSizeThresholds(resource_sizes, resource_thresholds_);
    }

    const RmtVirtualAllocationList& allocation_list  = snapshot->virtual_allocation_list;
    const int32_t                   allocation_count = allocation_list.allocation_count;
    if (allocation_count > 0)
    {
        alias_model_.Clear();
        for (int32_t loop = 0; loop < allocation_count; loop++)
        {
            alias_model_.Generate(&allocation_list.allocation_details[loop]);
        }
    }
}

void TraceManager::BuildResourceSizeThresholds(std::vector<uint64_t>& resource_sizes, uint64_t* resource_thresholds)
{
    std::stable_sort(resource_sizes.begin(), resource_sizes.end());

    float step_size = (resource_sizes.size() - 1) / static_cast<float>(rmv::kSizeSliderRange);
    float index     = 0.0F;
    for (int loop = 0; loop <= rmv::kSizeSliderRange; loop++)
    {
        resource_thresholds[loop] = resource_sizes[static_cast<int>(round(index))];
        index += step_size;
    }
}

uint64_t TraceManager::GetSizeFilterThreshold(int index) const
{
    return resource_thresholds_[index];
}

void TraceManager::SetComparedSnapshot(RmtDataSnapshot* snapshot_base, RmtDataSnapshot* snapshot_diff)
{
    compared_snapshots_[kSnapshotCompareBase] = snapshot_base;
    compared_snapshots_[kSnapshotCompareDiff] = snapshot_diff;
}

void TraceManager::SwapComparedSnapshots()
{
    RmtDataSnapshot* temp                     = compared_snapshots_[kSnapshotCompareBase];
    compared_snapshots_[kSnapshotCompareBase] = compared_snapshots_[kSnapshotCompareDiff];
    compared_snapshots_[kSnapshotCompareDiff] = temp;
}

const char* TraceManager::GetOpenSnapshotName() const
{
    return GetSnapshotName(open_snapshot_);
}

const char* TraceManager::GetCompareSnapshotName(int index) const
{
    return GetSnapshotName(compared_snapshots_[index]);
}

const char* TraceManager::GetSnapshotName(const RmtDataSnapshot* snapshot) const
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

bool TraceManager::SnapshotAlreadyOpened(const RmtDataSnapshot* snapshot) const
{
    return snapshot == open_snapshot_;
}

QString TraceManager::GetDefaultRmvName() const
{
    QString default_rmv_name;
    default_rmv_name.append(QDir::separator());
    default_rmv_name.append(rmv::kRmvExecutableBaseFilename);
#ifdef _DEBUG
    default_rmv_name.append(rmv::kRmvExecutableDebugIdentifier);
#endif

#ifdef WIN32
    // Append an extension only in Windows.
    default_rmv_name.append(".exe");
#endif

    return default_rmv_name;
}

QString TraceManager::GetTracePath() const
{
    return active_trace_path_.mid(active_trace_path_.lastIndexOf("/") + 1, active_trace_path_.length());
}

bool TraceManager::DataSetValid() const
{
    if (data_set_.file_handle == nullptr)
    {
        return false;
    }
    return true;
}

RmtDataSet* TraceManager::GetDataSet()
{
    return &data_set_;
}

RmtDataTimeline* TraceManager::GetTimeline()
{
    return &timeline_;
}

RmtDataSnapshot* TraceManager::GetOpenSnapshot() const
{
    return open_snapshot_;
}

RmtDataSnapshot* TraceManager::GetComparedSnapshot(int snapshot_id) const
{
    return compared_snapshots_[snapshot_id];
}

void TraceManager::ClearOpenSnapshot()
{
    open_snapshot_ = nullptr;
}

void TraceManager::ClearComparedSnapshots()
{
    compared_snapshots_[kSnapshotCompareBase] = nullptr;
    compared_snapshots_[kSnapshotCompareDiff] = nullptr;
}

const rmv::AliasedResourceModel& TraceManager::GetAliasModel()
{
    return alias_model_;
}
