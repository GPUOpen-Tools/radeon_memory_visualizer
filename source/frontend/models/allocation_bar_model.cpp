//=============================================================================
// Copyright (c) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the allocation bar model base class.
///
/// This model holds any state information for derived models that use a
/// graphical representation of an allocation and is used for a single
/// allocation bar, as seen in the allocation explorer pane. These allocations
/// are rendered using RMVAllocationBar objects.
///
//=============================================================================

#include "models/allocation_bar_model.h"

#include "rmt_assert.h"
#include "rmt_print.h"
#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"

#include "util/string_util.h"

#include "managers/trace_manager.h"
#include "models/snapshot/allocation_overview_model.h"

namespace rmv
{
    AllocationBarModel::AllocationBarModel(uint32_t model_count, bool show_details)
        : model_count_(model_count)
        , show_details_(show_details)
        , show_aliased_(false)
    {
        selection_state_ = new SelectionState[model_count];
        ClearSelectionState();
    }

    AllocationBarModel::~AllocationBarModel()
    {
        delete[] selection_state_;
    }

    QString AllocationBarModel::GetTitleText(int32_t allocation_index, int32_t model_index) const
    {
        const RmtVirtualAllocation* allocation = GetAllocation(allocation_index, model_index);
        if (allocation == nullptr)
        {
            return QString();
        }
        return GetTitleText(allocation);
    }

    QString AllocationBarModel::GetTitleText(const RmtVirtualAllocation* allocation) const
    {
        return "Allocation: " + QString::number(allocation->base_address) +
               " - Heap: " + QString(RmtGetHeapTypeNameFromHeapType(allocation->heap_preferences[0]));
    }

    QString AllocationBarModel::GetDescriptionText(int32_t allocation_index, int32_t model_index) const
    {
        const RmtVirtualAllocation* allocation = GetAllocation(allocation_index, model_index);
        if (allocation == nullptr)
        {
            return QString();
        }
        return GetDescriptionText(allocation);
    }

    QString AllocationBarModel::GetDescriptionText(const RmtVirtualAllocation* allocation) const
    {
        return "  (Size: " + rmv::string_util::LocalizedValueMemory(RmtVirtualAllocationGetSizeInBytes(allocation), false, false) +
               " - Resources: " + QString::number(allocation->resource_count) + ")";
    }

    double AllocationBarModel::GetBytesPerPixel(int32_t allocation_index, int32_t model_index, int32_t width) const
    {
        RMT_ASSERT(width > 0);
        const RmtVirtualAllocation* allocation = GetAllocation(allocation_index, model_index);
        RMT_ASSERT(allocation != nullptr);
        if (allocation != nullptr)
        {
            uint64_t allocation_size = RmtVirtualAllocationGetSizeInBytes(allocation);
            return static_cast<double>(allocation_size) / static_cast<double>(width);
        }

        return 1.0;
    }

    const RmtVirtualAllocation* AllocationBarModel::GetAllocation(int32_t scene_index, int32_t model_index) const
    {
        Q_UNUSED(scene_index);
        return selection_state_[model_index].selected_allocation;
    }

    bool AllocationBarModel::ShowDetails() const
    {
        return show_details_;
    }

    int32_t AllocationBarModel::GetHoveredResourceForAllocation(int32_t allocation_index, int32_t model_index) const
    {
        const RmtVirtualAllocation* allocation = GetAllocation(allocation_index, model_index);
        if (allocation == selection_state_[model_index].hovered_allocation && allocation != nullptr)
        {
            return selection_state_[model_index].hovered_resource;
        }
        return -1;
    }

    int32_t AllocationBarModel::GetSelectedResourceForAllocation(int32_t allocation_index, int32_t model_index) const
    {
        const RmtVirtualAllocation* allocation = GetAllocation(allocation_index, model_index);
        if (allocation == selection_state_[model_index].selected_allocation && allocation != nullptr)
        {
            return selection_state_[model_index].selected_resource;
        }
        return -1;
    }

    void AllocationBarModel::SetHoveredResourceForAllocation(int32_t allocation_index, int32_t resource_index, int32_t model_index)
    {
        const RmtVirtualAllocation* allocation = GetAllocation(allocation_index, model_index);
        if (allocation != nullptr)
        {
            selection_state_[model_index].hovered_allocation = allocation;
            selection_state_[model_index].hovered_resource   = resource_index;
        }
    }

    void AllocationBarModel::SetSelectedResourceForAllocation(int32_t allocation_index, int32_t resource_index, int32_t model_index)
    {
        const RmtVirtualAllocation* allocation = GetAllocation(allocation_index, model_index);
        SetSelectedResourceForAllocation(allocation, resource_index, model_index);
    }

    void AllocationBarModel::SetHoveredResourceForAllocation(int32_t        allocation_index,
                                                             int32_t        model_index,
                                                             int32_t        width,
                                                             int32_t        height,
                                                             const QPointF& mouse_pos)
    {
        qreal x_pos = mouse_pos.x();

        const RmtVirtualAllocation* allocation = GetAllocation(allocation_index, model_index);
        if (allocation == nullptr)
        {
            return;
        }

        const int resource_count = allocation->resource_count;
        if (resource_count == 0)
        {
            return;
        }

        SetHoveredResourceForAllocation(allocation_index, -1, model_index);

        // Calculate which row the mouse is in.
        qreal pixels_per_row = static_cast<qreal>(height) / static_cast<qreal>(GetNumRows(allocation));
        int   row            = mouse_pos.y() / pixels_per_row;

        // Find which resource is highlighted, and set the hovered bit.
        const double bytes_per_pixel = GetBytesPerPixel(allocation_index, model_index, width);

        for (int32_t current_resource_index = resource_count - 1; current_resource_index >= 0; --current_resource_index)
        {
            const RmtResource* current_resource = allocation->resources[current_resource_index];

            if (current_resource->resource_type == kRmtResourceTypeHeap)
            {
                continue;
            }

            if (GetRowForResourceAtIndex(allocation, current_resource_index) != row)
            {
                continue;
            }

            const uint64_t offset_in_bytes     = RmtResourceGetOffsetFromBoundAllocation(current_resource);
            const uint64_t target_offset_start = offset_in_bytes / bytes_per_pixel;
            const uint64_t target_offset_end   = (offset_in_bytes + current_resource->size_in_bytes - 1) / bytes_per_pixel;

            if (target_offset_start <= x_pos && x_pos <= target_offset_end)
            {
                SetHoveredResourceForAllocation(allocation_index, current_resource_index, model_index);
                break;
            }
        }
    }

    bool AllocationBarModel::SetSelectedResourceForAllocation(const RmtVirtualAllocation* allocation, int32_t resource_index, int32_t model_index)
    {
        if (allocation != nullptr)
        {
            selection_state_[model_index].selected_allocation = allocation;
            selection_state_[model_index].selected_resource   = resource_index;

            return SnapshotManager::Get().GetAliasModel().GetNumRows(allocation) > 1;
        }
        return false;
    }

    void AllocationBarModel::SelectResource(int32_t allocation_index, int32_t model_index, int32_t resource_index)
    {
        const RmtVirtualAllocation* allocation = GetAllocation(allocation_index, model_index);
        if (allocation == nullptr)
        {
            return;
        }

        const int resource_count = allocation->resource_count;
        if (resource_index >= 0 && resource_index < resource_count)
        {
            SetSelectedResourceForAllocation(allocation, resource_index, model_index);
        }
    }

    RmtResourceIdentifier AllocationBarModel::FindResourceIdentifier(int32_t allocation_index, int32_t model_index) const
    {
        const RmtVirtualAllocation* allocation = GetAllocation(allocation_index, model_index);
        if (allocation != nullptr)
        {
            const int resource_count = allocation->resource_count;

            for (int i = 0; i < resource_count; i++)
            {
                if (GetHoveredResourceForAllocation(allocation_index, model_index) == i)
                {
                    RmtResourceIdentifier resource_identifier = allocation->resources[i]->identifier;
                    return resource_identifier;
                }
            }
        }

        return 0;
    }

    void AllocationBarModel::ClearSelectionState()
    {
        for (uint32_t i = 0; i < model_count_; i++)
        {
            selection_state_[i] = {};
        }
    }

    void AllocationBarModel::ClearSelectionState(int32_t model_index)
    {
        selection_state_[model_index] = {};
    }

    const RmtVirtualAllocation* AllocationBarModel::GetAllocationFromResourceID(RmtResourceIdentifier resource_identifier, int32_t model_index)
    {
        if (resource_identifier == 0)
        {
            return nullptr;
        }

        if (!TraceManager::Get().DataSetValid())
        {
            return nullptr;
        }

        const RmtDataSnapshot* snapshot = SnapshotManager::Get().GetOpenSnapshot();
        if (snapshot == nullptr)
        {
            return nullptr;
        }

        const RmtResource* resource   = NULL;
        const RmtErrorCode error_code = RmtResourceListGetResourceByResourceId(&snapshot->resource_list, resource_identifier, &resource);
        if (error_code != kRmtOk)
        {
            return nullptr;
        }
        RMT_ASSERT(resource);

        int32_t                     resource_count = snapshot->resource_list.resource_count;
        const RmtVirtualAllocation* allocation     = resource->bound_allocation;
        if (allocation != nullptr)
        {
            for (int32_t current_resource_index = 0; current_resource_index < resource_count; current_resource_index++)
            {
                const RmtResource* current_resource = allocation->resources[current_resource_index];
                if (current_resource->identifier == resource_identifier)
                {
                    SetSelectedResourceForAllocation(resource->bound_allocation, current_resource_index, model_index);
                    break;
                }
            }
        }

        return resource->bound_allocation;
    }

    void AllocationBarModel::ShowAliased(bool aliased)
    {
        show_aliased_ = aliased;
    }

    int AllocationBarModel::GetNumRows(const RmtVirtualAllocation* allocation) const
    {
        if (!show_aliased_)
        {
            return 1;
        }

        return SnapshotManager::Get().GetAliasModel().GetNumRows(allocation);
    }

    int AllocationBarModel::GetRowForResourceAtIndex(const RmtVirtualAllocation* allocation, int index) const
    {
        if (!show_aliased_)
        {
            return 0;
        }

        return SnapshotManager::Get().GetAliasModel().GetRowForResourceAtIndex(allocation, index);
    }
}  // namespace rmv