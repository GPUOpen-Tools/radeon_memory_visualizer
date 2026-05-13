//=============================================================================
// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Backend allocations implementation.
//=============================================================================

#include <algorithm>

#include "rmt_address_helper.h"
#include "rmt_print.h"

#include "rmv_test_allocations.h"

namespace backend_test
{
    static const int32_t  kMaxNumAllocations           = 100000;        ///< Maximum number of allocations.
    static const int32_t  kMaxNumResourcesInAllocation = 100000;        ///< Maximum number of resources per allocation.
    static const uint64_t kMaxAllocationSize           = 0x400000000;   ///< Maximum size of a single allocation (16GB for now).
    static const uint64_t kMaxTotalAllocationSize      = 0x1000000000;  ///< Maximum size of all allocations (64GB for now).

    RMVTestAllocations::RMVTestAllocations()
    {
    }

    RMVTestAllocations::~RMVTestAllocations()
    {
    }

    bool RMVTestAllocations::RunTests(int32_t snapshot_index, const RmtDataSnapshot& snapshot, const Log& log)
    {
        log.Write("TestAllocations (%d)", snapshot_index);
        bool result = TestVirtualAllocationArray(snapshot, log);
        if (result == true)
        {
            TestVirtualAllocationSizes(snapshot, log, &result);
            TestVirtualAllocationBindings(snapshot, log, &result);
            TestVirtualAllocationGuids(snapshot, log, &result);
            TestVirtualAllocationResourceSizes(snapshot, log, &result);
        }
        return result;
    }

    bool RMVTestAllocations::TestVirtualAllocationArray(const RmtDataSnapshot& snapshot, const Log& log)
    {
        int32_t allocation_count = snapshot.virtual_allocation_list.allocation_count;

        // Check virtual allocation count.
        if (allocation_count <= 0 || allocation_count > kMaxNumAllocations)
        {
            log.Write(" ERROR: virtual allocation count %d is invalid", allocation_count);
            return false;
        }

        // Make sure pointers are valid.
        for (int32_t loop = 0; loop < allocation_count; loop++)
        {
            const RmtVirtualAllocation* allocation = &snapshot.virtual_allocation_list.allocation_details[loop];
            if (allocation == nullptr)
            {
                log.Write(" ERROR: virtual allocation at index %d is nullptr", loop);
                return false;
            }
        }
        log.Write(" Virtual allocation array OK");
        return true;
    }

    void RMVTestAllocations::TestVirtualAllocationSizes(const RmtDataSnapshot& snapshot, const Log& log, bool* result)
    {
        bool     test_result      = true;
        int32_t  allocation_count = snapshot.virtual_allocation_list.allocation_count;
        uint64_t total_size       = 0;
        for (int32_t loop = 0; loop < allocation_count; loop++)
        {
            const RmtVirtualAllocation* allocation      = &snapshot.virtual_allocation_list.allocation_details[loop];
            uint64_t                    allocation_size = static_cast<uint64_t>(allocation->size_in_4kb_page) * 4096;

            total_size += allocation_size;
            if (allocation_size > kMaxAllocationSize)
            {
                log.Write(" ERROR: Virtual allocation with base address %llu size too large at %llu bytes.", allocation->base_address, allocation_size);
                test_result = false;
            }
        }
        if (total_size > kMaxTotalAllocationSize)
        {
            log.Write(" ERROR: Total virtual allocation size too large at %llu bytes.", total_size);
            test_result = false;
        }

        if (test_result == true)
        {
            log.Write(" Virtual allocation sizes OK");
        }
        else
        {
            *result = test_result;
        }
    }

    void RMVTestAllocations::TestVirtualAllocationBindings(const RmtDataSnapshot& snapshot, const Log& log, bool* result)
    {
        bool    test_result      = true;
        int32_t allocation_count = snapshot.virtual_allocation_list.allocation_count;
        for (int loop = 0; loop < allocation_count; loop++)
        {
            const RmtVirtualAllocation* allocation     = &snapshot.virtual_allocation_list.allocation_details[loop];
            int32_t                     resource_count = allocation->resource_count;
            for (int32_t i = 0; i < resource_count; i++)
            {
                const RmtResource* resource = allocation->resources[i];
                if (resource == nullptr)
                {
                    log.Write(" ERROR: Resource at index %d in allocation with base address %lld is nullptr.", i, allocation->base_address);
                    test_result = false;
                }
                else
                {
                    if (resource->bound_allocation != 0 && resource->bound_allocation != allocation)
                    {
                        log.Write(" ERROR: Resource id %lld is bound to a different allocation.", resource->identifier);
                        test_result = false;
                    }
                }
            }
        }

        if (test_result == true)
        {
            log.Write(" Virtual allocation bindings OK");
        }
        else
        {
            *result = test_result;
        }
    }

    void RMVTestAllocations::TestVirtualAllocationGuids(const RmtDataSnapshot& snapshot, const Log& log, bool* result)
    {
        bool    test_result      = true;
        int32_t allocation_count = snapshot.virtual_allocation_list.allocation_count;
        for (int32_t loop = 0; loop < allocation_count; loop++)
        {
            const RmtVirtualAllocation* allocation = &snapshot.virtual_allocation_list.allocation_details[loop];
            if (allocation->guid != loop)
            {
                log.Write(" ERROR: Allocation at index %d has incorrect guid %d is nullptr.", loop, allocation->guid);
                test_result = false;
            }
        }

        if (test_result == true)
        {
            log.Write(" Allocation guid test OK");
        }
        else
        {
            *result = test_result;
        }
    }

    void RMVTestAllocations::TestVirtualAllocationResourceSizes(const RmtDataSnapshot& snapshot, const Log& log, bool* result)
    {
        bool    test_result      = true;
        int32_t allocation_count = snapshot.virtual_allocation_list.allocation_count;
        for (int32_t loop = 0; loop < allocation_count; loop++)
        {
            const RmtVirtualAllocation* allocation           = &snapshot.virtual_allocation_list.allocation_details[loop];
            const uint64_t              memory_region_size   = RmtVirtualAllocationGetTotalResourceMemoryInBytes(&snapshot, allocation);
            const uint64_t              resource_memory_size = GetTotalResourceSizeInAllocation(allocation, log, false);
            const uint64_t              size_in_bytes        = RmtGetAllocationSizeInBytes(allocation->size_in_4kb_page, kRmtPageSize4Kb);

            if (size_in_bytes < memory_region_size)
            {
                test_result = false;
                log.Write(" ERROR: Resources larger than allocation size for allocation %lld at index %d.", allocation->base_address, loop);
            }

            if (resource_memory_size != memory_region_size)
            {
                test_result = false;
                log.Write(" ERROR: Total resources size calculated for allocation %lld at index %d.", allocation->base_address, loop);
            }
        }

        if (test_result == true)
        {
            log.Write(" Virtual allocation resource size test OK");
        }
        else
        {
            *result = test_result;
        }
    }

    uint64_t RMVTestAllocations::GetTotalResourceSizeInAllocation(const RmtVirtualAllocation* virtual_allocation, const Log& log, bool dump_to_log)
    {
        uint64_t memory_size = 0;

        struct RmtResourceBounds
        {
            uint64_t offset;
            uint64_t end_address;
        };

        // Buckets to put the resources into. Will be used to flatten out aliased resources.
        static RmtResourceBounds resource_bounds[kMaxNumResourcesInAllocation];
        int32_t                  bound_index = 0;

        // Walk each resource in the allocation and work out what heap each resource is in.
        uint64_t total_resource_size = 0;
        uint64_t overlap_count       = 0;
        for (int32_t current_resource_index = 0; current_resource_index < virtual_allocation->resource_count; ++current_resource_index)
        {
            const RmtResource* current_resource = virtual_allocation->resources[current_resource_index];

            if (current_resource->resource_type == kRmtResourceTypeHeap)
            {
                continue;
            }

            total_resource_size += current_resource->size_in_bytes;

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

        if (total_resource_size > 0)
        {
            if (overlap_count == 0)
            {
                memory_size = total_resource_size;
            }
            else
            {
                for (int32_t index = 0; index < bound_index; index++)
                {
                    memory_size += (resource_bounds[index].end_address - resource_bounds[index].offset);
                }
            }

            if (dump_to_log)
            {
                log.Write("Allocation: %15llu, size = %15llu, total_resource_size = %10llu, memory_size = %10llu",
                          virtual_allocation->base_address,
                          static_cast<uint64_t>(virtual_allocation->size_in_4kb_page) * static_cast<uint64_t>(4096),
                          total_resource_size,
                          memory_size);
                for (int32_t index = 0; index < bound_index; index++)
                {
                    log.Write("index: %4d, start: %15llu, end: %15llu, size: %15llu",
                              index,
                              resource_bounds[index].offset,
                              resource_bounds[index].end_address,
                              resource_bounds[index].end_address - resource_bounds[index].offset);
                }
            }
        }

        return memory_size;
    }

}  // namespace backend_test
