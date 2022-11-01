//==============================================================================
// Copyright (c) 2018-2022 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the Trace Manager.
//==============================================================================

#include "managers/trace_manager.h"

#include <QtCore>
#include <QMessageBox>
#include <QByteArray>
#include <vector>

#include "qt_common/utils/qt_util.h"

#ifndef _WIN32
#include <linux/safe_crt.h>
#endif

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"
#include "rmt_error.h"
#include "rmt_virtual_allocation_list.h"

#include "managers/load_animation_manager.h"
#include "managers/message_manager.h"
#include "settings/rmv_settings.h"
#include "util/definitions.h"

namespace rmv
{
    /// @brief Spawns a thread to load a dataset.
    class LoadingThread : public QThread
    {
    public:
        /// @brief Constructor.
        ///
        /// @param [in] path The full path of the data set to load.
        explicit LoadingThread(const QString& path)
            : path_data(path.toLatin1())
        {
        }

        /// @brief Execute the loading thread.
        void run() Q_DECL_OVERRIDE
        {
            const char*               trace_path = path_data.data();
            const TraceLoadReturnCode error_code = TraceManager::Get().TraceLoad(trace_path);
            emit                      TraceManager::Get().TraceLoadThreadFinished(error_code);
        }

    private:
        QByteArray path_data;  ///< The path to the trace being loaded
    };

    /// @brief Pointer to the loading thread object.
    static LoadingThread* loading_thread = nullptr;

    /// The single instance of the trace manager.
    static TraceManager trace_manager;

    TraceManager& TraceManager::Get()
    {
        return trace_manager;
    }

    TraceManager::TraceManager(QObject* parent)
        : QObject(parent)
        , parent_(nullptr)
    {
        int id = qRegisterMetaType<TraceLoadReturnCode>();
        Q_UNUSED(id);

        ClearTrace();
    }

    TraceManager::~TraceManager()
    {
    }

    void TraceManager::Initialize(QWidget* parent)
    {
        parent_ = parent;
    }

    TraceLoadReturnCode TraceManager::TraceLoad(const char* trace_file_name)
    {
        // Load a snapshot for viewing.
        active_trace_path_ = QDir::toNativeSeparators(trace_file_name);

        rmv::SnapshotManager::Get().ClearOpenSnapshot();
        rmv::SnapshotManager::Get().ClearCompareSnapshots();

        // Loading regular binary data.
        RmtErrorCode error_code = RmtDataSetInitialize(trace_file_name, &data_set_);
        if (error_code != kRmtOk)
        {
            memset(&data_set_, 0, sizeof(RmtDataSet));
            RmtTokenClearPayloadCaches();
            return kTraceLoadReturnFail;
        }

        // Create the default timeline for the data set.
        error_code = RmtDataSetGenerateTimeline(&data_set_, kRmtDataTimelineTypeResourceUsageVirtualSize, &timeline_);
        if (error_code != kRmtOk)
        {
            return kTraceLoadReturnFail;
        }

        return kTraceLoadReturnSuccess;
    }

    void TraceManager::ClearTrace()
    {
        if (DataSetValid())
        {
            // Clean up any cached snapshots.
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

        rmv::SnapshotManager::Get().ClearOpenSnapshot();
        rmv::SnapshotManager::Get().ClearCompareSnapshots();
        memset(&data_set_, 0, sizeof(RmtDataSet));
        RmtTokenClearPayloadCaches();

        active_trace_path_.clear();
    }

    void TraceManager::LoadTrace(const QString& path)
    {
        bool result = false;

        if (TraceValidToLoad(path) == true)
        {
            QFileInfo trace_file(path);

            if (!path.isEmpty() && trace_file.exists())
            {
                // Nothing loaded, so load.
                if (!DataSetValid() && ReadyToLoadTrace())
                {
                    // Save the file location for future reference.
                    RMVSettings::Get().SetLastFileOpenLocation(path);

                    // Set up callback for when loading thread is done.
                    connect(this, &TraceManager::TraceLoadThreadFinished, this, &TraceManager::FinalizeTraceLoading);

                    loading_thread = new LoadingThread(path);
                    loading_thread->start();

                    result = true;
                }

                // Fire up a new instance if desired trace is different than current.
                else if (SameTrace(trace_file) == false)
                {
                    // Attempt to open a new instance of RMV using the selected trace file as an argument.
                    const QString executable_name = qApp->applicationDirPath() + GetDefaultExeName();

                    // If the RMV executable does not exist, put up a message box.
                    QFileInfo file(executable_name);
                    if (file.exists())
                    {
                        QProcess* process = new QProcess(this);
                        if (process != nullptr)
                        {
                            QStringList arguments;
                            arguments << path;

                            bool process_result = process->startDetached(executable_name, arguments);

                            if (!process_result)
                            {
                                // The selected trace file is missing on the disk so display a message box stating so.
                                const QString text = rmv::text::kOpenRecentTraceStart + trace_file.fileName() + rmv::text::kOpenRecentTraceEnd;
                                QtCommon::QtUtils::ShowMessageBox(parent_, QMessageBox::Ok, QMessageBox::Critical, rmv::text::kOpenRecentTraceTitle, text);
                            }
                        }
                    }
                    else
                    {
                        // If the executable does not exist, put up a message box.
                        const QString text = executable_name + " does not exist";
                        QtCommon::QtUtils::ShowMessageBox(parent_, QMessageBox::Ok, QMessageBox::Critical, rmv::text::kOpenRecentTraceTitle, text);
                    }
                }
                else  // Reload the same file.
                {
                    emit TraceClosed();

                    // Set up callback for when loading thread is done.
                    connect(this, &TraceManager::TraceLoadThreadFinished, this, &TraceManager::FinalizeTraceLoading);

                    loading_thread = new LoadingThread(path);
                    loading_thread->start();

                    result = true;
                }
            }
            else
            {
                // The selected trace file is missing on the disk so display a message box stating so.
                const QString text = rmv::text::kOpenRecentTraceStart + trace_file.fileName() + rmv::text::kOpenRecentTraceEnd;
                QtCommon::QtUtils::ShowMessageBox(parent_, QMessageBox::Ok, QMessageBox::Critical, rmv::text::kOpenRecentTraceTitle, text);
            }
        }
        else
        {
            // The selected trace file is missing on the disk so display a message box stating so.
            const QString text = rmv::text::kOpenRecentTraceStart + path + rmv::text::kOpenRecentTraceEnd;
            QtCommon::QtUtils::ShowMessageBox(parent_, QMessageBox::Ok, QMessageBox::Critical, rmv::text::kOpenRecentTraceTitle, text);
        }

        if (result == true)
        {
            rmv::LoadAnimationManager::Get().StartAnimation();
        }
    }

    void TraceManager::FinalizeTraceLoading(TraceLoadReturnCode error_code)
    {
        bool remove_from_list = false;

        if (error_code != kTraceLoadReturnSuccess)
        {
            // If the trace file doesn't exist, ask the user if they want to remove it from
            // the recent traces list. This has to be done from the main thread.
            QFileInfo file_info(active_trace_path_);
            QString   text = rmv::text::kDeleteRecentTraceText.arg(file_info.fileName());

            const int ret =
                QtCommon::QtUtils::ShowMessageBox(parent_, QMessageBox::Yes | QMessageBox::No, QMessageBox::Question, rmv::text::kDeleteRecentTraceTitle, text);

            if (ret == QMessageBox::Yes)
            {
                remove_from_list = true;
            }
        }

        bool read_only = false;
        if (DataSetValid())
        {
            const RmtDataSet* data_set = GetDataSet();
            if (data_set->read_only == true)
            {
                read_only = true;
            }

            RMVSettings::Get().TraceLoaded(active_trace_path_.toLatin1().data(), data_set, remove_from_list);
            RMVSettings::Get().SaveSettings();

            if (error_code == kTraceLoadReturnSuccess)
            {
                emit TraceOpened();
            }
        }
        rmv::LoadAnimationManager::Get().StopAnimation();

        disconnect(this, &TraceManager::TraceLoadThreadFinished, this, &TraceManager::FinalizeTraceLoading);

        // Defer deleting of this object until later, in case the thread is still executing something
        // under the hood and can't be deleted right now. Even though the thread may have finished working,
        // it may still have access to mutexes and deleting right now might be bad.
        loading_thread->deleteLater();
        loading_thread = nullptr;

        if (read_only)
        {
            // Another instance already has the trace file opened, so pop up an OK dialog box.
            const int ret = QtCommon::QtUtils::ShowMessageBox(
                parent_, QMessageBox::Ok, QMessageBox::Warning, rmv::text::kRecentTraceAlreadyOpenedTitle, rmv::text::kRecentTraceAlreadyOpenedText);
            Q_UNUSED(ret);
        }

        if (error_code != kTraceLoadReturnSuccess)
        {
            ClearTrace();
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
            const QString extension = trace_path.mid(trace_path.lastIndexOf("."), trace_path.length());

            if (extension.compare(rmv::text::kTraceFileExtension, Qt::CaseInsensitive) == 0)
            {
                may_load = true;
            }
        }

        return may_load;
    }

    QString TraceManager::GetDefaultExeName() const
    {
        QString default_exe_name;
        default_exe_name.append(QDir::separator());
        default_exe_name.append(rmv::kRmvExecutableBaseFilename);
#ifdef _DEBUG
        default_exe_name.append(rmv::kRmvExecutableDebugIdentifier);
#endif

#ifdef WIN32
        // Append an extension only in Windows.
        default_exe_name.append(".exe");
#endif

        return default_exe_name;
    }

    const QString& TraceManager::GetTracePath() const
    {
        return active_trace_path_;
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

}  // namespace rmv
