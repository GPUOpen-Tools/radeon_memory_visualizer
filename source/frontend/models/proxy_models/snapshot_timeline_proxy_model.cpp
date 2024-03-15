//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a proxy filter that processes multiple columns.
//=============================================================================

#include "models/proxy_models/snapshot_timeline_proxy_model.h"

#include "rmt_assert.h"

#include "models/timeline/snapshot_item_model.h"

namespace rmv
{
    SnapshotTimelineProxyModel::SnapshotTimelineProxyModel(QObject* parent)
        : TableProxyModel(parent)
    {
    }

    SnapshotTimelineProxyModel::~SnapshotTimelineProxyModel()
    {
    }

    bool SnapshotTimelineProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        if (FilterSizeSlider(source_row, kSnapshotTimelineColumnResources, source_parent) == false)
        {
            return false;
        }

        return FilterSearchString(source_row, source_parent);
    }

    bool SnapshotTimelineProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
    {
        if ((left.column() == kSnapshotTimelineColumnName && right.column() == kSnapshotTimelineColumnName))
        {
            const QString left_data  = sourceModel()->data(left).toString();
            const QString right_data = sourceModel()->data(right).toString();
            return QString::localeAwareCompare(left_data, right_data) < 0;
        }
        else if ((left.column() == kSnapshotTimelineColumnTime && right.column() == kSnapshotTimelineColumnTime))
        {
            const double left_data  = left.data(Qt::UserRole).toULongLong();
            const double right_data = right.data(Qt::UserRole).toULongLong();
            return left_data < right_data;
        }
        else if ((left.column() == kSnapshotTimelineColumnVirtualAllocations && right.column() == kSnapshotTimelineColumnVirtualAllocations))
        {
            const double left_data  = left.data(Qt::UserRole).toInt();
            const double right_data = right.data(Qt::UserRole).toInt();
            return left_data < right_data;
        }
        else if ((left.column() == kSnapshotTimelineColumnResources && right.column() == kSnapshotTimelineColumnResources))
        {
            const double left_data  = left.data(Qt::UserRole).toInt();
            const double right_data = right.data(Qt::UserRole).toInt();
            return left_data < right_data;
        }
        else if ((left.column() == kSnapshotTimelineColumnAllocatedTotalVirtualMemory && right.column() == kSnapshotTimelineColumnAllocatedTotalVirtualMemory))
        {
            const double left_data  = left.data(Qt::UserRole).toULongLong();
            const double right_data = right.data(Qt::UserRole).toULongLong();
            return left_data < right_data;
        }
        else if ((left.column() == kSnapshotTimelineColumnAllocatedBoundVirtualMemory && right.column() == kSnapshotTimelineColumnAllocatedBoundVirtualMemory))
        {
            const double left_data  = left.data(Qt::UserRole).toULongLong();
            const double right_data = right.data(Qt::UserRole).toULongLong();
            return left_data < right_data;
        }
        else if ((left.column() == kSnapshotTimelineColumnAllocatedUnboundVirtualMemory &&
                  right.column() == kSnapshotTimelineColumnAllocatedUnboundVirtualMemory))
        {
            const double left_data  = left.data(Qt::UserRole).toULongLong();
            const double right_data = right.data(Qt::UserRole).toULongLong();
            return left_data < right_data;
        }
        else if ((left.column() == kSnapshotTimelineColumnCommittedLocal && right.column() == kSnapshotTimelineColumnCommittedLocal))
        {
            const double left_data  = left.data(Qt::UserRole).toULongLong();
            const double right_data = right.data(Qt::UserRole).toULongLong();
            return left_data < right_data;
        }
        else if ((left.column() == kSnapshotTimelineColumnCommittedInvisible && right.column() == kSnapshotTimelineColumnCommittedInvisible))
        {
            const double left_data  = left.data(Qt::UserRole).toULongLong();
            const double right_data = right.data(Qt::UserRole).toULongLong();
            return left_data < right_data;
        }
        else if ((left.column() == kSnapshotTimelineColumnCommittedHost && right.column() == kSnapshotTimelineColumnCommittedHost))
        {
            const double left_data  = left.data(Qt::UserRole).toULongLong();
            const double right_data = right.data(Qt::UserRole).toULongLong();
            return left_data < right_data;
        }

        return QSortFilterProxyModel::lessThan(left, right);
    }
}  // namespace rmv