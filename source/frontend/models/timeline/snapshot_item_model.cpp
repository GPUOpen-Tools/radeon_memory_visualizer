//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a snapshot item model.
///
/// Used for the snapshot table in the snapshot generation pane in the timeline
/// tab.
///
//=============================================================================

#include "models/timeline/snapshot_item_model.h"

#include "rmt_assert.h"
#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"
#include "rmt_util.h"

#include "managers/trace_manager.h"
#include "util/string_util.h"
#include "util/time_util.h"

namespace rmv
{
    SnapshotItemModel::SnapshotItemModel(QObject* parent)
        : QAbstractItemModel(parent)
        , num_rows_(0)
        , num_columns_(0)
    {
    }

    SnapshotItemModel::~SnapshotItemModel()
    {
    }

    void SnapshotItemModel::SetRowCount(int rows)
    {
        num_rows_ = rows;
    }

    void SnapshotItemModel::SetColumnCount(int columns)
    {
        num_columns_ = columns;
    }

    QModelIndex SnapshotItemModel::buddy(const QModelIndex& current_index) const
    {
        // Only the snapshot name is editable.  If another column in the table is selected, switch editing to the name column.
        QModelIndex new_index = createIndex(current_index.row(), kSnapshotTimelineColumnName, current_index.internalPointer());
        return new_index;
    }

    bool SnapshotItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (index.column() == kSnapshotTimelineColumnName && role == Qt::EditRole)
        {
            if (!checkIndex(index))
            {
                return false;
            }

            TraceManager& trace_manager = TraceManager::Get();
            if (!trace_manager.DataSetValid())
            {
                return false;
            }

            int row = index.row();
            if (row >= RmtTraceLoaderGetSnapshotCount())
            {
                return false;
            }

            // Validate that the string isn't empty or is too long.
            QString new_snapshot_name = value.toString();
            if (new_snapshot_name.isEmpty())
            {
                return false;
            }

            if (new_snapshot_name.length() >= RMT_MAXIMUM_NAME_LENGTH)
            {
                return false;
            }

            // Make sure this new snapshot name doesn't exist already in the table. This also tests
            // the case to make sure the current snapshot name has changed.
            for (int i = 0; i < RmtTraceLoaderGetSnapshotCount(); i++)
            {
                const RmtSnapshotPoint* snapshot_point = RmtTraceLoaderGetSnapshotPoint(i);
                if (QString::compare(new_snapshot_name, snapshot_point->name, Qt::CaseSensitive) == 0)
                {
                    return false;
                }
            }

            // Set data in the model.
            RmtDataSet* data_set = trace_manager.GetDataSet();
            RmtDataSetRenameSnapshot(data_set, row, new_snapshot_name.toLatin1().data());
            return true;
        }

        return QAbstractItemModel::setData(index, value, role);
    }

    QVariant SnapshotItemModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
        {
            return QVariant();
        }

        TraceManager& trace_manager = TraceManager::Get();
        if (!trace_manager.DataSetValid())
        {
            return QVariant();
        }

        int row = index.row();
        if (row >= RmtTraceLoaderGetSnapshotCount())
        {
            return QVariant();
        }

        const RmtSnapshotPoint* snapshot_point = RmtTraceLoaderGetSnapshotPoint(row);
        int                     column         = index.column();

        if (role == Qt::DisplayRole)
        {
            switch (column)
            {
            case kSnapshotTimelineColumnID:
                return reinterpret_cast<qulonglong>(snapshot_point);
            case kSnapshotTimelineColumnName:
                return QString(snapshot_point->name);
            case kSnapshotTimelineColumnTime:
                return rmv::time_util::ClockToTimeUnit(snapshot_point->timestamp);
            default:
                break;
            }

            if (snapshot_point->cached_snapshot != nullptr)
            {
                switch (column)
                {
                case kSnapshotTimelineColumnVirtualAllocations:
                    return rmv::string_util::LocalizedValue(snapshot_point->virtual_allocations);
                case kSnapshotTimelineColumnResources:
                    return rmv::string_util::LocalizedValue(snapshot_point->resource_count);
                case kSnapshotTimelineColumnAllocatedTotalVirtualMemory:
                    return rmv::string_util::LocalizedValueMemory(snapshot_point->total_virtual_memory, false, false);
                case kSnapshotTimelineColumnAllocatedBoundVirtualMemory:
                    return rmv::string_util::LocalizedValueMemory(snapshot_point->bound_virtual_memory, false, false);
                case kSnapshotTimelineColumnAllocatedUnboundVirtualMemory:
                    return rmv::string_util::LocalizedValueMemory(snapshot_point->unbound_virtual_memory, false, false);
                case kSnapshotTimelineColumnCommittedLocal:
                    return rmv::string_util::LocalizedValueMemory(snapshot_point->committed_memory[kRmtHeapTypeLocal], false, false);
                case kSnapshotTimelineColumnCommittedInvisible:
                    return rmv::string_util::LocalizedValueMemory(snapshot_point->committed_memory[kRmtHeapTypeInvisible], false, false);
                case kSnapshotTimelineColumnCommittedHost:
                    return rmv::string_util::LocalizedValueMemory(snapshot_point->committed_memory[kRmtHeapTypeSystem], false, false);
                default:
                    break;
                }
            }
        }
        else if (role == Qt::UserRole)
        {
            switch (column)
            {
            case kSnapshotTimelineColumnID:
                return QVariant::fromValue<qulonglong>(reinterpret_cast<qulonglong>(snapshot_point));
            case kSnapshotTimelineColumnTime:
                return QVariant::fromValue<qulonglong>(snapshot_point->timestamp);
            default:
                break;
            }

            if (snapshot_point->cached_snapshot != nullptr)
            {
                switch (column)
                {
                case kSnapshotTimelineColumnVirtualAllocations:
                    return QVariant::fromValue<int>(snapshot_point->virtual_allocations);
                case kSnapshotTimelineColumnResources:
                    return QVariant::fromValue<int>(snapshot_point->resource_count);
                case kSnapshotTimelineColumnAllocatedTotalVirtualMemory:
                    return QVariant::fromValue<qulonglong>(snapshot_point->total_virtual_memory);
                case kSnapshotTimelineColumnAllocatedBoundVirtualMemory:
                    return QVariant::fromValue<qulonglong>(snapshot_point->bound_virtual_memory);
                case kSnapshotTimelineColumnAllocatedUnboundVirtualMemory:
                    return QVariant::fromValue<qulonglong>(snapshot_point->unbound_virtual_memory);
                case kSnapshotTimelineColumnCommittedLocal:
                    return QVariant::fromValue<qulonglong>(snapshot_point->committed_memory[kRmtHeapTypeLocal]);
                case kSnapshotTimelineColumnCommittedInvisible:
                    return QVariant::fromValue<qulonglong>(snapshot_point->committed_memory[kRmtHeapTypeInvisible]);
                case kSnapshotTimelineColumnCommittedHost:
                    return QVariant::fromValue<qulonglong>(snapshot_point->committed_memory[kRmtHeapTypeSystem]);
                default:
                    break;
                }
            }
        }
        if (role == Qt::EditRole)
        {
            if (column == kSnapshotTimelineColumnName)
            {
                return QString(snapshot_point->name);
            }
        }

        return QVariant();
    }

    Qt::ItemFlags SnapshotItemModel::flags(const QModelIndex& index) const
    {
        // Enable editing for all columns.
        // Editing is redirected to the snapshot name column if any other column has focus.
        // See SnapshotItemModel::buddy() method.
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    QVariant SnapshotItemModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section)
            {
            case kSnapshotTimelineColumnID:
                return "ID";
            case kSnapshotTimelineColumnName:
                return "Snapshot name";
            case kSnapshotTimelineColumnTime:
                return "Timestamp";
            case kSnapshotTimelineColumnVirtualAllocations:
                return "Virtual allocations";
            case kSnapshotTimelineColumnResources:
                return "Resources";
            case kSnapshotTimelineColumnAllocatedTotalVirtualMemory:
                return "Total virtual memory";
            case kSnapshotTimelineColumnAllocatedBoundVirtualMemory:
                return "Bound virtual memory";
            case kSnapshotTimelineColumnAllocatedUnboundVirtualMemory:
                return "Unbound virtual memory";
            case kSnapshotTimelineColumnCommittedLocal:
                return "Committed local memory";
            case kSnapshotTimelineColumnCommittedInvisible:
                return "Committed invisible memory";
            case kSnapshotTimelineColumnCommittedHost:
                return "Committed host memory";
            default:
                break;
            }
        }

        return QAbstractItemModel::headerData(section, orientation, role);
    }

    QModelIndex SnapshotItemModel::index(int row, int column, const QModelIndex& parent) const
    {
        if (!hasIndex(row, column, parent))
        {
            return QModelIndex();
        }

        return createIndex(row, column);
    }

    QModelIndex SnapshotItemModel::parent(const QModelIndex& index) const
    {
        Q_UNUSED(index);
        return QModelIndex();
    }

    int SnapshotItemModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);
        return num_rows_;
    }

    int SnapshotItemModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);
        return num_columns_;
    }
}  // namespace rmv
