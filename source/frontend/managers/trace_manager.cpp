//==============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the Trace Manager.
//==============================================================================

#include "managers/trace_manager.h"

#include <vector>

#include <QByteArray>
#include <QMessageBox>
#include <QtCore>

#include "qt_common/utils/qt_util.h"

#ifndef _WIN32
#include "linux/safe_crt.h"
#endif

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"
#include "rmt_error.h"
#include "rmt_virtual_allocation_list.h"

#include "managers/load_animation_manager.h"
#include "managers/message_manager.h"
#include "settings/rmv_settings.h"
#include "util/definitions.h"
#include "util/rmv_util.h"

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
            : trace_path_(path)
        {
        }

        /// @brief Execute the loading thread.
        void run() Q_DECL_OVERRIDE
        {
            const TraceLoadReturnCode error_code = TraceManager::Get().TraceLoad(trace_path_);
            emit                      TraceManager::Get().TraceLoadThreadFinished(error_code);
        }

    private:
        QString trace_path_;  ///< The path to the trace being loaded
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

    static void ErrorReporter(RmtDataSet* data_set, const RmtErrorCode error_code, RmtErrorResponseCode* out_error_response)
    {
        RMT_ASSERT(out_error_response != nullptr);

        if ((data_set == nullptr) || (out_error_response == nullptr))
        {
            return;
        }

        switch (error_code)
        {
        case kRmtErrorFileAccessFailed:
        {
            const int user_response = QtCommon::QtUtils::ShowMessageBox(nullptr,
                                                                        QMessageBox::Retry | QMessageBox::Ignore,
                                                                        QMessageBox::Warning,
                                                                        rmv::text::kCommitEditsFailedTitle,
                                                                        rmv::text::kCommitEditsFailedText.arg(data_set->file_path));

            if (user_response == QMessageBox::Retry)
            {
                *out_error_response = RmtErrorResponseCode::RmtErrorResponseCodeRetry;
            }
            else if (user_response == QMessageBox::Ignore)
            {
                *out_error_response = RmtErrorResponseCode::RmtErrorResponseCodeIgnore;
            }
            break;
        }

        default:
            *out_error_response = RmtErrorResponseCode::RmtErrorResponseCodeNone;
            break;
        }
    }

    TraceManager::~TraceManager()
    {
    }

    void TraceManager::Initialize(QWidget* parent)
    {
        parent_ = parent;
    }

    TraceLoadReturnCode TraceManager::TraceLoad(const QString& trace_file_name)
    {
        // Load a snapshot for viewing.
        active_trace_path_ = QDir::toNativeSeparators(trace_file_name);

        rmv::SnapshotManager::Get().ClearOpenSnapshot();
        rmv::SnapshotManager::Get().ClearCompareSnapshots();

        const RmtErrorCode return_code = RmtTraceLoaderTraceLoad(trace_file_name.toLatin1(), ErrorReporter);
        if (return_code != kRmtOk)
        {
            if (return_code == kRmtErrorPageTableSizeExceeded)
            {
                return kTraceLoadReturnOutOfVirtualGPUMemory;
            }
            else if (return_code == kRmtErrorTraceFileNotSupported)
            {
                return kTraceLoadReturnFileNotSupported;
            }
            return kTraceLoadReturnFail;
        }

        return kTraceLoadReturnSuccess;
    }

    void TraceManager::ClearTrace()
    {
        RmtTraceLoaderClearTrace();

        rmv::SnapshotManager::Get().ClearOpenSnapshot();
        rmv::SnapshotManager::Get().ClearCompareSnapshots();
        active_trace_path_.clear();
    }

    void TraceManager::LoadTrace(const QString& path)
    {
        bool result = false;

        if (rmv_util::TraceValidToLoad(path) == true)
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
            // If there's an error loading the trace and it is already in the recent traces list,
            // ask the user if they want to remove it. This has to be done from the main thread.
            QFileInfo file_info(active_trace_path_);
            QString   text(rmv::text::kOpenTraceErrorText.arg(file_info.fileName()));

            if (error_code == kTraceLoadReturnOutOfVirtualGPUMemory)
            {
                text += rmv::text::kOpenTraceOutOfVirtualGPUMemory;
            }
            else if (error_code == kTraceLoadReturnFileNotSupported)
            {
                text += rmv::text::kOpenTraceFileNotSupported;
            }

            if (RMVSettings::Get().DoesFileExistInRecentList(active_trace_path_.toStdString().c_str()))
            {
                text += rmv::text::kDeleteRecentTraceText;

                const int ret = QtCommon::QtUtils::ShowMessageBox(
                    parent_, QMessageBox::Yes | QMessageBox::No, QMessageBox::Question, rmv::text::kDeleteRecentTraceTitle, text);

                if (ret == QMessageBox::Yes)
                {
                    // Remove the file from the recent file list.
                    RMVSettings::Get().TraceLoaded(active_trace_path_, nullptr, true);
                    RMVSettings::Get().SaveSettings();

                    // Notify the view to refresh the list.
                    emit TraceOpenFailed();
                }
            }
            else
            {
                QtCommon::QtUtils::ShowMessageBox(parent_, QMessageBox::Ok, QMessageBox::Warning, rmv::text::kOpenTraceErrorTitle, text);
            }
        }

        bool read_only = false;
        if (DataSetValid())
        {
            const RmtDataSet* data_set = GetDataSet();
            if (data_set->flags.read_only == true)
            {
                read_only = true;
            }

            RMVSettings::Get().TraceLoaded(active_trace_path_, data_set, remove_from_list);
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
        return RmtTraceLoaderDataSetValid();
    }

    RmtDataSet* TraceManager::GetDataSet()
    {
        return RmtTraceLoaderGetDataSet();
    }

    RmtDataTimeline* TraceManager::GetTimeline()
    {
        return RmtTraceLoaderGetTimeline();
    }

}  // namespace rmv
