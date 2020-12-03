//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Model header for the Memory Leak Finder pane
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
    /// Enum containing the id's of UI elements needed by the model.
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

    /// Container class that holds model data for a given pane.
    class MemoryLeakFinderModel : public ModelViewMapper
    {
    public:
        /// Constructor.
        explicit MemoryLeakFinderModel();

        /// Destructor.
        virtual ~MemoryLeakFinderModel();

        /// Initialize the table model.
        /// \param table_view The view to the table.
        /// \param num_rows Total rows of the table.
        /// \param num_columns Total columns of the table.
        /// \param compare_id_filter starting filter.
        void InitializeTableModel(ScaledTableView* table_view, uint num_rows, uint num_columns, uint32_t compare_id_filter);

        /// Update the model.
        /// \param compare_filter The compare filter ID, to indicate which resources are to be displayed.
        void Update(SnapshotCompareId compare_filter);

        /// Initialize blank data for the model.
        void ResetModelValues();

        /// Handle what happens when user changes the 'filter by size' slider.
        /// \param filter The search text filter.
        void SearchBoxChanged(const QString& filter);

        /// Handle what happens when the size filter changes.
        /// \param min_value Minimum value of slider span.
        /// \param max_value Maximum value of slider span.
        void FilterBySizeChanged(int min_value, int max_value);

        /// Update the list of heaps selected. This is set up from the preferred heap combo box.
        /// \param preferred_heap_filter The regular expression string of selected heaps.
        void UpdatePreferredHeapList(const QString& preferred_heap_filter);

        /// Update the list of resources available. This is set up from the resource usage combo box.
        /// \param resource_usage_filter The regular expression string of selected resource usage types.
        void UpdateResourceUsageList(const QString& resource_usage_filter);

        /// Get the resource proxy model. Used to set up a connection between the table being sorted and the UI update.
        /// \return the proxy model.
        MemoryLeakFinderProxyModel* GetResourceProxyModel() const;

        /// Figure out which snapshot the selected table entry is from and load the snapshot
        /// if it's not already loaded. It will be in memory already but just needs assigning
        /// to be the snapshot that is visible in the snapshot tab.
        /// \param index The model index for the entry selected in the memory leak resource table.
        RmtSnapshotPoint* LoadSnapshot(const QModelIndex& index);

    private:
        /// Update the resource size buckets. This is used by the double-slider to
        /// group the resource sizes. Called whenever the table data changes.
        void UpdateResourceThresholds();

        /// Update labels at the bottom.
        void UpdateLabels();

        /// Reset the snapshot stats.
        void ResetStats();

        ResourceItemModel*          table_model_;                                ///< The data for the resource table.
        MemoryLeakFinderProxyModel* proxy_model_;                                ///< The proxy model for the resource table.
        uint64_t                    resource_thresholds_[kSizeSliderRange + 1];  ///< List of resource size thresholds for the filter by size sliders.

        // struct to describe the snapshot statistics
        struct SnapshotStats
        {
            uint32_t num_resources;
            uint64_t size;
        };

        SnapshotStats stats_in_both_;       ///< Attributes in both snapshots.
        SnapshotStats stats_in_base_only_;  ///< Attributes in the base snapshot only.
        SnapshotStats stats_in_diff_only_;  ///< Attributes in the diff snapshot only.
    };
}  // namespace rmv

#endif  // RMV_MODELS_COMPARE_MEMORY_LEAK_FINDER_MODEL_H_
