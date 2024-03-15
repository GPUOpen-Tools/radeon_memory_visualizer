//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the resource list functions.
//=============================================================================

#include "rmt_resource_list.h"

#include "rmt_memory_aliasing_timeline.h"
#include "rmt_virtual_allocation_list.h"
#include "rmt_assert.h"
#include "rmt_page_table.h"
#include "rmt_data_snapshot.h"
#include "rmt_address_helper.h"
#include "rmt_print.h"
#include "rmt_resource_userdata.h"

#include <string.h>  // for memcpy()

#ifndef _WIN32
#include "linux/safe_crt.h"
#endif

using namespace RmtMemoryAliasingTimelineAlgorithm;

RmtResourceUsageType RmtResourceGetUsageType(const RmtResource* resource)
{
    RMT_ASSERT(resource);
    RMT_RETURN_ON_ERROR(resource, kRmtResourceUsageTypeUnknown);

    switch (resource->resource_type)
    {
    case kRmtResourceTypeBuffer:
        if (resource->buffer.usage_flags == kRmtBufferUsageFlagVertexBuffer)
        {
            return kRmtResourceUsageTypeVertexBuffer;
        }

        if (resource->buffer.usage_flags == kRmtBufferUsageFlagIndexBuffer)
        {
            return kRmtResourceUsageTypeIndexBuffer;
        }

        if (resource->buffer.usage_flags == kRmtBufferUsageFlagRayTracing)
        {
            return kRmtResourceUsageTypeRayTracingBuffer;
        }

        return kRmtResourceUsageTypeBuffer;
        break;

    case kRmtResourceTypeImage:
        if ((resource->image.usage_flags & kRmtImageUsageFlagsColorTarget) == kRmtImageUsageFlagsColorTarget)
        {
            return kRmtResourceUsageTypeRenderTarget;
        }
        else if ((resource->image.usage_flags & kRmtImageUsageFlagsDepthStencil) == kRmtImageUsageFlagsDepthStencil)
        {
            return kRmtResourceUsageTypeDepthStencil;
        }
        else
        {
            return kRmtResourceUsageTypeTexture;
        }
        break;

    case kRmtResourceTypePipeline:
        return kRmtResourceUsageTypeShaderPipeline;

    case kRmtResourceTypeCommandAllocator:
        return kRmtResourceUsageTypeCommandBuffer;

    case kRmtResourceTypeHeap:
        return kRmtResourceUsageTypeHeap;

    case kRmtResourceTypeDescriptorHeap:
    case kRmtResourceTypeDescriptorPool:
        return kRmtResourceUsageTypeDescriptors;

    case kRmtResourceTypeGpuEvent:
        return kRmtResourceUsageTypeGpuEvent;

    case kRmtResourceTypeBorderColorPalette:
    case kRmtResourceTypeTimestamp:
    case kRmtResourceTypeMiscInternal:

    case kRmtResourceTypePerfExperiment:
    case kRmtResourceTypeMotionEstimator:
    case kRmtResourceTypeVideoDecoder:
    case kRmtResourceTypeVideoEncoder:
    case kRmtResourceTypeQueryHeap:
    case kRmtResourceTypeIndirectCmdGenerator:
        return kRmtResourceUsageTypeInternal;

    default:
        break;
    }

    return kRmtResourceUsageTypeUnknown;
}

uint64_t RmtResourceGetOffsetFromBoundAllocation(const RmtResource* resource)
{
    RMT_ASSERT(resource);
    RMT_RETURN_ON_ERROR(resource, 0);

    if (resource->bound_allocation != nullptr)
    {
        const RmtGpuAddress allocation_base_address = resource->bound_allocation->base_address;
        const RmtGpuAddress resource_base_address   = resource->address;
        return resource_base_address - allocation_base_address;
    }

    return 0;
}

RmtGpuAddress RmtResourceGetVirtualAddress(const RmtResource* resource)
{
    RMT_ASSERT(resource);
    RMT_RETURN_ON_ERROR(resource, 0);

    return resource->address;
}

bool RmtResourceOverlapsVirtualAddressRange(const RmtResource* resource, RmtGpuAddress address_start, RmtGpuAddress address_end)
{
    RMT_RETURN_ON_ERROR(resource, false);

    if (resource->bound_allocation == nullptr)
    {
        return false;
    }

    // Case 1: if the resource starts after the end of the range then it can't overlap
    //
    //  |--virtual address range--|
    //                                 |--resource--|
    if (resource->address > address_end)
    {
        return false;
    }

    // Case 2: if the resource ends before the start of the range then it can't overlap
    //
    //  |--resource--|
    //                 |--virtual address range--|
    const RmtGpuAddress resource_address_end = resource->address + resource->size_in_bytes - 1;
    if (resource_address_end < address_start)
    {
        return false;
    }

    // anything else must overlap.
    return true;
}

RmtErrorCode RmtResourceGetBackingStorageHistogram(const RmtDataSnapshot* snapshot, const RmtResource* resource, uint64_t* out_bytes_per_backing_storage_type)
{
    RMT_RETURN_ON_ERROR(resource, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_bytes_per_backing_storage_type, kRmtErrorInvalidPointer)

    const uint64_t size_of_minimum_page = RmtGetPageSize(kRmtPageSize4Kb);

    // stride through the resource in 4KB pages and figure out the mapping of each.
    RmtGpuAddress       current_virtual_address = resource->address;
    const RmtGpuAddress end_virtual_address     = resource->address + resource->size_in_bytes;

    // add all the resource into unmapped category initially.
    out_bytes_per_backing_storage_type[kRmtResourceBackingStorageUnmapped] = resource->size_in_bytes;

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

bool RmtResourceIsCompletelyInPreferredHeap(const RmtDataSnapshot* snapshot, const RmtResource* resource)
{
    RMT_RETURN_ON_ERROR(resource, false);
    RMT_RETURN_ON_ERROR(resource->bound_allocation, false);

    const uint64_t    size_of_minimum_page = RmtGetPageSize(kRmtPageSize4Kb);
    const RmtHeapType preferred_heap       = resource->bound_allocation->heap_preferences[0];

    // stride through the resource in 4KB pages and figure out the mapping of each.
    RmtGpuAddress       current_virtual_address = resource->address;
    const RmtGpuAddress end_virtual_address     = resource->address + resource->size_in_bytes;
    while (current_virtual_address < end_virtual_address)
    {
        // Handle edge case where last page isn't 4KB in size.
        const uint64_t size = RMT_MINIMUM(end_virtual_address - current_virtual_address, size_of_minimum_page);

        // get the physical address
        RmtGpuAddress      physical_address = 0;
        const RmtErrorCode error_code = RmtPageTableGetPhysicalAddressForVirtualAddress(&snapshot->page_table, current_virtual_address, &physical_address);
        if (error_code != kRmtOk)
        {
            return false;
        }

        if (physical_address == 0)
        {
            if (preferred_heap != kRmtHeapTypeSystem)
            {
                return false;
            }
        }
        else
        {
            const RmtHeapType segment_type = RmtDataSnapshotGetSegmentForAddress(snapshot, physical_address);
            if (segment_type != preferred_heap)
            {
                return false;
            }
        }

        current_virtual_address += size;
    }

    return true;
}

const char* RmtResourceGetHeapTypeName(const RmtResource* resource)
{
    RMT_ASSERT(resource);
    if (resource->bound_allocation != nullptr)
    {
        return RmtGetHeapTypeNameFromHeapType(resource->bound_allocation->heap_preferences[0]);
    }
    if ((resource->flags & kRmtResourceFlagDangling) == kRmtResourceFlagDangling)
    {
        return "Orphaned";
    }

    return "-";
}

bool RmtResourceGetName(const RmtResource* resource, int32_t buffer_size, char** out_resource_name)
{
    RMT_ASSERT(resource);
    RMT_ASSERT(out_resource_name);
    RMT_ASSERT(*out_resource_name);

    if (buffer_size < RMT_MAXIMUM_NAME_LENGTH)
    {
        return false;
    }

    if (resource->name != nullptr)
    {
        strcpy_s(*out_resource_name, buffer_size, resource->name);
    }
    else
    {
        sprintf_s(*out_resource_name, buffer_size, "Resource %llu", resource->identifier);
    }

    return true;
}

RmtHeapType RmtResourceGetActualHeap(const RmtDataSnapshot* snapshot, const RmtResource* resource)
{
    RMT_RETURN_ON_ERROR(snapshot, kRmtHeapTypeUnknown);
    RMT_RETURN_ON_ERROR(resource, kRmtHeapTypeUnknown);

    if (resource->bound_allocation != nullptr)
    {
        return resource->bound_allocation->heap_preferences[0];
    }

    return kRmtHeapTypeUnknown;
}

int32_t RmtResourceGetAliasCount(const RmtResource* resource)
{
    RMT_RETURN_ON_ERROR(resource, 0);
    RMT_RETURN_ON_ERROR(resource->bound_allocation, 0);
    RMT_RETURN_ON_ERROR(resource->resource_type != kRmtResourceTypeHeap, 0);

    const RmtVirtualAllocation* virtual_allocation     = resource->bound_allocation;
    const RmtGpuAddress         resource_start_address = resource->address;
    const RmtGpuAddress         resource_end_address   = resource->address + resource->size_in_bytes;

    int32_t total_resource_alias_count = 0;
    for (int32_t current_resource_index = 0; current_resource_index < virtual_allocation->resource_count; ++current_resource_index)
    {
        const RmtResource* current_resource = virtual_allocation->resources[current_resource_index];

        if (current_resource == resource)
        {
            continue;
        }

        // special handle for heaps.
        if (current_resource->resource_type == kRmtResourceTypeHeap)
        {
            continue;
        }

        // get the start and end address
        const RmtGpuAddress current_resource_start = current_resource->address;
        const RmtGpuAddress current_resource_end   = current_resource->address + current_resource->size_in_bytes;
        if (resource_start_address >= current_resource_end)
        {
            continue;
        }
        if (resource_end_address <= current_resource_start)
        {
            continue;
        }

        total_resource_alias_count++;
    }

    return total_resource_alias_count;
}

// Helper function to improve tree balance by hashing the handles.
static uint64_t GenerateResourceHandle(uint64_t handle)
{
    // NOTE: this function is vestigial now, as the hashing is done in rmt_token_heap
    //       as part of the process to de-duplicate reused driver handles.
    return handle;
}

// helper function to find smallest value in a branch
static RmtResourceIdNode* GetSmallestNode(RmtResourceIdNode* node)
{
    while ((node != nullptr) && (node->left != nullptr))
    {
        node = node->left;
    }
    return node;
}

// recursive function to find a node by ID
static RmtResourceIdNode* FindResourceNode(RmtResourceIdNode* root, RmtResourceIdentifier resource_identifier)
{
    if (root == nullptr)
    {
        return NULL;
    }

    if (root->identifier == resource_identifier)
    {
        return root;
    }

    if (resource_identifier < root->identifier)
    {
        return FindResourceNode(root->left, resource_identifier);
    }

    return FindResourceNode(root->right, resource_identifier);
}

// recursive function to insert a node
static RmtResourceIdNode* InsertNode(RmtResourceList* resource_list, RmtResourceIdNode* node, RmtResourceIdentifier resource_identifier, RmtResource* resource)
{
    if (node == nullptr)
    {
        // create a new node
        RmtResourceIdNode* new_node = (RmtResourceIdNode*)RmtPoolAllocate(&resource_list->resource_id_node_pool);
        new_node->identifier        = resource_identifier;
        new_node->resource          = resource;
        new_node->left              = NULL;
        new_node->right             = NULL;

        // store pointer to the node ID so we can update it on delete.
        resource->id_node = new_node;
        return new_node;
    }

    if (resource_identifier < node->identifier)
    {
        node->left = InsertNode(resource_list, node->left, resource_identifier, resource);
    }
    else if (resource_identifier > node->identifier)
    {
        node->right = InsertNode(resource_list, node->right, resource_identifier, resource);
    }
    else
    {
        RMT_ASSERT_FAIL("WTF");
    }

    return node;
}

// recursive function to delete a node
static RmtResourceIdNode* DeleteNode(RmtResourceList* resource_list, RmtResourceIdNode* node, RmtResourceIdentifier resource_identifier)
{
    if (node == nullptr)
    {
        return node;
    }

    if (resource_identifier < node->identifier)
    {
        node->left = DeleteNode(resource_list, node->left, resource_identifier);
    }
    else if (resource_identifier > node->identifier)
    {
        node->right = DeleteNode(resource_list, node->right, resource_identifier);
    }
    else
    {
        if (node->left == nullptr)
        {
            RmtResourceIdNode* child = node->right;
            node->identifier         = 0;
            node->left               = NULL;
            node->right              = NULL;
            RmtPoolFree(&resource_list->resource_id_node_pool, node);
            return child;
        }
        if (node->right == nullptr)
        {
            RmtResourceIdNode* child = node->left;
            node->identifier         = 0;
            node->left               = NULL;
            node->right              = NULL;
            RmtPoolFree(&resource_list->resource_id_node_pool, node);
            return child;
        }

        RmtResourceIdNode* smallest_child = GetSmallestNode(node->right);
        node->identifier                  = smallest_child->identifier;

        // update payload pointers.
        RMT_ASSERT(node->resource);
        node->resource = smallest_child->resource;
        RMT_ASSERT(node->resource->id_node);
        node->resource->id_node = node;

        // now delete the node we just moved.
        node->right = DeleteNode(resource_list, node->right, smallest_child->identifier);
    }

    return node;
}

// search the acceleration structure for a resource.
static RmtResource* FindResourceById(const RmtResourceList* resource_list, RmtResourceIdentifier resource_identifier)
{
    RmtResourceIdNode* found_node = FindResourceNode(resource_list->root, resource_identifier);
    if (found_node == nullptr)
    {
        return NULL;
    }

    return found_node->resource;
}

// add a resource to the acceleration structure.
static RmtErrorCode AddResourceToTree(RmtResourceList* resource_list, RmtResourceIdentifier resource_identifier, RmtResource* resource)
{
    const size_t pool_count = resource_list->resource_id_node_pool.allocated;
    resource_list->root     = InsertNode(resource_list, resource_list->root, resource_identifier, resource);
    RMT_ASSERT(resource_list->resource_id_node_pool.allocated == pool_count + 1);
    return kRmtOk;
}

// destroy a resource from the acceleration structure.
static RmtErrorCode RemoveResourceFromTree(RmtResourceList* resource_list, RmtResourceIdentifier resource_identifier)
{
    const size_t pool_count = resource_list->resource_id_node_pool.allocated;
    resource_list->root     = DeleteNode(resource_list, resource_list->root, resource_identifier);
    RMT_ASSERT(resource_list->resource_id_node_pool.allocated == pool_count - 1);
    return kRmtOk;
}

void UpdateTotalResourceUsageAliasedSize(RmtResourceList*                                                 resource_list,
                                         RmtMemoryAliasingTimelineAlgorithm::RmtMemoryAliasingCalculator* memory_aliasing_calculator)
{
    SizePerResourceUsageType sizes_per_resource_usage_type;
    SizeType                 unbound_size;
    memory_aliasing_calculator->CalculateSizes(sizes_per_resource_usage_type, unbound_size);
    for (int usage_index = 0; usage_index < RmtResourceUsageType::kRmtResourceUsageTypeCount; usage_index++)
    {
        resource_list->total_resource_usage_aliased_size[usage_index] = sizes_per_resource_usage_type.size_[usage_index];
    }
    resource_list->total_resource_usage_aliased_size[kRmtResourceUsageTypeFree] = unbound_size;
}

// destroy a resource
static RmtErrorCode DestroyResource(RmtResourceList* resource_list, RmtResource* resource)
{
    if (resource_list->resource_count == 0)
    {
        return kRmtOk;
    }

    resource_list->resource_usage_count[RmtResourceGetUsageType(resource)]--;
    RMT_ASSERT(resource_list->resource_usage_count[RmtResourceGetUsageType(resource)] >= 0);

    // don't count shareable resources
    bool is_shareable = (resource->resource_type == kRmtResourceTypeImage) &&
                        ((resource->image.create_flags & kRmtImageCreationFlagShareable) == kRmtImageCreationFlagShareable);

    if ((!is_shareable && (resource_list->enable_aliased_resource_usage_sizes) && resource->bound_allocation != nullptr) &&
        (RmtResourceGetUsageType(resource) != RmtResourceUsageType::kRmtResourceUsageTypeHeap))
    {
        RmtMemoryAliasingCalculator* memory_aliasing_calculator = RmtMemoryAliasingCalculatorInstance();
        RMT_ASSERT(memory_aliasing_calculator != nullptr);
        Allocation* aliased_resource_allocation = memory_aliasing_calculator->FindAllocation(resource->bound_allocation->allocation_identifier);
        if (aliased_resource_allocation != nullptr)
        {
            aliased_resource_allocation->DestroyResource(
                resource->address - resource->bound_allocation->base_address, resource->size_in_bytes, RmtResourceGetUsageType(resource));
            UpdateTotalResourceUsageAliasedSize(resource_list, memory_aliasing_calculator);
        }
    }

    const int64_t hashed_identifier = GenerateResourceHandle(resource->identifier);

    // get a pointer to the last resource, if its the one we're trying to delete then adjust the count.
    RmtResource* tail_resource = &resource_list->resources[resource_list->resource_count - 1];

    const RmtErrorCode error_code = RemoveResourceFromTree(resource_list, hashed_identifier);
    RMT_ASSERT(error_code == kRmtOk);

    // copy the tail into the target.
    if (tail_resource != resource)
    {
        memcpy(resource, tail_resource, sizeof(RmtResource));
        tail_resource->id_node->resource = resource;  // update acceleration structure pointer to this resource's new home.
    }

    resource_list->resource_count--;
    return kRmtOk;
}

size_t RmtResourceListGetBufferSize(int32_t maximum_concurrent_resources)
{
    return maximum_concurrent_resources * (sizeof(RmtResource) + sizeof(RmtResourceIdNode));
}

RmtErrorCode RmtResourceListInitialize(RmtResourceList*                resource_list,
                                       void*                           buffer,
                                       size_t                          buffer_size,
                                       const RmtVirtualAllocationList* virtual_allocation_list,
                                       int32_t                         maximum_concurrent_resources,
                                       bool                            enable_aliased_resource_usage_sizes)
{
    RMT_ASSERT(resource_list);
    RMT_RETURN_ON_ERROR(resource_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(buffer, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(buffer_size, kRmtErrorInvalidSize);
    RMT_RETURN_ON_ERROR(RmtResourceListGetBufferSize(maximum_concurrent_resources) <= buffer_size, kRmtErrorInvalidSize);

    // initialize the resource storage
    resource_list->resources                           = (RmtResource*)buffer;
    resource_list->resource_count                      = 0;
    resource_list->virtual_allocation_list             = virtual_allocation_list;
    resource_list->maximum_concurrent_resources        = maximum_concurrent_resources;
    resource_list->enable_aliased_resource_usage_sizes = enable_aliased_resource_usage_sizes;

    // initialize the acceleration structure.
    const uintptr_t resource_node_buffer      = ((uintptr_t)buffer) + (maximum_concurrent_resources * sizeof(RmtResource));
    resource_list->resource_id_nodes          = (RmtResourceIdNode*)resource_node_buffer;
    const size_t       resource_id_nodes_size = maximum_concurrent_resources * sizeof(RmtResourceIdNode);
    const RmtErrorCode error_code =
        RmtPoolInitialize(&resource_list->resource_id_node_pool, resource_list->resource_id_nodes, resource_id_nodes_size, sizeof(RmtResourceIdNode));
    RMT_ASSERT(error_code == kRmtOk);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);
    resource_list->root = NULL;

    memset(resource_list->resource_usage_count, 0, sizeof(resource_list->resource_usage_count));
    memset(resource_list->total_resource_usage_aliased_size, 0, sizeof(resource_list->total_resource_usage_aliased_size));

    return kRmtOk;
}

RmtErrorCode RmtResourceListAddResourceCreate(RmtResourceList* resource_list, const RmtTokenResourceCreate* resource_create)
{
    RMT_ASSERT(resource_list);
    RMT_ASSERT(resource_create);
    RMT_RETURN_ON_ERROR(resource_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(resource_create, kRmtErrorInvalidPointer);

    // Resource ID should be a thing.
    RMT_ASSERT(resource_create->resource_identifier);

    const uint64_t hashed_identifier = GenerateResourceHandle(resource_create->resource_identifier);

    // Check if a resource with this ID already exists insert an implicit unbind.
    RmtResource* resource = FindResourceById(resource_list, hashed_identifier);
    if (resource != nullptr)
    {
        // remove it
        if (resource->bound_allocation != nullptr)
        {
            if (resource->bound_allocation->resource_count > 0)
            {
                ((RmtVirtualAllocation*)resource->bound_allocation)->resource_count--;
                if (resource->resource_type != kRmtResourceTypeHeap)
                {
                    ((RmtVirtualAllocation*)resource->bound_allocation)->non_heap_resource_count--;
                }
            }
        }

        DestroyResource(resource_list, resource);
    }

    // Make sure we can allocate this resource.
    RMT_ASSERT(resource_list->resource_count + 1 <= resource_list->maximum_concurrent_resources);
    RMT_RETURN_ON_ERROR((resource_list->resource_count + 1) <= resource_list->maximum_concurrent_resources, kRmtErrorOutOfMemory);

    // fill out the stuff we know.
    RmtResource* new_resource = &resource_list->resources[resource_list->resource_count++];
    new_resource->name          = nullptr;
    new_resource->identifier    = resource_create->resource_identifier;
    new_resource->create_time   = resource_create->common.timestamp;
    new_resource->flags         = 0;
    new_resource->commit_type   = resource_create->commit_type;
    new_resource->resource_type = resource_create->resource_type;
    new_resource->owner_type    = resource_create->owner_type;

    switch (resource_create->resource_type)
    {
    case kRmtResourceTypeImage:
        memcpy(&new_resource->image, &resource_create->image, sizeof(RmtResourceDescriptionImage));
        break;

    case kRmtResourceTypeBuffer:
        memcpy(&new_resource->buffer, &resource_create->buffer, sizeof(RmtResourceDescriptionBuffer));
        break;

    case kRmtResourceTypeGpuEvent:
        memcpy(&new_resource->gpu_event, &resource_create->gpu_event, sizeof(RmtResourceDescriptionGpuEvent));
        break;

    case kRmtResourceTypeBorderColorPalette:
        memcpy(&new_resource->border_color_palette, &resource_create->border_color_palette, sizeof(RmtResourceDescriptionBorderColorPalette));
        break;

    case kRmtResourceTypePerfExperiment:
        memcpy(&new_resource->perf_experiment, &resource_create->perf_experiment, sizeof(RmtResourceDescriptionPerfExperiment));
        break;

    case kRmtResourceTypeQueryHeap:
        memcpy(&new_resource->query_heap, &resource_create->query_heap, sizeof(RmtResourceDescriptionQueryHeap));
        break;

    case kRmtResourceTypeVideoDecoder:
        memcpy(&new_resource->video_decoder, &resource_create->video_decoder, sizeof(RmtResourceDescriptionVideoDecoder));
        break;

    case kRmtResourceTypeVideoEncoder:
        memcpy(&new_resource->video_encoder, &resource_create->video_encoder, sizeof(RmtResourceDescriptionVideoEncoder));
        break;

    case kRmtResourceTypeHeap:
        memcpy(&new_resource->heap, &resource_create->heap, sizeof(RmtResourceDescriptionHeap));
        break;

    case kRmtResourceTypePipeline:
        memcpy(&new_resource->pipeline, &resource_create->pipeline, sizeof(RmtResourceDescriptionPipeline));
        break;

    case kRmtResourceTypeDescriptorHeap:
        memcpy(&new_resource->descriptor_heap, &resource_create->descriptor_heap, sizeof(RmtResourceDescriptionDescriptorHeap));
        break;

    case kRmtResourceTypeDescriptorPool:
        memcpy(&new_resource->descriptor_pool, &resource_create->descriptor_pool, sizeof(RmtResourceDescriptionDescriptorPool));
        break;

    case kRmtResourceTypeCommandAllocator:
        memcpy(&new_resource->command_allocator, &resource_create->command_allocator, sizeof(RmtResourceDescriptionCommandAllocator));
        break;

    case kRmtResourceTypeMiscInternal:
        memcpy(&new_resource->misc_internal, &resource_create->misc_internal, sizeof(RmtResourceDescriptionMiscInternal));
        break;

    case kRmtResourceTypeIndirectCmdGenerator:
    case kRmtResourceTypeMotionEstimator:
    case kRmtResourceTypeTimestamp:
        // NOTE: no data associated with these types, if this changes in future we'll need to copy it here.
        break;

    default:
        // shouldn't reach here.
        RMT_ASSERT(0);
        break;
    }

    // clear this ready for patch during bind.
    new_resource->bind_time        = 0;
    new_resource->address          = 0;
    new_resource->size_in_bytes    = 0;
    new_resource->bound_allocation = NULL;

    // insert node into the acceleration structure.
    AddResourceToTree(resource_list, hashed_identifier, new_resource);

    resource_list->resource_usage_count[RmtResourceGetUsageType(new_resource)]++;
    return kRmtOk;
}

// calculate commit type for the resource.
static void UpdateCommitType(RmtResourceList* resource_list, RmtResource* resource)
{
    RMT_ASSERT(resource);
    RMT_ASSERT(resource->bound_allocation);

    // NOTE: Casting const away here is intentional, as this structure is internal to backend and
    // const is designed for the external facing API.
    RmtVirtualAllocation* current_virtual_allocation = (RmtVirtualAllocation*)resource->bound_allocation;

    // PRT stuff is for sure virtual.
    if (resource->resource_type == kRmtResourceTypeImage)
    {
        if ((resource->image.create_flags & kRmtImageCreationFlagPrt) == kRmtImageCreationFlagPrt)
        {
            const RmtHeapType previous_heap_type = current_virtual_allocation->heap_preferences[0];
            RMT_ASSERT(previous_heap_type != kRmtHeapTypeNone);

            // mark everything as virtual and heap none, we shouldn't get allocations which contain virtual and non-virtual stuff.
            resource->commit_type                           = kRmtCommitTypeVirtual;
            current_virtual_allocation->heap_preferences[0] = kRmtHeapTypeNone;
            current_virtual_allocation->heap_preferences[1] = kRmtHeapTypeNone;
            current_virtual_allocation->heap_preferences[2] = kRmtHeapTypeNone;
            current_virtual_allocation->heap_preferences[3] = kRmtHeapTypeNone;

            // move the allocation's bytes into the NONE heap from wherever they came from.
            const uint64_t size_in_bytes = RmtGetAllocationSizeInBytes(current_virtual_allocation->size_in_4kb_page, kRmtPageSize4Kb);
            ((RmtVirtualAllocationList*)(resource_list->virtual_allocation_list))->allocations_per_preferred_heap[previous_heap_type] -= size_in_bytes;
            ((RmtVirtualAllocationList*)(resource_list->virtual_allocation_list))->allocations_per_preferred_heap[kRmtHeapTypeNone] += size_in_bytes;
        }
    }

    // NOTE: A more accurate commit type for non-vritual resources will be calculated in deferred pass. See
    // snapshotGeneratorCalculateCommitType in rmt_data_set.cpp.
}

RmtErrorCode RmtResourceListAddResourceBind(RmtResourceList* resource_list, const RmtTokenResourceBind* resource_bind, const bool track_user_data)
{
    RMT_ASSERT(resource_list);
    RMT_RETURN_ON_ERROR(resource_list, kRmtErrorInvalidPointer);

    const uint64_t handle = GenerateResourceHandle(resource_bind->resource_identifier);

    RmtResource* resource = FindResourceById(resource_list, handle);
    RMT_RETURN_ON_ERROR(resource, kRmtErrorNoResourceFound);

    // NOTE: We have multiple binds per resource for command buffer allocators,
    // This is because they grow in size to accomodate the allocators needs. GPU events
    // are often inlined into command buffers, so these are also affected by extension.
    // Heap and Buffer resources which have already been bound to a virtual memory allocation are
    // also flagged with the kRmtErrorResourceAlreadyBound return value.  The caller can then destroy
    // the existing resource, create a new resource and re-bind it to a different allocation.
    if (resource->bound_allocation != nullptr)
    {
        switch (resource->resource_type)
        {
        case kRmtResourceTypeHeap:
        case kRmtResourceTypeBuffer:
        case kRmtResourceTypeCommandAllocator:
            return kRmtErrorResourceAlreadyBound;

        case kRmtResourceTypeGpuEvent:
            return kRmtOk;

        default:
            // Should never reach this point, handle it just in case.
            RMT_ASSERT(false);
            return kRmtOk;
        }
    }

    // bind the allocation to the resource.
    resource->bind_time     = resource_bind->common.timestamp;
    resource->address       = resource_bind->virtual_address;
    resource->size_in_bytes = resource_bind->size_in_bytes;

    // find the bound allocation
    const RmtVirtualAllocation* allocation = nullptr;
    const RmtErrorCode          error_code =
        RmtVirtualAllocationListGetAllocationForAddress(resource_list->virtual_allocation_list, resource_bind->virtual_address, &allocation);
    resource->bound_allocation = allocation;

    // look for externally shared resources.
    if ((error_code == kRmtErrorNoAllocationFound) && (resource->resource_type == kRmtResourceTypeImage) &&
        ((resource->image.create_flags & kRmtImageCreationFlagShareable) == kRmtImageCreationFlagShareable))
    {
        // It is expected that we won't see a virtual allocate token for some shareable resources, as that memory is owned outside
        // the target process.  This error code will result in a dummy allocation being added to the list, so future resource calls
        // looking for it will be able to "find" the allocation.
        return kRmtErrorSharedAllocationNotFound;
    }

    // only external shared can fail to find the allocation.
    RMT_ASSERT(error_code == kRmtOk);

    RmtResourceUsageType usage_type = RmtResourceGetUsageType(resource);

    // Count the resources on each allocation. We fill in pointers later
    // when we do the fix up pass in snapshot generation.
    if (resource->bound_allocation != nullptr)
    {
        RMT_ASSERT(resource->bound_allocation->base_address <= resource->address);

        // count resources and non-heap resources.
        if (resource->resource_type != kRmtResourceTypeHeap)
        {
            ((RmtVirtualAllocation*)resource->bound_allocation)->non_heap_resource_count++;
        }
        ((RmtVirtualAllocation*)resource->bound_allocation)->resource_count++;

        // update the commit type of the allocation
        UpdateCommitType(resource_list, resource);

        if ((resource_list->enable_aliased_resource_usage_sizes) && (usage_type != RmtResourceUsageType::kRmtResourceUsageTypeHeap))
        {
            RmtMemoryAliasingCalculator* memory_aliasing_calculator = RmtMemoryAliasingCalculatorInstance();
            RMT_ASSERT(memory_aliasing_calculator);
            Allocation* aliased_resource_allocation = memory_aliasing_calculator->FindAllocation(resource->bound_allocation->allocation_identifier);
            if (aliased_resource_allocation != nullptr)
            {
                aliased_resource_allocation->CreateResource(
                    resource->address - resource->bound_allocation->base_address, resource->size_in_bytes, RmtResourceGetUsageType(resource));
                UpdateTotalResourceUsageAliasedSize(resource_list, memory_aliasing_calculator);
            }
        }

        if ((track_user_data) && (resource->address == allocation->base_address))
        {
            // Keep track of resources bound to a virtual allocation.
            RmtResourceUserDataTrackBoundResource(resource, allocation->allocation_identifier);
        }
    }

    return kRmtOk;
}

RmtErrorCode RmtResourceListAddResourceDestroy(RmtResourceList* resource_list, const RmtTokenResourceDestroy* resource_destroy)
{
    RMT_ASSERT(resource_list);
    RMT_RETURN_ON_ERROR(resource_list, kRmtErrorInvalidPointer);

    const uint64_t handle   = GenerateResourceHandle(resource_destroy->resource_identifier);
    RmtResource*   resource = FindResourceById(resource_list, handle);
    RMT_RETURN_ON_ERROR(resource, kRmtErrorNoResourceFound);

    // remove the resource from the parent allocation.
    if (resource->bound_allocation != nullptr)
    {
        ((RmtVirtualAllocation*)resource->bound_allocation)->resource_count--;

        if (resource->resource_type != kRmtResourceTypeHeap)
        {
            ((RmtVirtualAllocation*)resource->bound_allocation)->non_heap_resource_count--;
        }
    }

    // call destroy on it.
    const RmtErrorCode error_code = DestroyResource(resource_list, resource);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    return kRmtOk;
}

RmtErrorCode RmtResourceListGetResourceByResourceId(const RmtResourceList* resource_list,
                                                    RmtResourceIdentifier  resource_identifier,
                                                    const RmtResource**    out_resource)
{
    RMT_ASSERT(resource_list);
    RMT_ASSERT(out_resource);
    RMT_RETURN_ON_ERROR(resource_list, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(out_resource, kRmtErrorInvalidPointer);

    const uint64_t handle = GenerateResourceHandle(resource_identifier);

    RmtResource* resource = FindResourceById(resource_list, handle);
    RMT_RETURN_ON_ERROR(resource, kRmtErrorNoResourceFound);
    *out_resource = resource;
    return kRmtOk;
}

bool RmtResourceIsAliased(const RmtResource* resource)
{
    RMT_RETURN_ON_ERROR(resource != nullptr, false);
    RMT_RETURN_ON_ERROR(resource->bound_allocation != nullptr, false);
    RMT_RETURN_ON_ERROR(resource->resource_type != kRmtResourceTypeHeap, false);

    const RmtVirtualAllocation* virtual_allocation     = resource->bound_allocation;
    const RmtGpuAddress         resource_start_address = resource->address;
    const RmtGpuAddress         resource_end_address   = resource->address + resource->size_in_bytes;
    bool                        is_aliased             = false;

    for (int32_t current_resource_index = 0; current_resource_index < virtual_allocation->resource_count; ++current_resource_index)
    {
        const RmtResource* current_resource = virtual_allocation->resources[current_resource_index];

        if (current_resource == resource)
        {
            continue;
        }

        // Special handle for heaps.
        if (current_resource->resource_type == kRmtResourceTypeHeap)
        {
            continue;
        }

        // Get the start and end address.
        const RmtGpuAddress current_resource_start = current_resource->address;
        const RmtGpuAddress current_resource_end   = current_resource->address + current_resource->size_in_bytes;
        if (resource_start_address >= current_resource_end)
        {
            continue;
        }

        if (resource_end_address <= current_resource_start)
        {
            continue;
        }

        is_aliased = true;
        break;
    }

    return is_aliased;
}

uint64_t RmtResourceGetUsageTypeMask(RmtResourceUsageType usage_type)
{
    RMT_ASSERT((usage_type >= 0) && (usage_type < 64));

    uint64_t result = 0;
    if (usage_type != kRmtResourceUsageTypeUnknown)
    {
        result = 1ULL << (usage_type - 1);
    }

    return result;
}

RmtErrorCode RmtResourceUpdateAliasSize(const RmtResourceIdentifier resource_id, const RmtResourceList* resource_list, const uint64_t alias_size)
{
    RMT_RETURN_ON_ERROR(resource_list != nullptr, kRmtErrorInvalidPointer);

    RmtErrorCode result   = kRmtErrorNoResourceFound;
    RmtResource* resource = FindResourceById(resource_list, resource_id);
    if (resource != nullptr)
    {
        if (resource->resource_type == RmtResourceType::kRmtResourceTypeHeap)
        {
            // Heap resources are a special case.
            // Overlapping resources are not considered when calculating heap memory adjusted for aliasing.
            resource->adjusted_size_in_bytes = resource->size_in_bytes;
        }
        else
        {
            resource->adjusted_size_in_bytes = alias_size;
        }

        result = kRmtOk;
    }

    return result;
}
