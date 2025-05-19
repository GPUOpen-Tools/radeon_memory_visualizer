//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definitions of the RMT token.
//=============================================================================

#ifndef RMV_PARSER_RMT_TOKEN_H_
#define RMV_PARSER_RMT_TOKEN_H_

#include <stdbool.h>

#ifndef _WIN32
#include <stdlib.h>
#endif

#include "rmt_format.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// A structure encapsulating common fields for all RMT tokens.
typedef struct RmtTokenCommon
{
    uint64_t     thread_id;     ///< The thread ID that the token was emitted from.
    RmtProcessId process_id;    ///< The process ID that the token was emitted from.
    uint64_t     timestamp;     ///< The timestamp (in RMT clocks) when the token was generated.
    size_t       offset;        ///< The offset (in bytes) into the parent RMT stream.
    int32_t      stream_index;  ///< The index of the RMT stream that the token was parsed from.
} RmtTokenCommon;

/// A structure encapsulating a timestamp.
typedef struct RmtTokenTimestamp
{
    RmtTokenCommon common;     ///< Fields common to all tokens.
    uint64_t       timestamp;  ///< A 64bit timestamp (in RMT clocks).
    uint32_t       frequency;  ///< CPU frequency
} RmtTimestampToken;

/// A structure encapsulating a free of virtual memory.
typedef struct RmtTokenVirtualFree
{
    RmtTokenCommon common;           ///< Fields common to all tokens.
    RmtGpuAddress  virtual_address;  ///< The virtual or physical address being freed.
} RmtTokenVirtualFree;

/// A structure encapsulating page table updates.
typedef struct RmtTokenPageTableUpdate
{
    RmtTokenCommon         common;            ///< Fields common to all tokens.
    RmtGpuAddress          virtual_address;   ///< The virtual address of the allocation being mapped.
    RmtGpuAddress          physical_address;  ///< The physical address of the allocation being mapped.
    uint64_t               size_in_pages;     ///< The size of the mapping in pages.
    RmtPageSize            page_size;         ///< The page size for the mapping.
    bool                   is_unmapping;
    RmtPageTableUpdateType update_type;  ///< The type of the page table update.
    RmtPageTableController controller;   ///< The type of system controlling page table updates.
} RmtTokenPageTableUpdate;

/// A structure encapsulating user data.
typedef struct RmtTokenUserdata
{
    RmtTokenCommon  common;         ///< Fields common to all tokens.
    RmtUserdataType userdata_type;  ///< The type of the user data in the payload.
    int32_t         size_in_bytes;  ///< The size (in bytes) of the payload. The largest we can encode is 1MB.
    uint8_t*        payload_cache;  ///< Pointer to the payload of the user data.
    uint64_t        time_delay;     ///< Time delay, if any, between the RMT token creation and a previous event.

    RmtResourceIdentifier
        resource_identifier;  ///< The identifier used to match a name to a non-DX resource, only valid when usedataType is RMT_USERDATA_TYPE_NAME.
    RmtCorrelationIdentifier correlation_identifier;        ///< The identifier used to match correlation ID for DX traces.
    RmtResourceIdentifier    original_resource_identifier;  ///< The Original Resource ID contained in the RESOURCE_CREATE token.
    RmtImplicitResourceType  implicit_resource_type;        ///< The type of implicit resource.
} RmtTokenUserdata;

/// A structure encapsulating misc data.
typedef struct RmtTokenMisc
{
    RmtTokenCommon common;  ///< Fields common to all tokens.
    RmtMiscType    type;    ///< The type of miscellaneous event that occurred.
} RmtTokenMisc;

/// A structure encapsulating a residency update.
typedef struct RmtTokenResourceReference
{
    RmtTokenCommon         common;                 ///< Fields common to all tokens.
    RmtResidencyUpdateType residency_update_type;  ///< The type of residency update.
    RmtGpuAddress          virtual_address;        ///< The virtual address of the residency memory where the residency update was requested.
    RmtQueue               queue;                  ///< The queue where the reference was added or removed.
} RmtTokenResourceReference;

/// A structure encapsulating a resource being bound to a virtual memory address range.
typedef struct RmtTokenResourceBind
{
    RmtTokenCommon        common;               ///< Fields common to all tokens.
    RmtResourceIdentifier resource_identifier;  ///< A unique identifier for the resource being bound.
    RmtGpuAddress         virtual_address;      ///< The virtual address that the resource is being bound to.
    uint64_t              size_in_bytes;        ///< The size of the resource in bytes.
    bool                  is_system_memory;     ///< A boolean value indicates if the bind is in system memory.
} RmtTokenResourceBind;

/// A structure encapsulating a process event.
typedef struct RmtTokenProcessEvent
{
    RmtTokenCommon      common;      ///< Fields common to all tokens.
    RmtProcessEventType event_type;  ///< The process event type.
} RmtTokenProcessEvent;

/// A structure encapsulating a page reference.
typedef struct RmtTokenPageReference
{
    RmtTokenCommon common;                          ///< Fields common to all tokens.
    RmtPageSize    page_size;                       ///< The size of each page in <c><i>pageState</i></c>.
    uint8_t        page_state[RMT_PAGE_REF_COUNT];  ///< A bitfield of page state.
} RmtTokenPageReference;

/// A structure encapsulating a CPU map token.
typedef struct RmtTokenCpuMap
{
    RmtTokenCommon common;           ///< Fields common to all tokens.
    RmtGpuAddress  virtual_address;  ///< The virtual address that was mapped for CPU access.
    bool           is_unmap;         ///< The map operation is an unmap
} RmtTokenCpuMap;

/// A structure encapsulating a time delta.
typedef struct RmtTokenVirtualAllocate
{
    RmtTokenCommon common;                                ///< Fields common to all tokens.
    RmtGpuAddress  virtual_address;                       ///< The virtual address that was allocated.
    uint64_t       size_in_bytes;                         ///< The size (in bytes) of the allocation.
    RmtOwnerType   owner_type;                            ///< The owner of the allocation.
    RmtHeapType    preference[RMT_NUM_HEAP_PREFERENCES];  ///< An ordered list of heap preferences for the allocation.
    bool           is_external;                           ///< If true, indicates externally owned allocation opened by target application.  False, otherwise.
} RmtTokenVirtualAllocate;

/// A structure encapsulating a resource description.
typedef struct RmtTokenResourceCreate
{
    RmtTokenCommon           common;                        ///< Fields common to all tokens.
    RmtResourceIdentifier    resource_identifier;           ///< A unique identifier for the resource.
    RmtResourceIdentifier    original_resource_identifier;  ///< The original resource ID included in the Token's payload.
    RmtCorrelationIdentifier correlation_identifier;        ///< The Resource Name USERDATA correlation ID (set to 0 if unused).
    RmtOwnerType             owner_type;                    ///< The part of the software stack creating this resource.
    //RmtOwnerCategory              owner_category;                 ///< The owner category.
    RmtCommitType   commit_type;    ///< The type of commitment reqeuired for this resource.
    RmtResourceType resource_type;  ///< The resource type.

    // A union of the different resource descriptions, access based on resourceType.
    union
    {
        RmtResourceDescriptionImage    image;      ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeImage</i></c>.
        RmtResourceDescriptionBuffer   buffer;     ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeBuffer</i></c>.
        RmtResourceDescriptionGpuEvent gpu_event;  ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeGpuEvent</i></c>.
        RmtResourceDescriptionBorderColorPalette
            border_color_palette;                                ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeBorderColorPalette</i></c>.
        RmtResourceDescriptionPerfExperiment   perf_experiment;  ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypePerfExperiment</i></c>.
        RmtResourceDescriptionQueryHeap        query_heap;       ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeQueryHeap</i></c>.
        RmtResourceDescriptionPipeline         pipeline;         ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypePipeline</i></c>.
        RmtResourceDescriptionVideoDecoder     video_decoder;    ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeVideoDecoder</i></c>.
        RmtResourceDescriptionVideoEncoder     video_encoder;    ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeVideoEncoder</i></c>.
        RmtResourceDescriptionHeap             heap;             ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeHeap</i></c>.
        RmtResourceDescriptionDescriptorHeap   descriptor_heap;  ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeDescriptorHeap</i></c>.
        RmtResourceDescriptionDescriptorPool   descriptor_pool;  ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeDescriptorPool</i></c>.
        RmtResourceDescriptionCommandAllocator command_allocator;  ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourceTypeCommandAllocator</i></c>.
        RmtResourceDescriptionMiscInternal     misc_internal;      ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourcetypeMiscInternal</i></c>.
        RmtResourceDescriptionWorkGraph        work_graph;         ///< Valid when <c><i>resourceType</i></c> is <c><i>kRmtResourcetypeWorkGraph</i></c>.
    };
} RmtTokenResourceCreate;

/// A structure encapsulating a time delta.
typedef struct RmtTokenTimeDelta
{
    RmtTokenCommon common;  ///< Fields common to all tokens.
    uint64_t       delta;   ///< A 12bit delta (in RMT clocks).
} RmtTokenTimeDelta;

/// A structure encapsulating a resource being unbound from a virtual memory address range.
typedef struct RmtTokenResourceDestroy
{
    RmtTokenCommon        common;               ///< Fields common to all tokens.
    RmtResourceIdentifier resource_identifier;  ///< A unique identifier for the resource being unbound.
} RmtTokenResourceDestroy;

/// A structure for a resource update
typedef struct RmtTokenResourceUpdate
{
    RmtTokenCommon        common;               ///< Fields common to all tokens.
    RmtResourceIdentifier resource_identifier;  ///< Resource ID
    uint32_t              subresource_id;       ///< Subresource ID
    RmtResourceType       resource_type;        ///< Type of resource being updated
    uint64_t              before;               ///< Usage flags before
    uint64_t              after;                ///< Usage flags after
} RmtTokenResourceUpdate;

/// A structure encapsulating the token.
typedef struct RmtToken
{
    RmtTokenType type;  ///< The type of the RMT token.

    union
    {
        RmtTokenCommon            common;                   ///< Valid for any type, as all structures begin with a <c><i>RmtTokenCommon</i></c> structure.
        RmtTimestampToken         timestamp_token;          ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypeTimestamp</i></c>.
        RmtTokenVirtualFree       virtual_free_token;       ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypeVirtualFree</i></c>.
        RmtTokenPageTableUpdate   page_table_update_token;  ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypePageTableUpdate</i></c>.
        RmtTokenUserdata          userdata_token;           ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypeUserdata</i></c>.
        RmtTokenMisc              misc_token;               ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypeMisc</i></c>.
        RmtTokenResourceReference resource_reference;       ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypeResourceReference</i></c>.
        RmtTokenResourceBind      resource_bind_token;      ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypeResourceBind</i></c>.
        RmtTokenProcessEvent      process_event_token;      ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypeProcessEvent</i></c>.
        RmtTokenPageReference     page_reference_token;     ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypePageReference</i></c>.
        RmtTokenCpuMap            cpu_map_token;            ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypeCpuMap</i></c>.
        RmtTokenVirtualAllocate   virtual_allocate_token;   ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypeVirtualAllocate</i></c>.
        RmtTokenResourceCreate    resource_create_token;    ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypeResourceCreate</i></c>.
        RmtTokenTimeDelta         time_delta_token;         ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypeTimeDelta</i></c>.
        RmtTokenResourceDestroy   resource_destroy_token;   ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypeResourceDestroy</i></c>.
        RmtTokenResourceUpdate    resource_update_token;    ///< Valid when <c><i>type</i></c> is <c><i>kRmtTokenTypeResourceUpdate</i></c>.
    };

} RmtToken;

/// @brief Copy a token payload.
///
/// Since the Userdata token allocates memory, a memcpy will not work.
///
/// @param [in] dest A pointer to the destination token.
/// @param [in] src  A pointer to the source token.
void RmtTokenCopy(RmtToken* dest, const RmtToken* src);

/// @brief Allocate memory for the USERDATA token payload cache.
///
/// @param [in] size The number of bytes to allocate.
///
/// @return A pointer to the allocated memory.
uint8_t* RmtTokenAllocatePayloadCache(size_t size);

/// @brief Free all the memory associated with USERDATA token payload caches.
///
/// Should be done when the trace is closed and all the tokens have been cleared.
void RmtTokenClearPayloadCaches();

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_PARSER_RMT_TOKEN_H_
