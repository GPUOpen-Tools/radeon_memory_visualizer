//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Allocation explorer model.
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

namespace rmv
{
    /// @brief Enum containing indices for columns in the allocation table.
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

    /// @brief Enum containing indices for the widgets shared between the model and UI.
    enum VirtualAllocationExplorerWidgets
    {
        kVirtualAllocationExplorerNumWidgets,
    };

    /// @brief Container class that holds model data for the allocation explorer pane.
    class VirtualAllocationExplorerModel : public ModelViewMapper
    {
    public:
        /// @brief Constructor.
        ///
        /// param [in] num_allocation_models the number of models needed for the allocation bars.
        explicit VirtualAllocationExplorerModel(int32_t num_allocation_models);

        /// @brief Destructor.
        virtual ~VirtualAllocationExplorerModel();

        /// @brief Initialize blank data for the model.
        void ResetModelValues();

        /// @brief Set up the model when a snapshot is opened.
        ///
        /// @param [in] snapshot The snapshot being opened.
        ///
        /// @return true if snapshot is valid, false otherwise.
        bool OpenSnapshot(const RmtDataSnapshot* snapshot);

        /// @brief Initialize the allocation table model.
        ///
        /// @param [in] table_view  The view to the table.
        /// @param [in] num_rows    Total rows of the table.
        /// @param [in] num_columns Total columns of the table.
        void InitializeAllocationTableModel(ScaledTableView* table_view, uint num_rows, uint num_columns);

        /// @brief Initialize the resource table model.
        ///
        /// @param [in] table_view  The view to the table.
        /// @param [in] num_rows    Total rows of the table.
        /// @param [in] num_columns Total columns of the table.
        void InitializeResourceTableModel(ScaledTableView* table_view, uint num_rows, uint num_columns);

        /// @brief Handle what happens when the allocation table search filter changes.
        ///
        /// @param [in] filter The search text filter.
        void AllocationSearchBoxChanged(const QString& filter);

        /// @brief Handle what happens when the allocation table size filter changes.
        ///
        /// @param [in] min_value Minimum value of slider span.
        /// @param [in] max_value Maximum value of slider span.
        void AllocationSizeFilterChanged(int min_value, int max_value) const;

        /// @brief Handle what happens when the resource table search filter changes.
        ///
        /// @param [in] filter The search text filter.
        void ResourceSearchBoxChanged(const QString& filter);

        /// @brief Handle what happens when the resource table size filter changes.
        ///
        /// @param [in] min_value Minimum value of slider span.
        /// @param [in] max_value Maximum value of slider span.
        void ResourceSizeFilterChanged(int min_value, int max_value);

        /// @brief Update the allocation table.
        ///
        /// Only needs to be done when loading in a new snapshot.
        void UpdateAllocationTable();

        /// @brief Update the resource table.
        ///
        /// Updated when an allocation is selected.
        ///
        /// @return The number of resources in the allocation.
        int32_t UpdateResourceTable();

        /// @brief Get the allocation proxy model.
        ///
        /// Used to set up a connection between the table being sorted and the UI update.
        ///
        /// @return The proxy model.
        AllocationProxyModel* GetAllocationProxyModel() const;

        /// @brief Get the resource proxy model. Used to set up a connection between the table being sorted and the UI update.
        ///
        /// @return The proxy model.
        ResourceProxyModel* GetResourceProxyModel() const;

        /// @brief Get the model for the allocation bar.
        ///
        /// @return The allocation bar model.
        AllocationBarModel* GetAllocationBarModel() const;

    private:
        AllocationBarModel*   allocation_bar_model_;     ///< The model for the allocation bar graph.
        AllocationItemModel*  allocation_table_model_;   ///< Holds the allocation table data.
        ResourceItemModel*    resource_table_model_;     ///< Holds the resource table data.
        AllocationProxyModel* allocation_proxy_model_;   ///< Allocation table proxy model.
        ResourceProxyModel*   resource_proxy_model_;     ///< Resource table proxy model.
        uint64_t              minimum_allocation_size_;  ///< The size of the smallest allocation.
        uint64_t              maximum_allocation_size_;  ///< The size of the largest allocation.
    };
}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_ALLOCATION_EXPLORER_MODEL_H_
