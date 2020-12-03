//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Model implementation for the allocation overview pane
//=============================================================================

#include "models/snapshot/allocation_overview_model.h"

#include <vector>

#include "rmt_assert.h"
#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"

#include "models/trace_manager.h"
#include "util/string_util.h"

namespace rmv
{
    AllocationOverviewModel::AllocationOverviewModel(int32_t num_allocation_models)
        : ModelViewMapper(kAllocationOverviewNumWidgets)
        , sort_mode_(AllocationOverviewModel::SortMode::kSortModeAllocationSize)
        , sort_ascending_(false)
    {
        allocation_bar_model_ = new MultiAllocationBarModel(num_allocation_models);
        ResetModelValues();
    }

    AllocationOverviewModel::~AllocationOverviewModel()
    {
        delete allocation_bar_model_;
    }

    void AllocationOverviewModel::ResetModelValues()
    {
        allocation_bar_model_->ResetModelValues();
    }

    size_t AllocationOverviewModel::GetViewableAllocationCount() const
    {
        return allocation_bar_model_->GetViewableAllocationCount();
    }

    void AllocationOverviewModel::SetNormalizeAllocations(bool normalized)
    {
        allocation_bar_model_->SetNormalizeAllocations(normalized);
    }

    void AllocationOverviewModel::Sort(int sort_mode, bool ascending)
    {
        sort_mode_      = static_cast<AllocationOverviewModel::SortMode>(sort_mode);
        sort_ascending_ = ascending;
        allocation_bar_model_->Sort(sort_mode, ascending);
    }

    void AllocationOverviewModel::ApplyFilters(const QString& filter_text, const bool* heap_array_flags)
    {
        ResetModelValues();
        allocation_bar_model_->ApplyAllocationFilters(filter_text, heap_array_flags, sort_mode_, sort_ascending_);
    }

    size_t AllocationOverviewModel::SelectResource(RmtResourceIdentifier resource_identifier, int32_t model_index)
    {
        return allocation_bar_model_->SelectResource(resource_identifier, model_index);
    }

    MultiAllocationBarModel* AllocationOverviewModel::GetAllocationBarModel() const
    {
        return allocation_bar_model_;
    }
}  // namespace rmv