//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Model header for the Timeline pane
//=============================================================================

#ifndef RMV_MODELS_TIMELINE_TIMELINE_MODEL_H_
#define RMV_MODELS_TIMELINE_TIMELINE_MODEL_H_

#include <QTableView>

#include "qt_common/utils/model_view_mapper.h"

#include "rmt_data_set.h"
#include "rmt_data_timeline.h"

#include "models/proxy_models/snapshot_timeline_proxy_model.h"
#include "models/timeline/snapshot_item_model.h"
#include "views/custom_widgets/rmv_timeline_tooltip.h"
#include "util/definitions.h"
#include "util/thread_controller.h"

class MainWindow;

namespace rmv
{
    /// UI widgets that are updated by the model.
    enum TimelineModelWidgets
    {
        kTimelineSnapshotCount,

        kTimelineNumWidgets,
    };

    /// Container class that holds model data for a given pane.
    class TimelineModel : public ModelViewMapper
    {
    public:
        /// Constructor.
        explicit TimelineModel();

        /// Destructor.
        virtual ~TimelineModel();

        /// Initialize the table model.
        /// \param table_view The view to the table.
        /// \param num_rows Total rows of the table.
        /// \param num_columns Total columns of the table.
        void InitializeTableModel(QTableView* table_view, uint num_rows, uint num_columns);

        /// Call the backend to create the graphical representation of the timeline.
        /// \param timeline_type The timeline type.
        void GenerateTimeline(RmtDataTimelineType timeline_type);

        /// Read the dataset and update model.
        void Update();

        /// Add a new snapshot. Create the snapshot name and call the backend
        /// function to add the snapshot to the trace file. Also generates a
        /// RmtSnapshotPoint structure containing useful information about the
        /// snapshot that is displayed in the snapshot table.
        /// \param snapshot_time The time the snapshot was taken.
        /// \return An RmtSnapshotPoint structure containing data that is used to.
        /// populate the snapshot table, or nullptr if error.
        RmtSnapshotPoint* AddSnapshot(uint64_t snapshot_time);

        /// Remove a snapshot from the model.
        /// \param snapshot_point The snapshot to remove.
        void RemoveSnapshot(RmtSnapshotPoint* snapshot_point);

        /// Get number of rows in the snapshot table.
        /// \return number of visible rows.
        int RowCount();

        /// Handle what happens when user changes the filter.
        /// \param filter The filter string to search for.
        void SearchBoxChanged(const QString& filter);
        /// Handle what happens when user changes the size filter.
        /// \param min_value The minimum size.
        /// \param max_value The maximum size.

        void FilterBySizeChanged(int min_value, int max_value);
        /// Update the memory graph on the timeline. Recalculate the height data for
        /// the buckets depending on the current zoom level and offset into the
        /// timeline.
        /// This needs to be called when the user changes zoom level or scrolls around
        /// the timeline.
        /// \param [in] min_visible The minimum slider value.
        /// \param [in] max_visible The maximum slider value.
        void UpdateMemoryGraph(uint64_t min_visible, uint64_t max_visible);

        /// Initialize blank data for the model.
        void ResetModelValues();

        /// Get content from proxy model.
        /// \param row The table row.
        /// \param col The table column.
        /// \return The user role data at row, col.
        qulonglong GetProxyData(int row, int col);

        /// Function to return histogram data.
        /// \param [in] bucket_group_index The sub-bucket index.
        /// \param [in] bucket_index The bucket index.
        /// \param [out] out_y_pos The y position offset for this bucket and sub-bucket.
        /// \param [out] out_height The height for this bucket and sub-bucket.
        /// \return true if bucket/sub-bucket is valid, false if not.
        bool GetHistogramData(int bucket_group_index, int bucket_index, qreal& out_y_pos, qreal& out_height);

        /// Get the number of buckets.
        /// \return The number of buckets.
        int GetNumBuckets() const;

        /// Get the number of grouping modes.
        /// \return The number of grouping modes.
        int GetNumBucketGroups() const;

        /// Set the timeline type.
        /// \param new_timeline_type The new timeline type.
        void SetTimelineType(RmtDataTimelineType new_timeline_type);

        /// Get the tooltip string for the timeline.
        /// \param x_pos The x position of the mouse on the scene in logical coordinates
        /// (0.0 is left scene bound, 1.0 is right scene bound).
        /// \param tooltip_info_list A list of structs to receive the tooltip text and color.
        /// \return true if string is valid, false otherwise (not over valid data).
        bool GetTimelineTooltipInfo(qreal x_pos, QList<TooltipInfo>& tooltip_info_list);

        /// Get the proxy model. Used to set up a connection between the table being sorted and the UI update.
        /// \return the proxy model.
        SnapshotTimelineProxyModel* GetProxyModel() const;

        /// Validate the time units. Usually called after a trace is loaded. If the timestamps are invalid for some
        /// reason, use clocks to show timings and don't allow the user to toggle the time units.
        void ValidateTimeUnits() const;

        /// Get the maximum timestamp in the currently loaded trace.
        /// \return the maximum timestamp, or 0 if the timestamp is invalid.
        uint64_t GetMaxTimestamp() const;

    private slots:
        /// Handle what happens when the model data changes. Used to capture snapshot renaming.
        /// \param top_left The top left model index in the table that changed.
        /// \param bottom_right The bottom right model index in the table that changed.
        void OnModelChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

    private:
        /// Get a value as a string.
        /// \param value The value to convert.
        /// \param display_as_memory If true, display the value as an amount of memory (ie KB, MB etc).
        /// \return The value as a string.
        QString GetValueString(int64_t value, bool display_as_memory) const;

        /// Get the resource-specific tool tip Info. Sort the resources into numerical order and show details in the
        /// tooltip (color swatch and text).
        /// \param bucket_index The bucket index.
        /// \param display_as_memory If true, display the value as an amount of memory (ie KB, MB etc).
        /// \param tooltip_info_list A list of structs to receive the tooltip text and color.
        void GetResourceTooltipInfo(int bucket_index, bool display_as_memory, QList<TooltipInfo>& tooltip_info_list);

        SnapshotItemModel*          table_model_;    ///< Holds snapshot table data.
        SnapshotTimelineProxyModel* proxy_model_;    ///< Table proxy.
        uint64_t                    min_visible_;    ///< Minimum visible timestamp.
        uint64_t                    max_visible_;    ///< Maximum visible timestamp.
        RmtDataTimelineHistogram    histogram_;      ///< The histogram to render.
        RmtDataTimelineType         timeline_type_;  ///< The timeline type.
    };
}  // namespace rmv

#endif  // RMV_MODELS_TIMELINE_TIMELINE_MODEL_H_
