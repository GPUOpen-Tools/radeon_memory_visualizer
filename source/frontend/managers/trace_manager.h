//==============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Class definition for the Trace Manager.
//==============================================================================

#ifndef RMV_MANAGERS_TRACE_MANAGER_H_
#define RMV_MANAGERS_TRACE_MANAGER_H_

#include <QFileInfo>
#include <QObject>
#include <QVector>

#include "rmt_data_set.h"

#include "managers/snapshot_manager.h"

Q_DECLARE_METATYPE(RmtErrorCode);

/// @brief Enum of trace loading thread return codes.
enum TraceLoadReturnCode
{
    kTraceLoadReturnError,
    kTraceLoadReturnSuccess,
    kTraceLoadReturnFail,
    kTraceLoadReturnAlreadyOpened
};

Q_DECLARE_METATYPE(TraceLoadReturnCode)

namespace rmv
{
    /// @brief This class owns and manages growth and updating of the dataset.
    class TraceManager : public QObject
    {
        Q_OBJECT

    public:
        /// @brief Constructor.
        ///
        /// @param [in] parent The parent widget.
        explicit TraceManager(QObject* parent = nullptr);

        /// @brief Destructor.
        virtual ~TraceManager();

        /// @brief Accessor for singleton instance.
        static TraceManager& Get();

        /// @brief Initialize the trace manager.
        ///
        /// @param [in] main_window Pointer to main window widget. Used as the parent for
        ///  pop up message boxes.
        void Initialize(QWidget* main_window);

        /// @brief Determine if we're ready to load a trace.
        ///
        /// @return true if ready.
        bool ReadyToLoadTrace() const;

        /// @brief Load a trace into memory.
        ///
        /// Note: This function runs in a separate thread so doesn't have access to
        /// anything QT-related (including the Debug Window).
        ///
        /// @param [in] trace_file_name The name of the trace file.
        ///
        /// @return An error code returned from the loading thread.
        TraceLoadReturnCode TraceLoad(const char* trace_file_name);

        /// @brief Return whether a trace may be loaded.
        ///
        /// @param [in] trace_path the path to the trace.
        ///
        /// @return true if we may attempt an actual trace load.
        bool TraceValidToLoad(const QString& trace_path) const;

        /// @brief Clear a trace from memory.
        ///
        /// This function should effectively clean up the ActiveTraceData struct.
        void ClearTrace();

        /// @brief Get the full path to the trace file.
        ///
        /// @return The trace path.
        const QString& GetTracePath() const;

        /// @brief Is the data set valid, meaning does it contain a valid trace.
        ///
        /// @return true if data set is valid, false if not.
        bool DataSetValid() const;

        /// @brief Get a pointer to the loaded data set.
        ///
        /// @return The data set.
        RmtDataSet* GetDataSet();

        /// @brief Get a pointer to the timeline.
        ///
        /// @return The timeline.
        RmtDataTimeline* GetTimeline();

    public slots:
        /// @brief Load a trace.
        ///
        /// @param [in] path The path to the trace file.
        void LoadTrace(const QString& path);

    signals:
        /// @brief Signal to indicate that the trace loading thread has finished.
        ///
        /// @param [in] error_code The error code returned from the loader.
        void TraceLoadThreadFinished(TraceLoadReturnCode error_code);

        /// @brief Signal to indicate that a trace file has been loaded and opened
        /// and is ready to show in the UI.
        void TraceOpened();

        /// @brief Signal to indicate that a trace file has been closed and should be
        /// disabled in the UI.
        void TraceClosed();

    private slots:
        /// @brief Finalize the trace loading process.
        ///
        /// Destroy loading thread, evaluate thread loading error codes and inform the
        /// UI via a signal that the trace is ready to be viewed.
        ///
        /// @param [in] error_code An error code from the loading thread indicating if load was successful.
        void FinalizeTraceLoading(TraceLoadReturnCode error_code);

    private:
        /// @brief Compare a trace with one that is already open.
        ///
        /// @param [in] new_trace path to trace to compare with.
        ///
        /// @return true if both traces are the same.
        bool SameTrace(const QFileInfo& new_trace) const;

        /// @brief Get the default executable name (OS-aware).
        ///
        /// @return The default name string.
        QString GetDefaultExeName() const;

        RmtDataSet      data_set_ = {};      ///< The dataset read from file.
        RmtDataTimeline timeline_;           ///< The timeline.
        QWidget*        parent_;             ///< Pointer to the parent pane.
        QString         active_trace_path_;  ///< The path to currently opened file.
    };
}  // namespace rmv

#endif  // RMV_MANAGERS_TRACE_MANAGER_H_
