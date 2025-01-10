//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the aliased resource model.
//=============================================================================

#ifndef RMV_MODELS_ALIASED_RESOURCE_MODEL_H_
#define RMV_MODELS_ALIASED_RESOURCE_MODEL_H_

#include <stdint.h>
#include <map>
#include <vector>

#include "rmt_virtual_allocation_list.h"

namespace rmv
{
    /// @brief Class to generate and store the aliased resource data.
    ///
    /// This is generated when a snapshot is created. It consists of a map lookup of
    /// allocation to aliased data. If an allocation isn't in the map, then it isn't
    /// aliased.
    class AliasedResourceModel
    {
    public:
        /// @brief Constructor.
        AliasedResourceModel();

        /// @brief Destructor.
        ~AliasedResourceModel();

        /// @brief Clear out the aliased data model.
        void Clear();

        /// @brief Generate the aliased data.
        ///
        /// Build a temporary 2D vector of data to determine if resources overlap.
        /// From this data, have an array the same size as the number of resources and
        /// for each resource, save its row for quick lookup.
        ///
        /// @param [in] allocation The allocation containing the resources.
        ///
        /// @return true if this allocation contains aliased resources, false if not.
        bool Generate(const RmtVirtualAllocation* allocation);

        /// @brief Get the number of rows required for drawing the resources for the current allocation.
        ///
        /// @param [in] allocation The virtual allocation containing the resources.
        ///
        /// @return The number of rows.
        int GetNumRows(const RmtVirtualAllocation* allocation) const;

        /// @brief Get the row that a resource is on.
        ///
        /// @param [in] allocation The virtual allocation containing the resources.
        /// @param [in] index      The index of the resource in the rmt_resource_list structure.
        ///
        /// @return The row.
        int GetRowForResourceAtIndex(const RmtVirtualAllocation* allocation, int index) const;

    private:
        /// @brief Struct containing information for aliased resources.
        ///
        /// Consists of a vector indicating which row a resource is to be drawn in (the index
        /// in the vector is the same as the index in the resource list in the back end) and
        /// the total number of rows needed to show the resources.
        struct AliasData
        {
            std::vector<int> resource_rows;  ///< A lookup to get the row for a resource.
            int              num_rows = 0;   ///< The number of rows required for the resources.
        };

        std::map<const RmtVirtualAllocation*, AliasData> alias_data_;  ///< The alias data.
    };

}  // namespace rmv

#endif  // RMV_MODELS_ALIASED_RESOURCE_MODEL_H_
