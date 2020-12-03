//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Model implementation for the Allocation delta model
//=============================================================================

#include "models/compare/allocation_delta_model.h"

#include "rmt_error.h"
#include "rmt_util.h"

#include "models/trace_manager.h"
#include "util/string_util.h"

// The number of allocation models needed. For this pane, there are 2
// allocation graphics.
static const int32_t kNumAllocationModels = 2;

namespace rmv
{
    AllocationDeltaModel::AllocationDeltaModel()
        : ModelViewMapper(kAllocationDeltaCompareNumWidgets)
        , base_index_(kSnapshotCompareBase)
        , diff_index_(kSnapshotCompareDiff)
        , base_snapshot_(nullptr)
        , diff_snapshot_(nullptr)
        , largest_allocation_size_(0)
    {
        allocation_bar_model_ = new AllocationBarModel(kNumAllocationModels, false);
    }

    AllocationDeltaModel::~AllocationDeltaModel()
    {
        delete allocation_bar_model_;
    }

    void AllocationDeltaModel::ResetModelValues()
    {
        SetModelData(kAllocationDeltaCompareBaseName, "-");
        SetModelData(kAllocationDeltaCompareBaseGraphicName, "-");
        SetModelData(kAllocationDeltaCompareDiffName, "-");
        SetModelData(kAllocationDeltaCompareDiffGraphicName, "-");

        base_index_ = kSnapshotCompareBase;
        diff_index_ = kSnapshotCompareDiff;
    }

    bool AllocationDeltaModel::Update(int32_t base_allocation_index, int32_t diff_allocation_index)
    {
        const TraceManager& trace_manager = TraceManager::Get();
        if (!trace_manager.DataSetValid())
        {
            return false;
        }

        base_snapshot_ = trace_manager.GetComparedSnapshot(base_index_);
        diff_snapshot_ = trace_manager.GetComparedSnapshot(diff_index_);

        if (base_snapshot_ == nullptr || diff_snapshot_ == nullptr)
        {
            return false;
        }

        SetModelData(kAllocationDeltaCompareBaseName, trace_manager.GetCompareSnapshotName(base_index_));
        SetModelData(kAllocationDeltaCompareBaseGraphicName, trace_manager.GetCompareSnapshotName(base_index_));
        SetModelData(kAllocationDeltaCompareDiffName, trace_manager.GetCompareSnapshotName(diff_index_));
        SetModelData(kAllocationDeltaCompareDiffGraphicName, trace_manager.GetCompareSnapshotName(diff_index_));

        const RmtVirtualAllocation* base_allocation = &base_snapshot_->virtual_allocation_list.allocation_details[base_allocation_index];
        const RmtVirtualAllocation* diff_allocation = &diff_snapshot_->virtual_allocation_list.allocation_details[diff_allocation_index];

        int64_t base_selected_size = RmtVirtualAllocationGetSizeInBytes(base_allocation);
        int64_t diff_selected_size = RmtVirtualAllocationGetSizeInBytes(diff_allocation);

        largest_allocation_size_ = RMT_MAXIMUM(base_selected_size, diff_selected_size);

        return true;
    }

    bool AllocationDeltaModel::UpdateAllocationDeltas(int32_t base_allocation_index, int32_t diff_allocation_index, QVector<DeltaItem>& out_allocation_data)
    {
        const RmtVirtualAllocation* base_virtual_allocation = &base_snapshot_->virtual_allocation_list.allocation_details[base_allocation_index];
        const RmtVirtualAllocation* diff_virtual_allocation = &diff_snapshot_->virtual_allocation_list.allocation_details[diff_allocation_index];

        if (base_virtual_allocation != nullptr && diff_virtual_allocation != nullptr)
        {
            int64_t base_size = RmtVirtualAllocationGetSizeInBytes(base_virtual_allocation);
            int64_t diff_size = RmtVirtualAllocationGetSizeInBytes(diff_virtual_allocation);

            int64_t base_bound = RmtVirtualAllocationGetTotalResourceMemoryInBytes(base_snapshot_, base_virtual_allocation);
            int64_t diff_bound = RmtVirtualAllocationGetTotalResourceMemoryInBytes(diff_snapshot_, diff_virtual_allocation);

            int64_t base_unbound = RmtVirtualAllocationGetTotalUnboundSpaceInAllocation(base_snapshot_, base_virtual_allocation);
            int64_t diff_unbound = RmtVirtualAllocationGetTotalUnboundSpaceInAllocation(diff_snapshot_, diff_virtual_allocation);

            int64_t base_average_resource_size = RmtVirtualAllocationGetAverageResourceSizeInBytes(base_snapshot_, base_virtual_allocation);
            int64_t diff_average_resource_size = RmtVirtualAllocationGetAverageResourceSizeInBytes(diff_snapshot_, diff_virtual_allocation);

            int64_t base_resource_count = base_virtual_allocation->resource_count;
            int64_t diff_resource_count = diff_virtual_allocation->resource_count;

            int64_t base_stanrdard_deviation = RmtVirtualAllocationGetResourceStandardDeviationInBytes(base_snapshot_, base_virtual_allocation);
            int64_t diff_standard_deviation  = RmtVirtualAllocationGetResourceStandardDeviationInBytes(diff_snapshot_, diff_virtual_allocation);

            out_allocation_data[kAllocationDeltaDataTypeAvailableSize].value_num         = diff_size - base_size;
            out_allocation_data[kAllocationDeltaDataTypeAllocatedAndUsed].value_num      = diff_bound - base_bound;
            out_allocation_data[kAllocationDeltaDataTypeAllocatedAndUnused].value_num    = diff_unbound - base_unbound;
            out_allocation_data[kAllocationDeltaDataTypeAverageAllocationSize].value_num = diff_average_resource_size - base_average_resource_size;
            out_allocation_data[kAllocationDeltaDataTypeStandardDeviation].value_num     = diff_standard_deviation - base_stanrdard_deviation;
            out_allocation_data[kAllocationDeltaDataTypeAllocationCount].value_num       = diff_resource_count - base_resource_count;

            return true;
        }
        return false;
    }

    bool AllocationDeltaModel::SwapSnapshots()
    {
        int temp_index = diff_index_;
        diff_index_    = base_index_;
        base_index_    = temp_index;

        return true;
    }

    RmtDataSnapshot* AllocationDeltaModel::GetSnapshotFromSnapshotIndex(int32_t snapshot_index) const
    {
        const TraceManager& trace_manager = TraceManager::Get();
        if (!trace_manager.DataSetValid())
        {
            return nullptr;
        }

        RmtDataSnapshot* snapshot = nullptr;
        if (snapshot_index == kSnapshotCompareBase)
        {
            snapshot = trace_manager.GetComparedSnapshot(base_index_);
        }
        else
        {
            snapshot = trace_manager.GetComparedSnapshot(diff_index_);
        }
        return snapshot;
    }

    double AllocationDeltaModel::GetAllocationSizeRatio(int32_t allocation_index, int32_t model_index) const
    {
        const RmtDataSnapshot* snapshot = GetSnapshotFromSnapshotIndex(model_index);
        if (snapshot == nullptr)
        {
            return 0.0;
        }

        // Make sure allocation index is in range.
        if (allocation_index < 0 || allocation_index >= snapshot->virtual_allocation_list.allocation_count)
        {
            return 0.0;
        }

        const RmtVirtualAllocation* allocation    = &snapshot->virtual_allocation_list.allocation_details[allocation_index];
        const uint64_t              size_in_bytes = RmtVirtualAllocationGetSizeInBytes(allocation);
        return (static_cast<float>(size_in_bytes) / static_cast<float>(largest_allocation_size_));
    }

    void AllocationDeltaModel::InitializeComboBox(int32_t snapshot_index, ArrowIconComboBox* combo_box) const
    {
        const RmtDataSnapshot* snapshot = GetSnapshotFromSnapshotIndex(snapshot_index);
        if (snapshot == nullptr)
        {
            return;
        }

        combo_box->ClearItems();

        for (int32_t current_allocation_index = 0; current_allocation_index < snapshot->virtual_allocation_list.allocation_count; current_allocation_index++)
        {
            const RmtVirtualAllocation* allocation = &snapshot->virtual_allocation_list.allocation_details[current_allocation_index];

            QString allocation_name = "Allocation " + QString::number(allocation->base_address) + " | " +
                                      rmv::string_util::LocalizedValueMemory(RmtVirtualAllocationGetSizeInBytes(allocation), false, false);

            QListWidgetItem* item = new QListWidgetItem(allocation_name);
            item->setData(Qt::UserRole, (qulonglong)allocation);
            combo_box->AddItem(item);
        }

        if (snapshot->virtual_allocation_list.allocation_count > 0)
        {
            combo_box->SetSelectedRow(0);
        }
    }

    void AllocationDeltaModel::SelectAllocation(int32_t snapshot_index, int32_t allocation_index)
    {
        const RmtDataSnapshot* snapshot = GetSnapshotFromSnapshotIndex(snapshot_index);
        if (snapshot != nullptr)
        {
            RmtVirtualAllocation* allocation = nullptr;

            if (allocation_index < snapshot->virtual_allocation_list.allocation_count)
            {
                allocation = &snapshot->virtual_allocation_list.allocation_details[allocation_index];
            }

            if (allocation != allocation_bar_model_->GetAllocation(0, snapshot_index))
            {
                allocation_bar_model_->SetSelectedResourceForAllocation(allocation, -1, snapshot_index);
            }
        }
    }

    const RmtResource* AllocationDeltaModel::GetResource(int32_t snapshot_index, RmtResourceIdentifier resource_identifier) const
    {
        const RmtDataSnapshot* snapshot = GetSnapshotFromSnapshotIndex(snapshot_index);
        if (snapshot == nullptr)
        {
            return nullptr;
        }

        const RmtResource* resource   = nullptr;
        RmtErrorCode       error_code = RmtResourceListGetResourceByResourceId(&snapshot->resource_list, resource_identifier, &resource);
        if (error_code != RMT_OK)
        {
            return nullptr;
        }
        return resource;
    }

    AllocationBarModel* AllocationDeltaModel::GetAllocationBarModel() const
    {
        return allocation_bar_model_;
    }
}  // namespace rmv