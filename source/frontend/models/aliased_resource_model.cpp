//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation for the aliased resource model.
//=============================================================================

#include "models/aliased_resource_model.h"

#include "rmt_resource_list.h"

#include "util/rmv_util.h"

namespace rmv
{
    AliasedResourceModel::AliasedResourceModel()
    {
    }

    AliasedResourceModel::~AliasedResourceModel()
    {
    }

    void AliasedResourceModel::Clear()
    {
        alias_data_.clear();
    }

    bool AliasedResourceModel::Generate(const RmtVirtualAllocation* allocation)
    {
        auto alias_iter = alias_data_.find(allocation);
        if (alias_iter != alias_data_.end())
        {
            return ((*alias_iter).second.num_rows > 1);
        }

        int32_t resource_count = allocation->resource_count;

        // A list of the last resource added per row.
        AliasData new_alias_info;
        new_alias_info.resource_rows.resize(resource_count);

        std::vector<const RmtResource*> last_resource_list;
        last_resource_list.clear();
        last_resource_list.reserve(resource_count);

        for (int loop = 0; loop < resource_count; loop++)
        {
            const RmtResource* resource = allocation->resources[loop];

            // Ignore heap resources.
            if (resource->resource_type != kRmtResourceTypeHeap)
            {
                bool found     = false;
                int  row_count = static_cast<int>(last_resource_list.size());

                // For all of the rows added so far, look at the last element and see if there's an
                // overlap with that element and the one to be added. If there isn't, add it to this row.
                // If not, try the next row. Continue until it can be added to an existing row or if not,
                // add it to a new row.
                // Assumes resources are in chronological order.
                for (int i = 0; i < row_count; i++)
                {
                    const RmtResource* last_resource = last_resource_list[i];
                    if (resource->address >= (last_resource->address + last_resource->size_in_bytes))
                    {
                        last_resource_list[i]              = resource;
                        new_alias_info.resource_rows[loop] = i;
                        found                              = true;
                        break;
                    }
                }
                if (!found)
                {
                    // Add a new row.
                    new_alias_info.resource_rows[loop] = row_count;
                    last_resource_list.push_back(resource);
                }
            }
        }

        new_alias_info.num_rows = static_cast<int>(last_resource_list.size());
        alias_data_.insert(std::make_pair(allocation, new_alias_info));

        return (new_alias_info.num_rows > 1);
    }

    int AliasedResourceModel::GetNumRows(const RmtVirtualAllocation* allocation) const
    {
        auto iter = alias_data_.find(allocation);
        if (iter == alias_data_.end())
        {
            return 1;
        }
        return (*iter).second.num_rows;
    }

    int AliasedResourceModel::GetRowForResourceAtIndex(const RmtVirtualAllocation* allocation, int index) const
    {
        auto iter = alias_data_.find(allocation);
        if (iter == alias_data_.end())
        {
            return 0;
        }
        return (*iter).second.resource_rows[index];
    }
}  // namespace rmv
