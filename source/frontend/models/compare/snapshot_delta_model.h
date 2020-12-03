//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Model header for the Heap Delta pane
//=============================================================================

#ifndef RMV_MODELS_COMPARE_SNAPSHOT_DELTA_MODEL_H_
#define RMV_MODELS_COMPARE_SNAPSHOT_DELTA_MODEL_H_

#include "qt_common/utils/model_view_mapper.h"

#include "rmt_data_snapshot.h"
#include "rmt_types.h"

#include "views/custom_widgets/rmv_carousel.h"

namespace rmv
{
    /// Enum containing the id's of UI elements needed by the model.
    enum HeapDeltaWidgets
    {
        kHeapDeltaCompareBaseName,
        kHeapDeltaCompareDiffName,

        kHeapDeltaCompareNumWidgets,
    };

    /// Container class that holds model data for a given pane.
    class SnapshotDeltaModel : public ModelViewMapper
    {
    public:
        /// Contains aggregated delta data for a heap.
        struct HeapDeltaData
        {
            int64_t total_available_size;         ///< Full size of heap.
            int64_t total_allocated_and_bound;    ///< Allocated memory amount.
            int64_t total_allocated_and_unbound;  ///< Allocated but unused memory amount.
            int64_t free_space;                   ///< Amount of free space.
            int32_t resource_count;               ///< Number of resources.
            int32_t allocation_count;             ///< Number of allocations.
        };

        /// Constructor.
        explicit SnapshotDeltaModel();

        /// Destructor.
        virtual ~SnapshotDeltaModel();

        /// Update the model.
        /// \return true if the model was updated successfully, false otherwise.
        bool Update();

        /// Initialize blank data for the model.
        void ResetModelValues();

        /// Swap the snapshots.
        /// \return true if the snapshots were swapped successfully, false otherwise.
        bool SwapSnapshots();

        /// Update the carousel model.
        /// \param carousel The carousel to update.
        void UpdateCarousel(RMVCarousel* carousel);

        /// Get the heap name from the heap index.
        /// \param heap_index The heap index.
        const char* GetHeapName(int32_t heap_index);

        /// Compute heap delta between snapshots.
        /// \param heap_type The target heap type.
        /// \param out_delta_data Structure to hold the heap data from the model.
        /// \return true if data returned successfully, false otherwise.
        bool CalcPerHeapDelta(RmtHeapType heap_type, HeapDeltaData& out_delta_data);

    private:
        /// Compute delta between snapshots.
        /// \param snapshot The target snapshot.
        /// \param heap_type The target heap type.
        /// \param delta_Data Structure to hold the heap data from the model.
        /// \return true if data returned successfully, false otherwise.
        bool GetHeapDelta(RmtDataSnapshot* snapshot, RmtHeapType heap_type, HeapDeltaData& delta_data);

        int              base_index_;     ///< The index of the base snapshot.
        int              diff_index_;     ///< The index of the diff snapshot.
        RmtDataSnapshot* base_snapshot_;  ///< The base snapshot.
        RmtDataSnapshot* diff_snapshot_;  ///< The diff snapshot.
    };
}  // namespace rmv

#endif  // RMV_MODELS_COMPARE_SNAPSHOT_DELTA_MODEL_H_
