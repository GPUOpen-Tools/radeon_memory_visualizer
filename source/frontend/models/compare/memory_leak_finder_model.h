//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Memory Leak Finder model.
//=============================================================================

#ifndef RMV_MODELS_COMPARE_MEMORY_LEAK_FINDER_MODEL_H_
#define RMV_MODELS_COMPARE_MEMORY_LEAK_FINDER_MODEL_H_

#include "qt_common/custom_widgets/scaled_table_view.h"
#include "qt_common/utils/model_view_mapper.h"

#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"
#include "rmt_resource_list.h"
#include "rmt_virtual_allocation_list.h"

#include "models/proxy_models/memory_leak_finder_proxy_model.h"
#include "models/resource_item_model.h"
#include "util/constants.h"

namespace rmv
{
    /// @brief Enum containing the id's of UI elements needed by the model.
    enum MemoryLeakFinderWidgets
    {
        kMemoryLeakFinderBaseStats,
        kMemoryLeakFinderBothStats,
        kMemoryLeakFinderDiffStats,
        kMemoryLeakFinderTotalResources,
        kMemoryLeakFinderTotalSize,
        kMemoryLeakFinderBaseCheckbox,
        kMemoryLeakFinderDiffCheckbox,
        kMemoryLeakFinderBaseSnapshot,
        kMemoryLeakFinderDiffSnapshot,

        kMemoryLeakFinderNumWidgets,
    };

    /// @brief Container class that holds model data for the memory leak finder pane.
    class MemoryLeakFinderModel : public ModelViewMapper
    {
    public:
        /// @brief Constructor.
        explicit MemoryLeakFinderModel();

        /// @brief Destructor.
        virtual ~MemoryLeakFinderModel();

        /// @brief Initialize the table model.
        ///
        /// @param [in] table_view        The view to the table.
        /// @param [in] num_rows          Total rows of the table.
        /// @param [in] num_columns       Total columns of the table.
        /// @param [in] compare_id_filter The starting filter value.
        void InitializeTableModel(ScaledTableView* table_view, uint num_rows, uint num_columns, uint32_t compare_id_filter);

        /// @brief Update the model.
        ///
        /// @param [in] compare_filter The compare filter ID, to indicate which resources are to be displayed.
        void Update(SnapshotCompareId compare_filter);

        /// @brief Initialize blank data for the model.
        void ResetModelValues();

        /// @brief Handle what happens when user changes the search filter.
        ///
        /// @param [in] filter The search text filter.
        void SearchBoxChanged(const QString& filter);

        /// @brief Handle what happens when user changes the 'filter by size' slider.
        ///
        /// @param [in] min_value Minimum value of slider span.
        /// @param [in] max_value Maximum value of slider span.
        void FilterBySizeChanged(int min_value, int max_value);

        /// @brief Update the list of heaps selected. This is set up from the preferred heap combo box.
        ///
        /// @param [in] preferred_heap_filter The regular expression string of selected heaps.
        void UpdatePreferredHeapList(const QString& preferred_heap_filter);

        /// @brief Update the list of resources available. This is set up from the resource usage combo box.
        ///
        /// @param [in] resource_usage_filter The regular expression string of selected resource usage types.
        void UpdateResourceUsageList(const QString& resource_usage_filter);

        /// @brief Get the resource proxy model. Used to set up a connection between the table being sorted and the UI update.
        ///
        /// @return the proxy model.
        MemoryLeakFinderProxyModel* GetResourceProxyModel() const;

        /// @brief Figure out which snapshot the selected table entry is from and set up the snapshot
        /// for load if it's not already loaded.
        ///
        /// It will be in memory already but just needs assigning to be the snapshot that is visible
        /// in the snapshot tab.
        ///
        /// @param [in] index The model index for the entry selected in the memory leak resource table.
        ///
        /// @return The snapshot point of the snapshot containing the resource.
        RmtSnapshotPoint* FindSnapshot(const QModelIndex& index) const;

    private:
        /// @brief Update the resource size buckets.
        ///
        /// This is used by the double-slider to group the resource sizes. Called whenever the table data changes.
        void UpdateResourceThresholds();

        /// @brief Update labels at the bottom.
        void UpdateLabels();

        /// @brief Reset the snapshot stats.
        void ResetStats();

        ResourceItemModel*          table_model_;                                ///< The data for the resource table.
        MemoryLeakFinderProxyModel* proxy_model_;                                ///< The proxy model for the resource table.
        uint64_t                    resource_thresholds_[kSizeSliderRange + 1];  ///< List of resource size thresholds for the filter by size sliders.

        /// @brief struct to describe the snapshot statistics
        struct SnapshotStats
        {
            uint32_t num_resources;  ///< The number of resources.
            uint64_t size;           ///< The total size of all resources.
        };

        SnapshotStats stats_in_both_;       ///< Attributes in both snapshots.
        SnapshotStats stats_in_base_only_;  ///< Attributes in the base snapshot only.
        SnapshotStats stats_in_diff_only_;  ///< Attributes in the diff snapshot only.
    };
}  // namespace rmv

#endif  // RMV_MODELS_COMPARE_MEMORY_LEAK_FINDER_MODEL_H_
