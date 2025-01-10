//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a proxy filter that processes multiple columns.
//=============================================================================

#include "models/proxy_models/memory_leak_finder_proxy_model.h"

#include "rmt_assert.h"

#include "models/resource_item_model.h"

namespace rmv
{
    MemoryLeakFinderProxyModel::MemoryLeakFinderProxyModel(uint32_t compare_id_filter, QObject* parent)
        : ResourceProxyModel(parent)
        , compare_id_filter_(compare_id_filter)
    {
    }

    MemoryLeakFinderProxyModel::~MemoryLeakFinderProxyModel()
    {
    }

    void MemoryLeakFinderProxyModel::UpdateCompareFilter(SnapshotCompareId compare_filter)
    {
        compare_id_filter_ = compare_filter;
    }

    bool MemoryLeakFinderProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        bool pass = ResourceProxyModel::filterAcceptsRow(source_row, source_parent);

        if (pass)
        {
            QModelIndex index = sourceModel()->index(source_row, kResourceColumnCompareId, source_parent);

            QString index_data = index.data().toString();

            bool           ok         = false;
            const uint32_t compare_id = index_data.toULong(&ok, 0);

            pass = ((compare_id_filter_ & compare_id) != 0U);
        }

        return pass;
    }
}  // namespace rmv
