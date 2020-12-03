//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of a proxy filter that processes multiple columns
/// of the allocation table.
//=============================================================================

#include "models/proxy_models/allocation_proxy_model.h"

#include <QTableView>

#include "rmt_assert.h"

#include "models/allocation_item_model.h"
#include "models/snapshot/allocation_explorer_model.h"

namespace rmv
{
    AllocationProxyModel::AllocationProxyModel(QObject* parent)
        : TableProxyModel(parent)
    {
    }

    AllocationProxyModel::~AllocationProxyModel()
    {
    }

    AllocationItemModel* AllocationProxyModel::InitializeAllocationTableModels(QTableView* table_view, int num_rows, int num_columns)
    {
        AllocationItemModel* model = new AllocationItemModel();
        model->SetRowCount(num_rows);
        model->SetColumnCount(num_columns);

        setSourceModel(model);
        SetFilterKeyColumns({kVirtualAllocationColumnId,
                             kVirtualAllocationColumnAllocationSize,
                             kVirtualAllocationColumnBound,
                             kVirtualAllocationColumnUnbound,
                             kVirtualAllocationColumnAverageResourceSize,
                             kVirtualAllocationColumnResourceSizeStdDev,
                             kVirtualAllocationColumnResourceCount,
                             kVirtualAllocationColumnPreferredHeapName,
                             kVirtualAllocationColumnInvisiblePercentage,
                             kVirtualAllocationColumnLocalPercentage,
                             kVirtualAllocationColumnSystemPercentage,
                             kVirtualAllocationColumnUnmappedPercentage});

        table_view->setModel(this);
        return model;
    }

    bool AllocationProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        if (FilterSizeSlider(source_row, kVirtualAllocationColumnAllocationSize, source_parent) == false)
        {
            return false;
        }

        if (FilterSearchString(source_row, source_parent) == false)
        {
            return false;
        }

        return true;
    }

    bool AllocationProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
    {
        if ((left.column() == kVirtualAllocationColumnId && right.column() == kVirtualAllocationColumnId))
        {
            const qulonglong addr_left  = left.data().toULongLong();
            const qulonglong addr_right = right.data().toULongLong();

            return addr_left < addr_right;
        }
        else if ((left.column() == kVirtualAllocationColumnAllocationSize && right.column() == kVirtualAllocationColumnAllocationSize))
        {
            const qulonglong size_left  = left.data(Qt::UserRole).toULongLong();
            const qulonglong size_right = right.data(Qt::UserRole).toULongLong();

            return size_left < size_right;
        }
        else if ((left.column() == kVirtualAllocationColumnBound && right.column() == kVirtualAllocationColumnBound))
        {
            const qulonglong bound_left  = left.data(Qt::UserRole).toULongLong();
            const qulonglong bound_right = right.data(Qt::UserRole).toULongLong();

            return bound_left < bound_right;
        }
        else if ((left.column() == kVirtualAllocationColumnUnbound && right.column() == kVirtualAllocationColumnUnbound))
        {
            const qulonglong unbound_left  = left.data(Qt::UserRole).toULongLong();
            const qulonglong unbound_right = right.data(Qt::UserRole).toULongLong();

            return unbound_left < unbound_right;
        }
        else if ((left.column() == kVirtualAllocationColumnAverageResourceSize && right.column() == kVirtualAllocationColumnAverageResourceSize))
        {
            const qulonglong size_left  = left.data(Qt::UserRole).toULongLong();
            const qulonglong size_right = right.data(Qt::UserRole).toULongLong();

            return size_left < size_right;
        }
        else if ((left.column() == kVirtualAllocationColumnResourceSizeStdDev && right.column() == kVirtualAllocationColumnResourceSizeStdDev))
        {
            const qulonglong std_dev_left  = left.data(Qt::UserRole).toULongLong();
            const qulonglong std_dev_right = right.data(Qt::UserRole).toULongLong();

            return std_dev_left < std_dev_right;
        }
        else if ((left.column() == kVirtualAllocationColumnResourceCount && right.column() == kVirtualAllocationColumnResourceCount))
        {
            const int resource_count_left  = left.data(Qt::UserRole).toInt();
            const int resource_count_right = right.data(Qt::UserRole).toInt();

            return resource_count_left < resource_count_right;
        }
        else if ((left.column() == kVirtualAllocationColumnInvisiblePercentage && right.column() == kVirtualAllocationColumnInvisiblePercentage))
        {
            const int resource_count_left  = left.data(Qt::UserRole).toULongLong();
            const int resource_count_right = right.data(Qt::UserRole).toULongLong();

            return resource_count_left < resource_count_right;
        }
        else if ((left.column() == kVirtualAllocationColumnLocalPercentage && right.column() == kVirtualAllocationColumnLocalPercentage))
        {
            const int resource_count_left  = left.data(Qt::UserRole).toULongLong();
            const int resource_count_right = right.data(Qt::UserRole).toULongLong();

            return resource_count_left < resource_count_right;
        }
        else if ((left.column() == kVirtualAllocationColumnSystemPercentage && right.column() == kVirtualAllocationColumnSystemPercentage))
        {
            const int resource_count_left  = left.data(Qt::UserRole).toULongLong();
            const int resource_count_right = right.data(Qt::UserRole).toULongLong();

            return resource_count_left < resource_count_right;
        }
        else if ((left.column() == kVirtualAllocationColumnUnmappedPercentage && right.column() == kVirtualAllocationColumnUnmappedPercentage))
        {
            const int resource_count_left  = left.data(Qt::UserRole).toULongLong();
            const int resource_count_right = right.data(Qt::UserRole).toULongLong();

            return resource_count_left < resource_count_right;
        }
        return QSortFilterProxyModel::lessThan(left, right);
    }
}  // namespace rmv