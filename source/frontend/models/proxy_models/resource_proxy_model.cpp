//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of a proxy filter that processes multiple columns.
//=============================================================================

#include "models/proxy_models/resource_proxy_model.h"

#include <QTableView>

#include "rmt_assert.h"

#include "models/resource_item_model.h"
#include "models/snapshot/allocation_explorer_model.h"

namespace rmv
{
    ResourceProxyModel::ResourceProxyModel(QObject* parent)
        : TableProxyModel(parent)
    {
    }

    ResourceProxyModel::~ResourceProxyModel()
    {
    }

    ResourceItemModel* ResourceProxyModel::InitializeResourceTableModels(QTableView* view, int num_rows, int num_columns)
    {
        ResourceItemModel* model = new ResourceItemModel();
        model->SetRowCount(num_rows);
        model->SetColumnCount(num_columns);

        setSourceModel(model);
        SetFilterKeyColumns({kResourceColumnName,
                             kResourceColumnVirtualAddress,
                             kResourceColumnSize,
                             kResourceColumnMappedInvisible,
                             kResourceColumnMappedLocal,
                             kResourceColumnMappedHost,
                             kResourceColumnMappedNone,
                             kResourceColumnPreferredHeap,
                             kResourceColumnUsage});

        view->setModel(this);
        return model;
    }

    void ResourceProxyModel::SetPreferredHeapFilter(const QString& preferred_heap_filter)
    {
        preferred_heap_filter_ = QRegularExpression(preferred_heap_filter, QRegularExpression::CaseInsensitiveOption);
    }

    void ResourceProxyModel::SetResourceUsageFilter(const QString& resource_usage_filter)
    {
        resource_usage_filter_ = QRegularExpression(resource_usage_filter, QRegularExpression::CaseInsensitiveOption);
    }

    bool ResourceProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        if (FilterSizeSlider(source_row, kResourceColumnSize, source_parent) == false)
        {
            return false;
        }

        if (FilterSearchString(source_row, source_parent) == false)
        {
            return false;
        }

        // Apply the preferred heap filter.
        const QModelIndex& preferred_heap_index = sourceModel()->index(source_row, kResourceColumnPreferredHeap, source_parent);
        if (preferred_heap_index.column() == kResourceColumnPreferredHeap)
        {
            QString index_data = preferred_heap_index.data().toString();
            if (preferred_heap_filter_.match(index_data).hasMatch() == false)
            {
                return false;
            }
        }

        // Apply the resource usage filter.
        const QModelIndex& resource_usage_index = sourceModel()->index(source_row, kResourceColumnUsage, source_parent);
        if (resource_usage_index.column() == kResourceColumnUsage)
        {
            QString index_data = resource_usage_index.data().toString();
            if (resource_usage_filter_.match(index_data).hasMatch() == false)
            {
                return false;
            }
        }

        return true;
    }

    bool ResourceProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
    {
        if ((left.column() == kResourceColumnVirtualAddress && right.column() == kResourceColumnVirtualAddress))
        {
            const qulonglong left_data  = left.data(Qt::UserRole).toULongLong();
            const qulonglong right_data = right.data(Qt::UserRole).toULongLong();

            return left_data < right_data;
        }
        else if ((left.column() == kResourceColumnSize && right.column() == kResourceColumnSize))
        {
            const qulonglong left_data  = left.data(Qt::UserRole).toULongLong();
            const qulonglong right_data = right.data(Qt::UserRole).toULongLong();

            return left_data < right_data;
        }
        else if ((left.column() == kResourceColumnMappedInvisible && right.column() == kResourceColumnMappedInvisible))
        {
            const double left_data  = left.data(Qt::UserRole).toDouble();
            const double right_data = right.data(Qt::UserRole).toDouble();

            return left_data < right_data;
        }
        else if ((left.column() == kResourceColumnMappedLocal && right.column() == kResourceColumnMappedLocal))
        {
            const double left_data  = left.data(Qt::UserRole).toDouble();
            const double right_data = right.data(Qt::UserRole).toDouble();

            return left_data < right_data;
        }
        else if ((left.column() == kResourceColumnMappedHost && right.column() == kResourceColumnMappedHost))
        {
            const double left_data  = left.data(Qt::UserRole).toDouble();
            const double right_data = right.data(Qt::UserRole).toDouble();

            return left_data < right_data;
        }
        else if ((left.column() == kResourceColumnMappedNone && right.column() == kResourceColumnMappedNone))
        {
            const double left_data  = left.data(Qt::UserRole).toDouble();
            const double right_data = right.data(Qt::UserRole).toDouble();

            return left_data < right_data;
        }
        return QSortFilterProxyModel::lessThan(left, right);
    }
}  // namespace rmv