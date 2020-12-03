//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author
/// \brief Implementation of the virtual allocation list functions.
//=============================================================================

#include <math.h>

#include "rmt_virtual_allocation_list.h"
#include "rmt_resource_list.h"
#include "rmt_page_table.h"
#include "rmt_address_helper.h"
#include "rmt_data_snapshot.h"
#include <rmt_assert.h>
#include <string.h>  // memcpy

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
        new_node->base_address                 = gpu_address;
        new_node->size_in_4kb_pages            = size_in_pages;
        new_node->dead                         = 0;
        new_node->allocation                   = allocation;
        new_node->left                         = NULL;
        new_node->right                        = NULL;
        return new_node;
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
        RMT_ASSERT_FAIL("WTF");
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
    return RMT_OK;
}

// destroy a resource from the acceleration structure.
static RmtErrorCode RemoveAllocationFromTree(RmtVirtualAllocationList* virtual_allocation_list, RmtGpuAddress gpu_address)
{
    const size_t pool_count       = virtual_allocation_list->allocation_interval_pool.allocated;
    virtual_allocation_list->root = DeleteNode(virtual_allocation_list, virtual_allocation_list->root, gpu_address);
    RMT_ASSERT(virtual_allocation_list->allocation_interval_pool.allocated == pool_count - 1);
    return RMT_OK;
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
            region_stack[current_region_stack_top - 1].offset = current_resource_offset;
            region_stack[current_region_stack_top - 1].size   = new_size;
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
    if (virtual_allocation == NULL || virtual_allocation->resource_count == 0)
    {
        return 0;
    }

    const uint64_t total_resource_size = RmtVirtualAllocationGetTotalResourceMemoryInBytes(snapshot, virtual_allocation);
    return total_resource_size / virtual_allocation->resource_count;
}

uint64_t RmtVirtualAllocationGetResourceStandardDeviationInBytes(const RmtDataSnapshot* snapshot, const RmtVirtualAllocation* virtual_allocation)
{
    RMT_ASSERT(virtual_allocation);
    if (virtual_allocation == NULL || virtual_allocation->resource_count == 0)
    {
        return 0;
    }

    uint64_t variance          = 0;
    int64_t  avg_resource_size = RmtVirtualAllocationGetAverageResourceSizeInBytes(snapshot, virtual_allocation);
    for (int32_t current_resource_index = 0; current_resource_index < virtual_allocation->resource_count; ++current_resource_index)
    {
        const int64_t diff = virtual_allocation->resources[current_resource_index]->size_in_bytes - avg_resource_size;
        variance           = diff * diff;
    }
    variance /= virtual_allocation->resource_count;

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
        last_address = current_resource->bind_time + current_resource->size_in_bytes;
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
    RMT_RETURN_ON_ERROR(virtual_allocation_list, RMT_ERROR_INVALID_POINTER);
    RMT_RETURN_ON_ERROR(buffer, RMT_ERROR_INVALID_POINTER);
    RMT_RETURN_ON_ERROR(buffer_size, RMT_ERROR_INVALID_SIZE);
    RMT_RETURN_ON_ERROR(RmtVirtualAllocationListGetBufferSize(total_allocations, maximum_concurrent_resources) <= buffer_size, RMT_ERROR_INVALID_SIZE);

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
    const size_t resource_connectivity_size                 = maximum_concurrent_resources * sizeof(RmtResource*);
    virtual_allocation_list->unbound_memory_regions         = (RmtMemoryRegion*)((uintptr_t)buffer + allocation_details_size + resource_connectivity_size);
    virtual_allocation_list->unbound_memory_region_count    = maximum_concurrent_resources + 1;

    // initialize interval pool.
    RmtPoolInitialize(&virtual_allocation_list->allocation_interval_pool,
                      virtual_allocation_list->allocation_intervals,
                      interval_size_in_bytes,
                      sizeof(RmtVirtualAllocationInterval));
    virtual_allocation_list->root = NULL;

    memset(virtual_allocation_list->allocations_per_preferred_heap, 0, sizeof(virtual_allocation_list->allocations_per_preferred_heap));
    return RMT_OK;
}

RmtErrorCode RmtVirtualAllocationListAddAllocation(RmtVirtualAllocationList* virtual_allocation_list,
                                                   uint64_t                  timestamp,
                                                   RmtGpuAddress             address,
                                                   int32_t                   size_in_4kb_pages,
                                                   const RmtHeapType         preferences[4],
                                                   RmtOwnerType              owner)
{
    RMT_ASSERT(virtual_allocation_list);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, RMT_ERROR_INVALID_POINTER);
    RMT_RETURN_ON_ERROR(size_in_4kb_pages, RMT_ERROR_INVALID_SIZE);
    RMT_RETURN_ON_ERROR((address >> 12) + size_in_4kb_pages < RMT_PAGE_TABLE_MAX_SIZE, RMT_ERROR_INVALID_SIZE);
    RMT_RETURN_ON_ERROR(virtual_allocation_list->allocation_count < virtual_allocation_list->total_allocations, RMT_ERROR_OUT_OF_MEMORY);

    // check if this region overlaps an existing region
    const RmtVirtualAllocation* found_allocation = NULL;
    const RmtErrorCode          error_code       = RmtVirtualAllocationListGetAllocationForAddress(virtual_allocation_list, address, &found_allocation);
    if (error_code == RMT_OK)
    {
        RMT_ASSERT((found_allocation->flags & kRmtAllocationDetailIsDead) != kRmtAllocationDetailIsDead);
    }
    RMT_RETURN_ON_ERROR(error_code != RMT_OK, error_code);

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

    for (int32_t current_heap_preference_index = 0; current_heap_preference_index < 4; ++current_heap_preference_index)
    {
        allocation_details->heap_preferences[current_heap_preference_index] = preferences[current_heap_preference_index];
    }

    // fill out the allocation interval
    const RmtGpuAddress hashed_address = HashGpuAddress(address);
    AddAllocationToTree(virtual_allocation_list, hashed_address, size_in_4kb_pages, allocation_details);

    const uint64_t size_in_bytes = (size_in_4kb_pages << 12);
    virtual_allocation_list->total_allocated_bytes += size_in_bytes;
    virtual_allocation_list->allocations_per_preferred_heap[allocation_details->heap_preferences[0]] += size_in_bytes;
    return RMT_OK;
}

RmtErrorCode RmtVirtualAllocationListRemoveAllocation(RmtVirtualAllocationList* virtual_allocation_list, RmtGpuAddress address)
{
    RMT_ASSERT(virtual_allocation_list);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, RMT_ERROR_INVALID_POINTER);
    RMT_RETURN_ON_ERROR(virtual_allocation_list->allocation_count, RMT_ERROR_NO_ALLOCATION_FOUND);

    const RmtGpuAddress           hashed_address              = HashGpuAddress(address);
    RmtVirtualAllocationInterval* current_allocation_interval = FindAllocationIntervalByAddress(virtual_allocation_list, hashed_address);
    if (current_allocation_interval == nullptr)
    {
        return RMT_ERROR_NO_ALLOCATION_FOUND;
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

    const uint64_t size_in_bytes = current_allocation_interval->allocation->size_in_4kb_page << 12;
    virtual_allocation_list->total_allocated_bytes -= size_in_bytes;
    virtual_allocation_list->allocations_per_preferred_heap[current_allocation_interval->allocation->heap_preferences[0]] -= size_in_bytes;

    // remove efrom the tree
    RemoveAllocationFromTree(virtual_allocation_list, hashed_address);

    return RMT_OK;
}

RmtErrorCode RmtVirtualAllocationListAddResourceReference(RmtVirtualAllocationList* virtual_allocation_list,
                                                          uint64_t                  timestamp,
                                                          RmtGpuAddress             address,
                                                          RmtResidencyUpdateType    update_type,
                                                          RmtQueue                  queue)
{
    RMT_UNUSED(queue);

    RMT_ASSERT(virtual_allocation_list);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, RMT_ERROR_INVALID_POINTER);
    RMT_RETURN_ON_ERROR(virtual_allocation_list->allocation_count, RMT_ERROR_NO_ALLOCATION_FOUND);

    // find the allocation index.
    const RmtGpuAddress           hashed_address = HashGpuAddress(address);
    RmtVirtualAllocationInterval* interval       = FindAllocationIntervalByAddress(virtual_allocation_list, hashed_address);
    if (interval == nullptr)
    {
        return RMT_ERROR_NO_ALLOCATION_FOUND;
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

    return RMT_OK;
}

RmtErrorCode RmtVirtualAllocationListAddCpuMap(RmtVirtualAllocationList* virtual_allocation_list, uint64_t timestamp, RmtGpuAddress address)
{
    RMT_ASSERT(virtual_allocation_list);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, RMT_ERROR_INVALID_POINTER);
    RMT_RETURN_ON_ERROR(virtual_allocation_list->allocation_count, RMT_ERROR_NO_ALLOCATION_FOUND);

    // find the allocation index.
    const RmtGpuAddress           hashed_address = HashGpuAddress(address);
    RmtVirtualAllocationInterval* interval       = FindAllocationIntervalByAddress(virtual_allocation_list, hashed_address);
    if (interval == nullptr)
    {
        return RMT_ERROR_NO_ALLOCATION_FOUND;
    }

    // store the residency update on the details structure.
    RmtVirtualAllocation* current_details = interval->allocation;
    current_details->flags |= kRmtAllocationDetailIsCpuMapped;
    current_details->flags |= kRmtAllocationDetailHasBeenCpuMapped;
    current_details->last_cpu_map = timestamp;
    current_details->map_count++;

    return RMT_OK;
}

RmtErrorCode RmtVirtualAllocationListAddCpuUnmap(RmtVirtualAllocationList* virtual_allocation_list, uint64_t timestamp, RmtGpuAddress address)
{
    RMT_ASSERT(virtual_allocation_list);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, RMT_ERROR_INVALID_POINTER);
    RMT_RETURN_ON_ERROR(virtual_allocation_list->allocation_count, RMT_ERROR_NO_ALLOCATION_FOUND);

    // find the allocation index.
    const RmtGpuAddress           hashed_address = HashGpuAddress(address);
    RmtVirtualAllocationInterval* interval       = FindAllocationIntervalByAddress(virtual_allocation_list, hashed_address);
    if (interval == nullptr)
    {
        return RMT_ERROR_NO_ALLOCATION_FOUND;
    }

    // store the residency update on the details structure.
    RmtVirtualAllocation* current_details = interval->allocation;
    current_details->flags &= ~kRmtAllocationDetailIsCpuMapped;
    current_details->last_cpu_un_map = timestamp;
    current_details->map_count--;

    return RMT_OK;
}

RmtErrorCode RmtVirtualAllocationListGetAllocationForAddress(const RmtVirtualAllocationList* virtual_allocation_list,
                                                             RmtGpuAddress                   address,
                                                             const RmtVirtualAllocation**    out_allocation)
{
    RMT_ASSERT(virtual_allocation_list);
    RMT_ASSERT(out_allocation);
    RMT_RETURN_ON_ERROR(virtual_allocation_list, RMT_ERROR_INVALID_POINTER);
    RMT_RETURN_ON_ERROR(out_allocation, RMT_ERROR_INVALID_POINTER);

    // find the allocation interval.
    const RmtGpuAddress           hashed_address = HashGpuAddress(address);
    RmtVirtualAllocationInterval* interval       = FindAllocationIntervalByAddress(virtual_allocation_list, hashed_address);
    if (interval == nullptr)
    {
        return RMT_ERROR_NO_ALLOCATION_FOUND;
    }

    // store the residency update on the details structure.
    RmtVirtualAllocation* current_details = interval->allocation;
    *out_allocation                       = current_details;
    return RMT_OK;
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

    // now perform compaction.
    for (int32_t current_virtual_allocation_index = 0; current_virtual_allocation_index < virtual_allocation_list->allocation_count;
         ++current_virtual_allocation_index)
    {
        RmtVirtualAllocation* current_virtual_allocation = &virtual_allocation_list->allocation_details[current_virtual_allocation_index];

        // if the allocation is alive, then we can leave it be.
        if ((current_virtual_allocation->flags & kRmtAllocationDetailIsDead) != kRmtAllocationDetailIsDead)
        {
            continue;
        }

        // copy the allocation from the end of the list into this slot and then fix up
        // the boundAllocation pointers on each resource to point at the new location.
        DropDeadAllocations(virtual_allocation_list);
        const int32_t last_virtual_allocation_index = virtual_allocation_list->allocation_count - 1;

        // special case for deleting the last element of the list.
        if (current_virtual_allocation_index == last_virtual_allocation_index)
        {
            virtual_allocation_list->allocation_count--;
            continue;
        }

        // otherwise do a full copy and fix it all up.
        const RmtVirtualAllocation* last_virtual_allocation = &virtual_allocation_list->allocation_details[last_virtual_allocation_index];

        // get the original guid before copying so it can be fixed up
        int32_t guid = current_virtual_allocation->guid;
        memcpy(current_virtual_allocation, last_virtual_allocation, sizeof(RmtVirtualAllocation));

        // fix up guid
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

    return RMT_OK;
}

RmtErrorCode RmtVirtualAllocationGetBackingStorageHistogram(const RmtDataSnapshot*      snapshot,
                                                            const RmtVirtualAllocation* virtual_allocation,
                                                            uint64_t*                   out_bytes_per_backing_storage_type,
                                                            uint64_t*                   out_histogram_total)
{
    RMT_RETURN_ON_ERROR(virtual_allocation, RMT_ERROR_INVALID_POINTER);
    RMT_RETURN_ON_ERROR(out_bytes_per_backing_storage_type, RMT_ERROR_INVALID_POINTER)
    RMT_RETURN_ON_ERROR(out_histogram_total, RMT_ERROR_INVALID_POINTER);

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

        if (error_code == RMT_OK)
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

    return RMT_OK;
}
