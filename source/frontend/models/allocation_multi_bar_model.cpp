//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the allocation multi bar model class.
///
/// This model derives from the allocation bar base class and contains
/// additional support for displays with multiple allocations as seen in the
/// allocation overview pane. These allocation are rendered using
/// RMVAllocationBar objects.
///
//=============================================================================

#include "models/allocation_multi_bar_model.h"

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"
#include "rmt_print.h"

#include "managers/trace_manager.h"
#include "models/snapshot/allocation_overview_model.h"
#include "util/rmv_util.h"
#include "util/string_util.h"

namespace rmv
{
    /// @brief Compare functor used to sort allocations.
    ///
    /// Handles the compare functions for all the "sort by" modes used by the allocation overview pane.
    /// Also handles ascending and descending ordering.
    /// This has the advantage of being able to do the whole comparison within a single 'function call'
    /// rather than having several static comparison functions.
    class SortComparator
    {
    public:
        /// @brief Constructor.
        ///
        /// @param [in] sort_mode The sort mode required.
        /// @param [in] ascending If true, sort ascending, Otherwise descending.
        SortComparator(AllocationOverviewModel::SortMode sort_mode, bool ascending)
            : sort_mode_(sort_mode)
            , ascending_(ascending)
        {
        }

        /// @brief Destructor.
        ~SortComparator()
        {
        }

        /// @brief Overloaded () operator which does the actual comparisons.
        ///
        /// This gets called by std::sort.
        ///
        /// @param [in] virtual_allocation_a Pointer to first RmtVirtualAllocation to compare.
        /// @param [in] virtual_allocation_b Pointer to second RmtVirtualAllocation to compare.
        bool operator()(const RmtVirtualAllocation* virtual_allocation_a, const RmtVirtualAllocation* virtual_allocation_b) const
        {
            if (sort_mode_ == AllocationOverviewModel::kSortModeAllocationID)
            {
                const QString value_a = rmv_util::GetVirtualAllocationName(virtual_allocation_a);
                const QString value_b = rmv_util::GetVirtualAllocationName(virtual_allocation_b);

                // Decide on whether ascending or descending sort is required and return the appropriate result.
                if (ascending_)
                {
                    return value_a.compare(value_b, Qt::CaseInsensitive) < 0;
                }

                return value_a.compare(value_b, Qt::CaseInsensitive) > 0;
            }
            else
            {
                uint64_t value_a = 0;
                uint64_t value_b = 0;

                // Decide which sort mode to use and calculate the comparison arguments.
                switch (sort_mode_)
                {
                case AllocationOverviewModel::kSortModeAllocationSize:
                    value_a = RmtVirtualAllocationGetSizeInBytes(virtual_allocation_a);
                    value_b = RmtVirtualAllocationGetSizeInBytes(virtual_allocation_b);
                    break;

                case AllocationOverviewModel::kSortModeAllocationAge:
                    value_a = virtual_allocation_a->timestamp;
                    value_b = virtual_allocation_b->timestamp;
                    break;

                case AllocationOverviewModel::kSortModeResourceCount:
                    value_a = virtual_allocation_a->resource_count;
                    value_b = virtual_allocation_b->resource_count;
                    break;

                case AllocationOverviewModel::kSortModeFragmentationScore:
                    value_a = RmtVirtualAllocationGetFragmentationQuotient(virtual_allocation_a);
                    value_b = RmtVirtualAllocationGetFragmentationQuotient(virtual_allocation_b);
                    break;

                default:
                    // Should not get here.
                    RMT_ASSERT_MESSAGE(false, "Allocation overview pane: Invalid sort mode.");
                    break;
                }

                // Decide on whether ascending or descending sort is required and return the appropriate result.
                if (ascending_)
                {
                    return value_a < value_b;
                }

                return value_a > value_b;
            }
        }

    private:
        AllocationOverviewModel::SortMode sort_mode_;  ///< The sort mode to use for the comparison
        bool                              ascending_;  ///< If true, use ascending sort. Otherwise descending
    };

    MultiAllocationBarModel::MultiAllocationBarModel(uint32_t model_count)
        : AllocationBarModel(model_count, true)
        , largest_allocation_size_(0)
        , normalize_allocations_(false)
        , allocation_offset_(0)
    {
    }

    MultiAllocationBarModel::~MultiAllocationBarModel()
    {
    }

    double MultiAllocationBarModel::GetBytesPerPixel(int32_t scene_index, int32_t model_index, int32_t width) const
    {
        RMT_ASSERT(width > 0);
        const RmtVirtualAllocation* allocation = GetAllocation(scene_index, model_index);
        RMT_ASSERT(allocation != nullptr);
        if (allocation != nullptr)
        {
            uint64_t allocation_size = RmtVirtualAllocationGetSizeInBytes(allocation);
            if (normalize_allocations_)
            {
                return static_cast<double>(allocation_size) / static_cast<double>(width);
            }

            return static_cast<double>(largest_allocation_size_) / static_cast<double>(width);
        }

        return 1.0;
    }

    const RmtVirtualAllocation* MultiAllocationBarModel::GetAllocation(int32_t scene_index, int32_t model_index) const
    {
        Q_UNUSED(model_index);
        scene_index += allocation_offset_;
        if (scene_index < static_cast<int32_t>(shown_allocation_list_.size()))
        {
            return shown_allocation_list_[scene_index];
        }
        return nullptr;
    }

    void MultiAllocationBarModel::ResetModelValues()
    {
        largest_allocation_size_ = 0;
        shown_allocation_list_.clear();
    }

    size_t MultiAllocationBarModel::GetAllocationIndex(const RmtVirtualAllocation* allocation) const
    {
        for (size_t index = 0; index < shown_allocation_list_.size(); index++)
        {
            if (shown_allocation_list_[index] == allocation)
            {
                return index;
            }
        }
        return UINT64_MAX;
    }

    size_t MultiAllocationBarModel::GetViewableAllocationCount() const
    {
        return shown_allocation_list_.size();
    }

    void MultiAllocationBarModel::SetNormalizeAllocations(bool normalized)
    {
        normalize_allocations_ = normalized;
    }

    void MultiAllocationBarModel::Sort(int sort_mode, bool ascending)
    {
        std::stable_sort(
            shown_allocation_list_.begin(), shown_allocation_list_.end(), SortComparator(static_cast<AllocationOverviewModel::SortMode>(sort_mode), ascending));
    }

    void MultiAllocationBarModel::ApplyAllocationFilters(const QString& filter_text, const bool* heap_array_flags, int sort_mode, bool ascending)
    {
        if (TraceManager::Get().DataSetValid())
        {
            const RmtDataSnapshot* open_snapshot = SnapshotManager::Get().GetOpenSnapshot();
            if (open_snapshot != nullptr)
            {
                const RmtVirtualAllocationList* allocation_list  = &open_snapshot->virtual_allocation_list;
                int32_t                         allocation_count = allocation_list->allocation_count;

                if (allocation_count > 0)
                {
                    for (int32_t i = 0; i < allocation_count; i++)
                    {
                        const RmtVirtualAllocation* virtual_allocation = &allocation_list->allocation_details[i];
                        int                         heap_index         = virtual_allocation->heap_preferences[0];
                        bool                        allow              = false;

                        if (heap_array_flags[heap_index])
                        {
                            QString description = GetTitleText(virtual_allocation) + GetDescriptionText(virtual_allocation);

                            if (filter_text.isEmpty() == false)
                            {
                                if (description.contains(filter_text, Qt::CaseInsensitive))
                                {
                                    allow = true;
                                }
                            }
                            else
                            {
                                allow = true;
                            }
                        }

                        if (allow)
                        {
                            shown_allocation_list_.push_back(virtual_allocation);
                        }

                        uint64_t allocation_size = RmtVirtualAllocationGetSizeInBytes(virtual_allocation);
                        if (allocation_size > largest_allocation_size_)
                        {
                            largest_allocation_size_ = allocation_size;
                        }
                    }
                }
                Sort(sort_mode, ascending);
            }
        }
    }

    size_t MultiAllocationBarModel::SelectResource(RmtResourceIdentifier resource_identifier, int32_t model_index)
    {
        const RmtVirtualAllocation* resource_allocation = GetAllocationFromResourceID(resource_identifier, model_index);
        size_t                      index               = GetAllocationIndex(resource_allocation);

        if (index != UINT64_MAX)
        {
            for (int32_t resource_index = 0; resource_index < resource_allocation->resource_count; resource_index++)
            {
                if (resource_allocation->resources[resource_index]->identifier == resource_identifier)
                {
                    AllocationBarModel::SelectResource(static_cast<int32_t>(index - allocation_offset_), model_index, resource_index);
                    break;
                }
            }
        }
        return index;
    }

    void MultiAllocationBarModel::SetAllocationOffset(int32_t scene_offset)
    {
        allocation_offset_ = scene_offset;
    }
}  // namespace rmv
