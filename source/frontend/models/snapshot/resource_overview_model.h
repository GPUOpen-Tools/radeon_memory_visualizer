//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Resource Overview model.
//=============================================================================

#ifndef RMV_MODELS_SNAPSHOT_RESOURCE_OVERVIEW_MODEL_H_
#define RMV_MODELS_SNAPSHOT_RESOURCE_OVERVIEW_MODEL_H_

#include "qt_common/utils/model_view_mapper.h"

#include "rmt_resource_list.h"

namespace rmv
{
    /// @brief Enum containing indices for the widgets shared between the model and UI.
    enum ResourceOverviewWidgets
    {
        kResourceOverviewTotalAvailableSize,
        kResourceOverviewTotalAllocatedAndUsed,
        kResourceOverviewTotalAllocatedAndUnused,
        kResourceOverviewAllocationCount,
        kResourceOverviewResourceCount,

        kResourceOverviewNumWidgets,
    };

    /// @brief Container class that holds model data for the resource overview pane.
    class ResourceOverviewModel : public ModelViewMapper
    {
    public:
        /// @brief Constructor.
        explicit ResourceOverviewModel();

        /// @brief Destructor.
        virtual ~ResourceOverviewModel();

        /// @brief Handle what happens when the size filter changes.
        ///
        /// @param [in] min_value Minimum value of slider span.
        /// @param [in] max_value Maximum value of slider span.
        void FilterBySizeChanged(int32_t min_value, int32_t max_value);

        /// @brief Check to see if a resource size is within the slider range.
        ///
        /// @param [in] resource_size The size of the resource to check.
        ///
        /// @return true if the size is in range, false otherwise.
        bool IsSizeInSliderRange(uint64_t resource_size) const;

        /// @brief Update the model.
        ///
        /// @param [in] use_unbound Use unbound resources when considering the max resource size.
        void Update(bool use_unbound);

        /// @brief Get the data required for the tooltip.
        ///
        /// @param [in]  resource    The resource the tooltip is over.
        /// @param [out] text_string A string to accept the tooltip string.
        ///
        /// @return true if the tooltip is valid, false if error.
        bool GetTooltipString(const RmtResource* resource, QString& text_string) const;

    private:
        /// @brief Initialize blank data for the model.
        ///
        /// @param [in] use_unbound Use unbound resources when considering the max resource size.
        void ResetModelValues(bool use_unbound);

        uint64_t min_resource_size_;  ///< The minimum resource size to show.
        uint64_t max_resource_size_;  ///< The maximum resource size to show.
    };
}  // namespace rmv

#endif  // RMV_MODELS_SNAPSHOT_RESOURCE_OVERVIEW_MODEL_H_
