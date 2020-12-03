//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Model header for the Resource Overview pane
//=============================================================================

#ifndef RMV_MODELS_SNAPSHOT_RESOURCE_OVERVIEW_MODEL_H_
#define RMV_MODELS_SNAPSHOT_RESOURCE_OVERVIEW_MODEL_H_

#include "qt_common/utils/model_view_mapper.h"

namespace rmv
{
    /// Enum containing indices for the widgets shared between the model and UI.
    enum ResourceOverviewWidgets
    {
        kResourceOverviewTotalAvailableSize,
        kResourceOverviewTotalAllocatedAndUsed,
        kResourceOverviewTotalAllocatedAndUnused,
        kResourceOverviewAllocationCount,
        kResourceOverviewResourceCount,

        kResourceOverviewNumWidgets,
    };

    /// Container class that holds model data for a given pane.
    class ResourceOverviewModel : public ModelViewMapper
    {
    public:
        /// Constructor.
        explicit ResourceOverviewModel();

        /// Destructor.
        virtual ~ResourceOverviewModel();

        /// Handle what happens when the size filter changes.
        /// \param min_value Minimum value of slider span.
        /// \param max_value Maximum value of slider span.
        void FilterBySizeChanged(int min_value, int max_value);

        /// Check to see if a resource size is within the slider range.
        /// \param resource_size The size of the resource to check.
        /// \return true if the size is in range, false otherwise.
        bool IsSizeInRange(uint64_t resource_size) const;

        /// Update the model.
        void Update();

    private:
        /// Initialize blank data for the model.
        void ResetModelValues();

        uint64_t min_resource_size_;  ///< The minimum resource size to show.
        uint64_t max_resource_size_;  ///< The maximum resource size to show.
    };
}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_RESOURCE_OVERVIEW_MODEL_H_
