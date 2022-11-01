//=============================================================================
// Copyright (c) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the allocation bar model base class.
///
/// This model holds any state information for derived models that use a
/// graphical representation of an allocation and is used for a single
/// allocation bar, as seen in the allocation explorer pane. These allocations
/// are rendered using RMVAllocationBar objects.
///
//=============================================================================

#ifndef RMV_MODELS_ALLOCATION_BAR_MODEL_H_
#define RMV_MODELS_ALLOCATION_BAR_MODEL_H_

#include <QPoint>
#include <QString>
#include <stdint.h>
#include <vector>

#include "rmt_virtual_allocation_list.h"

namespace rmv
{
    /// @brief Class to describe an allocation bar model.
    class AllocationBarModel
    {
    public:
        /// @brief Structure to describe the selection state of the allocation overview.
        ///
        /// Contains the allocations and resources that the mouse is currently over or selected.
        struct SelectionState
        {
            SelectionState()
                : hovered_allocation(nullptr)
                , selected_allocation(nullptr)
                , hovered_resource(0)
                , selected_resource(0)
            {
            }

            const RmtVirtualAllocation* hovered_allocation;   ///< The allocation hovered over, or nullptr if nothing
            const RmtVirtualAllocation* selected_allocation;  ///< The allocation selected, or nullptr if nothing
            int32_t                     hovered_resource;     ///< The resource hovered over (or -1 if none hovered over)
            int32_t                     selected_resource;    ///< The resource selected (or -1 if none selected)
        };

        /// @brief Constructor.
        ///
        /// @param [in] model_count  The number of models used to represent the allocations.
        /// @param [in] show_details If true, show the stats for this allocation bar graph.
        explicit AllocationBarModel(uint32_t model_count, bool show_details);

        /// @brief Destructor.
        virtual ~AllocationBarModel();

        /// @brief Get the title text.
        ///
        /// @param [in] allocation_index The index in the scene of the allocation to return.
        /// @param [in] model_index      The index of the model referred to.
        ///
        /// @return The title text.
        QString GetTitleText(int32_t allocation_index, int32_t model_index) const;

        /// @brief Get the description text.
        ///
        /// @param [in] allocation_index The index in the scene of the allocation to return.
        /// @param [in] model_index      The index of the model referred to.
        ///
        /// @return The description text.
        QString GetDescriptionText(int32_t allocation_index, int32_t model_index) const;

        /// @brief Get the number of bytes per pixel of an allocation.
        ///
        /// @param [in] allocation_index The index in the scene of the allocation to return.
        /// @param [in] model_index      The index of the model referred to.
        /// @param [in] width            The width of the UI element, in pixels.
        ///
        /// @return The number of bytes per pixel.
        virtual double GetBytesPerPixel(int32_t allocation_index, int32_t model_index, int32_t width) const;

        /// @brief Get the allocation.
        ///
        /// In the allocation overview, each allocation is assigned an index in the scene and they all
        /// reference the same model. The scene index will remain the same but the model will return a
        /// different allocation depending on how the allocations are sorted in the model. In the
        /// allocation explorer, there is one allocation at scene index 0.
        ///
        /// @param [in] allocation_index The index in the scene of the allocation to return.
        /// @param [in] model_index      The index of the model referred to.
        ///
        /// @return The allocation.
        virtual const RmtVirtualAllocation* GetAllocation(int32_t allocation_index, int32_t model_index) const;

        /// @brief Should the allocation details be shown with the graphics representation of the allocation
        /// and its resources.
        ///
        /// @return true if the allocation details should be shown, false if not.
        bool ShowDetails() const;

        /// @brief Get the hovered resource for a specified allocation.
        ///
        /// @param [in] allocation_index The index of the allocation the resource is in.
        /// @param [in] model_index      The index of the model where the allocation can be found.
        ///
        /// @return The hovered resource index.
        int32_t GetHoveredResourceForAllocation(int32_t allocation_index, int32_t model_index) const;

        /// @brief Get the selected resource for a specified allocation.
        ///
        /// @param [in] allocation_index The index of the allocation the resource is in.
        /// @param [in] model_index      The index of the model where the allocation can be found.
        ///
        /// @return The selected resource index.
        int32_t GetSelectedResourceForAllocation(int32_t allocation_index, int32_t model_index) const;

        /// @brief Set the hovered resource for a specified allocation.
        ///
        /// @param [in] allocation_index The index of the allocation the resource is in.
        /// @param [in] resource_index   The index of the resource that is hovered over.
        /// @param [in] model_index      The index of the model where the allocation can be found.
        void SetHoveredResourceForAllocation(int32_t allocation_index, int32_t resource_index, int32_t model_index);

        /// @brief Set the hovered resource for a specified allocation.
        ///
        /// @param [in] allocation_index The index of the allocation the resource is in.
        /// @param [in] model_index      The index of the model where the allocation can be found.
        /// @param [in] width            The width of the UI element, in pixels.
        /// @param [in] height           The height of the UI element, in pixels.
        /// @param [in] mouse_pos        The position of the mouse within the UI element, in pixels.
        void SetHoveredResourceForAllocation(int32_t allocation_index, int32_t model_index, int32_t width, int32_t height, const QPointF& mouse_pos);

        /// @brief Set the selected resource for a specified allocation.
        ///
        /// @param [in] allocation_index The index of the allocation the resource is in.
        /// @param [in] resource_index   The index of the resource that is selected.
        /// @param [in] model_index      The index of the model where the allocation can be found.
        void SetSelectedResourceForAllocation(int32_t allocation_index, int32_t resource_index, int32_t model_index);

        /// @brief Set the selected resource for a specified allocation.
        ///
        /// @param [in] allocation     The allocation to search for the resource in.
        /// @param [in] resource_index The index of the resource that is selected.
        /// @param [in] model_index    The index of the model where the allocation can be found.
        ///
        /// @return true if the allocation contains aliased resources, false if not.
        bool SetSelectedResourceForAllocation(const RmtVirtualAllocation* allocation, int32_t resource_index, int32_t model_index);

        /// @brief Select a resource.
        ///
        /// @param [in] allocation_index The index of the allocation the resource is in.
        /// @param [in] model_index      The index of the model where the allocation can be found.
        /// @param [in] resource_index   The index of the resource selected.
        void SelectResource(int32_t allocation_index, int32_t model_index, int32_t resource_index);

        /// @brief Find a resource identifier for a hovered-over resource (if it exists).
        ///
        /// @param [in] allocation_index The index of the allocation the resource is in.
        /// @param [in] model_index      The index of the model where the allocation can be found.
        ///
        /// @return The identifier of the requested resource, or 0 if nothing is selectable.
        RmtResourceIdentifier FindResourceIdentifier(int32_t allocation_index, int32_t model_index) const;

        /// @brief Clear the selection state for all allocations. No resources are selected or hovered over.
        void ClearSelectionState();

        /// @brief Clear the selection state. No resources are selected or hovered over.
        ///
        /// @param [in] model_index The index of the model whose state is to be cleared.
        void ClearSelectionState(int32_t model_index);

        /// @brief Get an allocation from a resource ID
        ///
        /// @param [in] resource_identifier The resource identifier.
        /// @param [in] model_index         The index of the model where the resource can be found.
        ///
        /// @return the allocation the resource is contained in
        const RmtVirtualAllocation* GetAllocationFromResourceID(RmtResourceIdentifier resource_identifier, int32_t model_index);

        /// @brief Get the title text.
        ///
        /// @param [in] allocation The allocation to get the title text for.
        ///
        /// @return The title text.
        QString GetTitleText(const RmtVirtualAllocation* allocation) const;

        /// @brief Get the description text.
        ///
        /// @param [in] allocation The allocation to get the description text for.
        ///
        /// @return The description text.
        QString GetDescriptionText(const RmtVirtualAllocation* allocation) const;

        /// @brief Should the resources be displayed to show aliasing (ie stacked).
        ///
        /// @param [in] aliased Whether the resources should be shown aliased.
        void ShowAliased(bool aliased);

        /// @brief Get the number of rows needed to show the resources.
        ///
        /// @param [in] allocation The virtual allocation containing the resources.
        ///
        /// @return The number of rows.
        int GetNumRows(const RmtVirtualAllocation* allocation) const;

        /// @brief Get the row that a resource is in.
        ///
        /// @param [in] allocation The virtual allocation containing the resources.
        /// @param [in] index      The resource index.
        ///
        /// @return The row.
        int GetRowForResourceAtIndex(const RmtVirtualAllocation* allocation, int index) const;

    private:
        SelectionState* selection_state_;  ///< The selected/hovered resource state information.
        uint32_t        model_count_;      ///< The number of graphic models required.
        bool            show_details_;     ///< Should the allocation details text be shown.
        bool            show_aliased_;     ///< Should the resources be shown aliased (stacked).
    };
}  // namespace rmv

#endif  // RMV_MODELS_ALLOCATION_BAR_MODEL_H_
