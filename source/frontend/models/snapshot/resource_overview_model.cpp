//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Model implementation for the Resource Overview pane
//=============================================================================

#include "models/snapshot/resource_overview_model.h"

#include <QTableView>

#include "rmt_assert.h"
#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"

#include "models/trace_manager.h"
#include "util/string_util.h"

namespace rmv
{
    ResourceOverviewModel::ResourceOverviewModel()
        : ModelViewMapper(kResourceOverviewNumWidgets)
        , min_resource_size_(0)
        , max_resource_size_(0)
    {
    }

    ResourceOverviewModel::~ResourceOverviewModel()
    {
    }

    void ResourceOverviewModel::ResetModelValues()
    {
        SetModelData(kResourceOverviewTotalAvailableSize, "-");
        SetModelData(kResourceOverviewTotalAllocatedAndUsed, "-");
        SetModelData(kResourceOverviewTotalAllocatedAndUnused, "-");
        SetModelData(kResourceOverviewAllocationCount, "-");
        SetModelData(kResourceOverviewResourceCount, "-");

        const TraceManager&    trace_manager = TraceManager::Get();
        const RmtDataSnapshot* open_snapshot = trace_manager.GetOpenSnapshot();
        if (trace_manager.DataSetValid() && (open_snapshot != nullptr))
        {
            min_resource_size_ = open_snapshot->minimum_resource_size_in_bytes;
            max_resource_size_ = open_snapshot->maximum_resource_size_in_bytes;
        }
    }

    void ResourceOverviewModel::FilterBySizeChanged(int min_value, int max_value)
    {
        const TraceManager&    trace_manager = TraceManager::Get();
        const RmtDataSnapshot* open_snapshot = trace_manager.GetOpenSnapshot();
        if (trace_manager.DataSetValid() && (open_snapshot != nullptr))
        {
            min_resource_size_ = trace_manager.GetSizeFilterThreshold(min_value);
            max_resource_size_ = trace_manager.GetSizeFilterThreshold(max_value);
        }
    }

    bool ResourceOverviewModel::IsSizeInRange(uint64_t resource_size) const
    {
        if (resource_size >= min_resource_size_ && resource_size <= max_resource_size_)
        {
            return true;
        }
        return false;
    }

    void ResourceOverviewModel::Update()
    {
        ResetModelValues();

        const TraceManager&    trace_manager = TraceManager::Get();
        const RmtDataSnapshot* snapshot      = trace_manager.GetOpenSnapshot();
        if (!trace_manager.DataSetValid() || snapshot == nullptr)
        {
            return;
        }

        const uint64_t total_available      = RmtVirtualAllocationListGetTotalSizeInBytes(&snapshot->virtual_allocation_list);
        const uint64_t allocated_and_used   = RmtVirtualAllocationListGetBoundTotalSizeInBytes(snapshot, &snapshot->virtual_allocation_list);
        const uint64_t allocated_and_unused = RmtVirtualAllocationListGetUnboundTotalSizeInBytes(snapshot, &snapshot->virtual_allocation_list);

        SetModelData(kResourceOverviewTotalAvailableSize, rmv::string_util::LocalizedValueMemory(total_available, false, false));
        SetModelData(kResourceOverviewTotalAllocatedAndUsed, rmv::string_util::LocalizedValueMemory(allocated_and_used, false, false));
        SetModelData(kResourceOverviewTotalAllocatedAndUnused, rmv::string_util::LocalizedValueMemory(allocated_and_unused, false, false));

        SetModelData(kResourceOverviewAllocationCount, rmv::string_util::LocalizedValue(snapshot->virtual_allocation_list.allocation_count));
        SetModelData(kResourceOverviewResourceCount, rmv::string_util::LocalizedValue(snapshot->resource_list.resource_count));
    }
}  // namespace rmv