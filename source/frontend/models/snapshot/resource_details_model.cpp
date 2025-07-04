//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the Resource details model.
//=============================================================================

#include "models/snapshot/resource_details_model.h"

#include <QTableView>

#include "qt_common/utils/qt_util.h"

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"
#include "rmt_print.h"
#include "rmt_util.h"
#include "rmt_virtual_allocation_list.h"

#include "managers/trace_manager.h"
#include "models/colorizer.h"
#include "settings/rmv_settings.h"
#include "util/rmv_util.h"
#include "util/string_util.h"
#include "util/time_util.h"

namespace rmv
{
    /// @brief Worker class definition to do the processing of the resource history data
    /// on a separate thread.
    class ResourceWorker : public rmv::BackgroundTask
    {
    public:
        /// @brief Constructor.
        ResourceWorker(rmv::ResourceDetailsModel* model, RmtResourceIdentifier resource_identifier)
            : BackgroundTask(false)
            , model_(model)
            , resource_identifier_(resource_identifier)
        {
        }

        /// @brief Destructor.
        ~ResourceWorker()
        {
        }

        /// @brief Worker thread function.
        virtual void ThreadFunc()
        {
            model_->GenerateResourceHistory(resource_identifier_);
        }

    private:
        rmv::ResourceDetailsModel* model_;                ///< Pointer to the model data.
        RmtResourceIdentifier      resource_identifier_;  ///< The selected resource identifier.
    };

    ResourceDetailsModel::ResourceDetailsModel()
        : ModelViewMapper(kResourceDetailsNumWidgets)
        , timeline_model_(nullptr)
        , timeline_proxy_model_(nullptr)
        , properties_model_(nullptr)
        , highlighted_row_(-1)
    {
        resource_history_.event_count = -1;
    }

    ResourceDetailsModel::~ResourceDetailsModel()
    {
        delete timeline_model_;
        delete properties_model_;
    }

    void ResourceDetailsModel::InitializeTimelineTableModel(ScaledTableView* timeline_table_view, uint num_rows, uint num_columns)
    {
        if (timeline_proxy_model_ != nullptr)
        {
            delete timeline_proxy_model_;
            timeline_proxy_model_ = nullptr;
        }

        timeline_proxy_model_ = new ResourceDetailsProxyModel();
        timeline_model_       = timeline_proxy_model_->InitializeResourceTableModels(timeline_table_view, num_rows, num_columns);

        timeline_table_view->horizontalHeader()->setSectionsClickable(true);

        // The Resource timeline table has lots of horizontal space,
        // so these column widths are a bit wider than the actual table contents.
        timeline_table_view->SetColumnPadding(0);
        timeline_table_view->SetColumnWidthEms(kResourceHistoryColumnLegend, 6);
        timeline_table_view->SetColumnWidthEms(kResourceHistoryColumnEvent, 30);
        timeline_table_view->SetColumnWidthEms(kResourceHistoryColumnTime, 15);
        timeline_table_view->SetColumnWidthEms(kResourceHistoryColumnVirtualAddress, 15);
        timeline_table_view->SetColumnWidthEms(kResourceHistoryColumnPhysicalAddress, 15);
        timeline_table_view->SetColumnWidthEms(kResourceHistoryColumnSize, 15);
        timeline_table_view->SetColumnWidthEms(kResourceHistoryColumnPageSize, 15);

        // Still let the user resize the columns if desired.
        timeline_table_view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    }

    void ResourceDetailsModel::InitializePropertiesTableModel(QTableView* properties_table_view, uint num_rows, uint num_columns)
    {
        RMT_ASSERT(properties_model_ == nullptr);
        properties_model_ = new rmv::ResourcePropertiesModel();
        properties_model_->InitializeTableModel(properties_table_view, num_rows, num_columns);
    }

    bool ResourceDetailsModel::IsResourceValid(RmtResourceIdentifier resource_identifier) const
    {
        const RmtResource* resource = nullptr;

        return GetResourceFromResourceId(resource_identifier, &resource);
    }

    bool ResourceDetailsModel::IsResourceBaseAddressValid(RmtResourceIdentifier resource_identifier) const
    {
        const RmtResource* resource = nullptr;
        if (GetResourceFromResourceId(resource_identifier, &resource) == true)
        {
            const uint64_t base_address = (resource->bound_allocation != nullptr) ? resource->bound_allocation->base_address : 0;
            if (base_address != 0)
            {
                return true;
            }
        }
        return false;
    }

    bool ResourceDetailsModel::GetResourceFromResourceId(RmtResourceIdentifier resource_identifier, const RmtResource** resource) const
    {
        if (TraceManager::Get().DataSetValid())
        {
            const RmtDataSnapshot* open_snapshot = SnapshotManager::Get().GetOpenSnapshot();
            if (open_snapshot != nullptr)
            {
                const RmtErrorCode error_code = RmtResourceListGetResourceByResourceId(&open_snapshot->resource_list, resource_identifier, resource);
                if (error_code == kRmtOk)
                {
                    return true;
                }
            }
        }
        return false;
    }

    void ResourceDetailsModel::ResetModelValues()
    {
        timeline_model_->removeRows(0, timeline_model_->rowCount());
        SetModelData(kResourceDetailsResourceName, "-");

        SetModelData(kResourceDetailsAllocationBaseAddress, "-");
        SetModelData(kResourceDetailsAllocationOffset, "-");
        SetModelData(kResourceDetailsBaseAddress, "-");
        SetModelData(kResourceDetailsSize, "-");
        SetModelData(kResourceDetailsType, "-");
        SetModelData(kResourceDetailsHeap, "-");
        SetModelData(kResourceDetailsFullyMapped, "-");
        SetModelData(kResourceDetailsUnmappedPercentage, "-");
        SetModelData(kResourceDetailsCreateTime, "-");
        SetModelData(kResourceDetailsBindTime, "-");
        SetModelData(kResourceDetailsCommitTime, "-");
        SetModelData(kResourceDetailsOwnerTime, "-");
        SetModelData(kResourceDetailsFlags, "-");

        highlighted_row_ = -1;
    }

    int32_t ResourceDetailsModel::Update(RmtResourceIdentifier resource_identifier)
    {
        ResetModelValues();
        UpdateTimelineTable();

        const RmtResource* resource = nullptr;
        if (GetResourceFromResourceId(resource_identifier, &resource) == true)
        {
            const char  buffer[RMT_MAXIMUM_NAME_LENGTH] = " - ";
            const char* buf_ptr                         = &buffer[0];
            RmtResourceGetName(resource, RMT_MAXIMUM_NAME_LENGTH, (char**)&buf_ptr);
            SetModelData(kResourceDetailsResourceName, buffer);
            SetModelData(kResourceDetailsAllocationBaseAddress, rmv_util::GetVirtualAllocationName(resource->bound_allocation));
            SetModelData(kResourceDetailsAllocationOffset, rmv::string_util::LocalizedValueAddress(RmtResourceGetOffsetFromBoundAllocation(resource)));
            SetModelData(kResourceDetailsBaseAddress, QString("0x") + QString::number(resource->address, 16));
            SetModelData(kResourceDetailsSize, rmv::string_util::LocalizedValueMemory(resource->size_in_bytes, false, false));
            SetModelData(kResourceDetailsType, RmtGetResourceUsageTypeNameFromResourceUsageType(RmtResourceGetUsageType(resource)));
            SetModelData(kResourceDetailsHeap, RmtResourceGetHeapTypeName(resource));

            if (TraceManager::Get().DataSetValid())
            {
                const RmtDataSnapshot* open_snapshot = SnapshotManager::Get().GetOpenSnapshot();
                if (open_snapshot != nullptr)
                {
                    SetModelData(kResourceDetailsFullyMapped, QString(RmtResourceIsCompletelyInPreferredHeap(open_snapshot, resource) ? "Yes" : "No"));

                    // Calculate histogram.
                    uint64_t memory_segment_histogram[kRmtResourceBackingStorageCount] = {0};
                    RmtResourceGetBackingStorageHistogram(open_snapshot, resource, memory_segment_histogram);
                    double unmapped_percentage = ((double)memory_segment_histogram[kRmtResourceBackingStorageUnmapped] / (double)resource->size_in_bytes) * 100;
                    SetModelData(kResourceDetailsUnmappedPercentage, QString::number(unmapped_percentage) + "%");
                }
            }

            SetModelData(kResourceDetailsCreateTime, rmv::time_util::ClockToTimeUnit(resource->create_time));
            SetModelData(kResourceDetailsBindTime, rmv::time_util::ClockToTimeUnit(resource->bind_time));
            SetModelData(kResourceDetailsCommitTime, RmtGetCommitTypeNameFromCommitType(resource->commit_type));
            SetModelData(kResourceDetailsOwnerTime, QString::number(resource->owner_type));
            SetModelData(kResourceDetailsFlags, QString::number(resource->flags));
        }

        return properties_model_->Update(resource_identifier);
    }

    void ResourceDetailsModel::UpdateTimelineTable()
    {
        uint64_t snapshot_timestamp = 0;

        if (TraceManager::Get().DataSetValid())
        {
            const RmtDataSnapshot* snapshot = SnapshotManager::Get().GetOpenSnapshot();
            if (snapshot != nullptr)
            {
                snapshot_timestamp = snapshot->timestamp;
            }
        }

        int32_t event_count = resource_history_.event_count;
        timeline_model_->SetRowCount(event_count + 1);

        // Find the index of the snapshot timestamp.
        int32_t event_index = 0;
        for (int32_t table_row = 0; table_row < event_count + 1; table_row++)
        {
            uint64_t timestamp = resource_history_.events[event_index].timestamp;
            if (table_row >= event_count || timestamp > snapshot_timestamp)
            {
                timeline_model_->SetSnapshotParameters(table_row, snapshot_timestamp, &resource_history_);
                break;
            }

            event_index++;
        }
        timeline_proxy_model_->invalidate();
    }

    void ResourceDetailsModel::TimelineEventSelected(const QModelIndex& proxy_index)
    {
        if (proxy_index.isValid())
        {
            QModelIndex source_index = timeline_proxy_model_->mapToSource(proxy_index);
            highlighted_row_         = source_index.row();
        }
    }

    QColor ResourceDetailsModel::GetColorFromEventType(RmtResourceHistoryEventType event_type, bool highlighted) const
    {
        if (highlighted)
        {
            return RMVSettings::Get().GetColorResourceHistoryHighlight();
        }

        switch (event_type)
        {
        case kRmtResourceHistoryEventResourceCreated:
        case kRmtResourceHistoryEventResourceDestroyed:
        case kRmtResourceHistoryEventResourceBound:
        case kRmtResourceHistoryEventResourceNamed:
            return RMVSettings::Get().GetColorResourceHistoryResourceEvent();

        case kRmtResourceHistoryEventVirtualMemoryMapped:
        case kRmtResourceHistoryEventVirtualMemoryUnmapped:
        case kRmtResourceHistoryEventVirtualMemoryAllocated:
        case kRmtResourceHistoryEventVirtualMemoryFree:
            return RMVSettings::Get().GetColorResourceHistoryCpuMapping();

        case kRmtResourceHistoryEventVirtualMemoryMakeResident:
        case kRmtResourceHistoryEventVirtualMemoryEvict:
            return RMVSettings::Get().GetColorResourceHistoryResidencyUpdate();

        case kRmtResourceHistoryEventPhysicalMapToLocal:
        case kRmtResourceHistoryEventPhysicalMapToHost:
        case kRmtResourceHistoryEventPhysicalUnmap:
        case kRmtResourceHistoryEventBackingMemoryPaged:
            return RMVSettings::Get().GetColorResourceHistoryPageTableUpdate();

        case kRmtResourceHistoryEventSnapshotTaken:
            return RMVSettings::Get().GetColorResourceHistorySnapshot();

        default:
            RMT_ASSERT_MESSAGE(false, "Invalid event type");
            return QColor(Qt::black);
        }
    }

    ResourceIconShape ResourceDetailsModel::GetShapeFromEventType(RmtResourceHistoryEventType event_type) const
    {
        switch (event_type)
        {
        case kRmtResourceHistoryEventResourceCreated:
        case kRmtResourceHistoryEventVirtualMemoryMapped:
        case kRmtResourceHistoryEventVirtualMemoryMakeResident:
        case kRmtResourceHistoryEventPhysicalMapToLocal:
        case kRmtResourceHistoryEventPhysicalMapToHost:
            return kIconShapeCircle;

        case kRmtResourceHistoryEventResourceDestroyed:
        case kRmtResourceHistoryEventVirtualMemoryUnmapped:
        case kRmtResourceHistoryEventVirtualMemoryEvict:
        case kRmtResourceHistoryEventPhysicalUnmap:
            return kIconShapeCross;

        case kRmtResourceHistoryEventResourceBound:
        case kRmtResourceHistoryEventVirtualMemoryAllocated:
        case kRmtResourceHistoryEventResourceNamed:
            return kIconShapeTriangle;

        case kRmtResourceHistoryEventVirtualMemoryFree:
            return kIconShapeSquare;

        case kRmtResourceHistoryEventBackingMemoryPaged:
            return kIconShapeSquare;

        case kRmtResourceHistoryEventSnapshotTaken:
            return kIconShapeInvertedTriangle;

        default:
            RMT_ASSERT_MESSAGE(false, "Invalid event type");
            return kIconShapeCross;
        }
    }

    void ResourceDetailsModel::GenerateResourceHistory(RmtResourceIdentifier resource_identifier)
    {
        if (TraceManager::Get().DataSetValid())
        {
            RmtDataSnapshot* open_snapshot = SnapshotManager::Get().GetOpenSnapshot();
            if (open_snapshot != nullptr)
            {
                resource_history_.event_count = -1;
                const RmtResource* resource   = nullptr;
                GetResourceFromResourceId(resource_identifier, &resource);

                RmtDataSnapshotGenerateResourceHistory(open_snapshot, resource, &resource_history_);
            }
        }
    }

    int ResourceDetailsModel::GetEventRowFromTimeline(double logical_position, double icon_size)
    {
        int32_t  event_count     = resource_history_.event_count;
        uint64_t start_timestamp = resource_history_.events[0].timestamp;
        uint64_t end_timestamp   = resource_history_.events[event_count - 1].timestamp;
        double   duration        = end_timestamp - start_timestamp;

        logical_position *= duration;
        logical_position += start_timestamp;
        icon_size *= duration;

        // Go through the list and decide what's been clicked on.
        highlighted_row_ = 0;

        for (int32_t count = 0; count < timeline_model_->rowCount(); count++)
        {
            uint64_t min_time = timeline_model_->data(timeline_model_->index(count, kResourceHistoryColumnTime), Qt::UserRole).toULongLong();
            uint64_t max_time = min_time + icon_size;
            if (logical_position > min_time && logical_position < max_time)
            {
                // Map from proxy model.
                QModelIndex index       = timeline_model_->index(highlighted_row_, 0, QModelIndex());
                QModelIndex proxy_index = timeline_proxy_model_->mapFromSource(index);
                return proxy_index.row();
            }
            highlighted_row_++;
        }
        highlighted_row_ = -1;

        return highlighted_row_;
    }

    bool ResourceDetailsModel::GetEventData(int index, int width, ResourceEvent& out_event_data) const
    {
        uint64_t snapshot_timestamp = UINT64_MAX;
        if (TraceManager::Get().DataSetValid())
        {
            const RmtDataSnapshot* snapshot = SnapshotManager::Get().GetOpenSnapshot();
            if (snapshot != nullptr)
            {
                snapshot_timestamp = snapshot->timestamp;
            }
        }

        int32_t  event_count     = timeline_model_->rowCount();
        uint64_t start_timestamp = resource_history_.events[0].timestamp;
        uint64_t end_timestamp   = resource_history_.events[resource_history_.event_count - 1].timestamp;

        if ((snapshot_timestamp < UINT64_MAX) && (end_timestamp < snapshot_timestamp))
        {
            end_timestamp = snapshot_timestamp;
        }

        double duration = end_timestamp - start_timestamp;

        if (index < 0 || index >= event_count)
        {
            return false;
        }

        if (highlighted_row_ != -1 && index >= highlighted_row_)
        {
            // If there's a highlighted row, defer the drawing of it to last.
            if (index < event_count - 1)
            {
                index++;
            }
            else
            {
                index = highlighted_row_;
            }
        }

        const QModelIndex           legend_model_index = timeline_model_->index(index, kResourceHistoryColumnLegend);
        RmtResourceHistoryEventType event_type         = static_cast<RmtResourceHistoryEventType>(timeline_model_->data(legend_model_index).toInt());
        int                         event_index        = timeline_model_->data(legend_model_index, Qt::UserRole).toInt();
        uint64_t                    timestamp          = 0;
        if (event_type == kRmtResourceHistoryEventSnapshotTaken)
        {
            timestamp = snapshot_timestamp;
        }
        else
        {
            timestamp = resource_history_.events[event_index].timestamp;
        }

        timestamp -= start_timestamp;
        timestamp *= width;
        timestamp /= duration;
        out_event_data.timestamp = timestamp;

        bool highlighted = false;
        if (index == highlighted_row_)
        {
            highlighted = true;
        }

        out_event_data.color = GetColorFromEventType(event_type, highlighted);
        out_event_data.shape = GetShapeFromEventType(event_type);

        return true;
    }

    bool ResourceDetailsModel::GetResidencyData(RmtResourceIdentifier resource_identifier, int index, float& value, QString& name, QColor& color) const
    {
        if (index < 0 || index >= kRmtResourceBackingStorageCount)
        {
            return false;
        }

        if (!TraceManager::Get().DataSetValid())
        {
            return false;
        }
        const RmtResource* resource = nullptr;

        if (GetResourceFromResourceId(resource_identifier, &resource) == false)
        {
            return false;
        }

        // Calculate histogram to get residency per heap.
        uint64_t memory_segment_histogram[kRmtResourceBackingStorageCount] = {0};

        const RmtDataSnapshot* open_snapshot = SnapshotManager::Get().GetOpenSnapshot();
        if (open_snapshot != nullptr)
        {
            RmtResourceGetBackingStorageHistogram(open_snapshot, resource, memory_segment_histogram);
        }

        value = 0.0f;
        if (resource->size_in_bytes > 0)
        {
            value = (memory_segment_histogram[index] * 100.0f) / resource->size_in_bytes;
        }

        color = Colorizer::GetHeapColor(static_cast<RmtHeapType>(index));

        switch (index)
        {
        case kRmtHeapTypeLocal:
            name = "Local";
            break;
        case kRmtHeapTypeInvisible:
            name = "Invisible";
            break;
        case kRmtHeapTypeSystem:
            name = "Host";
            break;
        case kRmtResourceBackingStorageUnmapped:
            name = "Unmapped";
            break;
        }

        return true;
    }

    bool ResourceDetailsModel::GetUnmappedResidencyData(RmtResourceIdentifier resource_identifier, int& value, QString& name, QColor& color) const
    {
        if (!TraceManager::Get().DataSetValid())
        {
            return false;
        }

        const RmtResource* resource = nullptr;

        if (GetResourceFromResourceId(resource_identifier, &resource) == false)
        {
            return false;
        }

        // Calculate histogram to get residency per heap.
        uint64_t memory_segment_histogram[kRmtResourceBackingStorageCount] = {0};

        const RmtDataSnapshot* open_snapshot = SnapshotManager::Get().GetOpenSnapshot();
        if (open_snapshot != nullptr)
        {
            RmtResourceGetBackingStorageHistogram(open_snapshot, resource, memory_segment_histogram);
        }

        value = 0;
        if (resource->size_in_bytes > 0)
        {
            value = (memory_segment_histogram[kRmtResourceBackingStorageUnmapped] * 100) / resource->size_in_bytes;
        }

        color = RMVSettings::Get().GetColorResourceFreeSpace();

        name = "Unmapped";

        return true;
    }

    bool ResourceDetailsModel::PhysicalMemoryInPreferredHeap(RmtResourceIdentifier resource_identifier) const
    {
        const RmtResource* resource = nullptr;
        if (GetResourceFromResourceId(resource_identifier, &resource) == true)
        {
            if (TraceManager::Get().DataSetValid())
            {
                const RmtDataSnapshot* open_snapshot = SnapshotManager::Get().GetOpenSnapshot();
                if (open_snapshot != nullptr)
                {
                    if (resource->bound_allocation != nullptr)
                    {
                        // If preferred heap is unspecified, then don't care if the memory is mapped or not.
                        RmtHeapType preferred_heap = resource->bound_allocation->heap_preferences[0];
                        if (preferred_heap == kRmtHeapTypeNone)
                        {
                            return true;
                        }
                    }
                    bool all_mapped = RmtPageTableIsEntireResourcePhysicallyMapped(&open_snapshot->page_table, resource);
                    if (all_mapped)
                    {
                        // If it's all physically mapped, make sure it's all in the preferred heap.
                        return RmtResourceIsCompletelyInPreferredHeap(open_snapshot, resource);
                    }
                }
            }
        }
        return false;
    }

    ResourceDetailsProxyModel* ResourceDetailsModel::GetTimelineProxyModel() const
    {
        return timeline_proxy_model_;
    }

    BackgroundTask* ResourceDetailsModel::CreateWorkerThread(RmtResourceIdentifier resource_identifier)
    {
        return new ResourceWorker(this, resource_identifier);
    }

    QString ResourceDetailsModel::GetPropertyString(QString name, QString value, int32_t max_name_length) const
    {
        QString property_string;
        if (max_name_length < 0)
        {
            property_string += name;
            property_string += ",";
        }
        else
        {
            property_string = name.leftJustified(max_name_length + 1, ' ');
        }
        property_string += value;
        property_string += "\n";
        return property_string;
    }

    QString ResourceDetailsModel::GetPropertiesString(RmtResourceIdentifier resource_identifier, bool as_csv) const
    {
        QString       properties_string;
        int32_t       rows                = properties_model_->GetNumRows();
        const QString base_address_string = "Parent allocation base address:";
        int32_t       string_length       = -1;

        if (!as_csv)
        {
            // Get longest string length to determine the space padding value.
            string_length = base_address_string.length();
            for (auto row = 0; row < rows; row++)
            {
                int32_t length = properties_model_->GetPropertyNameForRow(row).length();
                if (length > string_length)
                {
                    string_length = length;
                }
            }
        }

        // Get base address and offset.
        const RmtResource* resource = nullptr;
        QString            str;
        if (GetResourceFromResourceId(resource_identifier, &resource) == true)
        {
            const char  buffer[RMT_MAXIMUM_NAME_LENGTH] = " - ";
            const char* buf_ptr                         = &buffer[0];
            RmtResourceGetName(resource, RMT_MAXIMUM_NAME_LENGTH, (char**)&buf_ptr);

            properties_string += GetPropertyString(base_address_string, rmv_util::GetVirtualAllocationName(resource->bound_allocation), string_length);
            properties_string += GetPropertyString("Resource virtual address:", QString("0x") + QString::number(resource->address, 16), string_length);
            properties_string += "\n";
        }

        properties_string += GetPropertyString("Property name", "Property value", string_length);

        QString delimiter = "";
        if (as_csv)
        {
            // For csv files, add quotes around values, since some numbers are displayed as '4,906', and this confuses the parser.
            delimiter = "\"";
        }
        for (auto row = 0; row < rows; row++)
        {
            QString value_string = delimiter + properties_model_->GetPropertyValueForRow(row) + delimiter;
            properties_string += GetPropertyString(properties_model_->GetPropertyNameForRow(row), value_string, string_length);
        }
        return properties_string;
    }

}  // namespace rmv
