//=============================================================================
// Copyright (c) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
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
            case kResourceHistoryLegend:
                return event_type;
            case kResourceHistoryEvent:
                return GetTextFromEventType(event_type);
            case kResourceHistoryTime:
                return rmv::time_util::ClockToTimeUnit(timestamp);

            default:
                break;
            }
        }
        else if (role == Qt::UserRole)
        {
            switch (index.column())
            {
            case kResourceHistoryLegend:
                return QVariant::fromValue<int>(row_index);
            case kResourceHistoryEvent:
                return QVariant::fromValue<int>(event_type);
            case kResourceHistoryTime:
                return QVariant::fromValue<qulonglong>(timestamp);

            default:
                break;
            }
        }
        else if (role == Qt::ForegroundRole)
        {
            switch (index.column())
            {
            case kResourceHistoryEvent:
            case kResourceHistoryTime:
                if (row > snapshot_table_index_)
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
            case kResourceHistoryLegend:
                return "Legend";
            case kResourceHistoryEvent:
                return "Event";
            case kResourceHistoryTime:
                return "Timestamp";
            case kResourceHistoryDetails:
                return "Details";

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

        default:
            return "-";
            break;
        }
    }
}  // namespace rmv