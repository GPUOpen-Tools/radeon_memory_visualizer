//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the Snapshot Delta model.
//=============================================================================

#include "models/compare/snapshot_delta_model.h"

#include "rmt_assert.h"
#include "rmt_print.h"

#include "managers/trace_manager.h"
#include "views/custom_widgets/rmv_carousel.h"

namespace rmv
{
    SnapshotDeltaModel::SnapshotDeltaModel()
        : ModelViewMapper(kHeapDeltaCompareNumWidgets)
        , base_index_(kSnapshotCompareBase)
        , diff_index_(kSnapshotCompareDiff)
        , base_snapshot_(nullptr)
        , diff_snapshot_(nullptr)
    {
    }

    SnapshotDeltaModel::~SnapshotDeltaModel()
    {
    }

    void SnapshotDeltaModel::ResetModelValues()
    {
        SetModelData(kHeapDeltaCompareBaseName, "-");
        SetModelData(kHeapDeltaCompareDiffName, "-");

        base_index_ = kSnapshotCompareBase;
        diff_index_ = kSnapshotCompareDiff;
    }

    bool SnapshotDeltaModel::Update()
    {
        if (!TraceManager::Get().DataSetValid())
        {
            return false;
        }

        const SnapshotManager& snapshot_manager = SnapshotManager::Get();
        base_snapshot_                          = snapshot_manager.GetCompareSnapshot(base_index_);
        diff_snapshot_                          = snapshot_manager.GetCompareSnapshot(diff_index_);

        if (base_snapshot_ == nullptr || diff_snapshot_ == nullptr)
        {
            return false;
        }
        RMT_ASSERT(base_snapshot_->snapshot_point != diff_snapshot_->snapshot_point);

        SetModelData(kHeapDeltaCompareBaseName, snapshot_manager.GetCompareSnapshotName(base_index_));
        SetModelData(kHeapDeltaCompareDiffName, snapshot_manager.GetCompareSnapshotName(diff_index_));

        return true;
    }

    bool SnapshotDeltaModel::SwapSnapshots()
    {
        CompareSnapshots temp = diff_index_;
        diff_index_           = base_index_;
        base_index_           = temp;

        return Update();
    }

    void SnapshotDeltaModel::UpdateCarousel(RMVCarousel* carousel)
    {
        carousel->UpdateModel(base_snapshot_, diff_snapshot_);
    }

    const char* SnapshotDeltaModel::GetHeapName(int32_t heap_index) const
    {
        return RmtGetHeapTypeNameFromHeapType((RmtHeapType)heap_index);
    }

    bool SnapshotDeltaModel::GetHeapDelta(RmtDataSnapshot* snapshot, RmtHeapType heap_type, HeapDeltaData& delta_data) const
    {
        bool success = false;

        if (TraceManager::Get().DataSetValid())
        {
            if (snapshot != nullptr)
            {
                success = true;

                delta_data               = {};
                uint64_t total_allocated = 0;

                for (int32_t currentVirtualAllocationIndex = 0; currentVirtualAllocationIndex < snapshot->virtual_allocation_list.allocation_count;
                     currentVirtualAllocationIndex++)
                {
                    const RmtVirtualAllocation* virtual_allocation = &snapshot->virtual_allocation_list.allocation_details[currentVirtualAllocationIndex];

                    if (virtual_allocation->heap_preferences[0] == heap_type)
                    {
                        total_allocated += RmtVirtualAllocationGetSizeInBytes(virtual_allocation);

                        // Accumulate in here.
                        delta_data.allocation_count++;
                        delta_data.resource_count += virtual_allocation->resource_count;
                        delta_data.total_allocated_and_bound += RmtVirtualAllocationGetTotalResourceMemoryInBytes(snapshot, virtual_allocation);
                        delta_data.total_allocated_and_unbound += RmtVirtualAllocationGetTotalUnboundSpaceInAllocation(snapshot, virtual_allocation);
                    }
                }

                delta_data.total_available_size = 0;
                delta_data.free_space           = 0;
            }
        }

        return success;
    }

    bool SnapshotDeltaModel::CalcPerHeapDelta(RmtHeapType heap_type, HeapDeltaData& out_delta_data) const
    {
        if (TraceManager::Get().DataSetValid())
        {
            if (base_snapshot_ != nullptr && diff_snapshot_ != nullptr)
            {
                HeapDeltaData base_data    = {};
                bool          base_success = GetHeapDelta(base_snapshot_, heap_type, base_data);

                HeapDeltaData diff_data   = {};
                bool          dff_success = GetHeapDelta(diff_snapshot_, heap_type, diff_data);

                if (base_success && dff_success)
                {
                    out_delta_data.total_available_size        = base_data.total_available_size;
                    out_delta_data.total_allocated_and_bound   = diff_data.total_allocated_and_bound - base_data.total_allocated_and_bound;
                    out_delta_data.total_allocated_and_unbound = diff_data.total_allocated_and_unbound - base_data.total_allocated_and_unbound;
                    out_delta_data.free_space                  = diff_data.free_space - base_data.free_space;
                    out_delta_data.resource_count              = diff_data.resource_count - base_data.resource_count;
                    out_delta_data.allocation_count            = diff_data.allocation_count - base_data.allocation_count;
                    return true;
                }
            }
        }

        return false;
    }
}  // namespace rmv