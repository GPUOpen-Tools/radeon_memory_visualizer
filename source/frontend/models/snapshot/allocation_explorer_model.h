//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Model header for the Allocation explorer pane.
//=============================================================================

#ifndef RMV_MODELS_SNAPSHOT_ALLOCATION_EXPLORER_MODEL_H_
#define RMV_MODELS_SNAPSHOT_ALLOCATION_EXPLORER_MODEL_H_

#include "qt_common/custom_widgets/scaled_table_view.h"
#include "qt_common/utils/model_view_mapper.h"

#include "rmt_virtual_allocation_list.h"

#include "models/allocation_bar_model.h"
#include "models/allocation_item_model.h"
#include "models/proxy_models/resource_proxy_model.h"
#include "models/proxy_models/allocation_proxy_model.h"
#include "models/resource_item_model.h"
#include "util/definitions.h"

/// Enum containing indices for columns in the allocation table.
enum VirtualAllocationColumn
{
    kVirtualAllocationColumnId,
    kVirtualAllocationColumnAllocationSize,
    kVirtualAllocationColumnBound,
    kVirtualAllocationColumnUnbound,
    kVirtualAllocationColumnAverageResourceSize,
    kVirtualAllocationColumnResourceSizeStdDev,
    kVirtualAllocationColumnResourceCount,
    kVirtualAllocationColumnPreferredHeapName,
    kVirtualAllocationColumnInvisiblePercentage,
    kVirtualAllocationColumnLocalPercentage,
    kVirtualAllocationColumnSystemPercentage,
    kVirtualAllocationColumnUnmappedPercentage,

    kVirtualAllocationColumnCount,
};

namespace rmv
{
    /// Enum containing indices for the widgets shared between the model and UI.
    enum VirtualAllocationExplorerWidgets
    {
        kVirtualAllocationExplorerNumWidgets,
    };

    /// Container class that holds model data for a given pane.
    class VirtualAllocationExplorerModel : public ModelViewMapper
    {
    public:
        /// Constructor.
        explicit VirtualAllocationExplorerModel(int32_t num_allocation_models);

        /// Destructor.
        virtual ~VirtualAllocationExplorerModel();

        /// Initialize blank data for the model.
        void ResetModelValues();

        /// Set up the model when a snapshot is opened.
        /// \param snapshot The snapshot being opened.
        /// \return true if snapshot is valid, false otherwise.
        bool OpenSnapshot(const RmtDataSnapshot* snapshot);

        /// Initialize the allocation table model.
        /// \param table_view The view to the table.
        /// \param num_rows Total rows of the table.
        /// \param num_columns Total columns of the table.
        void InitializeAllocationTableModel(ScaledTableView* table_view, uint num_rows, uint num_columns);

        /// Initialize the resource table model.
        /// \param table_view The view to the table.
        /// \param num_rows Total rows of the table.
        /// \param num_columns Total columns of the table.
        void InitializeResourceTableModel(ScaledTableView* table_view, uint num_rows, uint num_columns);

        /// Handle what happens when the allocation table search filter changes.
        /// \param filter The search text filter.
        void AllocationSearchBoxChanged(const QString& filter);

        /// Handle what happens when the allocation table size filter changes.
        /// \param min_value Minimum value of slider span.
        /// \param max_value Maximum value of slider span.
        void AllocationSizeFilterChanged(int min_value, int max_value) const;

        /// Handle what happens when the resource table search filter changes.
        /// \param filter The search text filter.
        void ResourceSearchBoxChanged(const QString& filter);

        /// Handle what happens when the resource table size filter changes.
        /// \param min_value Minimum value of slider span.
        /// \param max_value Maximum value of slider span.
        void ResourceSizeFilterChanged(int min_value, int max_value);

        /// Update the allocation table. Only needs to be done when loading in a new snapshot.
        void UpdateAllocationTable();

        /// Update the resource table. Updated when an allocation is selected.
        /// \return The number of resources in the allocation.
        int32_t UpdateResourceTable();

        /// Build a list of resource size thresholds for the filter by size slider.
        void BuildResourceSizeThresholds();

        /// Get the allocation proxy model. Used to set up a connection between the table being sorted and the UI update.
        /// \return the proxy model.
        AllocationProxyModel* GetAllocationProxyModel() const;

        /// Get the resource proxy model. Used to set up a connection between the table being sorted and the UI update.
        /// \return the proxy model.
        ResourceProxyModel* GetResourceProxyModel() const;

        /// Get the model for the allocation bar.
        /// \return The allocation bar model.
        AllocationBarModel* GetAllocationBarModel() const;

    private:
        AllocationBarModel*   allocation_bar_model_;                       ///< The model for the allocation bar graph.
        AllocationItemModel*  allocation_table_model_;                     ///< Holds the allocation table data.
        ResourceItemModel*    resource_table_model_;                       ///< Holds the resource table data.
        AllocationProxyModel* allocation_proxy_model_;                     ///< Allocation table proxy model.
        ResourceProxyModel*   resource_proxy_model_;                       ///< Resource table proxy model.
        uint64_t              resource_thresholds_[kSizeSliderRange + 1];  ///< List of resource size thresholds for the filter by size sliders.
        uint64_t              minimum_allocation_size_;                    ///< The size of the smallest allocation.
        uint64_t              maximum_allocation_size_;                    ///< The size of the largest allocation.
    };
}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_ALLOCATION_EXPLORER_MODEL_H_
