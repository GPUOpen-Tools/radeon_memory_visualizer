//=============================================================================
// Copyright (c) 2018-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Resource List model.
//=============================================================================

#ifndef RMV_MODELS_SNAPSHOT_RESOURCE_LIST_MODEL_H_
#define RMV_MODELS_SNAPSHOT_RESOURCE_LIST_MODEL_H_

#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
#include "qt_common/custom_widgets/scaled_table_view.h"
#include "qt_common/utils/model_view_mapper.h"

#include "rmt_resource_list.h"

#include "models/proxy_models/resource_proxy_model.h"
#include "models/resource_item_model.h"
#include "util/definitions.h"

namespace rmv
{
    /// @brief Enum containing indices for the widgets shared between the model and UI.
    enum ResourceListWidgets
    {
        kResourceListTotalResources,
        kResourceListTotalSize,

        kResourceListNumWidgets,
    };

    /// @brief Container class that holds model data for the resource list pane.
    class ResourceListModel : public ModelViewMapper
    {
    public:
        /// @brief Constructor.
        explicit ResourceListModel();

        /// @brief Destructor.
        virtual ~ResourceListModel();

        /// @brief Initialize the table model.
        ///
        /// @param [in] table_view  The view to the table.
        /// @param [in] num_rows    Total rows of the table.
        /// @param [in] num_columns Total columns of the table.
        void InitializeTableModel(ScaledTableView* table_view, uint num_rows, uint num_columns);

        /// @brief Handle what happens when user changes the filter.
        ///
        /// @param [in] filter The new text filter.
        void SearchBoxChanged(const QString& filter);

        /// @brief Handle what happens when the size filter changes.
        ///
        /// @param [in] min_value Minimum value of slider span.
        /// @param [in] max_value Maximum value of slider span.
        void FilterBySizeChanged(int min_value, int max_value);

        /// @brief Read the dataset and update model.
        void Update();

        /// @brief Update the list of heaps selected. This is set up from the preferred heap combo box.
        ///
        /// \@param [in] preferred_heap_filter The regular expression string of selected heaps.
        void UpdatePreferredHeapList(const QString& preferred_heap_filter);

        /// @brief Update the list of resources available. This is set up from the resource usage combo box.
        ///
        /// @param [in] resource_usage_filter The regular expression string of selected resource usage types.
        void UpdateResourceUsageList(const QString& resource_usage_filter);

        /// @brief Initialize blank data for the model.
        void ResetModelValues();

        /// @brief Get the resource proxy model. Used to set up a connection between the table being sorted and the UI update.
        ///
        /// @return the proxy model.
        ResourceProxyModel* GetResourceProxyModel() const;

    private:
        /// @brief Update the labels on the bottom.
        void UpdateBottomLabels();

        /// @brief Update the resource list table.
        void UpdateTable();

        ResourceItemModel*  table_model_;  ///< Resource table model data.
        ResourceProxyModel* proxy_model_;  ///< Proxy model for resource table.
    };
}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_RESOURCE_LIST_MODEL_H_
