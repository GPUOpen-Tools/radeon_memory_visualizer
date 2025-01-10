//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a proxy filter that processes the resource details
/// table in the resource details pane.
//=============================================================================

#include "models/proxy_models/resource_details_proxy_model.h"

#include <QTableView>

#include "rmt_assert.h"

#include "models/snapshot/allocation_explorer_model.h"
#include "models/snapshot/resource_timeline_item_model.h"

namespace rmv
{
    ResourceDetailsProxyModel::ResourceDetailsProxyModel(QObject* parent)
        : TableProxyModel(parent)
    {
    }

    ResourceDetailsProxyModel::~ResourceDetailsProxyModel()
    {
    }

    ResourceTimelineItemModel* ResourceDetailsProxyModel::InitializeResourceTableModels(QTableView* view, int num_rows, int num_columns)
    {
        ResourceTimelineItemModel* model = new ResourceTimelineItemModel();
        model->SetRowCount(num_rows);
        model->SetColumnCount(num_columns);

        setSourceModel(model);
        SetFilterKeyColumns({
            kResourceHistoryColumnEvent,
            kResourceHistoryColumnTime,
            kResourceHistoryColumnVirtualAddress,
            kResourceHistoryColumnPhysicalAddress,
            kResourceHistoryColumnSize,
            kResourceHistoryColumnPageSize,
        });

        view->setModel(this);

        return model;
    }

    bool ResourceDetailsProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        Q_UNUSED(source_row);
        Q_UNUSED(source_parent);
        bool pass = true;
        return pass;
    }

    bool ResourceDetailsProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
    {
        if ((left.column() == kResourceHistoryColumnEvent && right.column() == kResourceHistoryColumnEvent))
        {
            QModelIndex left_data  = sourceModel()->index(left.row(), kResourceHistoryColumnEvent, QModelIndex());
            QModelIndex right_data = sourceModel()->index(right.row(), kResourceHistoryColumnEvent, QModelIndex());
            return QString::localeAwareCompare(left_data.data().toString(), right_data.data().toString()) < 0;
        }

        if ((left.column() == kResourceHistoryColumnTime && right.column() == kResourceHistoryColumnTime))
        {
            const double left_data  = left.data(Qt::UserRole).toULongLong();
            const double right_data = right.data(Qt::UserRole).toULongLong();
            return left_data < right_data;
        }
        return QSortFilterProxyModel::lessThan(left, right);
    }
}  // namespace rmv
