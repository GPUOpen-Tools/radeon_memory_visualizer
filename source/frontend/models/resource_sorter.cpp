//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation for a resource sorter. Contains a list of resources
/// and allows them to be sorted and returns sorted values and smaller values
/// grouped together as "other"
//=============================================================================

#include "models/resource_sorter.h"

#include "rmt_assert.h"

namespace rmv
{
    ResourceSorter::ResourceSorter()
    {
    }

    ResourceSorter::~ResourceSorter()
    {
    }

    void ResourceSorter::Clear()
    {
        resource_list_.clear();
    }

    void ResourceSorter::AddResource(RmtResourceUsageType type, uint64_t count)
    {
        ResourceInfo resource_info;
        resource_info.type  = type;
        resource_info.count = count;
        resource_list_.push_back(resource_info);
    }

    void ResourceSorter::Sort()
    {
        std::stable_sort(resource_list_.begin(), resource_list_.end(), &ResourceSorter::SortComparator);
    }

    size_t ResourceSorter::GetNumResources()
    {
        return resource_list_.size();
    }

    RmtResourceUsageType ResourceSorter::GetResourceType(size_t index)
    {
        RMT_ASSERT(index < resource_list_.size());
        return resource_list_[index].type;
    }

    uint64_t ResourceSorter::GetResourceValue(size_t index)
    {
        RMT_ASSERT(index < resource_list_.size());
        return resource_list_[index].count;
    }

    int64_t ResourceSorter::GetRemainder(int start_index)
    {
        int64_t value = 0;
        for (size_t i = start_index; i < GetNumResources(); i++)
        {
            value += resource_list_[i].count;
        }
        return value;
    }

    bool ResourceSorter::SortComparator(const ResourceInfo& resource_info_a, const ResourceInfo& resource_info_b)
    {
        return resource_info_a.count > resource_info_b.count;
    }
}  // namespace rmv