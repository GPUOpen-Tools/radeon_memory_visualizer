//=============================================================================
// Copyright (c) 2019-2022 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a model for a heap layout for the Heap Overview pane.
//=============================================================================

#ifndef RMV_MODELS_SNAPSHOT_HEAP_OVERVIEW_HEAP_MODEL_H_
#define RMV_MODELS_SNAPSHOT_HEAP_OVERVIEW_HEAP_MODEL_H_

#include "qt_common/utils/model_view_mapper.h"

#include "rmt_data_snapshot.h"
#include "rmt_types.h"

namespace rmv
{
    /// @brief Enum containing indices for the widgets shared between the model and UI.
    enum HeapOverviewWidgets
    {
        // global widgets
        kHeapOverviewTitle,
        kHeapOverviewDescription,
        kHeapOverviewSamStatus,

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

    /// @brief Container class that holds model data for a heap in the heap overview pane.
    class HeapOverviewHeapModel : public ModelViewMapper
    {
    public:
        /// @brief Constructor.
        ///
        /// @param [in] heap The heap this model refers to.
        explicit HeapOverviewHeapModel(RmtHeapType heap);

        /// @brief Destructor.
        virtual ~HeapOverviewHeapModel();

        /// @brief Should the subscription warning be shown.
        ///
        /// @return true if a warning should be shown, false if not.
        bool ShowSubscriptionWarning();

        /// @brief Read the dataset and update model.
        void Update();

        /// @brief Get the data for the resources.
        ///
        /// Returns a list of resources and their amounts, including groups of smaller resources
        /// lumped under "other".
        ///
        /// @param [in] num_resources The number of resources to show explicitly.
        /// @param [in] resource_info An array to receive the explicit resource info.
        /// @param [in] other_value   The value of the "other" resources.
        ///
        /// @return The number of resources in the array.
        int GetResourceData(int num_resources, uint64_t* resource_info, int* other_value) const;

        /// @brief Get the memory parameters. Displayed in the UI as a series of horizontal bars.
        ///
        /// @param [out] total_physical_size                      A variable to receive the total physical memory size.
        /// @param [out] total_virtual_memory_requested           A variable to receive the total virtual memory requested.
        /// @param [out] total_bound_virtual_memory               A variable to receive the total virtual memory that was bound.
        /// @param [out] total_physical_mapped_by_process         A variable to receive the total physical mapped memory by the current process.
        /// @param [out] total_physical_mapped_by_other_processes A variable to receive the total physical mapped memory by other processes.
        /// @param [out] subscription_status                      A variable to receive the subscription status.
        void GetMemoryParameters(uint64_t&                     total_physical_size,
                                 uint64_t&                     total_virtual_memory_requested,
                                 uint64_t&                     total_bound_virtual_memory,
                                 uint64_t&                     total_physical_mapped_by_process,
                                 uint64_t&                     total_physical_mapped_by_other_processes,
                                 RmtSegmentSubscriptionStatus& subscription_status) const;


        /// @brief Determines SAM (Smart Memory Access) was enabled when the memory trace was taken.
        ///
        /// @return If SAM is enabled, returns true, otherwise returns false.
        static bool IsSAMSupported();

        /// @brief Retrieves the heap type for this model.
        ///
        /// @return The model's heap type.
        RmtHeapType GetHeapType() const;

    private:
        /// @brief Initialize blank data for the model.
        void ResetModelValues();

        /// @brief Get the currently opened snapshot.
        ///
        /// @return A pointer to the currently opened snapshot (or nullptr if a snapshot isn't opened).
        const RmtDataSnapshot* GetSnapshot() const;

        RmtHeapType      heap_;            ///< The heap for this widget.
        RmtSegmentStatus segment_status_;  ///< The currently cached segment status.
    };
}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_HEAP_OVERVIEW_MODEL_H_
