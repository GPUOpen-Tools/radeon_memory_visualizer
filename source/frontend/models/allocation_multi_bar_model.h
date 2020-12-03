//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the allocation multi bar model class. This model
/// derives from the allocation bar base class and contains additional
/// support for displays with multiple allocations as seen in the allocation
/// overview pane. These allocation are rendered using
/// RMVAllocationBar objects.
//=============================================================================

#ifndef RMV_MODELS_ALLOCATION_MULTI_BAR_MODEL_H_
#define RMV_MODELS_ALLOCATION_MULTI_BAR_MODEL_H_

#include <QString>
#include <stdint.h>
#include <vector>

#include "rmt_types.h"
#include "rmt_virtual_allocation_list.h"

#include "models/allocation_bar_model.h"

namespace rmv
{
    class MultiAllocationBarModel : public AllocationBarModel
    {
    public:
        /// Constructor.
        /// \param model_count The number of models used to represent the allocations.
        explicit MultiAllocationBarModel(uint32_t model_count);

        /// Destructor.
        virtual ~MultiAllocationBarModel();

        /// Get the number of bytes per pixel of an allocation.
        /// \param scene_index The index in the scene of the allocation to return.
        /// \param model_index The index of the model referred to.
        /// \param width The width of the UI element, in pixels.
        /// \return The number of bytes per pixel.
        virtual double GetBytesPerPixel(int32_t scene_index, int32_t model_index, int32_t width) const;

        /// Get the allocation. In the allocation overview, each allocation is assigned an index
        /// in the scene and they all reference the same model. The scene index will remain the same
        /// but the model will return a different allocation depending on how the allocations are
        /// sorted in the model. In the allocation explorer, there is one allocation at scene index 0.
        /// \param scene_index The index in the scene of the allocation to return.
        /// \param model_index The index of the model referred to.
        /// \return The allocation.
        virtual const RmtVirtualAllocation* GetAllocation(int32_t scene_index, int32_t model_index) const;

        /// Initialize blank data for the model.
        void ResetModelValues();

        /// Get the number of viewable allocations.
        /// \return The number of viewable allocations.
        size_t GetViewableAllocationCount() const;

        /// Set whether the allocations should be normalized.
        /// \param normalized true if the allocations should be normalized, false if not.
        void SetNormalizeAllocations(bool normalized);

        /// Sort the allocations.
        /// \param sort_mode The sort mode to sort by.
        /// \param ascending Whether to use ascending or descending ordering.
        void Sort(int sort_mode, bool ascending);

        /// Apply filters and rebuild the list of allocations.
        /// \param filter_text The search text specified in the UI.
        /// \param heap_array_flags An array of flags indicating if the corresponding heap
        /// should be shown.
        void ApplyAllocationFilters(const QString& filter_text, const bool* heap_array_flags, int sort_mode, bool ascending);

        /// Select a resource on this pane. This is usually called when selecting a resource
        /// on a different pane to make sure the resource selection is propagated to all
        /// interested panes.
        /// \param resource_identifier the resource identifier of the resource to select.
        /// \param model_index The index of the model referred to. This pane uses a single model
        /// to represent all the allocations.
        /// \return The index in the scene of the selected resource.
        size_t SelectResource(RmtResourceIdentifier resource_identifier, int32_t model_index);

        /// Set the offset of the allocation in the scene. This is the index of the allocation at the
        /// top of the visible area of the scene.
        /// \param allocation_offset The new allocation offset.
        void SetAllocationOffset(int32_t allocation_offset);

    private:
        /// Get the allocation index from the allocation.
        /// \param allocation The allocation whose index is to be found.
        /// \return The allocation index or UINT64_MAX if the allocation can't be found.
        size_t GetAllocationIndex(const RmtVirtualAllocation* allocation) const;

        std::vector<const RmtVirtualAllocation*> shown_allocation_list_;    ///< The list of shown allocations.
        uint64_t                                 largest_allocation_size_;  ///< The largest allocation size.
        bool                                     normalize_allocations_;    ///< Should the allocations be drawn normalized.
        int32_t                                  allocation_offset_;        ///< The allocation offset in the visible area of the scene.
    };
}  // namespace rmv

#endif  // RMV_MODELS_ALLOCATION_MULTI_BAR_MODEL_H_
