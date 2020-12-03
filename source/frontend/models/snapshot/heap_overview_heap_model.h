//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a model for a heap layout for the Heap Overview pane
//=============================================================================

#ifndef RMV_MODELS_SNAPSHOT_HEAP_OVERVIEW_HEAP_MODEL_H_
#define RMV_MODELS_SNAPSHOT_HEAP_OVERVIEW_HEAP_MODEL_H_

#include "qt_common/utils/model_view_mapper.h"

#include "rmt_data_snapshot.h"
#include "rmt_types.h"

namespace rmv
{
    /// Enum containing indices for the widgets shared between the model and UI.
    enum HeapOverviewWidgets
    {
        // global widgets
        kHeapOverviewTitle,
        kHeapOverviewDescription,

        // bar graph widgets (first column)
        kHeapOverviewWarningText,

        // summary widgets (middle column)
        kHeapOverviewLocation,
        kHeapOverviewCpuCached,
        kHeapOverviewCpuVisible,
        kHeapOverviewGpuCached,
        kHeapOverviewGpuVisible,
        kHeapOverviewSmallestAllocation,
        kHeapOverviewLargestAllocation,
        kHeapOverviewMeanAllocation,

        kHeapOverviewNumWidgets,
    };

    class QTableView;

    /// Container class that holds model data for a given pane.
    class HeapOverviewHeapModel : public ModelViewMapper
    {
    public:
        /// Constructor.
        explicit HeapOverviewHeapModel(RmtHeapType heap);

        /// Destructor.
        virtual ~HeapOverviewHeapModel();

        /// Should the subscription warning be shown.
        /// \return true if a warning should be shown, false if not.
        bool ShowSubscriptionWarning();

        /// Read the dataset and update model.
        void Update();

        /// Get the data for the resources. Returns a list of resources and their amounts, including
        /// groups of smaller resources lumped under "other".
        /// \param num_resources The number of resources to show explicitly. All other resources
        ///  will be grouped under "other".
        /// \param resource_info An array to receive the explicit resource info.
        /// \param other_value The value of the "other" resources.
        /// \return The number of resources in the array.
        int GetResourceData(int num_resources, uint64_t* resource_info, int* other_value) const;

        /// Get the memory parameters. Displayed in the UI as a series of horizontal bars.
        /// \param total_physical_size A variable to receive the total physical memory size.
        /// \param total_virtual_memory_requested A variable to receive the total virtual memory requested.
        /// \param total_boud_virtual_memor A variable to receive the total virtual memory that was bound.
        /// \param total_physical_mapped_by_process A variable to receive the total physical mapped memory
        ///  by the current process.
        /// \param total_physical_mapped_by_other_processes A variable to receive the total physical mapped
        ///  memory by other processes.
        /// \param subscription_status A variable to receive the subscription status.
        void GetMemoryParameters(uint64_t&                     total_physical_size,
                                 uint64_t&                     total_virtual_memory_requested,
                                 uint64_t&                     total_bound_virtual_memory,
                                 uint64_t&                     total_physical_mapped_by_process,
                                 uint64_t&                     total_physical_mapped_by_other_processes,
                                 RmtSegmentSubscriptionStatus& subscription_status) const;

    private:
        /// Initialize blank data for the model.
        void ResetModelValues();

        /// Get the currently opened snapshot.
        /// \return A pointer to the currently opened snapshot (or nullptr if a snapshot isn't opened).
        const RmtDataSnapshot* GetSnapshot() const;

        RmtHeapType      heap_;            ///< The heap for this widget.
        RmtSegmentStatus segment_status_;  ///< The currently cached segment status.
    };
}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_HEAP_OVERVIEW_MODEL_H_
