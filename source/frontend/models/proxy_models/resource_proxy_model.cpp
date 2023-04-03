//=============================================================================
// Copyright (c) 2018-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a proxy filter that processes multiple columns.
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

        // Apply the range-based searching to the virtual address and resource size.
        // Only fail if the search string is outside the address range.
        bool found_range = false;

        // Convert search string to int64_t. Try decimal first and if that doesn't work, try hex.
        bool     ok      = false;
        uint64_t address = search_filter_.toULongLong(&ok, 10);
        if (!ok)
        {
            address = search_filter_.toULongLong(&ok, 16);
        }

        if (ok)
        {
            const QModelIndex& virtual_address_index = sourceModel()->index(source_row, kResourceColumnVirtualAddress, source_parent);
            const QModelIndex& size_index            = sourceModel()->index(source_row, kResourceColumnSize, source_parent);
            if (virtual_address_index.column() == kResourceColumnVirtualAddress && size_index.column() == kResourceColumnSize)
            {
                bool va_ok   = false;
                bool size_ok = false;
                // Get the virtual address data from the table user role so it doesn't have to be converted from a string.
                uint64_t virtual_address = virtual_address_index.data(Qt::UserRole).toULongLong(&va_ok);
                uint64_t size            = size_index.data(Qt::UserRole).toULongLong(&size_ok);
                if (size_ok && va_ok && size)
                {
                    if (address >= virtual_address && address < (virtual_address + size))
                    {
                        found_range = true;
                    }
                }
            }
        }

        // Range search not found, so just do the usual text search.
        if (found_range == false)
        {
            if (FilterSearchString(source_row, source_parent) == false)
            {
                return false;
            }
        }

        // Apply the preferred heap filter.
        const QModelIndex& preferred_heap_index = sourceModel()->index(source_row, kResourceColumnPreferredHeap, source_parent);
        if (preferred_heap_index.column() == kResourceColumnPreferredHeap)
        {
            QString index_data = preferred_heap_index.data().toString();
            if ((preferred_heap_filter_.isValid() == false) || (preferred_heap_filter_.match(index_data).hasMatch() == false))
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