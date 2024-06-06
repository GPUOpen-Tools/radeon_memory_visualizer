//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the Timeline model.
//=============================================================================

#include "models/timeline/timeline_model.h"

#include "qt_common/utils/common_definitions.h"
#include "qt_common/utils/qt_util.h"

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"
#include "rmt_print.h"
#include "rmt_util.h"

#include "managers/trace_manager.h"
#include "models/colorizer.h"
#include "models/resource_sorter.h"
#include "settings/rmv_settings.h"
#include "util/log_file_writer.h"
#include "util/string_util.h"
#include "util/time_util.h"

#ifndef _WIN32
#include <linux/safe_crt.h>
#endif

namespace rmv
{
    // The tooltip string to display if no resources are selected.
    static const QString kNoResourcesSelected = "No resources selected";

    /// @brief Worker class definition to do the processing of the timeline generation
    /// on a separate thread.
    class TimelineWorker : public rmv::BackgroundTask
    {
    public:
        /// @brief Constructor.
        ///
        /// @param [in] model         The model containing the timeline data to work with.
        /// @param [in] timeline_type The type of timeline being generated.
        explicit TimelineWorker(rmv::TimelineModel* model, RmtDataTimelineType timeline_type, const uint32_t filter_mask)
            : BackgroundTask(timeline_type == RmtDataTimelineType::kRmtDataTimelineTypeResourceUsageVirtualSize)
            , model_(model)
            , timeline_type_(timeline_type)
            , filter_mask_(filter_mask)
        {
        }

        /// @brief Destructor.
        ~TimelineWorker()
        {
        }

        /// @brief Worker thread function.
        virtual void ThreadFunc()
        {
            model_->GenerateTimeline(timeline_type_, filter_mask_);
        }

        virtual void Cancel()
        {
            model_->CancelBackgroundTask();
        }

    private:
        rmv::TimelineModel* model_;          ///< Pointer to the model data.
        RmtDataTimelineType timeline_type_;  ///< The timeline type.
        uint64_t            filter_mask_;    ///< A bit mask used to show or hide series on the timeline (true = show, false = hide).
    };

    // Thread count for the job queue.
    static const int32_t kThreadCount = 8;

    // The number of buckets used for the timeline graph. This tan be visualized as taking
    // the whole timeline display and slicing it vertically into this number of buckets.
    static const int32_t kNumBuckets = 500;

    // The maximum number of lines of info to show in the timeline tooltip.
    static const int kMaxTooltipLines = 6;

    TimelineModel::TimelineModel()
        : ModelViewMapper(kTimelineNumWidgets)
        , table_model_(nullptr)
        , proxy_model_(nullptr)
        , min_visible_(0)
        , max_visible_(0)
        , histogram_{}
        , timeline_type_(kRmtDataTimelineTypeResourceUsageVirtualSize)
        , timeline_series_filter_(UINT32_MAX)
        , is_timeline_generation_in_progress(false)
    {
        const RmtErrorCode error_code = RmtJobQueueInitialize(&job_queue_, kThreadCount);
        RMT_ASSERT(error_code == kRmtOk);
    }

    TimelineModel::~TimelineModel()
    {
        delete table_model_;
        RmtJobQueueShutdown(&job_queue_);
    }

    void TimelineModel::ResetModelValues()
    {
        table_model_->removeRows(0, table_model_->rowCount());
        table_model_->SetRowCount(0);
        proxy_model_->invalidate();

        // Reset the series filter so that all strips appear on the timeline.
        timeline_series_filter_ = UINT32_MAX;

        SetModelData(kTimelineSnapshotCount, "-");
    }

    void TimelineModel::GenerateTimeline(RmtDataTimelineType timeline_type, const uint32_t filter_mask)
    {
        TraceManager& trace_manager = TraceManager::Get();

        if (trace_manager.DataSetValid())
        {
            TimelineGenerationBegin();

            // Recreate the timeline for the data set.
            RmtDataSet*      data_set   = trace_manager.GetDataSet();
            RmtDataTimeline* timeline   = trace_manager.GetTimeline();
            RmtErrorCode     error_code = RmtDataTimelineDestroy(timeline);
            RMT_UNUSED(error_code);
            RMT_ASSERT_MESSAGE(error_code == kRmtOk, "Error destroying old timeline");
            error_code = RmtDataSetGenerateTimeline(data_set, timeline_type, timeline);
            RMT_UNUSED(error_code);
            RMT_ASSERT_MESSAGE(error_code == kRmtOk, "Error generating new timeline type");
            TimelineGenerationEnd();
            SetTimelineSeriesFilter(filter_mask, timeline);
        }
    }

    void TimelineModel::Update()
    {
        ResetModelValues();

        TraceManager& trace_manager = TraceManager::Get();
        if (trace_manager.DataSetValid())
        {
            int32_t snapshot_count = RmtTraceLoaderGetSnapshotCount();
            table_model_->SetRowCount(snapshot_count);
            SetModelData(kTimelineSnapshotCount, rmv::string_util::LocalizedValue(snapshot_count));
        }

        proxy_model_->invalidate();
    }

    void TimelineModel::InitializeTableModel(QTableView* table_view, uint num_rows, uint num_columns)
    {
        if (table_model_ == nullptr)
        {
            table_model_ = new SnapshotItemModel();
            table_model_->SetRowCount(num_rows);
            table_model_->SetColumnCount(num_columns);

            proxy_model_ = new SnapshotTimelineProxyModel();

            proxy_model_->setSourceModel(table_model_);
            proxy_model_->SetFilterKeyColumns({kSnapshotTimelineColumnName,
                                               kSnapshotTimelineColumnTime,
                                               kSnapshotTimelineColumnVirtualAllocations,
                                               kSnapshotTimelineColumnResources,
                                               kSnapshotTimelineColumnAllocatedTotalVirtualMemory,
                                               kSnapshotTimelineColumnAllocatedBoundVirtualMemory,
                                               kSnapshotTimelineColumnAllocatedUnboundVirtualMemory,
                                               kSnapshotTimelineColumnCommittedLocal,
                                               kSnapshotTimelineColumnCommittedInvisible,
                                               kSnapshotTimelineColumnCommittedHost});

            connect(table_model_, &QAbstractItemModel::dataChanged, this, &TimelineModel::OnModelChanged);

            table_view->setModel(proxy_model_);
        }
    }

    RmtSnapshotPoint* TimelineModel::AddSnapshot(uint64_t snapshot_time)
    {
        // Create a snapshot point.
        TraceManager& trace_manager = TraceManager::Get();
        if (!trace_manager.DataSetValid())
        {
            return nullptr;
        }

        // Generate the snapshot name. Name will be "Snapshot N" where N is
        // snapshot_num. Use the number of snapshots so far as a start value
        // for snapshot_num. If this snapshot exists already, increment snapshot_num
        // and repeat until the snapshot name is unique.
        RmtDataSet*              data_set      = trace_manager.GetDataSet();
        int                      snapshot_num  = RmtTraceLoaderGetSnapshotCount();
        static const char* const kSnapshotName = "Snapshot ";
        char                     name_buffer[128];
        bool                     found_duplicate = false;

        do
        {
            found_duplicate = false;
            sprintf_s(name_buffer, 128, "%s%d", kSnapshotName, snapshot_num);
            for (int i = 0; i < snapshot_num; i++)
            {
                if (strcmp(RmtTraceLoaderGetSnapshotPoint(i)->name, name_buffer) == 0)
                {
                    found_duplicate = true;
                    break;
                }
            }
            if (found_duplicate)
            {
                snapshot_num++;
            }
        } while (found_duplicate);

        RmtSnapshotPoint* snapshot_point = nullptr;
        RmtDataSetAddSnapshot(data_set, name_buffer, snapshot_time, &snapshot_point);
        if (snapshot_point != nullptr)
        {
            Update();
        }

        return snapshot_point;
    }

    void TimelineModel::OnModelChanged(const QModelIndex& top_left, const QModelIndex& bottom_right)
    {
        Q_UNUSED(bottom_right);
        Q_UNUSED(top_left);
    }

    void TimelineModel::RemoveSnapshot(RmtSnapshotPoint* snapshot_point)
    {
        TraceManager& trace_manager = TraceManager::Get();
        RmtDataSet*   data_set      = trace_manager.GetDataSet();

        for (int32_t current_snapshot_point_index = 0; current_snapshot_point_index < RmtTraceLoaderGetSnapshotCount(); ++current_snapshot_point_index)
        {
            const RmtSnapshotPoint* current_snapshot_point = RmtTraceLoaderGetSnapshotPoint(current_snapshot_point_index);
            if (current_snapshot_point == snapshot_point)
            {
                RmtDataSnapshot* open_snapshot = SnapshotManager::Get().GetOpenSnapshot();
                RmtDataSetRemoveSnapshot(data_set, current_snapshot_point_index, open_snapshot);
            }
        }

        // Update the model as we've done edits.
        Update();
    }

    int TimelineModel::RowCount()
    {
        return proxy_model_->rowCount();
    }

    void TimelineModel::SearchBoxChanged(const QString& filter)
    {
        proxy_model_->SetSearchFilter(filter);
        proxy_model_->invalidate();
    }

    void TimelineModel::FilterBySizeChanged(int min_value, int max_value)
    {
        if (TraceManager::Get().DataSetValid())
        {
            uint64_t max_usage = 0;

            const uint64_t scaled_min = max_usage * ((double)min_value / 100.0F);
            const uint64_t scaled_max = max_usage * ((double)max_value / 100.0F);

            proxy_model_->SetSizeFilter(scaled_min, scaled_max);
            proxy_model_->invalidate();
        }
    }

    void TimelineModel::CancelBackgroundTask()
    {
        TraceManager& trace_manager = TraceManager::Get();
        RmtDataSet*   data_set      = trace_manager.GetDataSet();
        RMT_ASSERT(data_set != nullptr);
        RmtDataSetCancelBackgroundTask(data_set);
    }

    bool TimelineModel::IsBackgroundTaskCancelled() const
    {
        TraceManager& trace_manager = TraceManager::Get();
        RmtDataSet*   data_set      = trace_manager.GetDataSet();
        RMT_ASSERT(data_set != nullptr);
        return RmtDataSetIsBackgroundTaskCancelled(data_set);
    }

    void TimelineModel::TimelineGenerationBegin()
    {
        is_timeline_generation_in_progress = true;
    }

    void TimelineModel::TimelineGenerationEnd()
    {
        is_timeline_generation_in_progress = false;
    }

    bool TimelineModel::IsTimelineGenerationInProgress() const
    {
        return is_timeline_generation_in_progress;
    }

    void TimelineModel::UpdateMemoryGraph(uint64_t min_visible, uint64_t max_visible)
    {
        min_visible_ = min_visible;
        max_visible_ = max_visible;

        LogFileWriter::Get().WriteLog(LogFileWriter::kDebug, "UpdateMemoryUsage: minVisible %lld, maxVisible %lld", min_visible, max_visible);

        TraceManager& trace_manager = TraceManager::Get();
        if (!trace_manager.DataSetValid())
        {
            return;
        }

        RmtDataTimeline* timeline = trace_manager.GetTimeline();
        uint64_t         duration = max_visible - min_visible;

        Q_ASSERT(duration > 0);

        double bucket_step = duration / (double)kNumBuckets;

        const RmtErrorCode error_code =
            RmtDataTimelineCreateHistogram(timeline, &job_queue_, kNumBuckets, bucket_step, min_visible_, max_visible_, &histogram_);

        RMT_ASSERT(error_code == kRmtOk);
        RMT_UNUSED(error_code);

        // Set the filter used to show/hide strips on the timeline graph and update the maximum height of the data in the timeline.
        SetTimelineSeriesFilter(timeline_series_filter_, timeline);
    }

    int TimelineModel::GetNumBuckets() const
    {
        return kNumBuckets;
    }

    int TimelineModel::RemapBucketGroupNumberToIndex(const int bucket_group_number) const
    {
        // Lookup table to re-order heap types displayed on the timeline graph.
        static int heap_bucket_order_[] = {
            RmtHeapType::kRmtHeapTypeSystem,
            RmtHeapType::kRmtHeapTypeLocal,
            RmtHeapType::kRmtHeapTypeInvisible,
            RmtHeapType::kRmtHeapTypeNone,
        };

        // Get the number of heap type items in the lookup table.
        static int          heap_type_count       = static_cast<int>(sizeof(heap_bucket_order_) / sizeof(int));
        int                 bucket_group_index    = bucket_group_number;
        RmtDataTimelineType current_timeline_type = histogram_.timeline->timeline_type;

        if ((current_timeline_type == RmtDataTimelineType::kRmtDataTimelineTypeCommitted) ||
            (current_timeline_type == RmtDataTimelineType::kRmtDataTimelineTypeVirtualMemory))
        {
            if ((bucket_group_number < 0) || (bucket_group_number >= heap_type_count))
            {
                RMT_ASSERT_MESSAGE(false, "Invalid heap type");
                bucket_group_index = RmtHeapType::kRmtHeapTypeNone;
            }
            else
            {
                bucket_group_index = heap_bucket_order_[bucket_group_number];
            }
        }

        return bucket_group_index;
    }

    int TimelineModel::GetNumBucketGroups() const
    {
        return histogram_.bucket_group_count;
    }

    void TimelineModel::SetTimelineType(RmtDataTimelineType new_timeline_type)
    {
        timeline_type_ = new_timeline_type;
    }

    RmtErrorCode TimelineModel::SetTimelineSeriesFilter(const uint32_t new_filter_mask, RmtDataTimeline* out_timeline)
    {
        RMT_RETURN_ON_ERROR(out_timeline != nullptr, kRmtErrorInvalidPointer);

        timeline_series_filter_                   = new_filter_mask;
        out_timeline->filter_mask                 = new_filter_mask;
        out_timeline->maximum_value_in_all_series = 0;
        for (int64_t value_index = 0; value_index < histogram_.bucket_count; value_index++)
        {
            uint64_t total_for_all_series = 0;
            for (int32_t current_series_index = 0; current_series_index < histogram_.bucket_group_count; ++current_series_index)
            {
                if (new_filter_mask & (static_cast<uint32_t>(1 << current_series_index)))
                {
                    int32_t histogram_index = RmtDataTimelineHistogramGetIndex(&histogram_, value_index, current_series_index);
                    total_for_all_series += histogram_.bucket_data[histogram_index];
                }
            }

            // Track the maximum value for all series.
            out_timeline->maximum_value_in_all_series = RMT_MAXIMUM(out_timeline->maximum_value_in_all_series, total_for_all_series);
        }

        return kRmtOk;
    }

    QString TimelineModel::GetValueString(int64_t value, bool display_as_memory) const
    {
        QString value_string;
        if (!display_as_memory)
        {
            value_string = QString::number(value);
        }
        else
        {
            value_string = rmv::string_util::LocalizedValueMemory(value, false, false);
        }
        return value_string;
    }

    void TimelineModel::GetResourceTooltipInfo(const int bucket_index, const bool display_as_memory, QString& out_text_string, QString& out_color_string)
    {
        ResourceSorter sorter;

        // Build an array of resource type to count.
        for (int i = 0; i < GetNumBucketGroups(); i++)
        {
            if ((i == kRmtResourceUsageTypeUnknown) || (i == RmtResourceUsageType::kRmtResourceUsageTypeHeap))
            {
                continue;
            }

            if (!(timeline_series_filter_ & (static_cast<uint32_t>(1 << i))))
            {
                // Skip usage types that have been filtered.
                continue;
            }

            sorter.AddResource((RmtResourceUsageType)i, RmtDataTimelineHistogramGetValue(&histogram_, bucket_index, i));
        }

        const size_t num_resources = sorter.GetNumResources();
        if (num_resources < 1)
        {
            out_text_string = kNoResourcesSelected;
        }
        else
        {
            // Take the top n values and show them.
            const size_t count = std::min<size_t>(kMaxTooltipLines - 1, sorter.GetNumResources());
            int64_t      value = 0;

            for (size_t i = 0; i < count; i++)
            {
                value          = sorter.GetResourceValue(i);
                const int type = sorter.GetResourceType(i);
                out_text_string += QString("%1: %2")
                                       .arg(RmtGetResourceUsageTypeNameFromResourceUsageType(static_cast<RmtResourceUsageType>(type)))
                                       .arg(GetValueString(value, display_as_memory));
                out_color_string += QString("#%1").arg(QString::number(Colorizer::GetResourceUsageColor(static_cast<RmtResourceUsageType>(type)).rgb(), 16));

                // Add a newline unless this is the last item.
                if ((i + 1) < count)
                {
                    out_text_string += "\n";
                    out_color_string += "\n";
                }
            }

            // Only show the "Other" value if there are more than one resource types.
            if (num_resources > 1)
            {
                // Total up the rest and show them as "Other".
                value = sorter.GetRemainder(kMaxTooltipLines - 1);

                // Only display the "Other values" on the tooltip if they are greater than zero.
                if (value > 0)
                {
                    out_text_string += QString("\nOther: %1").arg(GetValueString(value, display_as_memory));
                    out_color_string += QString("\n#%1").arg(QString::number(Colorizer::GetResourceUsageColor(kRmtResourceUsageTypeFree).rgb(), 16));
                }
            }
        }
    }

    void TimelineModel::BuildToolTipInfoString(const RmtHeapType heap_type, const int bucket_index, QString& out_text_string, QString& out_color_string)
    {
        const int64_t value = RmtDataTimelineHistogramGetValue(&histogram_, bucket_index, heap_type);
        if (!out_text_string.isEmpty())
        {
            out_text_string += QString("\n");
            out_color_string += QString("\n");
        }

        out_text_string +=
            QString("%1: %2").arg(RmtGetHeapTypeNameFromHeapType(RmtHeapType(heap_type))).arg(rmv::string_util::LocalizedValueMemory(value, false, false));
        out_color_string += QString("#%1").arg(QString::number(Colorizer::GetHeapColor(static_cast<RmtHeapType>(heap_type)).rgb(), 16));
    }

    bool TimelineModel::GetTimelineTooltipInfo(qreal x_pos, QString& text_string, QString& color_string)
    {
        int bucket_index = x_pos * kNumBuckets;

        switch (timeline_type_)
        {
        case kRmtDataTimelineTypeResourceUsageCount:
            // Number of each type of resource.
            GetResourceTooltipInfo(bucket_index, false, text_string, color_string);
            break;

        case kRmtDataTimelineTypeResourceUsageVirtualSize:
            // Memory for each type of resource.
            GetResourceTooltipInfo(bucket_index, true, text_string, color_string);
            break;

        case kRmtDataTimelineTypeVirtualMemory:
        case kRmtDataTimelineTypeCommitted:
            BuildToolTipInfoString(RmtHeapType::kRmtHeapTypeNone, bucket_index, text_string, color_string);
            BuildToolTipInfoString(RmtHeapType::kRmtHeapTypeInvisible, bucket_index, text_string, color_string);
            BuildToolTipInfoString(RmtHeapType::kRmtHeapTypeLocal, bucket_index, text_string, color_string);
            BuildToolTipInfoString(RmtHeapType::kRmtHeapTypeSystem, bucket_index, text_string, color_string);
            break;

        default:
            return false;
        }

        return true;
    }

    bool TimelineModel::GetHistogramData(int bucket_group_number, int bucket_index, const int bucket_group_count, qreal& out_y_pos, qreal& out_height)
    {
        if (bucket_index < kNumBuckets)
        {
            TraceManager&          trace_manager = TraceManager::Get();
            const RmtDataTimeline* timeline      = trace_manager.GetTimeline();

            // The heights in the bucket consist of the memory allocated for each process.
            // Since the timeline view is a stacked graph, the heights of the previous buckets
            // need to be taken into account and used as an offset for the current bucket.
            out_y_pos             = 0.0;
            qreal histogram_value = 0.0;

            if ((timeline->timeline_type == RmtDataTimelineType::kRmtDataTimelineTypeResourceUsageVirtualSize) ||
                (timeline->timeline_type == RmtDataTimelineType::kRmtDataTimelineTypeResourceUsageCount))
            {
                // For the Resource Usage Size timeline view, reverse the order of the items in the stacked graph.
                for (int i = bucket_group_count; i >= bucket_group_number; i--)
                {
                    if ((static_cast<uint32_t>(1 << i)) & timeline->filter_mask)
                    {
                        histogram_value = (double)RmtDataTimelineHistogramGetValue(&histogram_, bucket_index, i);
                    }
                    else
                    {
                        histogram_value = 0;
                    }
                    histogram_value /= timeline->maximum_value_in_all_series;
                    out_y_pos += histogram_value;
                }
            }
            else
            {
                for (int i = 0; i <= bucket_group_number; i++)
                {
                    if ((static_cast<uint32_t>(1 << i)) & timeline->filter_mask)
                    {
                        histogram_value = (double)RmtDataTimelineHistogramGetValue(&histogram_, bucket_index, RemapBucketGroupNumberToIndex(i));
                    }
                    else
                    {
                        histogram_value = 0;
                    }
                    histogram_value /= timeline->maximum_value_in_all_series;
                    out_y_pos += histogram_value;
                }
            }

            // Height is just the data for this particular sub-bucket.
            out_height = histogram_value;

            return true;
        }
        return false;
    }

    qulonglong TimelineModel::GetProxyData(int row, int col)
    {
        return proxy_model_->GetData(row, col);
    }

    SnapshotTimelineProxyModel* TimelineModel::GetProxyModel() const
    {
        return proxy_model_;
    }

    void TimelineModel::ValidateTimeUnits() const
    {
        TraceManager& trace_manager = TraceManager::Get();
        bool          valid         = false;

        if (trace_manager.DataSetValid())
        {
            const RmtDataSet* data_set = trace_manager.GetDataSet();
            if (RmtDataSetGetCpuClockTimestampValid(data_set) == kRmtOk)
            {
                valid = true;
            }
            else
            {
                RMVSettings::Get().SetUnits(kTimeUnitTypeClk);
            }

            RMVSettings::Get().SetUnitsOverrideEnable(!valid);
        }
    }

    uint64_t TimelineModel::GetMaxTimestamp() const
    {
        TraceManager& trace_manager = TraceManager::Get();
        if (trace_manager.DataSetValid())
        {
            const RmtDataSet* data_set = trace_manager.GetDataSet();
            return data_set->maximum_timestamp;
        }

        return 0;
    }

    BackgroundTask* TimelineModel::CreateWorkerThread(RmtDataTimelineType timeline_type, const uint32_t filter_mask)
    {
        return new TimelineWorker(this, timeline_type, filter_mask);
    }

}  // namespace rmv