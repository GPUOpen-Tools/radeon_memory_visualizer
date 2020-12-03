//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Model implementation for Allocation List pane
//=============================================================================

#include "models/snapshot/resource_list_model.h"

#include <QTableView>

#include "rmt_assert.h"
#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"
#include "rmt_print.h"
#include "rmt_util.h"
#include "rmt_virtual_allocation_list.h"

#include "models/trace_manager.h"
#include "util/string_util.h"

namespace rmv
{
    ResourceListModel::ResourceListModel()
        : ModelViewMapper(kResourceListNumWidgets)
        , table_model_(nullptr)
        , proxy_model_(nullptr)
    {
    }

    ResourceListModel::~ResourceListModel()
    {
        delete table_model_;
    }

    void ResourceListModel::ResetModelValues()
    {
        table_model_->removeRows(0, table_model_->rowCount());
        table_model_->SetRowCount(0);

        SetModelData(kResourceListTotalResources, "-");
        SetModelData(kResourceListTotalSize, "-");
    }

    void ResourceListModel::UpdateBottomLabels()
    {
        int32_t row_count  = proxy_model_->rowCount();
        int64_t total_size = 0;

        for (int32_t i = 0; i < row_count; i++)
        {
            const uint64_t size = proxy_model_->GetData(i, kResourceColumnSize);
            total_size += size;
        }

        SetModelData(kResourceListTotalResources, rmv::string_util::LocalizedValue(proxy_model_->rowCount()));
        SetModelData(kResourceListTotalSize, rmv::string_util::LocalizedValueMemory(total_size, false, false));
    }

    void ResourceListModel::Update()
    {
        ResetModelValues();
        UpdateTable();
        UpdateBottomLabels();
    }

    void ResourceListModel::UpdateTable()
    {
        const TraceManager& trace_manager = TraceManager::Get();
        RmtDataSnapshot*    snapshot      = trace_manager.GetOpenSnapshot();

        if (trace_manager.DataSetValid() && (snapshot != nullptr))
        {
            const RmtResourceList* resource_list = &snapshot->resource_list;

            RMT_ASSERT(resource_list);

            table_model_->SetRowCount(resource_list->resource_count);
            for (int32_t currentResourceIndex = 0; currentResourceIndex < resource_list->resource_count; currentResourceIndex++)
            {
                const RmtResource* resource = &resource_list->resources[currentResourceIndex];
                table_model_->AddResource(snapshot, resource, kSnapshotCompareIdUndefined);
            }
            proxy_model_->invalidate();
        }
    }

    void ResourceListModel::UpdatePreferredHeapList(const QString& preferred_heap_filter)
    {
        proxy_model_->SetPreferredHeapFilter(preferred_heap_filter);
        proxy_model_->invalidate();

        UpdateBottomLabels();
    }

    void ResourceListModel::UpdateResourceUsageList(const QString& resource_usage_filter)
    {
        proxy_model_->SetResourceUsageFilter(resource_usage_filter);
        proxy_model_->invalidate();

        UpdateBottomLabels();
    }

    void ResourceListModel::InitializeTableModel(ScaledTableView* table_view, uint num_rows, uint num_columns)
    {
        if (proxy_model_ != nullptr)
        {
            delete proxy_model_;
            proxy_model_ = nullptr;
        }

        proxy_model_ = new ResourceProxyModel();
        table_model_ = proxy_model_->InitializeResourceTableModels(table_view, num_rows, num_columns);
        table_model_->Initialize(table_view, false);
    }

    void ResourceListModel::SearchBoxChanged(const QString& filter)
    {
        proxy_model_->SetSearchFilter(filter);
        proxy_model_->invalidate();

        UpdateBottomLabels();
    }

    void ResourceListModel::FilterBySizeChanged(int min_value, int max_value)
    {
        const TraceManager& trace_manager = TraceManager::Get();
        const uint64_t      scaled_min    = trace_manager.GetSizeFilterThreshold(min_value);
        const uint64_t      scaled_max    = trace_manager.GetSizeFilterThreshold(max_value);

        proxy_model_->SetSizeFilter(scaled_min, scaled_max);
        proxy_model_->invalidate();

        UpdateBottomLabels();
    }

    ResourceProxyModel* ResourceListModel::GetResourceProxyModel() const
    {
        return proxy_model_;
    }
}  // namespace rmv