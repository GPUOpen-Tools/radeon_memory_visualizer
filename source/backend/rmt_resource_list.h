//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Structures and functions for working with a resource list.
//=============================================================================

#ifndef RMV_BACKEND_RMT_RESOURCE_LIST_H_
#define RMV_BACKEND_RMT_RESOURCE_LIST_H_

#include "rmt_types.h"
#include "rmt_error.h"
#include "rmt_configuration.h"
#include "rmt_pool.h"
#include "rmt_format.h"
#include "rmt_token.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

typedef struct RmtVirtualAllocationList RmtVirtualAllocationList;
typedef struct RmtVirtualAllocation     RmtVirtualAllocation;
typedef struct RmtResourceIdNode        RmtResourceIdNode;
typedef struct RmtDataSnapshot          RmtDataSnapshot;

/// An enumeration of the different types of a backing store that can sit behind a resource.
typedef enum RmtResourceBackingStorage
{
    kRmtResourceBackingStorageUnmapped = kRmtHeapTypeCount,        ///< Unmapped histogram entry.
    kRmtResourceBackingStorageUnknown  = (kRmtHeapTypeCount + 1),  ///< Unknown histogram entry.

    // Add above this.
    kRmtResourceBackingStorageCount
} RmtResourceBackingStorage;

/// An enumeration of all bits for the <c><i>flags</i></c> field of <c><i>RmtResource</i></c>.
typedef enum RmtResourceFlagBits
{
    kRmtResourceFlagDangling = (1 << 0),  ///< The resource was left dangling by freeing the underlaying virtual allocation without destroying the resource.
} RmtResourceFlagBits;

/// A structure encapsulating a single resource.
typedef struct RmtResource
{
    const char*           name;           ///< The name of the resource.
    RmtResourceIdentifier identifier;     ///< A GUID for the this resource.
    uint64_t              create_time;    ///< The time the resource was created.
    uint64_t              bind_time;      ///< The time the resource was last bound to a virtual address range.
    uint64_t              address;        ///< The virtual address of the resource.
    uint64_t              size_in_bytes;  ///< The total size of the resource.
    const RmtVirtualAllocation*
                    bound_allocation;  ///< An pointers to a <c><i>RmtAllocation</i></c> structure containing the virtual address allocation containing this resource. This is set to NULL if the resource isn't bound to a virtual address.
    uint32_t        flags;             ///< Flags on the resource.
    RmtCommitType   commit_type;       ///< The commit type of the resource.
    RmtResourceType resource_type;     ///< The type of the resource.
    RmtOwnerType    owner_type;        ///< The owner of the resource.

    union
    {
        RmtResourceDescriptionImage    image;      ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeImage</i></c>.
        RmtResourceDescriptionBuffer   buffer;     ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeBuffer</i></c>.
        RmtResourceDescriptionGpuEvent gpu_event;  ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeGpuEvent</i></c>.
        RmtResourceDescriptionBorderColorPalette
            border_color_palette;                              ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeBorderColorPalette</i></c>.
        RmtResourceDescriptionPerfExperiment perf_experiment;  ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypePerfExperiment</i></c>.
        RmtResourceDescriptionQueryHeap      query_heap;       ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeQueryHeap</i></c>.
        RmtResourceDescriptionPipeline       pipeline;         ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypePipeline</i></c>.
        RmtResourceDescriptionVideoDecoder   video_decoder;    ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeVideoDecoder</i></c>.
        RmtResourceDescriptionVideoEncoder   video_encoder;    ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeVideoEncoder</i></c>.
        RmtResourceDescriptionHeap           heap;             ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeHeap</i></c>.
        RmtResourceDescriptionDescriptorHeap descriptor_heap;  ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeDescriptorHeap</i></c>.
        RmtResourceDescriptionDescriptorPool descriptor_pool;  ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeDescriptorPool</i></c>.
        RmtResourceDescriptionCommandAllocator
                                           command_allocator;  ///< Valid when <c><i>resourceType</i></c> is <c><i>RMT_RESOURCE_TYPE_COMMAND_ALLOCATOR</i></c>.
        RmtResourceDescriptionMiscInternal misc_internal;      ///< Valid when <c><i>resourceType</i></c> is <c><i>RMT_RESOURCE_TYPE_MISC_INTERNAL</i></c>.
    };

    RmtResourceIdNode* id_node;  ///< A pointer to the <c><i>RmtResourceIdNode</i></c> structure in the tree, used to quickly locate this resource by ID.
} RmtResource;

/// Get the resource usage type from the resource.
///
/// @param [in] resource                            A pointer to a <c><i>RmtResource</i></c> structure.
///
/// @returns
/// The usage type of the resource.
RmtResourceUsageType RmtResourceGetUsageType(const RmtResource* resource);

/// Calculate the offset (in bytes) from the start of the base allocation that the resource is bound to.
///
/// @param [in] resource                            A pointer to a <c><i>RmtResource</i></c> structure.
///
/// @returns
/// The offset from the bound allocation.
uint64_t RmtResourceGetOffsetFromBoundAllocation(const RmtResource* resource);

/// Get the base virtual address for the resource.
///
/// @param [in] resource                            A pointer to a <c><i>RmtResource</i></c> structure.
///
/// @returns
/// The virtual address of this resource.
RmtGpuAddress RmtResourceGetVirtualAddress(const RmtResource* resource);

/// Check if a virtual allocation contains all or part of a resource.
///
/// @param [in] resource                            A pointer to a <c><i>RmtResource</i></c> structure.
/// @param [in] address_start                       The start of the virtual address range.
/// @param [in] address_end                         The end of the virtual address range.
///
/// @returns
/// True if any of the specified virtual address range overlaps any of the virtual address used by the resource.
bool RmtResourceOverlapsVirtualAddressRange(const RmtResource* resource, RmtGpuAddress address_start, RmtGpuAddress address_end);

/// Check if a resource is completely in the preferred heap.
///
/// @param [in] snapshot                            A pointer to a <c><i>RmtDataSnapshot</i></c> structure that contains the page table to check.
/// @param [in] resource                            A pointer to a <c><i>RmtResource</i></c> structure.
///
/// @returns
/// True if the resource is completely in its preferred heap.
bool RmtResourceIsCompletelyInPreferredHeap(const RmtDataSnapshot* snapshot, const RmtResource* resource);

/// Calculate a histogram demonstrating the number of bytes of memory in each backing store type.
///
/// @param [in] snapshot                            A pointer to a <c><i>RmtDataSnapshot</i></c> structure that contains the page table to check.
/// @param [in] resource                            A pointer to a <c><i>RmtResource</i></c> structure.
/// @param [out] out_bytes_per_backing_storage_type The number of bytes the resource has in each backing storage type.
///
RmtErrorCode RmtResourceGetBackingStorageHistogram(const RmtDataSnapshot* snapshot, const RmtResource* resource, uint64_t* out_bytes_per_backing_storage_type);

/// Calculate the number of resource that alias the memory underpinning this resource.
///
/// @param [in] resource                            A pointer to a <c><i>RmtResource</i></c> structure.
///
/// @returns
/// The number of resources that alias this one.
int32_t RmtResourceGetAliasCount(const RmtResource* resource);

/// A structure for fast searching by resource ID.
typedef struct RmtResourceIdNode
{
    RmtResourceIdentifier identifier;  ///< The guid to search on.
    RmtResource*          resource;    ///< A pointer to a <c><i>RmtResource</i></c> structure containing the resource payload.
    RmtResourceIdNode*    left;        ///< A pointer to a <c><i>RmtResourceNodeId</i></c> structure that is the left child of this node.
    RmtResourceIdNode*    right;       ///< A pointer to a <c><i>RmtResourceNodeId</i></c> structure that is the right child of this node.
} RmtResourceIdNode;

/// A structure encapsulating a list of resources.
typedef struct RmtResourceList
{
    // Data structure for fast lookups based on resource GUID.
    RmtResourceIdNode* root;                   ///< The root node of the acceleration structure.
    RmtResourceIdNode* resource_id_nodes;      ///< A pointer to an array of <c><i>RmtResourceIdNode</i></c> structures for the search acceleration structure.
    RmtPool            resource_id_node_pool;  ///< The pool allocator for the memory buffer pointed to be <c><i>resourceIdNodes</i></c>.

    // Storage for resources.
    RmtResource*                    resources;                     ///< A buffer of extra resource details.
    int32_t                         resource_count;                ///< The number of live resources in the list.
    int32_t                         maximum_concurrent_resources;  ///< The maximum number of resources that can be in flight at once.
    const RmtVirtualAllocationList* virtual_allocation_list;       ///< The virtual allocation to query for bindings.

    int32_t  resource_usage_count[kRmtResourceUsageTypeCount];  ///< The number of each resource usage currently in the list.
    uint64_t resource_usage_size[kRmtResourceUsageTypeCount];

} RmtResourceList;

/// Calculate how many bytes of memory we need for resource list buffers.
///
/// @param [in] maximum_concurrent_resources        The maximum number of resources that can be in flight at once.
///
/// @returns
/// The buffer size needed, in bytes.
size_t RmtResourceListGetBufferSize(int32_t maximum_concurrent_resources);

/// Initialize the resource list.
///
/// @param [in] resource_list                       A pointer to a <c><i>RmtResourceList</i></c> structure.
/// @param [in] buffer                              A pointer to a buffer containing the raw resource data.
/// @param [in] buffer_size                         The buffer size, in bytes.
/// @param [in] virtual_allocation_list             A pointer to a <c><i>RmtVirtualAllocationList</i></c> structure.
/// @param [in] maximum_concurrent_resources        The maximum number of resources that can be in flight at once.
///
/// @retval
/// kRmtOk                          The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer         The operation failed because <c><i>resource_list</i></c> or <c><i>buffer</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorInvalidSize            The operation failed because <c><i>buffer_size</i></c> is invalid.
RmtErrorCode RmtResourceListInitialize(RmtResourceList*                resource_list,
                                       void*                           buffer,
                                       size_t                          buffer_size,
                                       const RmtVirtualAllocationList* virtual_allocation_list,
                                       int32_t                         maximum_concurrent_resources);

/// Add a resource create to the list.
///
/// @param [in] resource_list                       A pointer to a <c><i>RmtResourceList</i></c> structure.
/// @param [in] resource_create                     A pointer to a <c><i>RmtTokenResourceCreate</i></c> structure.
///
/// @retval
/// kRmtOk                          The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer         The operation failed because <c><i>resource_list</i></c> or <c><i>resource_create</i></c> being set to <c><i>NULL</i></c>.
RmtErrorCode RmtResourceListAddResourceCreate(RmtResourceList* resource_list, const RmtTokenResourceCreate* resource_create);

/// Add a resource bind to the list.
///
/// @param [in] resource_list                       A pointer to a <c><i>RmtResourceList</i></c> structure.
/// @param [in] resource_bind                       A pointer to a <c><i>RmtTokenResourceBind</i></c> structure.
///
/// @retval
/// kRmtOk                              The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer             The operation failed because <c><i>resource_list</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorNoResourceFound            The operation failed because the resource in <c><i>resource_bind</i></c> can't be found.
/// @retval
/// kRmtErrorResourceAlreadyBound       The operation failed because the resource in <c><i>resource_bind</i></c> is already bound.
RmtErrorCode RmtResourceListAddResourceBind(RmtResourceList* resource_list, const RmtTokenResourceBind* resource_bind);

/// Add a resource destroy to the list.
///
/// @param [in] resource_list                       A pointer to a <c><i>RmtResourceList</i></c> structure.
/// @param [in] resource_destroy                    A pointer to a <c><i>RmtTokenResourceDestroy</i></c> structure.
///
/// @retval
/// kRmtOk                              The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer             The operation failed because <c><i>resource_list</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorNoResourceFound            The operation failed because the resource in <c><i>resource_destroy</i></c> can't be found.
RmtErrorCode RmtResourceListAddResourceDestroy(RmtResourceList* resource_list, const RmtTokenResourceDestroy* resource_destroy);

/// Find resource in the resource list from resource ID.
///
/// @param [in] resource_list                       A pointer to a <c><i>RmtResourceList</i></c> structure.
/// @param [in] resource_identifier                 The identifier of the resource to find.
/// @param [out] out_resource                       A pointer to a pointer to a <c><i>RmtResource</i></c> structure to receive the resource.
///
/// @retval
/// kRmtOk                              The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer             The operation failed because <c><i>resource_list</i></c> or <c><i>out_resource</i></c> being set to <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorNoResourceFound            The operation failed because the resource can't be found.
RmtErrorCode RmtResourceListGetResourceByResourceId(const RmtResourceList* resource_list,
                                                    RmtResourceIdentifier  resource_identifier,
                                                    const RmtResource**    out_resource);

/// Get the heap name for the resource passed in.
///
/// @param [in] resource                            A pointer to a <c><i>RmtResource</i></c> structure.
///
/// @returns
/// Pointer to a string containing the heap name.
const char* RmtResourceGetHeapTypeName(const RmtResource* resource);

/// Get the resource name from the resource
///
/// @param [in] resource                            A pointer to a <c><i>RmtResource</i></c> structure.
/// @param [in] buffer_size                         The size of the buffer to receive the resource name, in bytes.
/// @param [in] out_resource_name                   A pointer to a buffer to receive the resource name.
///
/// @returns
/// true if the resource name was successfully written to the output buffer, false if not.
bool RmtResourceGetName(const RmtResource* resource, int32_t buffer_size, char** out_resource_name);

/// Get the actual physical heap from the resource.
///
/// @param [in] snapshot                            A pointer to a <c><i>RmtDataSnapshot</i></c> structure.
/// @param [in] resource                            A pointer to a <c><i>RmtResource</i></c> structure.
///
/// @returns
/// The heap type ID as an <c><i>RmtHeapType</i></c> type.
RmtHeapType RmtResourceGetActualHeap(const RmtDataSnapshot* snapshot, const RmtResource* resource);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_RESOURCE_LIST_H_
