//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the Memory Leak Finder model.
//=============================================================================

#include "models/compare/memory_leak_finder_model.h"

#include <QTableView>
#include <unordered_map>
#include <unordered_set>

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"
#include "rmt_print.h"
#include "rmt_resource_list.h"
#include "rmt_util.h"

#include "managers/snapshot_manager.h"
#include "managers/trace_manager.h"
#include "models/proxy_models/memory_leak_finder_proxy_model.h"
#include "models/resource_item_model.h"
#include "util/rmv_util.h"
#include "util/string_util.h"

static const QString kStatsText = "%1 resources, %2";

namespace rmv
{
    MemoryLeakFinderModel::MemoryLeakFinderModel()
        : ModelViewMapper(kMemoryLeakFinderNumWidgets)
        , table_model_(nullptr)
        , proxy_model_(nullptr)
        , stats_in_both_{}
        , stats_in_base_only_{}
        , stats_in_diff_only_{}
        , base_index_(kSnapshotCompareBase)
        , diff_index_(kSnapshotCompareDiff)

    {
        ResetStats();
    }

    MemoryLeakFinderModel::~MemoryLeakFinderModel()
    {
        delete table_model_;
    }

    void MemoryLeakFinderModel::InitializeTableModel(ScaledTableView* table_view, uint num_rows, uint num_columns, uint32_t compare_id_filter)
    {
        if (proxy_model_ != nullptr)
        {
            delete proxy_model_;
            proxy_model_ = nullptr;
        }

        proxy_model_ = new MemoryLeakFinderProxyModel(compare_id_filter);
        table_model_ = proxy_model_->InitializeResourceTableModels(table_view, num_rows, num_columns);
        table_model_->Initialize(table_view, true);
    }

    void MemoryLeakFinderModel::ResetModelValues()
    {
        table_model_->removeRows(0, table_model_->rowCount());
        table_model_->SetRowCount(0);

        SetModelData(kMemoryLeakFinderBaseStats, "-");
        SetModelData(kMemoryLeakFinderBothStats, "-");
        SetModelData(kMemoryLeakFinderDiffStats, "-");
        SetModelData(kMemoryLeakFinderTotalResources, "-");
        SetModelData(kMemoryLeakFinderTotalSize, "-");
        SetModelData(kMemoryLeakFinderBaseCheckbox, "-");
        SetModelData(kMemoryLeakFinderDiffCheckbox, "-");
        SetModelData(kMemoryLeakFinderBaseSnapshot, "-");
        SetModelData(kMemoryLeakFinderDiffSnapshot, "-");
    }

    bool MemoryLeakFinderModel::Update(SnapshotCompareId compare_filter)
    {
        if (!TraceManager::Get().DataSetValid())
        {
            return false;
        }

        const SnapshotManager& snapshot_manager = SnapshotManager::Get();
        const RmtDataSnapshot* base_snapshot    = snapshot_manager.GetCompareSnapshot(base_index_);
        const RmtDataSnapshot* diff_snapshot    = snapshot_manager.GetCompareSnapshot(diff_index_);

        if (base_snapshot == nullptr || diff_snapshot == nullptr)
        {
            return false;
        }

        RmtSnapshotPoint* base_snapshot_point = base_snapshot->snapshot_point;
        RmtSnapshotPoint* diff_snapshot_point = diff_snapshot->snapshot_point;

        if (base_snapshot_point == nullptr || diff_snapshot_point == nullptr)
        {
            return false;
        }

        const QString base_snapshot_name = QString(base_snapshot_point->name);
        const QString diff_snapshot_name = QString(diff_snapshot_point->name);

        SetModelData(kMemoryLeakFinderBaseCheckbox, "Resources unique to snapshot " + base_snapshot_name);
        SetModelData(kMemoryLeakFinderDiffCheckbox, "Resources unique to snapshot " + diff_snapshot_name);
        SetModelData(kMemoryLeakFinderBaseSnapshot, base_snapshot_name);
        SetModelData(kMemoryLeakFinderDiffSnapshot, diff_snapshot_name);

        // Set up the lists to contain worst-case scenario.
        // Uses unordered set/map for fast insert/delete.
        std::unordered_map<RmtResourceIdentifier, const RmtResource*> resources_in_base_only;
        std::unordered_set<const RmtResource*>                        resources_in_diff_only;
        std::unordered_set<const RmtResource*>                        resources_in_both;
        resources_in_base_only.reserve(base_snapshot->resource_list.resource_count);
        resources_in_diff_only.reserve(diff_snapshot->resource_list.resource_count);
        resources_in_both.reserve(RMT_MAXIMUM(base_snapshot->resource_list.resource_count, diff_snapshot->resource_list.resource_count));

        // Add the base resources to an unordered map, with the resource id as the key.
        // If this needs speeding up further, look for which resource list is smallest (base or diff) and populate that instead
        // of the base.
        for (int32_t current_resource_index = 0; current_resource_index < base_snapshot->resource_list.resource_count; ++current_resource_index)
        {
            const RmtResource* current_resource = &base_snapshot->resource_list.resources[current_resource_index];
            resources_in_base_only.insert(std::make_pair(current_resource->identifier, current_resource));
        }

        // Add the diff resources. If the diff resource is in the base resource list (look it up via resource id),
        // add it to the 'both' list and remove from base map, otherwise just add it to the compare (diff)
        for (int32_t current_resource_index = 0; current_resource_index < diff_snapshot->resource_list.resource_count; ++current_resource_index)
        {
            const RmtResource* current_resource = &diff_snapshot->resource_list.resources[current_resource_index];

            auto iter = resources_in_base_only.find(current_resource->identifier);
            if (iter != resources_in_base_only.end())
            {
                // Found, so add to both and remove from base.
                resources_in_both.insert(current_resource);
                resources_in_base_only.erase(iter);
            }
            else
            {
                // Not found, just add to the diff list.
                resources_in_diff_only.insert(current_resource);
            }
        }

        size_t size = resources_in_base_only.size() + resources_in_diff_only.size() + resources_in_both.size();
        table_model_->SetRowCount(static_cast<int>(size));

        ResetStats();

        int32_t row_index = 0;
        for (auto iter = resources_in_both.begin(); iter != resources_in_both.end(); ++iter)
        {
            const RmtResource* resource = *iter;
            table_model_->AddResource(base_snapshot, resource, kSnapshotCompareIdCommon);
            stats_in_both_.num_resources++;
            stats_in_both_.size += resource->size_in_bytes;
            row_index++;
        }

        for (auto iter = resources_in_base_only.begin(); iter != resources_in_base_only.end(); ++iter)
        {
            const RmtResource* resource = (*iter).second;
            table_model_->AddResource(base_snapshot, resource, kSnapshotCompareIdOpen);
            stats_in_base_only_.num_resources++;
            stats_in_base_only_.size += resource->size_in_bytes;
            row_index++;
        }

        for (auto iter = resources_in_diff_only.begin(); iter != resources_in_diff_only.end(); ++iter)
        {
            const RmtResource* resource = *iter;
            table_model_->AddResource(base_snapshot, resource, kSnapshotCompareIdCompared);
            stats_in_diff_only_.num_resources++;
            stats_in_diff_only_.size += resource->size_in_bytes;
            row_index++;
        }

        proxy_model_->UpdateCompareFilter(compare_filter);
        proxy_model_->invalidate();

        UpdateLabels();

        return true;
    }

    bool MemoryLeakFinderModel::SwapSnapshots(SnapshotCompareId compare_filter)
    {
        CompareSnapshots temp = diff_index_;
        diff_index_           = base_index_;
        base_index_           = temp;

        return Update(compare_filter);
    }

    void MemoryLeakFinderModel::ResetStats()
    {
        stats_in_both_.num_resources      = 0;
        stats_in_both_.size               = 0;
        stats_in_base_only_.num_resources = 0;
        stats_in_base_only_.size          = 0;
        stats_in_diff_only_.num_resources = 0;
        stats_in_diff_only_.size          = 0;
    }

    void MemoryLeakFinderModel::UpdateLabels()
    {
        SetModelData(kMemoryLeakFinderBaseStats,
                     kStatsText.arg(rmv::string_util::LocalizedValue(stats_in_base_only_.num_resources))
                         .arg(rmv::string_util::LocalizedValueMemory(stats_in_base_only_.size, false, false)));
        SetModelData(kMemoryLeakFinderBothStats,
                     kStatsText.arg(rmv::string_util::LocalizedValue(stats_in_both_.num_resources))
                         .arg(rmv::string_util::LocalizedValueMemory(stats_in_both_.size, false, false)));
        SetModelData(kMemoryLeakFinderDiffStats,
                     kStatsText.arg(rmv::string_util::LocalizedValue(stats_in_diff_only_.num_resources))
                         .arg(rmv::string_util::LocalizedValueMemory(stats_in_diff_only_.size, false, false)));

        uint32_t row_count  = proxy_model_->rowCount();
        uint64_t total_size = 0;

        for (uint32_t i = 0; i < row_count; i++)
        {
            const uint64_t size = proxy_model_->GetData(i, kResourceColumnSize);
            total_size += size;
        }

        SetModelData(kMemoryLeakFinderTotalResources, rmv::string_util::LocalizedValue(row_count));
        SetModelData(kMemoryLeakFinderTotalSize, rmv::string_util::LocalizedValueMemory(total_size, false, false));
    }

    void MemoryLeakFinderModel::SearchBoxChanged(const QString& filter)
    {
        proxy_model_->SetSearchFilter(filter);
        proxy_model_->invalidate();

        UpdateLabels();
    }

    void MemoryLeakFinderModel::FilterBySizeChanged(int min_value, int max_value)
    {
        const uint64_t scaled_min = rmv_util::CalculateSizeThresholdFromStepValue(min_value, rmv::kSizeSliderRange - 1);
        const uint64_t scaled_max = rmv_util::CalculateSizeThresholdFromStepValue(max_value, rmv::kSizeSliderRange - 1);

        proxy_model_->SetSizeFilter(scaled_min, scaled_max);
        proxy_model_->invalidate();

        UpdateLabels();
    }

    void MemoryLeakFinderModel::UpdatePreferredHeapList(const QString& preferred_heap_filter)
    {
        proxy_model_->SetPreferredHeapFilter(preferred_heap_filter);
        proxy_model_->invalidate();
    }

    void MemoryLeakFinderModel::UpdateResourceUsageList(const QString& resource_usage_filter)
    {
        proxy_model_->SetResourceUsageFilter(resource_usage_filter);
        proxy_model_->invalidate();
    }

    MemoryLeakFinderProxyModel* MemoryLeakFinderModel::GetResourceProxyModel() const
    {
        return proxy_model_;
    }

    RmtSnapshotPoint* MemoryLeakFinderModel::FindSnapshot(const QModelIndex& index) const
    {
        const int              compare_id       = GetResourceProxyModel()->GetData(index.row(), kResourceColumnCompareId);
        RmtDataSnapshot*       snapshot         = nullptr;
        const SnapshotManager& snapshot_manager = SnapshotManager::Get();

        // Use base snapshot if open or common to both.
        if ((compare_id & kSnapshotCompareIdOpen) != 0 || (compare_id & kSnapshotCompareIdCommon) != 0)
        {
            snapshot = snapshot_manager.GetCompareSnapshot(base_index_);
        }
        else
        {
            snapshot = snapshot_manager.GetCompareSnapshot(diff_index_);
        }

        // Set up the single snapshot point for loading (if necessary).
        if (snapshot != nullptr)
        {
            RmtSnapshotPoint* snapshot_point = snapshot->snapshot_point;
            SnapshotManager::Get().SetSelectedSnapshotPoint(snapshot_point);
            return snapshot_point;
        }
        return nullptr;
    }
}  // namespace rmv
