//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a resource sorter.
///
/// Contains a list of resources and allows them to be sorted and returns
/// sorted values and smaller values grouped together as "other".
///
//=============================================================================

#ifndef RMV_MODELS_RESOURCE_SORTER_H_
#define RMV_MODELS_RESOURCE_SORTER_H_

#include <algorithm>
#include <vector>

#include "rmt_resource_list.h"

namespace rmv
{
    /// @brief Container class for a list of resources that need to be sorted.
    class ResourceSorter
    {
    public:
        /// @brief Constructor.
        ResourceSorter();

        /// @brief Destructor.
        ~ResourceSorter();

        /// @brief Clear the list of resources.
        void Clear();

        /// @brief Add a resource to the list.
        ///
        /// @param [in] type  The type of resource to add.
        /// @param [in] count The amount of this type.
        void AddResource(RmtResourceUsageType type, uint64_t count);

        /// @brief Sort the resource list, ordered by amount, largest first.
        void Sort();

        /// @brief Get the number of resources in the list.
        ///
        /// @return The number of resources.
        size_t GetNumResources() const;

        /// @brief Get the resource type for a particular index.
        ///
        /// @param [in] index The index of the resource.
        ///
        /// @return The value of the resource at the index specified.
        RmtResourceUsageType GetResourceType(size_t index) const;

        /// @brief Get the resource value for a particular index.
        ///
        /// @param [in] index The index of the resource.
        ///
        /// @return The value of the resource at the index specified.
        uint64_t GetResourceValue(size_t index) const;

        /// @brief Get the remainder of resources in the list from the specified index to the end.
        ///
        /// This value is shown in the UI as "other".
        ///
        /// @param [in] start_index The first index where counting is to begin.
        ///
        /// @return The total value of all resources from start_index to the end of the list.
        int64_t GetRemainder(int start_index) const;

    private:
        /// @brief Struct for sorting resource types by their amount.
        struct ResourceInfo
        {
            RmtResourceUsageType type;   ///< The type of resource.
            uint64_t             count;  ///< The amount (could be a count, memory used or some other amount).
        };

        /// @brief Sort comparator for sorting the resources into order by quantity.
        ///
        /// @param [in] resource_info_a The first resource to compare.
        /// @param [in] resource_info_b The second resource to compare.
        ///
        /// @return true if the first resource is larger than the second.
        static bool SortComparator(const ResourceInfo& resource_info_a, const ResourceInfo& resource_info_b);

        std::vector<ResourceInfo> resource_list_;  ///< The list of resources.
    };
}  // namespace rmv

#endif  // RMV_MODELS_RESOURCE_SORTER_H_
