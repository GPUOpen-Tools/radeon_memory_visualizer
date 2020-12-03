//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Model header for the Allocation delta model
//=============================================================================

#ifndef RMV_MODELS_COMPARE_ALLOCATION_DELTA_MODEL_H_
#define RMV_MODELS_COMPARE_ALLOCATION_DELTA_MODEL_H_

#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"
#include "rmt_types.h"

#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
#include "qt_common/utils/model_view_mapper.h"

#include "models/allocation_bar_model.h"
#include "views/custom_widgets/rmv_delta_display.h"

namespace rmv
{
    /// Enum containing the allocation delta data indices.
    enum AllocationDeltaDataType
    {
        kAllocationDeltaDataTypeAvailableSize,
        kAllocationDeltaDataTypeAllocatedAndUsed,
        kAllocationDeltaDataTypeAllocatedAndUnused,
        kAllocationDeltaDataTypeAverageAllocationSize,
        kAllocationDeltaDataTypeStandardDeviation,
        kAllocationDeltaDataTypeAllocationCount,

        kAllocationDeltaDataTypeCount,
    };

    /// Enum containing the id's of UI elements needed by the model.
    enum AllocationDeltaWidgets
    {
        kAllocationDeltaCompareBaseName,
        kAllocationDeltaCompareBaseGraphicName,
        kAllocationDeltaCompareDiffName,
        kAllocationDeltaCompareDiffGraphicName,

        kAllocationDeltaCompareNumWidgets,
    };

    /// Container class that holds model data for a given pane.
    class AllocationDeltaModel : public ModelViewMapper
    {
    public:
        /// Constructor.
        explicit AllocationDeltaModel();

        /// Destructor.
        virtual ~AllocationDeltaModel();

        /// Update the model.
        /// \param base_allocation_index The index of the base allocation.
        /// \param diff_allocation_index The index of the diff allocation.
        /// \return true if the model was updated successfully, false otherwise.
        bool Update(int32_t base_allocation_index, int32_t diff_allocation_index);

        /// Initialize blank data for the model.
        void ResetModelValues();

        /// Update the allocation deltas.
        /// \param base_allocation_index The index of the base allocation.
        /// \param diff_allocation_index The index of the diff allocation.
        /// \param out_allocation_data A structure where the allocation delta information is written to
        /// by the model.
        /// \return true if the data was written correctly, false otherwise.
        bool UpdateAllocationDeltas(int32_t base_allocation_index, int32_t diff_allocation_index, QVector<DeltaItem>& out_allocation_data);

        /// Swap the snapshots.
        /// \return true if the snapshots were swapped successfully, false otherwise.
        bool SwapSnapshots();

        /// Get the relative size as a ratio of this allocation compared to the largest allocation. In
        /// this case, the largest allocation is the largest selected allocations from the base and diff snapshots.
        /// As an example, if the current allocation is half the size of the largest allocation, the value returned
        /// would be 0.5.
        /// \param allocation_index The index of the allocation in the list of allocations in the snapshot.
        /// \param model_index In this case, the index of the snapshot (base or diff).
        /// \return The relative size as a percentage.
        double GetAllocationSizeRatio(int32_t allocation_index, int32_t model_index) const;

        /// Initialize a combo box with allocation data from the model.
        /// \param snapshot_index The index of the snapshot (base or diff).
        /// \param combo_box The combo box to populate.
        void InitializeComboBox(int32_t snapshot_index, ArrowIconComboBox* combo_box) const;

        /// Get an allocation from the model.
        /// \param snapshot_index The index of the snapshot (base or diff).
        /// \param allocation_index The index of the allocation in the list of allocations in the snapshot.
        void SelectAllocation(int32_t snapshot_index, int32_t allocation_index);

        /// Get a resource from the model.
        /// \param snapshot_index The index of the snapshot (base or diff).
        /// \param resource_identifier The identifier of the resource to get.
        /// \return The resource (or nullptr if resource not available).
        const RmtResource* GetResource(int32_t snapshot_index, RmtResourceIdentifier resource_identifier) const;

        /// Get the model for the allocation bar.
        /// \return The allocation bar model.
        AllocationBarModel* GetAllocationBarModel() const;

    private:
        /// Get the snapshot from the snapshot index.
        /// \param snapshot_index The index of the snapshot (base or diff).
        /// \return The snapshot (or nullptr if no snapshot available).
        RmtDataSnapshot* GetSnapshotFromSnapshotIndex(int32_t snapshot_index) const;

        AllocationBarModel* allocation_bar_model_;     ///< The model for the allocation bar graphs.
        int                 base_index_;               ///< The index of the base snapshot.
        int                 diff_index_;               ///< The index of the diff snapshot.
        RmtDataSnapshot*    base_snapshot_;            ///< The base snapshot.
        RmtDataSnapshot*    diff_snapshot_;            ///< The diff snapshot.
        int64_t             largest_allocation_size_;  ///< the current largest allocation size.
    };
}  // namespace rmv

#endif  // RMV_MODELS_COMPARE_ALLOCATION_DELTA_MODEL_H_
