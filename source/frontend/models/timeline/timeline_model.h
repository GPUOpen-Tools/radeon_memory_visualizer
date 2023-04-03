//=============================================================================
// Copyright (c) 2018-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Timeline model.
//=============================================================================

#ifndef RMV_MODELS_TIMELINE_TIMELINE_MODEL_H_
#define RMV_MODELS_TIMELINE_TIMELINE_MODEL_H_

#include <QTableView>

#include "qt_common/utils/model_view_mapper.h"

#include "rmt_data_snapshot.h"
#include "rmt_data_timeline.h"
#include "rmt_job_system.h"

#include "models/proxy_models/snapshot_timeline_proxy_model.h"
#include "models/timeline/snapshot_item_model.h"
#include "util/definitions.h"
#include "util/thread_controller.h"

namespace rmv
{
    /// @brief UI widgets that are updated by the model.
    enum TimelineModelWidgets
    {
        kTimelineSnapshotCount,

        kTimelineNumWidgets,
    };

    /// @brief Container class that holds model data for the snapshot generation (timeline) pane.
    class TimelineModel : public ModelViewMapper
    {
    public:
        /// @brief Constructor.
        explicit TimelineModel();

        /// @brief Destructor.
        virtual ~TimelineModel();

        /// @brief Initialize the table model.
        ///
        /// @param [in] table_view  The view to the table.
        /// @param [in] num_rows    Total rows of the table.
        /// @param [in] num_columns Total columns of the table.
        void InitializeTableModel(QTableView* table_view, uint num_rows, uint num_columns);

        /// @brief Call the backend to create the graphical representation of the timeline.
        ///
        /// @param [in] timeline_type The timeline type.
        void GenerateTimeline(RmtDataTimelineType timeline_type);

        /// @brief Update the model.
        void Update();

        /// @brief Add a new snapshot.
        ///
        /// Create the snapshot name and call the backend function to add the
        /// snapshot to the trace file. Also generates a RmtSnapshotPoint
        /// structure containing useful information about the snapshot that is
        /// displayed in the snapshot table.
        ///
        /// @param [in] snapshot_time The time the snapshot was taken.
        ///
        /// @return An RmtSnapshotPoint structure containing data that is used to
        ///  populate the snapshot table, or nullptr if error.
        RmtSnapshotPoint* AddSnapshot(uint64_t snapshot_time);

        /// @brief Remove a snapshot from the model.
        ///
        /// @param [in] snapshot_point The snapshot to remove.
        void RemoveSnapshot(RmtSnapshotPoint* snapshot_point);

        /// @brief Get number of rows in the snapshot table.
        ///
        /// @return number of visible rows.
        int RowCount();

        /// @brief Handle what happens when user changes the filter.
        ///
        /// @param [in] filter The filter string to search for.
        void SearchBoxChanged(const QString& filter);

        /// @brief Handle what happens when user changes the size filter.
        ///
        /// @param [in] min_value The minimum size.
        /// @param [in] max_value The maximum size.
        void FilterBySizeChanged(int min_value, int max_value);

        /// @brief Update the memory graph on the timeline.
        ///
        /// Recalculate the height data for the buckets depending on the current zoom
        /// level and offset into the timeline.
        /// This needs to be called when the user changes zoom level or scrolls around
        /// the timeline.
        ///
        /// @param [in] min_visible The minimum slider value.
        /// @param [in] max_visible The maximum slider value.
        void UpdateMemoryGraph(uint64_t min_visible, uint64_t max_visible);

        /// @brief Initialize blank data for the model.
        void ResetModelValues();

        /// @brief Get content from proxy model.
        ///
        /// @param [in] row The table row.
        /// @param [in] col The table column.
        ///
        /// @return The user role data at row, col.
        qulonglong GetProxyData(int row, int col);

        /// @brief Function to return histogram data.
        ///
        /// @param [in]  bucket_group_index The sub-bucket index.
        /// @param [in]  bucket_index       The bucket index.
        /// @param [out] out_y_pos          The y position offset for this bucket and sub-bucket.
        /// @param [out] out_height         The height for this bucket and sub-bucket.
        /// @return true if bucket/sub-bucket is valid, false if not.
        bool GetHistogramData(int bucket_group_index, int bucket_index, qreal& out_y_pos, qreal& out_height);

        /// @brief Get the number of buckets.
        ///
        /// @return The number of buckets.
        int GetNumBuckets() const;

        /// @brief Get the number of grouping modes.
        ///
        /// @return The number of grouping modes.
        int GetNumBucketGroups() const;

        /// @brief Set the timeline type.
        ///
        /// @param [in] new_timeline_type The new timeline type.
        void SetTimelineType(RmtDataTimelineType new_timeline_type);

        /// @brief Get the tooltip string for the timeline.
        ///
        /// @param [in] x_pos The x position of the mouse on the scene in logical coordinates
        ///  (0.0 is left scene bound, 1.0 is right scene bound).
        /// @param [in] text_string A string to receive the tooltip text.
        /// @param [in] color_string A string to receive the tooltip colors.
        ///
        /// @return true if strings are valid, false otherwise (not over valid data).
        bool GetTimelineTooltipInfo(qreal x_pos, QString& text_string, QString& color_string);

        /// @brief Get the proxy model. Used to set up a connection between the table being sorted and the UI update.
        ///
        /// @return the proxy model.
        SnapshotTimelineProxyModel* GetProxyModel() const;

        /// @brief Validate the time units.
        ///
        /// Usually called after a trace is loaded. If the timestamps are invalid for some reason,
        /// use clocks to show timings and don't allow the user to toggle the time units.
        void ValidateTimeUnits() const;

        /// @brief Get the maximum timestamp in the currently loaded trace.
        ///
        /// @return the maximum timestamp, or 0 if the timestamp is invalid.
        uint64_t GetMaxTimestamp() const;

        /// @brief Create a worker thread to process the backend data and build the timeline.
        ///
        /// @param [in] timeline_type The type of timeline to create.
        ///
        /// @return A pointer to the worker thread object.
        BackgroundTask* CreateWorkerThread(RmtDataTimelineType timeline_type);

    private slots:
        /// @brief Handle what happens when the model data changes.
        ///
        /// Used to capture snapshot renaming.
        ///
        /// @param [in] top_left The top left model index in the table that changed.
        /// @param [in] bottom_right The bottom right model index in the table that changed.
        void OnModelChanged(const QModelIndex& top_left, const QModelIndex& bottom_right);

    private:
        /// @brief Get a value as a string.
        ///
        /// @param [in] value The value to convert.
        /// @param [in] display_as_memory If true, display the value as an amount of memory (ie KB, MB etc).
        ///
        /// @return The value as a string.
        QString GetValueString(int64_t value, bool display_as_memory) const;

        /// @brief Get the resource-specific tool tip Info.
        ///
        /// Sort the resources into numerical order and show details in the tooltip (color swatch and text).
        ///
        /// @param [in]  bucket_index The bucket index.
        /// @param [in]  display_as_memory If true, display the value as an amount of memory (ie KB, MB etc).
        /// @param [out] text_string A string to receive the tooltip text.
        /// @param [out] color_string A string to receive the tooltip colors.
        void GetResourceTooltipInfo(int bucket_index, bool display_as_memory, QString& text_string, QString& color_string);

        SnapshotItemModel*          table_model_;    ///< Holds snapshot table data.
        SnapshotTimelineProxyModel* proxy_model_;    ///< Table proxy.
        uint64_t                    min_visible_;    ///< Minimum visible timestamp.
        uint64_t                    max_visible_;    ///< Maximum visible timestamp.
        RmtDataTimelineHistogram    histogram_;      ///< The histogram to render.
        RmtDataTimelineType         timeline_type_;  ///< The timeline type.
        RmtJobQueue                 job_queue_;      ///< The job queue.
    };
}  // namespace rmv

#endif  // RMV_MODELS_TIMELINE_TIMELINE_MODEL_H_
