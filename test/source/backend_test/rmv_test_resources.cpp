//=============================================================================
// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend test resources implementation.
//=============================================================================

#include "rmv_test_resources.h"

#include <algorithm>
#include <map>
#include <vector>

#include "rmt_assert.h"
#include "rmt_data_set.h"
#include "rmt_print.h"

#ifndef _WIN32
#include "linux/safe_crt.h"
#endif

namespace backend_test
{
    // These aren't really errors per-se, but would like to keep track of what apps are doing.
    // Use some sufficiently large numbers.
    static const int32_t  kMaxResources         = 300000;        ///< The maximum number of resources.
    static const uint64_t kMaxResourceSize      = 0x400000000;   ///< The maximum size of a single resource (16GB for now).
    static const uint64_t kMaxTotalResourceSize = 0x1000000000;  ///< The maximum size for all resources (64GB for now).

    RMVTestResources::RMVTestResources()
    {
    }

    RMVTestResources::~RMVTestResources()
    {
    }

    bool RMVTestResources::RunTests(int32_t snapshot_index, const RmtDataSnapshot& snapshot, const Log& log)
    {
        log.Write("TestResources (%d):", snapshot_index);
        bool result = TestResourceArray(snapshot, log);
        if (result == true)
        {
            TestResourceCreateFlags(snapshot, log, &result);
            TestResourceUsageTypes(snapshot, log, &result);
            TestResourceSizes(snapshot, log, &result);
            TestResourceTimestamps(snapshot, log, &result);
        }
        return result;
    }

    bool RMVTestResources::TestResourceArray(const RmtDataSnapshot& snapshot, const Log& log)
    {
        const RmtResourceList& resource_list  = snapshot.resource_list;
        const int32_t          resource_count = resource_list.resource_count;

        // Check resource count.
        if (resource_count <= 0 || resource_count > kMaxResources)
        {
            log.Write(" ERROR: resource count %d is invalid", resource_count);
            return false;
        }

        // Make sure pointers are valid.
        for (int32_t loop = 0; loop < resource_count; loop++)
        {
            const RmtResource* resource = &resource_list.resources[loop];
            if (resource == nullptr)
            {
                log.Write(" ERROR: resource at index %d is nullptr", loop);
                return false;
            }
        }
        log.Write(" Resource array OK");
        return true;
    }

    void RMVTestResources::TestResourceCreateFlags(const RmtDataSnapshot& snapshot, const Log& log, bool* result)
    {
        bool          test_result    = true;
        const int32_t resource_count = snapshot.resource_list.resource_count;
        for (int32_t loop = 0; loop < resource_count; loop++)
        {
            const RmtResource* resource = &snapshot.resource_list.resources[loop];
            switch (resource->resource_type)
            {
            case kRmtResourceTypePipeline:
            {
                uint32_t create_flags = resource->pipeline.create_flags;
                if ((create_flags & kRmtPipelineCreateFlagReserved0) == kRmtPipelineCreateFlagReserved0)
                {
                    log.Write(" ERROR: resource id %llu: create flag is kRmtPipelineCreateFlagReserved0", resource->identifier);
                    test_result = false;
                }
                if ((create_flags & kRmtPipelineCreateFlagReserved1) == kRmtPipelineCreateFlagReserved1)
                {
                    log.Write(" ERROR: resource id %llu: create flag is kRmtPipelineCreateFlagReserved1", resource->identifier);
                    test_result = false;
                }
                if ((create_flags & kRmtPipelineCreateFlagReserved2) == kRmtPipelineCreateFlagReserved2)
                {
                    log.Write(" ERROR: resource id %llu: create flag is kRmtPipelineCreateFlagReserved2", resource->identifier);
                    test_result = false;
                }
                if ((create_flags & kRmtPipelineCreateFlagReserved3) == kRmtPipelineCreateFlagReserved3)
                {
                    log.Write(" ERROR: resource id %llu: create flag is kRmtPipelineCreateFlagReserved3", resource->identifier);
                    test_result = false;
                }
                if ((create_flags & kRmtPipelineCreateFlagReserved4) == kRmtPipelineCreateFlagReserved4)
                {
                    log.Write(" ERROR: resource id %llu: create flag is kRmtPipelineCreateFlagReserved4", resource->identifier);
                    test_result = false;
                }
                if ((create_flags & kRmtPipelineCreateFlagReserved5) == kRmtPipelineCreateFlagReserved5)
                {
                    log.Write(" ERROR: resource id %llu: create flag is kRmtPipelineCreateFlagReserved5", resource->identifier);
                    test_result = false;
                }
                break;
            }

            case kRmtResourceTypeImage:
            {
                uint32_t create_flags = resource->image.create_flags;
                if ((create_flags & kRmtImageCreationFlagReserved0) == kRmtImageCreationFlagReserved0)
                {
                    log.Write(" ERROR: resource id %llu: create flag is kRmtImageCreationFlagReserved0", resource->identifier);
                    test_result = false;
                }
            }

            default:
                break;
            }
        }
        if (test_result == true)
        {
            log.Write(" Resource create flags OK");
        }
        else
        {
            *result = test_result;
        }
    }

    void RMVTestResources::TestResourceUsageTypes(const RmtDataSnapshot& snapshot, const Log& log, bool* result)
    {
        bool    test_result    = true;
        int32_t resource_count = snapshot.resource_list.resource_count;
        for (int loop = 0; loop < resource_count; loop++)
        {
            const RmtResource*         resource   = &snapshot.resource_list.resources[loop];
            const RmtResourceUsageType usage_type = RmtResourceGetUsageType(resource);

            if (usage_type == kRmtResourceUsageTypeUnknown)
            {
                char info_string[1024];
                sprintf_s(info_string,
                          1024,
                          " Resource id %llu, type is %s, size is %llu bytes",
                          resource->identifier,
                          RmtGetResourceTypeNameFromResourceType(resource->resource_type),
                          resource->size_in_bytes);

                log.Write(" ERROR: Resource usage type is Unknown. %s", info_string);
                test_result = false;
            }
        }
        if (test_result == true)
        {
            log.Write(" Resource usage types OK");
        }
        else
        {
            *result = test_result;
        }
    }

    void RMVTestResources::TestResourceSizes(const RmtDataSnapshot& snapshot, const Log& log, bool* result)
    {
        bool          test_result    = true;
        const int32_t resource_count = snapshot.resource_list.resource_count;
        uint64_t      total_size     = 0;  // Total resource size, ignoring heaps, aliasing duplication ignored.
        uint64_t      aliased_size   = 0;  // Resource sizes accounting for aliasing.
        uint64_t      heap_size      = 0;  // Total size of heap resources

        // map of virtual allocation to list of resources
        std::map<const RmtVirtualAllocation*, std::vector<const RmtResource*> > alias_list;

        for (int32_t loop = 0; loop < resource_count; loop++)
        {
            const RmtResource* resource      = &snapshot.resource_list.resources[loop];
            const uint64_t     resource_size = resource->size_in_bytes;

            // Ignore heap resources.
            if (resource->resource_type == kRmtResourceTypeHeap)
            {
                heap_size += resource_size;
                continue;
            }
            total_size += resource_size;

            if (RmtResourceGetAliasCount(resource) > 0)
            {
                // For aliased resources, add them to a map and process them later.
                // Need to make sure they aren't counted more than once.
                // Also need to take into account resources that overlap.
                auto it1 = alias_list.find(resource->bound_allocation);
                if (it1 != alias_list.end())
                {
                    // For each memory range, check to see if this one is in range.
                    (*it1).second.push_back(resource);
                }
                else
                {
                    std::vector<const RmtResource*> resource_list;
                    resource_list.push_back(resource);
                    alias_list.insert(std::make_pair(resource->bound_allocation, resource_list));
                }
            }
            else
            {
                aliased_size += resource_size;
            }

            if (resource_size > kMaxResourceSize)
            {
                log.Write(" ERROR: Resource id %llu size too large at %llu bytes.", resource->identifier, resource_size);
                test_result = false;
            }
        }

        // Process the aliased resources here.
        for (auto it = alias_list.begin(); it != alias_list.end(); ++it)
        {
            const RmtVirtualAllocation* virtual_allocation = (*it).first;
            const auto                  resource_list      = (*it).second;

            struct RmtResourceBounds
            {
                uint64_t offset;
                uint64_t end_address;
            };

            // Buckets to put the resources into. Will be used to flatten out aliased resources.
            static RmtResourceBounds resource_bounds[kMaxResources];
            int32_t                  bound_index   = 0;
            uint64_t                 overlap_count = 0;

            for (size_t current_resource_index = 0; current_resource_index < resource_list.size(); ++current_resource_index)
            {
                const RmtResource* current_resource = resource_list[current_resource_index];

                // Find resources which share the same address ranges and bucket them together if their address
                // ranges overlap. This is needed to remove aliasing so resource memory is only counted once.
                bool     found_overlap = false;
                uint64_t start_address = current_resource->address - virtual_allocation->base_address;
                int64_t  end_address   = start_address + current_resource->size_in_bytes;
                for (int32_t index = 0; index < bound_index; index++)
                {
                    // The extra +/-1 here is to allow the current resource to be concatenated to the last resource even though they don't
                    // overlap.
                    if (start_address > (resource_bounds[index].end_address + 1) || end_address < static_cast<int64_t>(resource_bounds[index].offset - 1))
                    {
                        // No overlap, nothing to do.
                    }
                    else
                    {
                        resource_bounds[index].offset      = std::min<uint64_t>(start_address, resource_bounds[index].offset);
                        resource_bounds[index].end_address = std::max<uint64_t>(static_cast<uint64_t>(end_address), resource_bounds[index].end_address);
                        found_overlap                      = true;
                        overlap_count++;
                        break;
                    }
                }

                if (found_overlap == false)
                {
                    resource_bounds[bound_index].offset      = start_address;
                    resource_bounds[bound_index].end_address = resource_bounds[bound_index].offset + current_resource->size_in_bytes;
                    bound_index++;
                }
            }

            for (int32_t index = 0; index < bound_index; index++)
            {
                aliased_size += (resource_bounds[index].end_address - resource_bounds[index].offset);
            }
        }

        if (aliased_size > kMaxTotalResourceSize)
        {
            log.Write(" ERROR: Total resource size too large at %llu bytes.", total_size);
            test_result = false;
        }

        if (test_result == true)
        {
            log.Write(" Resource sizes OK. Total size excluding (including) heaps is %llu (%llu), size without aliasing %llu.",
                      total_size,
                      total_size + heap_size,
                      aliased_size);
        }
        else
        {
            *result = test_result;
        }
    }

    void RMVTestResources::TestResourceTimestamps(const RmtDataSnapshot& snapshot, const Log& log, bool* result)
    {
        bool           test_result    = true;
        const int32_t  resource_count = snapshot.resource_list.resource_count;
        const uint64_t max_timestamp  = snapshot.data_set->maximum_timestamp;
        for (int32_t loop = 0; loop < resource_count; loop++)
        {
            const RmtResource* resource    = &snapshot.resource_list.resources[loop];
            const uint64_t     create_time = resource->create_time;
            const uint64_t     bind_time   = resource->bind_time;

            // Make sure bind time comes after create time if resource is bound.
            if (bind_time < create_time && bind_time != 0)
            {
                log.Write(
                    " ERROR: resource id %llu bound before being created. Create time %llu, bind time %llu.", resource->identifier, create_time, bind_time);
                test_result = false;
            }

            // If resource is bound, make sure it has a bound allocation.
            // This could be caused by an error in the application where a resource is bound and the allocation is
            // destroyed before unbinding the resource.
            // Mark as a warning for now.
            if (bind_time != 0 && resource->bound_allocation == nullptr)
            {
                log.Write(" WARNING: resource id %llu has a bound time but no bound allocation (orphaned).", resource->identifier);
            }

            // Make sure bind time < global timestamp.
            if (bind_time > max_timestamp)
            {
                log.Write(" ERROR: resource id %llu bind time larger than max timestamp.", resource->identifier);
                test_result = false;
            }
        }

        if (test_result == true)
        {
            log.Write(" Resource timestamps OK");
        }
        else
        {
            *result = test_result;
        }
    }

}  // namespace backend_test
