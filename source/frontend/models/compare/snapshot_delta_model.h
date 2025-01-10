//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Snapshot Delta model.
//=============================================================================

#ifndef RMV_MODELS_COMPARE_SNAPSHOT_DELTA_MODEL_H_
#define RMV_MODELS_COMPARE_SNAPSHOT_DELTA_MODEL_H_

#include "qt_common/utils/model_view_mapper.h"

#include "rmt_data_snapshot.h"
#include "rmt_types.h"

#include "managers/snapshot_manager.h"
#include "views/custom_widgets/rmv_carousel.h"

namespace rmv
{
    /// @brief Enum containing the id's of UI elements needed by the model.
    enum HeapDeltaWidgets
    {
        kHeapDeltaCompareBaseName,
        kHeapDeltaCompareDiffName,

        kHeapDeltaCompareNumWidgets,
    };

    /// @brief Container class that holds model data for the snapshot delta pane.
    class SnapshotDeltaModel : public ModelViewMapper
    {
    public:
        /// @brief Contains aggregated delta data for a heap.
        struct HeapDeltaData
        {
            int64_t total_available_size;         ///< Full size of heap.
            int64_t total_allocated_and_bound;    ///< Allocated memory amount.
            int64_t total_allocated_and_unbound;  ///< Allocated but unused memory amount.
            int64_t free_space;                   ///< Amount of free space.
            int32_t resource_count;               ///< Number of resources.
            int32_t allocation_count;             ///< Number of allocations.
        };

        /// @brief Constructor.
        explicit SnapshotDeltaModel();

        /// @brief Destructor.
        virtual ~SnapshotDeltaModel();

        /// @brief Update the model.
        ///
        /// @return true if the model was updated successfully, false otherwise.
        bool Update();

        /// @brief Initialize blank data for the model.
        void ResetModelValues();

        /// @brief Swap the snapshots.
        ///
        /// @return true if the snapshots were swapped successfully, false otherwise.
        bool SwapSnapshots();

        /// @brief Update the carousel model.
        ///
        /// @param [in,out] carousel The carousel to update.
        void UpdateCarousel(RMVCarousel* carousel);

        /// @brief Get the heap name from the heap index.
        ///
        /// @param [in] heap_index The heap index.
        const char* GetHeapName(int32_t heap_index) const;

        /// @brief Compute heap delta between snapshots.
        ///
        /// @param [in]  heap_type      The target heap type.
        /// @param [out] out_delta_data A structure to hold the heap data from the model.
        ///
        /// @return true if data returned successfully, false otherwise.
        bool CalcPerHeapDelta(RmtHeapType heap_type, HeapDeltaData& out_delta_data) const;

    private:
        /// @brief Compute delta between snapshots.
        ///
        /// @param [in]  snapshot   The target snapshot.
        /// @param [in]  heap_type  The target heap type.
        /// @param [out] delta_data A structure to hold the heap data from the model.
        ///
        /// @return true if data returned successfully, false otherwise.
        bool GetHeapDelta(RmtDataSnapshot* snapshot, RmtHeapType heap_type, HeapDeltaData& delta_data) const;

        CompareSnapshots base_index_;     ///< The index of the base snapshot.
        CompareSnapshots diff_index_;     ///< The index of the diff snapshot.
        RmtDataSnapshot* base_snapshot_;  ///< The base snapshot.
        RmtDataSnapshot* diff_snapshot_;  ///< The diff snapshot.
    };
}  // namespace rmv

#endif  // RMV_MODELS_COMPARE_SNAPSHOT_DELTA_MODEL_H_
