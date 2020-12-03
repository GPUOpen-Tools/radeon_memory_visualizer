//==============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Class definition for the RMV Trace Manager.
//==============================================================================

#ifndef RMV_MODELS_TRACE_MANAGER_H_
#define RMV_MODELS_TRACE_MANAGER_H_

#include <QFileInfo>
#include <QObject>
#include <QVector>

#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"

#include "models/aliased_resource_model.h"
#include "models/snapshot_manager.h"
#include "util/definitions.h"
#include "views/main_window.h"

Q_DECLARE_METATYPE(RmtErrorCode);

/// Enum of trace loading thread return codes.
enum TraceLoadReturnCode
{
    kTraceLoadReturnError,
    kTraceLoadReturnSuccess,
    kTraceLoadReturnFail,
    kTraceLoadReturnAlreadyOpened
};

Q_DECLARE_METATYPE(TraceLoadReturnCode)

/// This class owns and manages growth and updating of the dataset.
class TraceManager : public QObject
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The parent widget.
    explicit TraceManager(QObject* parent = nullptr);

    /// Destructor.
    ~TraceManager();

    /// Accessor for singleton instance.
    static TraceManager& Get();

    /// Initialize the trace manager.
    /// \param main_window Pointer to main window widget. Used as the parent for
    /// pop up message boxes.
    void Initialize(MainWindow* main_window);

    /// Determine if we're ready to load a trace.
    /// \return true if ready.
    bool ReadyToLoadTrace() const;

    /// Load a trace into memory. Note: This function runs in a separate thread so
    /// doesn't have access to anything QT-related (including the Debug Window).
    /// \param trace_file_name the name of the RMV trace file.
    /// \return A LoadTraceErrorCode.
    TraceLoadReturnCode TraceLoad(const char* trace_file_name);

    /// Update the currently active snapshot.
    /// \param snapshot The snapshot to open.
    void SetOpenSnapshot(RmtDataSnapshot* snapshot);

    /// Update the currently active snapshot.
    /// \param snapshot_base The base snapshot to compare.
    /// \param snapshot_diff The snapshot to compare with the base snapshot.
    void SetComparedSnapshot(RmtDataSnapshot* snapshot_base, RmtDataSnapshot* snapshot_diff);

    /// Swap the comparison snapshots.
    void SwapComparedSnapshots();

    /// Return whether a trace may be loaded.
    /// \param trace_path the path to the trace.
    /// \return true if we may attempt an actual trace load.
    bool TraceValidToLoad(const QString& trace_path) const;

    /// Clear a trace from memory.
    /// This function should effectively clean up the ActiveTraceData struct.
    void ClearTrace();

    /// Get the snapshot name from a snapshot. Prefer the name from the snapshot point.
    /// If that doesn't exist, use the name from the snapshot itself.
    /// \return The snapshot name.
    const char* GetOpenSnapshotName() const;

    /// Get the snapshot name from a snapshot. Prefer the name from the snapshot point.
    /// If that doesn't exist, use the name from the snapshot itself.
    /// \param index The compare snapshot index.
    /// \return The snapshot name.
    const char* GetCompareSnapshotName(int index) const;

    /// Is a snapshot already opened.
    /// \param snapshot The snapshot to compare.
    /// \return true if opened, false if not.
    bool SnapshotAlreadyOpened(const RmtDataSnapshot* snapshot) const;

    /// Build a list of resource thresholds used by the 'filter by size' slider.
    /// \param resource_sizes An vector containing a list of resource sizes.
    /// \param resource_thresholds An array to accept the resource thresholds.
    void BuildResourceSizeThresholds(std::vector<uint64_t>& resource_sizes, uint64_t* resource_thresholds);

    /// Get the 'filter by size' value based on where the slider is. The filtering
    /// used by the 'filter by size' slider is non-linear so this uses a lookup
    /// to get the threshold values used to compare the resource sizes depending on
    /// where the slider value is.
    /// \param index The index position of the slider.
    /// \return The value of the filter threshold used for comparison.
    uint64_t GetSizeFilterThreshold(int index) const;

    /// Get the full path to the trace file.
    /// \return The trace path.
    QString GetTracePath() const;

    /// Is the data set valid, meaning does it contain a valid trace.
    /// \return true if data set is valid, false if not.
    bool DataSetValid() const;

    /// Get a pointer to the loaded data set.
    /// \return The data set.
    RmtDataSet* GetDataSet();

    /// Get a pointer to the timeline.
    /// \return The timeline.
    RmtDataTimeline* GetTimeline();

    /// Get a pointer to the opened snapshot.
    /// \return The opened snapshot.
    RmtDataSnapshot* GetOpenSnapshot() const;

    /// Get a pointer to a comparison snapshot.
    /// \param snapshot_id The snapshot id, either base or diff.
    /// \return The snapshot.
    RmtDataSnapshot* GetComparedSnapshot(int snapshot_id) const;

    /// Clear the opened snapshot.
    void ClearOpenSnapshot();

    /// Clear the comparison snapshots.
    void ClearComparedSnapshots();

    /// Get the model responsible for managing resource aliasing.
    /// \return The alias model.
    const rmv::AliasedResourceModel& GetAliasModel();

public slots:
    /// Load an RMV trace.
    /// \param path path to the trace file.
    /// \param compare this trace should be used for comparison.
    /// \return true if the trace path is valid.
    bool LoadTrace(const QString& path, bool compare);

signals:
    /// Signal to indicate that the snapshot has completed loading.
    /// \param error_code The error code returned from the loader.
    void TraceLoadComplete(TraceLoadReturnCode error_code);

private slots:
    /// Kill loading thread and emit a signal saying loading completed.
    /// \param error_code process the error code.
    void OnTraceLoadComplete(TraceLoadReturnCode error_code);

private:
    /// Compare a trace with one that is already open.
    /// \param new_trace path to trace to compare with.
    /// \return true if both traces are the same.
    bool SameTrace(const QFileInfo& new_trace) const;

    /// Get the snapshot name from a snapshot. Prefer the name from the snapshot point.
    /// If that doesn't exist, use the name from the snapshot itself.
    /// \param snapshot The snapshot.
    /// \return The snapshot name.
    const char* GetSnapshotName(const RmtDataSnapshot* snapshot) const;

    /// Get the default RMV name (OS-aware).
    /// \return The default name string.
    QString GetDefaultRmvName() const;

    RmtDataSet                data_set_ = {};                                   ///< The dataset read from file.
    RmtDataTimeline           timeline_;                                        ///< The timeline.
    RmtDataSnapshot*          open_snapshot_;                                   ///< A pointer to the open snapshot.
    RmtDataSnapshot*          compared_snapshots_[kSnapshotCompareCount];       ///< A pointer to the compared snapshot.
    MainWindow*               main_window_;                                     ///< Pointer to the main window.
    QString                   active_trace_path_;                               ///< The path to currently opened file.
    uint64_t                  resource_thresholds_[rmv::kSizeSliderRange + 1];  ///< List of resource size thresholds for the filter by size sliders.
    rmv::AliasedResourceModel alias_model_;                                     ///< The model used for showing aliased resources.
};
#endif  // RMV_MODELS_TRACE_MANAGER_H_
