//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Model header for the Allocation List pane
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
    /// Enum containing indices for the widgets shared between the model and UI.
    enum ResourceListWidgets
    {
        kResourceListTotalResources,
        kResourceListTotalSize,

        kResourceListNumWidgets,
    };

    /// Container class that holds model data for a given pane.
    class ResourceListModel : public ModelViewMapper
    {
    public:
        /// Constructor.
        explicit ResourceListModel();

        /// Destructor.
        virtual ~ResourceListModel();

        /// Initialize the table model.
        /// \param table_view The view to the table.
        /// \param num_rows Total rows of the table.
        /// \param num_columns Total columns of the table.
        void InitializeTableModel(ScaledTableView* table_view, uint num_rows, uint num_columns);

        /// Handle what happens when user changes the filter.
        /// \param filter new text filter.
        void SearchBoxChanged(const QString& filter);

        /// Handle what happens when the size filter changes.
        /// \param min_value Minimum value of slider span.
        /// \param max_value Maximum value of slider span.
        void FilterBySizeChanged(int min_value, int max_value);

        /// Read the dataset and update model.
        void Update();

        /// Update the list of heaps selected. This is set up from the preferred heap combo box.
        /// \param preferred_heap_filter The regular expression string of selected heaps.
        void UpdatePreferredHeapList(const QString& preferred_heap_filter);

        /// Update the list of resources available. This is set up from the resource usage combo box.
        /// \param resource_usage_filter The regular expression string of selected resource usage types.
        void UpdateResourceUsageList(const QString& resource_usage_filter);

        /// Initialize blank data for the model.
        void ResetModelValues();

        /// Get the resource proxy model. Used to set up a connection between the table being sorted and the UI update.
        /// \return the proxy model.
        ResourceProxyModel* GetResourceProxyModel() const;

    private:
        /// Update the labels on the bottom.
        void UpdateBottomLabels();

        /// Update the resource list table.
        void UpdateTable();

        ResourceItemModel*  table_model_;  ///< Resource table model data.
        ResourceProxyModel* proxy_model_;  ///< Proxy model for resource table.
    };
}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_RESOURCE_LIST_MODEL_H_
