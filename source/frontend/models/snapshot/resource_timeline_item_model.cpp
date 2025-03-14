//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a resource timeline item model.
///
/// Used for the resource timeline table in the resource details pane.
///
//=============================================================================

#include "models/snapshot/resource_timeline_item_model.h"

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"
#include "rmt_resource_list.h"
#include "rmt_util.h"

#include "managers/trace_manager.h"
#include "util/string_util.h"
#include "util/time_util.h"

namespace rmv
{
    ResourceTimelineItemModel::ResourceTimelineItemModel(QObject* parent)
        : QAbstractItemModel(parent)
        , num_rows_(0)
        , num_columns_(0)
        , snapshot_table_index_(0)
        , snapshot_timestamp_(0)
        , resource_history_(nullptr)
    {
    }

    ResourceTimelineItemModel::~ResourceTimelineItemModel()
    {
    }

    void ResourceTimelineItemModel::SetRowCount(int rows)
    {
        num_rows_ = rows;
    }

    void ResourceTimelineItemModel::SetColumnCount(int columns)
    {
        num_columns_ = columns;
    }

    void ResourceTimelineItemModel::SetSnapshotParameters(int32_t snapshot_table_index, uint64_t snapshot_timestamp, RmtResourceHistory* resource_history)
    {
        snapshot_table_index_ = snapshot_table_index;
        snapshot_timestamp_   = snapshot_timestamp;
        resource_history_     = resource_history;
    }

    QVariant ResourceTimelineItemModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
        {
            return QVariant();
        }

        if (resource_history_ == nullptr)
        {
            return QVariant();
        }

        int row = index.row();

        // Set up data for snapshot entry.
        uint64_t                    timestamp  = snapshot_timestamp_;
        RmtResourceHistoryEventType event_type = kRmtResourceHistoryEventSnapshotTaken;
        int                         row_index  = row;

        if (row != snapshot_table_index_)
        {
            // If not snapshot and after the snapshot position, row index in the history data is
            // 1 less than the table row since the snapshot position has been added to the table.
            if (row > snapshot_table_index_)
            {
                row_index = row - 1;
            }
            event_type = resource_history_->events[row_index].event_type;
            timestamp  = resource_history_->events[row_index].timestamp;
        }

        if (role == Qt::DisplayRole)
        {
            switch (index.column())
            {
            case kResourceHistoryColumnLegend:
                return event_type;
            case kResourceHistoryColumnEvent:
            {
                // If the event type is 'ResourceNamed' then append the resource name string to the event name.
                const char*  resource_name        = nullptr;
                RmtErrorCode resource_name_result = kRmtOk;
                if (event_type == kRmtResourceHistoryEventResourceNamed)
                {
                    resource_name_result = RmtResourceUserdataGetResourceNameAtTimestamp(
                        resource_history_->resource->identifier, resource_history_->resource->create_time, timestamp, &resource_name);
                }

                if ((resource_name_result == kRmtOk) && (resource_name != nullptr))
                {
                    const QString resource_named_event_string = GetTextFromEventType(event_type) + " '" + resource_name + "'";
                    return resource_named_event_string;
                }
                else
                {
                    return GetTextFromEventType(event_type);
                }
            }
            case kResourceHistoryColumnTime:
                return rmv::time_util::ClockToTimeUnit(timestamp);
            case kResourceHistoryColumnVirtualAddress:
                switch (event_type)
                {
                case kRmtResourceHistoryEventResourceBound:
                case kRmtResourceHistoryEventVirtualMemoryAllocated:
                case kRmtResourceHistoryEventVirtualMemoryFree:
                case kRmtResourceHistoryEventVirtualMemoryMapped:
                case kRmtResourceHistoryEventVirtualMemoryUnmapped:
                case kRmtResourceHistoryEventVirtualMemoryMakeResident:
                case kRmtResourceHistoryEventVirtualMemoryEvict:
                case kRmtResourceHistoryEventPhysicalMapToLocal:
                case kRmtResourceHistoryEventPhysicalUnmap:
                case kRmtResourceHistoryEventPhysicalMapToHost:
                    return rmv::string_util::LocalizedValueAddress(resource_history_->events[row_index].virtual_address);

                default:
                    return "n/a";
                }

            case kResourceHistoryColumnPhysicalAddress:
                switch (event_type)
                {
                case kRmtResourceHistoryEventPhysicalMapToLocal:
                case kRmtResourceHistoryEventPhysicalUnmap:
                    return rmv::string_util::LocalizedValueAddress(resource_history_->events[row_index].physical_address);

                default:
                    return "n/a";
                }

            case kResourceHistoryColumnSize:
                switch (event_type)
                {
                case kRmtResourceHistoryEventVirtualMemoryAllocated:
                case kRmtResourceHistoryEventVirtualMemoryFree:
                case kRmtResourceHistoryEventPhysicalMapToLocal:
                case kRmtResourceHistoryEventPhysicalUnmap:
                case kRmtResourceHistoryEventPhysicalMapToHost:
                    return rmv::string_util::LocalizedValueMemory(resource_history_->events[row_index].size_in_bytes, false, false);

                default:
                    return "n/a";
                }

            case kResourceHistoryColumnPageSize:
                switch (event_type)
                {
                case kRmtResourceHistoryEventPhysicalMapToLocal:
                case kRmtResourceHistoryEventPhysicalMapToHost:
                case kRmtResourceHistoryEventPhysicalUnmap:
                    return rmv::string_util::LocalizedValueMemory(resource_history_->events[row_index].page_size_in_bytes, false, false);

                default:
                    return "n/a";
                }

            default:
                break;
            }
        }
        else if (role == Qt::UserRole)
        {
            switch (index.column())
            {
            case kResourceHistoryColumnLegend:
                return QVariant::fromValue<int>(row_index);
            case kResourceHistoryColumnEvent:
                return QVariant::fromValue<int>(event_type);
            case kResourceHistoryColumnTime:
                return QVariant::fromValue<qulonglong>(timestamp);
            case kResourceHistoryColumnVirtualAddress:
                switch (event_type)
                {
                case kRmtResourceHistoryEventResourceBound:
                case kRmtResourceHistoryEventVirtualMemoryAllocated:
                case kRmtResourceHistoryEventVirtualMemoryFree:
                case kRmtResourceHistoryEventVirtualMemoryMapped:
                case kRmtResourceHistoryEventVirtualMemoryUnmapped:
                case kRmtResourceHistoryEventVirtualMemoryMakeResident:
                case kRmtResourceHistoryEventVirtualMemoryEvict:
                case kRmtResourceHistoryEventPhysicalMapToLocal:
                case kRmtResourceHistoryEventPhysicalUnmap:
                case kRmtResourceHistoryEventPhysicalMapToHost:
                    return QVariant::fromValue<qulonglong>(resource_history_->events[row_index].virtual_address);

                default:
                    return 0;
                }

            case kResourceHistoryColumnPhysicalAddress:
                switch (event_type)
                {
                case kRmtResourceHistoryEventPhysicalMapToLocal:
                case kRmtResourceHistoryEventPhysicalUnmap:
                    return QVariant::fromValue<qulonglong>(resource_history_->events[row_index].physical_address);

                default:
                    return 0;
                }

            case kResourceHistoryColumnSize:
                switch (event_type)
                {
                case kRmtResourceHistoryEventVirtualMemoryAllocated:
                case kRmtResourceHistoryEventVirtualMemoryFree:
                case kRmtResourceHistoryEventPhysicalMapToLocal:
                case kRmtResourceHistoryEventPhysicalUnmap:
                    return QVariant::fromValue<qulonglong>(resource_history_->events[row_index].size_in_bytes);

                default:
                    return 0;
                }

            case kResourceHistoryColumnPageSize:
                switch (event_type)
                {
                case kRmtResourceHistoryEventPhysicalMapToLocal:
                case kRmtResourceHistoryEventPhysicalMapToHost:
                case kRmtResourceHistoryEventPhysicalUnmap:
                    return QVariant::fromValue<qulonglong>(resource_history_->events[row_index].page_size_in_bytes);

                default:
                    return 0;
                }

            default:
                break;
            }
        }
        else if (role == Qt::ForegroundRole)
        {
            switch (index.column())
            {
            case kResourceHistoryColumnEvent:
            case kResourceHistoryColumnTime:
            case kResourceHistoryColumnVirtualAddress:
            case kResourceHistoryColumnPhysicalAddress:
            case kResourceHistoryColumnSize:
            case kResourceHistoryColumnPageSize:
                if (timestamp > snapshot_timestamp_)
                {
                    return QColor(Qt::lightGray);
                }

            default:
                break;
            }
        }

        return QVariant();
    }

    Qt::ItemFlags ResourceTimelineItemModel::flags(const QModelIndex& index) const
    {
        return QAbstractItemModel::flags(index);
    }

    QVariant ResourceTimelineItemModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section)
            {
            case kResourceHistoryColumnLegend:
                return "Legend";
            case kResourceHistoryColumnEvent:
                return "Event";
            case kResourceHistoryColumnTime:
                return "Timestamp";
            case kResourceHistoryColumnVirtualAddress:
                return "Virtual address";
            case kResourceHistoryColumnPhysicalAddress:
                return "Physical address";
            case kResourceHistoryColumnSize:
                return "Size";
            case kResourceHistoryColumnPageSize:
                return "Page size";

            default:
                break;
            }
        }

        return QAbstractItemModel::headerData(section, orientation, role);
    }

    QModelIndex ResourceTimelineItemModel::index(int row, int column, const QModelIndex& parent) const
    {
        if (!hasIndex(row, column, parent))
        {
            return QModelIndex();
        }

        return createIndex(row, column);
    }

    QModelIndex ResourceTimelineItemModel::parent(const QModelIndex& index) const
    {
        Q_UNUSED(index);
        return QModelIndex();
    }

    int ResourceTimelineItemModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);
        return num_rows_;
    }

    int ResourceTimelineItemModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);
        return num_columns_;
    }

    QString ResourceTimelineItemModel::GetTextFromEventType(RmtResourceHistoryEventType event_type) const
    {
        switch (event_type)
        {
        case kRmtResourceHistoryEventResourceCreated:
            return "Resource created";

        case kRmtResourceHistoryEventResourceBound:
            return "Resource bound";

        case kRmtResourceHistoryEventVirtualMemoryMapped:
            return "CPU Mapped";

        case kRmtResourceHistoryEventVirtualMemoryUnmapped:
            return "CPU Unmapped";

        case kRmtResourceHistoryEventBackingMemoryPaged:
            return "Page table updated";

        case kRmtResourceHistoryEventVirtualMemoryMakeResident:
            return "Made Resident";

        case kRmtResourceHistoryEventVirtualMemoryEvict:
            return "Evicted";

        case kRmtResourceHistoryEventResourceDestroyed:
            return "Resource destroyed";

        case kRmtResourceHistoryEventVirtualMemoryAllocated:
            return "Virtual memory allocated";

        case kRmtResourceHistoryEventVirtualMemoryFree:
            return "Virtual memory freed";

        case kRmtResourceHistoryEventPhysicalMapToLocal:
            return "Physical memory mapped to VRAM";

        case kRmtResourceHistoryEventPhysicalUnmap:
            return "Physical memory unmapped";

        case kRmtResourceHistoryEventPhysicalMapToHost:
            return "Physical memory mapped to host";

        case kRmtResourceHistoryEventSnapshotTaken:
            return "Snapshot taken";

        case kRmtResourceHistoryEventResourceNamed:
            return "Resource named";

        default:
            return "-";
            break;
        }
    }
}  // namespace rmv
