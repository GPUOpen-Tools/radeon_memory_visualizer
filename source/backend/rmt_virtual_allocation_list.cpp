//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the virtual allocation list functions.
//=============================================================================

#include "rmt_virtual_allocation_list.h"

#include <math.h>
#include <string.h>  // memcpy
#include <algorithm>
#include <vector>

#include "rmt_address_helper.h"
#include "rmt_assert.h"
#include "rmt_data_snapshot.h"
#include "rmt_page_table.h"
#include "rmt_resource_list.h"
#include "rmt_resource_userdata.h"
#include "rmt_tree.h"

typedef rmt_tree::IntervalTree<uint64_t, RmtResource*> AliasedResourceIntervalTreeType;
typedef rmt_tree::Interval<uint64_t, RmtResource*>     AliasedResourceIntervalType;

// The ResourcePriorityComparator handles ordering resources based on aliased resource priority.
// A comparison is first made on the resource usage type enum value (highest to lowest).  If the two resources
// being compared have the same usage type, the resources are compared by size (smallest size is highest priority).
// If the size is also the same, then resource IDs are compared (largest value is highest priority).
struct ResourcePriorityComparator
{
    bool operator()(const RmtResource* lhs, const RmtResource* rhs) const
    {
        bool result = false;

        const RmtResourceUsageType lhs_usage_type = RmtResourceGetUsageType(lhs);
        const RmtResourceUsageType rhs_usage_type = RmtResourceGetUsageType(rhs);

        if (lhs_usage_type != rhs_usage_type)
        {
            // Compare by usage type.
            result = lhs_usage_type > rhs_usage_type;
        }
        else
        {
            if (lhs->size_in_bytes != rhs->size_in_bytes)
            {
                // Compare by size.
                result = lhs->size_in_bytes < rhs->size_in_bytes;
            }
            else
            {
                // Compare by resource identifier (last resort, since the other attributes are the same).
                result = lhs->identifier > rhs->identifier;
            }
        }

        return result;
    }
};

// Helper function to improve tree balance by hashing the handles.
static RmtGpuAddress HashGpuAddress(RmtGpuAddress address)
{
#if 0
    int32_t low = (int32_t)address;
    int32_t high = (int32_t)(address >> 32);
    return (int32_t)((0x1337c0dedeadbeef * low + 0x12ed89f849021acd * high + 0x293fff346aba878e) >> 32);
#else
    return address;
#endif
}

// helper function to find smallest value in a branch
static RmtVirtualAllocationInterval* GetSmallestNode(RmtVirtualAllocationInterval* node)
{
    while ((node != nullptr) && (node->left != nullptr))
    {
        node = node->left;
    }
    return node;
}

// recursive function to find a node by ID
static RmtVirtualAllocationInterval* FindAllocationNode(RmtVirtualAllocationInterval* root, RmtGpuAddress gpu_address)
{
    if (root == nullptr)
    {
        return NULL;
    }

    if (root->base_address <= gpu_address && gpu_address < (root->base_address + (RmtGetPageSize(kRmtPageSize4Kb) * root->size_in_4kb_pages)))
    {
        return root;
    }

    if (gpu_address < root->base_address)
    {
        return FindAllocationNode(root->left, gpu_address);
    }

    return FindAllocationNode(root->right, gpu_address);
}

// recursive function to insert a node
static RmtVirtualAllocationInterval* InsertNode(RmtVirtualAllocationList*     virtual_allocation_list,
                                                RmtVirtualAllocationInterval* node,
                                                RmtGpuAddress                 gpu_address,
                                                int32_t                       size_in_pages,
                                                RmtVirtualAllocation*         allocation)
{
    if (node == nullptr)
    {
        // create a new node
        RmtVirtualAllocationInterval* new_node = (RmtVirtualAllocationInterval*)RmtPoolAllocate(&virtual_allocation_list->allocation_interval_pool);
        if (new_node != nullptr)
        {
            new_node->base_address      = gpu_address;
            new_node->size_in_4kb_pages = size_in_pages;
            new_node->dead              = 0;
            new_node->allocation        = allocation;
            new_node->left              = NULL;
            new_node->right             = NULL;
            return new_node;
        }
        else
        {
            RMT_ASSERT_FAIL("RmtVirtualAllocationInterval InsertNode alloc failed");
        }
    }

    if (gpu_address < node->base_address)
    {
        node->left = InsertNode(virtual_allocation_list, node->left, gpu_address, size_in_pages, allocation);
    }
    else if (gpu_address >= node->base_address)
    {
        node->right = InsertNode(virtual_allocation_list, node->right, gpu_address, size_in_pages, allocation);
    }
    else
    {
        RMT_ASSERT_FAIL("RmtVirtualAllocationInterval InsertNode failed");
    }

    return node;
}

// recursive function to delete a node
static RmtVirtualAllocationInterval* DeleteNode(RmtVirtualAllocationList*     virtual_allocation_list,
                                                RmtVirtualAllocationInterval* node,
                                                RmtGpuAddress                 gpu_address)
{
    if (node == nullptr)
    {
        return node;
    }

    if (gpu_address < node->base_address)
    {
        node->left = DeleteNode(virtual_allocation_list, node->left, gpu_address);
    }
    else if (gpu_address > node->base_address)
    {
        node->right = DeleteNode(virtual_allocation_list, node->right, gpu_address);
    }
    else
    {
        if (node->left == nullptr)
        {
            RmtVirtualAllocationInterval* child = node->right;
            node->base_address                  = 0;
            node->size_in_4kb_pages             = 0;
            node->left                          = NULL;
            node->right                         = NULL;
            node->dead                          = 0;
            RmtPoolFree(&virtual_allocation_list->allocation_interval_pool, node);
            return child;
        }
        if (node->right == nullptr)
        {
            RmtVirtualAllocationInterval* child = node->left;
            node->base_address                  = 0;
            node->size_in_4kb_pages             = 0;
            node->dead                          = 0;
            node->left                          = NULL;
            node->right                         = NULL;
            RmtPoolFree(&virtual_allocation_list->allocation_interval_pool, node);
            return child;
        }

        RmtVirtualAllocationInterval* smallest_child = GetSmallestNode(node->right);
        node->base_address                           = smallest_child->base_address;
        node->size_in_4kb_pages                      = smallest_child->size_in_4kb_pages;
        node->dead                                   = smallest_child->dead;

        // update payload pointers.
        RMT_ASSERT(node->allocation);
        node->allocation = smallest_child->allocation;

        // now delete the node we just moved.
        node->right = DeleteNode(virtual_allocation_list, node->right, smallest_child->base_address);
    }

    return node;
}

// search the acceleration structure for a resource.
static RmtVirtualAllocationInterval* FindAllocationIntervalByAddress(const RmtVirtualAllocationList* virtual_allocation_list, RmtGpuAddress gpu_address)
{
    RmtVirtualAllocationInterval* found_node = FindAllocationNode(virtual_allocation_list->root, gpu_address);
    if (found_node == nullptr)
    {
        return NULL;
    }

    return found_node;
}

// add a resource to the acceleration structure.
static RmtErrorCode AddAllocationToTree(RmtVirtualAllocationList* virtual_allocation_list,
                                        RmtGpuAddress             gpu_address,
                                        int32_t                   size_in_4kb_pages,
                                        RmtVirtualAllocation*     virtual_allocation)
{
    const size_t pool_count       = virtual_allocation_list->allocation_interval_pool.allocated;
    virtual_allocation_list->root = InsertNode(virtual_allocation_list, virtual_allocation_list->root, gpu_address, size_in_4kb_pages, virtual_allocation);
    RMT_ASSERT(virtual_allocation_list->allocation_interval_pool.allocated == pool_count + 1);
    return kRmtOk;
}

// destroy a resource from the acceleration structure.
static RmtErrorCode RemoveAllocationFromTree(RmtVirtualAllocationList* virtual_allocation_list, RmtGpuAddress gpu_address)
{
    const size_t pool_count       = virtual_allocation_list->allocation_interval_pool.allocated;
    virtual_allocation_list->root = DeleteNode(virtual_allocation_list, virtual_allocation_list->root, gpu_address);
    RMT_ASSERT(virtual_allocation_list->allocation_interval_pool.allocated == pool_count - 1);
    return kRmtOk;
}

uint64_t RmtVirtualAllocationGetSizeInBytes(const RmtVirtualAllocation* virtual_allocation)
{
    RMT_RETURN_ON_ERROR(virtual_allocation, 0);

    return ((uint64_t)virtual_allocation->size_in_4kb_page) << 12;
}

uint64_t RmtVirtualAllocationGetLargestResourceSize(const RmtVirtualAllocation* virtual_allocation)
{
    RMT_RETURN_ON_ERROR(virtual_allocation, 0);
    RMT_RETURN_ON_ERROR(virtual_allocation->resource_count, 0);

    uint64_t maximum_resource_size = 0;
    for (int32_t current_resource_index = 0; current_resource_index < virtual_allocation->resource_count; ++current_resource_index)
    {
        maximum_resource_size = RMT_MAXIMUM(maximum_resource_size, (uint64_t)virtual_allocation->resources[current_resource_index]->size_in_bytes);
    }

    return maximum_resource_size;
}

uint64_t RmtVirtualAllocationGetTotalResourceMemoryInBytes(const RmtDataSnapshot* snapshot, const RmtVirtualAllocation* virtual_allocation)
{
    RMT_ASSERT(snapshot);
    RMT_ASSERT(virtual_allocation);
    RMT_RETURN_ON_ERROR(virtual_allocation, 0);
    RMT_RETURN_ON_ERROR(virtual_allocation->resource_count, 0);
    RMT_ASSERT(virtual_allocation->resource_count <= snapshot->region_stack_count);
    RMT_RETURN_ON_ERROR(virtual_allocation->resource_count <= snapshot->region_stack_count, 0);

    RmtMemoryRegion* region_stack             = snapshot->region_stack_buffer;
    int32_t          current_region_stack_top = 0;

    uint64_t total_resource_size = 0;

    for (int32_t current_resource_index = 0; current_resource_index < virtual_allocation->resource_count; ++current_resource_index)
    {
        const RmtResource* current_resource        = virtual_allocation->resources[current_resource_index];
        const size_t       current_resource_offset = current_resource->address - virtual_allocation->base_address;

        // Ignore heaps.
        if (current_resource->resource_type == kRmtResourceTypeHeap)
        {
            continue;
        }

        if (current_region_stack_top == 0)
        {
            region_stack[current_region_stack_top].offset = current_resource_offset;
            region_stack[current_region_stack_top].size   = current_resource->size_in_bytes;
            current_region_stack_top++;
        }
        else
        {
            RmtMemoryRegion* top_region = &region_stack[current_region_stack_top - 1];
            if ((top_region->offset + top_region->size) <= current_resource_offset)
            {
                region_stack[current_region_stack_top].offset = current_resource_offset;
                region_stack[current_region_stack_top].size   = current_resource->size_in_bytes;
                current_region_stack_top++;
                continue;
            }

            // merge ranges.
            const size_t new_size = (current_resource_offset + current_resource->size_in_bytes) - region_stack[current_region_stack_top - 1].offset;
            region_stack[current_region_stack_top - 1].size = RMT_MAXIMUM(new_size, region_stack[current_region_stack_top - 1].size);
        }
    }

    for (int32_t current_stack_index = 0; current_stack_index < current_region_stack_top; ++current_stack_index)
    {
        total_resource_size += region_stack[current_stack_index].size;
    }

    RMT_ASSERT(RmtVirtualAllocationGetSizeInBytes(virtual_allocation) >= total_resource_size);
    return total_resource_size;
}

uint64_t RmtVirtualAllocationGetTotalUnboundSpaceInAllocation(const RmtDataSnapshot* snapshot, const RmtVirtualAllocation* virtual_allocation)
{
    RMT_ASSERT(virtual_allocation);
    RMT_RETURN_ON_ERROR(virtual_allocation, 0);

    const uint64_t total_resource_memory = RmtVirtualAllocationGetTotalResourceMemoryInBytes(snapshot, virtual_allocation);

    RMT_ASSERT(total_resource_memory <= RmtVirtualAllocationGetSizeInBytes(virtual_allocation));
    return RmtVirtualAllocationGetSizeInBytes(virtual_allocation) - total_resource_memory;
}

uint64_t RmtVirtualAllocationGetAverageResourceSizeInBytes(const RmtDataSnapshot* snapshot, const RmtVirtualAllocation* virtual_allocation)
{
    RMT_ASSERT(virtual_allocation);
    if (virtual_allocation == NULL || virtual_allocation->non_heap_resource_count == 0)
    {
        return 0;
    }

    const uint64_t total_resource_size = RmtVirtualAllocationGetTotalResourceMemoryInBytes(snapshot, virtual_allocation);
    return total_resource_size / virtual_allocation->non_heap_resource_count;
}

uint64_t RmtVirtualAllocationGetResourceStandardDeviationInBytes(const RmtDataSnapshot* snapshot, const RmtVirtualAllocation* virtual_allocation)
{
    RMT_ASSERT(virtual_allocation);
    if (virtual_allocation == NULL || virtual_allocation->non_heap_resource_count == 0)
    {
        return 0;
    }

    uint64_t variance          = 0;
    int64_t  avg_resource_size = RmtVirtualAllocationGetAverageResourceSizeInBytes(snapshot, virtual_allocation);
    for (int32_t current_resource_index = 0; current_resource_index < virtual_allocation->resource_count; ++current_resource_index)
    {
        const RmtResource* current_resource = virtual_allocation->resources[current_resource_index];

        if (current_resource->resource_type == RmtResourceType::kRmtResourceTypeHeap)
        {
            // Skip heaps.
            continue;
        }

        const int64_t diff = current_resource->size_in_bytes - avg_resource_size;
        variance           = diff * diff;
    }
    variance /= virtual_allocation->non_heap_resource_count;

    return (uint64_t)sqrt((double)variance);
}

float RmtVirtualAllocationGetFragmentationQuotient(const RmtVirtualAllocation* virtual_allocation)
{
    RMT_ASSERT(virtual_allocation);

    if (virtual_allocation == NULL || virtual_allocation->resource_count == 0)
    {
        return 0.0F;
    }

    int32_t  gaps_in_virtual_memory = 0;
    uint64_t last_address           = virtual_allocation->base_address;

    for (int32_t current_resource_index = 0; current_resource_index < virtual_allocation->resource_count; ++current_resource_index)
    {
        const RmtResource* current_resource = virtual_allocation->resources[current_resource_index];
        if (current_resource->address != last_address)
        {
            gaps_in_virtual_memory++;
        }

        // advance the last address to the next byte in the allocation.
        last_address = current_resource->address + current_resource->size_in_bytes;
    }

    return (float)gaps_in_virtual_memory;
}

size_t RmtVirtualAllocationListGetBufferSize(int32_t total_allocations, int32_t max_concurrent_resources)
{
    return (total_allocations * (sizeof(RmtVirtualAllocationInterval) + sizeof(RmtVirtualAllocation))) + (max_concurrent_resources * sizeof(uintptr_t)) +
           ((total_allocations + max_concurrent_resources) * sizeof(RmtMemoryRegion));
}

RmtErrorCode RmtVirtualAllocationListInitialize(RmtVirtualAllocationList* virtual_allocation_list,
                                                void*                     buffer,
                                                size_t                    buffer_size,
                                                int32_t                   maximum_concurrent_allocations,
                                                int32_t                   maximum_concurrent_resources,
                                                int32_t                   total_allocations)
{
    RMT_ASSERT(virtual_allocation_list);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(buffer, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(buffer_size, kRmtErrorInvalidSize);
    RMT_RETURN_ON_ERROR(RmtVirtualAllocationListGetBufferSize(total_allocations, maximum_concurrent_resources) <= buffer_size, kRmtErrorInvalidSize);

    const size_t interval_size_in_bytes = (total_allocations * sizeof(RmtVirtualAllocationInterval));

    // dice up the buffer
    virtual_allocation_list->allocation_intervals           = (RmtVirtualAllocationInterval*)buffer;
    virtual_allocation_list->allocation_details             = (RmtVirtualAllocation*)(((uintptr_t)buffer) + interval_size_in_bytes);
    virtual_allocation_list->allocation_count               = 0;
    virtual_allocation_list->next_allocation_guid           = 0;
    virtual_allocation_list->maximum_concurrent_allocations = maximum_concurrent_allocations;
    virtual_allocation_list->total_allocations              = total_allocations;
    virtual_allocation_list->total_allocated_bytes          = 0;
    const size_t allocation_details_size                    = total_allocations * (sizeof(RmtVirtualAllocationInterval) + sizeof(RmtVirtualAllocation));
    virtual_allocation_list->resource_connectivity          = (RmtResource**)((uintptr_t)buffer + allocation_details_size);
    virtual_allocation_list->resource_connectivity_count    = maximum_concurrent_resources;
    virtual_allocation_list->unbound_memory_regions         = nullptr;
    virtual_allocation_list->unbound_memory_region_count    = 0;

    // initialize interval pool.
    RmtPoolInitialize(&virtual_allocation_list->allocation_interval_pool,
                      virtual_allocation_list->allocation_intervals,
                      interval_size_in_bytes,
                      sizeof(RmtVirtualAllocationInterval));
    virtual_allocation_list->root = NULL;

    memset(virtual_allocation_list->allocations_per_preferred_heap, 0, sizeof(virtual_allocation_list->allocations_per_preferred_heap));
    return kRmtOk;
}

RmtErrorCode RmtVirtualAllocationListAddAllocation(RmtVirtualAllocationList* virtual_allocation_list,
                                                   uint64_t                  timestamp,
                                                   RmtGpuAddress             address,
                                                   int32_t                   size_in_4kb_pages,
                                                   const RmtHeapType         preferences[4],
                                                   RmtOwnerType              owner,
                                                   uint64_t                  allocation_identifier)
{
    RMT_ASSERT(virtual_allocation_list);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(size_in_4kb_pages, kRmtErrorInvalidSize);
    RMT_RETURN_ON_ERROR(virtual_allocation_list->allocation_count < virtual_allocation_list->total_allocations, kRmtErrorOutOfMemory);

    // check if this region overlaps an existing region
    const RmtVirtualAllocation* found_allocation = NULL;
    const RmtErrorCode          error_code       = RmtVirtualAllocationListGetAllocationForAddress(virtual_allocation_list, address, &found_allocation);
    if (error_code == kRmtOk)
    {
        RMT_ASSERT((found_allocation->flags & kRmtAllocationDetailIsDead) != kRmtAllocationDetailIsDead);
    }
    RMT_RETURN_ON_ERROR(error_code != kRmtOk, error_code);

    const int32_t next_allocation_index = virtual_allocation_list->allocation_count++;

    // fill out the details.
    RmtVirtualAllocation* allocation_details    = &virtual_allocation_list->allocation_details[next_allocation_index];
    allocation_details->base_address            = address;
    allocation_details->size_in_4kb_page        = size_in_4kb_pages;
    allocation_details->guid                    = virtual_allocation_list->next_allocation_guid++;
    allocation_details->flags                   = 0;
    allocation_details->timestamp               = timestamp;
    allocation_details->last_residency_update   = 0;
    allocation_details->last_cpu_map            = 0;
    allocation_details->last_cpu_un_map         = 0;
    allocation_details->add_count               = 0;
    allocation_details->remove_count            = 0;
    allocation_details->non_heap_resource_count = 0;
    allocation_details->map_count               = 0;
    allocation_details->owner                   = owner;
    allocation_details->commit_type             = 0;
    allocation_details->resource_count          = 0;
    allocation_details->next_resource_index     = 0;
    allocation_details->allocation_identifier   = allocation_identifier;
    memset(allocation_details->resource_usage_aliased_size, 0, sizeof(allocation_details->resource_usage_aliased_size));

    for (int32_t current_heap_preference_index = 0; current_heap_preference_index < RMT_NUM_HEAP_PREFERENCES; ++current_heap_preference_index)
    {
        allocation_details->heap_preferences[current_heap_preference_index] = preferences[current_heap_preference_index];
    }

    // fill out the allocation interval
    const RmtGpuAddress hashed_address = HashGpuAddress(address);
    AddAllocationToTree(virtual_allocation_list, hashed_address, size_in_4kb_pages, allocation_details);

    const uint64_t size_in_bytes = (static_cast<uint64_t>(size_in_4kb_pages) << 12);
    virtual_allocation_list->total_allocated_bytes += size_in_bytes;
    virtual_allocation_list->allocations_per_preferred_heap[allocation_details->heap_preferences[0]] += size_in_bytes;
    return kRmtOk;
}

RmtErrorCode RmtVirtualAllocationListRemoveAllocation(RmtVirtualAllocationList* virtual_allocation_list, RmtGpuAddress address)
{
    RMT_ASSERT(virtual_allocation_list);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(virtual_allocation_list->allocation_count, kRmtErrorNoAllocationFound);

    const RmtGpuAddress           hashed_address              = HashGpuAddress(address);
    RmtVirtualAllocationInterval* current_allocation_interval = FindAllocationIntervalByAddress(virtual_allocation_list, hashed_address);
    if (current_allocation_interval == nullptr)
    {
        return kRmtErrorNoAllocationFound;
    }

    // mark the allocation as dead, the allocation will then be removed later on
    // when the resource pointers are set on the allocation. Removal of allocations
    // is deferred in this way, as moving the virtual allocation structures
    // on-demand means we'd have to dive off and fix up the boundAllocation pointers
    // on the resources. As we don't set these until the end of the parsing process
    // this is undesireable. Additionally, this has the benefit that we can
    // potentially detect "dangling" resources, i.e.: resources which are not
    // destroyed that are still bound to a free'd range in the virtual address map.
    // See the rmtVirtualAllocationListCompact for more info.
    current_allocation_interval->dead = 1;
    current_allocation_interval->allocation->flags |= kRmtAllocationDetailIsDead;

    const uint64_t size_in_bytes = static_cast<uint64_t>(current_allocation_interval->allocation->size_in_4kb_page) << 12;
    virtual_allocation_list->total_allocated_bytes -= size_in_bytes;
    virtual_allocation_list->allocations_per_preferred_heap[current_allocation_interval->allocation->heap_preferences[0]] -= size_in_bytes;

    // remove efrom the tree
    RemoveAllocationFromTree(virtual_allocation_list, hashed_address);

    return kRmtOk;
}

RmtErrorCode RmtVirtualAllocationListAddResourceReference(RmtVirtualAllocationList* virtual_allocation_list,
                                                          uint64_t                  timestamp,
                                                          RmtGpuAddress             address,
                                                          RmtResidencyUpdateType    update_type,
                                                          RmtQueue                  queue)
{
    RMT_UNUSED(queue);

    RMT_ASSERT(virtual_allocation_list);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(virtual_allocation_list->allocation_count, kRmtErrorNoAllocationFound);

    // find the allocation index.
    const RmtGpuAddress           hashed_address = HashGpuAddress(address);
    RmtVirtualAllocationInterval* interval       = FindAllocationIntervalByAddress(virtual_allocation_list, hashed_address);
    if (interval == nullptr)
    {
        return kRmtErrorNoAllocationFound;
    }

    // store the residency update on the details structure.
    RmtVirtualAllocation* current_details = interval->allocation;
    RMT_ASSERT(current_details);

    if (update_type == kRmtResidencyUpdateTypeAdd)
    {
        current_details->flags |= kRmtAllocationDetailHasBeenMadeResident;
        current_details->flags |= kRmtAllocationDetailIsMadeResident;
        current_details->add_count++;
    }
    else if (update_type == kRmtResidencyUpdateTypeRemove)
    {
        current_details->flags |= kRmtAllocationDetailHasBeenEvicted;
        current_details->flags &= ~kRmtAllocationDetailIsMadeResident;
        current_details->remove_count++;
    }

    current_details->last_residency_update = timestamp;

    return kRmtOk;
}

RmtErrorCode RmtVirtualAllocationListAddCpuMap(RmtVirtualAllocationList* virtual_allocation_list, uint64_t timestamp, RmtGpuAddress address)
{
    RMT_ASSERT(virtual_allocation_list);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(virtual_allocation_list->allocation_count, kRmtErrorNoAllocationFound);

    // find the allocation index.
    const RmtGpuAddress           hashed_address = HashGpuAddress(address);
    RmtVirtualAllocationInterval* interval       = FindAllocationIntervalByAddress(virtual_allocation_list, hashed_address);
    if (interval == nullptr)
    {
        return kRmtErrorNoAllocationFound;
    }

    // store the residency update on the details structure.
    RmtVirtualAllocation* current_details = interval->allocation;
    current_details->flags |= kRmtAllocationDetailIsCpuMapped;
    current_details->flags |= kRmtAllocationDetailHasBeenCpuMapped;
    current_details->last_cpu_map = timestamp;
    current_details->map_count++;

    return kRmtOk;
}

RmtErrorCode RmtVirtualAllocationListAddCpuUnmap(RmtVirtualAllocationList* virtual_allocation_list, uint64_t timestamp, RmtGpuAddress address)
{
    RMT_ASSERT(virtual_allocation_list);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(virtual_allocation_list->allocation_count, kRmtErrorNoAllocationFound);

    // find the allocation index.
    const RmtGpuAddress           hashed_address = HashGpuAddress(address);
    RmtVirtualAllocationInterval* interval       = FindAllocationIntervalByAddress(virtual_allocation_list, hashed_address);
    if (interval == nullptr)
    {
        return kRmtErrorNoAllocationFound;
    }

    // store the residency update on the details structure.
    RmtVirtualAllocation* current_details = interval->allocation;
    current_details->flags &= ~kRmtAllocationDetailIsCpuMapped;
    current_details->last_cpu_un_map = timestamp;
    current_details->map_count--;

    return kRmtOk;
}

RmtErrorCode RmtVirtualAllocationListGetAllocationForAddress(const RmtVirtualAllocationList* virtual_allocation_list,
                                                             RmtGpuAddress                   address,
                                                             const RmtVirtualAllocation**    out_allocation)
{
    RMT_ASSERT(virtual_allocation_list);
    RMT_ASSERT(out_allocation);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_allocation, kRmtErrorInvalidPointer);

    // find the allocation interval.
    const RmtGpuAddress           hashed_address = HashGpuAddress(address);
    RmtVirtualAllocationInterval* interval       = FindAllocationIntervalByAddress(virtual_allocation_list, hashed_address);
    if (interval == nullptr)
    {
        return kRmtErrorNoAllocationFound;
    }

    // store the residency update on the details structure.
    RmtVirtualAllocation* current_details = interval->allocation;
    *out_allocation                       = current_details;
    return kRmtOk;
}

uint64_t RmtVirtualAllocationListGetTotalSizeInBytes(const RmtVirtualAllocationList* virtual_allocation_list)
{
    RMT_ASSERT(virtual_allocation_list);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, 0);

    uint64_t total_size_in_bytes = 0;

    for (int32_t current_allocation_index = 0; current_allocation_index < virtual_allocation_list->allocation_count; ++current_allocation_index)
    {
        const RmtVirtualAllocation* current_virtual_allocation = &virtual_allocation_list->allocation_details[current_allocation_index];
        total_size_in_bytes += RmtVirtualAllocationGetSizeInBytes(current_virtual_allocation);
    }

    return total_size_in_bytes;
}

uint64_t RmtVirtualAllocationListGetBoundTotalSizeInBytes(const RmtDataSnapshot* snapshot, const RmtVirtualAllocationList* virtual_allocation_list)
{
    uint64_t total_bound_size = 0;

    for (int32_t current_allocation_index = 0; current_allocation_index < virtual_allocation_list->allocation_count; ++current_allocation_index)
    {
        const RmtVirtualAllocation* current_virtual_allocation = &virtual_allocation_list->allocation_details[current_allocation_index];
        total_bound_size += RmtVirtualAllocationGetTotalResourceMemoryInBytes(snapshot, current_virtual_allocation);
    }

    return total_bound_size;
}

uint64_t RmtVirtualAllocationListGetUnboundTotalSizeInBytes(const RmtDataSnapshot* snapshot, const RmtVirtualAllocationList* virtual_allocation_list)
{
    uint64_t total_unbound_size = 0;

    for (int32_t current_allocation_index = 0; current_allocation_index < virtual_allocation_list->allocation_count; ++current_allocation_index)
    {
        const RmtVirtualAllocation* current_virtual_allocation = &virtual_allocation_list->allocation_details[current_allocation_index];
        total_unbound_size += RmtVirtualAllocationGetTotalUnboundSpaceInAllocation(snapshot, current_virtual_allocation);
    }

    return total_unbound_size;
}

static void DropDeadAllocations(RmtVirtualAllocationList* virtual_allocation_list)
{
    RMT_ASSERT(virtual_allocation_list);

    // drop any allocations from the end of the list, until we spot the first non-dead allocation
    for (int32_t current_virtual_allocation_index = virtual_allocation_list->allocation_count - 1; current_virtual_allocation_index >= 0;
         --current_virtual_allocation_index)
    {
        RmtVirtualAllocation* current_virtual_allocation = &virtual_allocation_list->allocation_details[current_virtual_allocation_index];
        if ((current_virtual_allocation->flags & kRmtAllocationDetailIsDead) != kRmtAllocationDetailIsDead)
        {
            break;
        }
        virtual_allocation_list->allocation_count--;
    }
}

RmtErrorCode RmtVirtualAllocationListCompact(RmtVirtualAllocationList* virtual_allocation_list, bool fixup_resources)
{
    RMT_ASSERT(virtual_allocation_list);

    DropDeadAllocations(virtual_allocation_list);

    // Now perform compaction.
    for (int32_t current_virtual_allocation_index = 0; current_virtual_allocation_index < virtual_allocation_list->allocation_count;
         ++current_virtual_allocation_index)
    {
        RmtVirtualAllocation* current_virtual_allocation = &virtual_allocation_list->allocation_details[current_virtual_allocation_index];

        // If the allocation is alive, then we can leave it be.
        if ((current_virtual_allocation->flags & kRmtAllocationDetailIsDead) != kRmtAllocationDetailIsDead)
        {
            continue;
        }

        // Copy the allocation from the end of the list into this slot and then fix up
        // the boundAllocation pointers on each resource to point at the new location.
        DropDeadAllocations(virtual_allocation_list);

        const int32_t last_virtual_allocation_index = virtual_allocation_list->allocation_count - 1;

        // If the current allocation index is at or past the end of the list, then we're done.
        if (current_virtual_allocation_index >= last_virtual_allocation_index)
        {
            continue;
        }

        // Otherwise do a full copy and fix it all up.
        const RmtVirtualAllocation* last_virtual_allocation = &virtual_allocation_list->allocation_details[last_virtual_allocation_index];

        // Get the original guid before copying so it can be fixed up.
        int32_t guid = current_virtual_allocation->guid;
        memcpy(current_virtual_allocation, last_virtual_allocation, sizeof(RmtVirtualAllocation));

        // Fix up guid.
        current_virtual_allocation->guid = guid;

        RMT_ASSERT((current_virtual_allocation->flags & kRmtAllocationDetailIsDead) != kRmtAllocationDetailIsDead);

        if (fixup_resources)
        {
            for (int32_t current_resource_index = 0; current_resource_index < current_virtual_allocation->resource_count; ++current_resource_index)
            {
                RmtResource* current_resource      = current_virtual_allocation->resources[current_resource_index];
                current_resource->bound_allocation = current_virtual_allocation;
            }
        }

        virtual_allocation_list->allocation_count--;
    }

    return kRmtOk;
}

RmtErrorCode RmtVirtualAllocationGetBackingStorageHistogram(const RmtDataSnapshot*      snapshot,
                                                            const RmtVirtualAllocation* virtual_allocation,
                                                            uint64_t*                   out_bytes_per_backing_storage_type,
                                                            uint64_t*                   out_histogram_total)
{
    RMT_RETURN_ON_ERROR(virtual_allocation, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_bytes_per_backing_storage_type, kRmtErrorInvalidPointer)
    RMT_RETURN_ON_ERROR(out_histogram_total, kRmtErrorInvalidPointer);

    const uint64_t size_of_minimum_page = RmtGetPageSize(kRmtPageSize4Kb);

    const uint64_t size_in_bytes = RmtGetAllocationSizeInBytes(virtual_allocation->size_in_4kb_page, kRmtPageSize4Kb);

    // stride through the resource in 4KB pages and figure out the mapping of each.
    RmtGpuAddress       current_virtual_address = virtual_allocation->base_address;
    const RmtGpuAddress end_virtual_address     = virtual_allocation->base_address + size_in_bytes;

    // add all the resource into unmapped category initially.
    out_bytes_per_backing_storage_type[kRmtHeapTypeInvisible]              = 0;
    out_bytes_per_backing_storage_type[kRmtHeapTypeLocal]                  = 0;
    out_bytes_per_backing_storage_type[kRmtHeapTypeSystem]                 = 0;
    out_bytes_per_backing_storage_type[kRmtResourceBackingStorageUnmapped] = size_in_bytes;
    *out_histogram_total                                                   = size_in_bytes;

    while (current_virtual_address < end_virtual_address)
    {
        // Handle edge case where last page isn't 4KB in size.
        const uint64_t size = RMT_MINIMUM(end_virtual_address - current_virtual_address, size_of_minimum_page);

        // get the physical address
        RmtGpuAddress      physical_address = 0;
        const RmtErrorCode error_code = RmtPageTableGetPhysicalAddressForVirtualAddress(&snapshot->page_table, current_virtual_address, &physical_address);

        if (error_code == kRmtOk)
        {
            // remove bytes from unmapped count.
            if (size <= out_bytes_per_backing_storage_type[kRmtResourceBackingStorageUnmapped])
            {
                out_bytes_per_backing_storage_type[kRmtResourceBackingStorageUnmapped] -= size;
            }

            if (physical_address == 0)
            {
                out_bytes_per_backing_storage_type[kRmtHeapTypeSystem] += size;
            }
            else
            {
                const RmtHeapType segment_type = RmtDataSnapshotGetSegmentForAddress(snapshot, physical_address);
                if (segment_type != kRmtHeapTypeUnknown)
                {
                    out_bytes_per_backing_storage_type[segment_type] += size;
                }
            }
        }

        current_virtual_address += size;
    }

    return kRmtOk;
}

/// @Brief Update the resource's size after aliasing (adjusted_size_in_bytes).
///
/// Builds an interval tree to keep track of resource segments with the highest priority when compared against other resources in a virtual allocation.
/// The interval tree is used to calculate the adjusted size of resources taking into account overlaps.
///
/// Resources are first sorted by priority and then inserted into the interval tree.  Any portion of a resource that overlaps with an interval already
/// in the tree is excluded (since it has a lower priority than the already existing intervals).  As intervals are inserted, the resource's adjusted size
/// is increased by the size of the interval being inserted.
///
/// Interval ranges include the start offset and exclude the end offset.
///
/// @param [in] allocation                             A pointer to a <c><i>RmtVirtualAllocation</i></c> structure.
/// @param [in] resource_list                          A pointer to a <c><i>RmtResourceList</i></c> structure.
/// @param [in] resource_usage_mask                    A bit mask of all usage types that should be included when calculating alias size.
/// @param [in] sorted_resources                       A vector used to store resources sorted by priority.  The caller can reserve the expected max size.
/// @param [in] interval_overlaps                      A vector used to store intervals that overlap with the new resource.  The caller can reserve the expected max size.
///
/// @retval
/// kRmtOk                                  The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                 The operation failed because <c><i>virtual_allocation_list</i></c> or <c><i>resource_list</i></c> was <c><i>NULL</i></c>.
static RmtErrorCode AdjustAliasedResourceSizesForAllocation(const RmtVirtualAllocation*               allocation,
                                                            const RmtResourceList*                    resource_list,
                                                            uint64_t                                  resource_usage_mask,
                                                            std::vector<RmtResource*>&                sorted_resources,
                                                            std::vector<AliasedResourceIntervalType>& interval_overlaps)
{
    RMT_RETURN_ON_ERROR(allocation != nullptr, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(resource_list != nullptr, kRmtErrorInvalidPointer);

    // Add resources to a list to be sorted.  Filter the resources not included by the usage mask.  Initialize the resource's adjusted size after aliasing to zero.
    sorted_resources.clear();
    for (int i = 0; i < allocation->resource_count; i++)
    {
        RmtResource* resource = allocation->resources[i];
        RMT_ASSERT(resource != nullptr);

        // Special case handling for heaps.  Heaps aren't adjusted for aliased resources.
        if (resource->resource_type == RmtResourceType::kRmtResourceTypeHeap)
        {
            resource->adjusted_size_in_bytes = resource->size_in_bytes;
            continue;
        }

        // Reset the alias size.
        resource->adjusted_size_in_bytes = 0;
        resource->is_aliased             = false;

        // Skip this resource if it is disabled by the usage filter.
        const RmtResourceUsageType usage_type = RmtResourceGetUsageType(resource);
        if (!(RmtResourceGetUsageTypeMask(usage_type) & resource_usage_mask))
        {
            continue;
        }

        sorted_resources.push_back(resource);
    }

    // Sort the resources by priority.  This guarantees that resources with the highest priority are processed first.
    // It also guarantees that once intervals are added to the interval tree there will be no overlaps (intervals are trimmed when inserted).
    std::sort(sorted_resources.begin(), sorted_resources.end(), ResourcePriorityComparator());

    AliasedResourceIntervalTreeType interval_tree;
    bool                            interval_inserted = false;  // The processing of overlapped resources can be skip if this flag is false.

    // Process each resource and add intervals to the interval tree for the parts that don't overlap.
    for (auto& new_resource : sorted_resources)
    {
        const uint64_t new_resource_start          = new_resource->address - allocation->base_address;
        const uint64_t new_resource_end            = new_resource_start + new_resource->size_in_bytes;
        uint64_t       new_interval_start          = new_resource_start;
        uint64_t       new_interval_end            = new_resource_end;
        uint64_t       new_resource_remaining_size = new_resource->size_in_bytes;

        // Skip this block if there are no intervals in the tree yet.  In this case, a new interval can be added for the entire resource (i.e. no overlaps).
        if (interval_inserted)
        {
            // Get a list of existing intervals in the interval tree that overlap with the new resource.
            interval_overlaps.clear();

            // Create an interval to search for overlaps.  Make the interval exclusive (i.e. exclude the start and end values).
            // The interval tree's FindOverlappingIntervals() method expects the interval parameter specified to be an inclusive
            // interval, meaning the start and end points are included when detecting overlaps.  The start and end values should
            // not be considered so that resources aren't incorrectly marked as being aliased.
            const AliasedResourceIntervalType search_interval = {new_resource_start + 1LL, new_resource_end - 1LL, nullptr};
            interval_tree.FindOverlappingIntervals(search_interval, interval_overlaps);

            // Sort the overlapping intervals by range start value.
            std::sort(interval_overlaps.begin(), interval_overlaps.end(), [](const AliasedResourceIntervalType& lhs, const AliasedResourceIntervalType& rhs) {
                return lhs.start < rhs.start;
            });

            for (const auto& overlap_interval : interval_overlaps)
            {
                // This new resource overlaps with a resource in the interval tree.  Mark both as aliased.
                overlap_interval.index->is_aliased     = true;
                new_resource->is_aliased               = true;
                const uint64_t existing_interval_start = overlap_interval.start;
                const uint64_t existing_interval_end   = overlap_interval.end;

                if (existing_interval_start <= new_resource_start && existing_interval_end > new_resource_end)
                {
                    // The existing interval completely overlaps with all of the new resource.
                    new_resource_remaining_size = 0;
                    break;
                }

                if (existing_interval_start <= new_interval_start)
                {
                    // Trim the start of the new interval.
                    const uint64_t trim_amount = std::min(new_interval_end, existing_interval_end) - new_interval_start;
                    new_interval_start += trim_amount;
                    new_resource_remaining_size -= trim_amount;
                    RMT_ASSERT(static_cast<int64_t>(new_resource_remaining_size) >= 0);
                }
                else
                {
                    // Trim the end of the new interval.
                    if (new_interval_end > existing_interval_end)
                    {
                        new_interval_end = existing_interval_end;
                    }

                    // Calculate the amount to trim from the new interval end.
                    const uint64_t trim_amount = new_interval_end - existing_interval_start;
                    new_interval_end -= trim_amount;
                    new_resource_remaining_size -= trim_amount;
                    RMT_ASSERT(static_cast<int64_t>(new_resource_remaining_size) >= 0);

                    // Add an interval for the section of the resource that doesn't overlap with the existing interval.
                    const AliasedResourceIntervalType new_interval = {new_interval_start, new_interval_end, new_resource};
                    RMT_ASSERT(new_interval.end > new_interval.start);
                    interval_tree.Insert(new_interval);

                    // Increase the adjusted size of the resource.
                    const uint64_t interval_size = new_interval_end - new_interval_start;
                    new_resource->adjusted_size_in_bytes += interval_size;

                    new_resource_remaining_size -= interval_size;
                    RMT_ASSERT(static_cast<int64_t>(new_resource_remaining_size) >= 0);

                    // Prepare for the next interval.  Add one to the new start interval so that it points to the offset immediately following the previous existing interval.
                    new_interval_start = existing_interval_end + 1;
                    new_interval_end   = new_resource_end;
                }
            }
        }

        // Add an interval for the remaining section of the resource that doesn't overlap with any other existing intervals.
        if (new_resource_remaining_size > 0)
        {
            // Calculate the end offset and subtract one so that the end point is excluded for the new interval.
            new_interval_end                               = (new_interval_start + new_resource_remaining_size) - 1;
            const AliasedResourceIntervalType new_interval = {new_interval_start, new_interval_end, new_resource};
            interval_tree.Insert(new_interval);

            // Set the flag that indicates subsequent resources that are processed need to be checked for overlaps in the interval tree.
            interval_inserted = true;

            // Increase the adjusted size of the resource.
            new_resource->adjusted_size_in_bytes += new_resource_remaining_size;
        }
    }

    return kRmtOk;
}

RmtErrorCode RmtVirtualAllocationListUpdateAliasedResourceSizes(const RmtVirtualAllocationList* allocation_list,
                                                                const RmtResourceList*          resource_list,
                                                                const uint64_t                  resource_usage_mask)
{
    RMT_RETURN_ON_ERROR(allocation_list != nullptr, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(resource_list != nullptr, kRmtErrorInvalidPointer);

    RmtErrorCode result = kRmtOk;

    // Find the maximum number of resources in any of the virtual allocations.
    int max_resource = 0;
    for (int index = 0; index < allocation_list->allocation_count; index++)
    {
        // Update the maximum number of resources found.
        max_resource = RMT_MAXIMUM(max_resource, allocation_list->allocation_details[index].resource_count);
    }

    if (max_resource == 0)
    {
        // There are no resources bound to any of the virtual allocation so just exit.
        return kRmtOk;
    }

    // Temporary list of resource pointers that get sorted by start address.
    std::vector<RmtResource*> sorted_resources;

    // Temporary list of resource intervals that overlap with a resource.
    std::vector<AliasedResourceIntervalType> interval_overlaps;

    // Reserve space up front for the temporary lists.
    sorted_resources.reserve(max_resource);
    interval_overlaps.reserve(max_resource - 1);

    // Iterate over each of the virtual allocations and calculate resource sizes taking into account aliasing.
    for (int32_t index = 0; index < allocation_list->allocation_count; index++)
    {
        result = AdjustAliasedResourceSizesForAllocation(
            &allocation_list->allocation_details[index], resource_list, resource_usage_mask, sorted_resources, interval_overlaps);
        if (result != kRmtOk)
        {
            break;
        }
    }

    return result;
}
