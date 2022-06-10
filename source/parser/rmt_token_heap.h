//=============================================================================
// Copyright (c) 2019-2022 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  A priority queue data structure for RmtToken structures.
//=============================================================================

#ifndef RMV_PARSER_RMT_TOKEN_HEAP_H_
#define RMV_PARSER_RMT_TOKEN_HEAP_H_

#include "rmt_error.h"
#include "rmt_file_format.h"
#include "rmt_format.h"
#include "rmt_util.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

typedef struct RmtToken  RmtToken;
typedef struct RmtParser RmtParser;

/// A structure for fast lookup of unique resource ID based on a driver provided ID.
typedef struct ResourceIdMapNode
{
    uint64_t              base_driver_id;
    RmtResourceIdentifier unique_id;
    ResourceIdMapNode*    left;
    ResourceIdMapNode*    right;
} ResourceIdMapNode;

/// A structure wrapping an allocation used to contain ResourceIdMapNodes
typedef struct ResourceIdMapAllocator
{
    void*    allocation_base;
    size_t   allocation_size;
    size_t   bytes_used;
    void*    curr_offset;
    uint32_t resource_count;

    RmtResourceIdentifier GenUniqueId(uint64_t base_driver_id)
    {
        return ((base_driver_id & 0xFFFFFFFF) << 32) | (resource_count++ & 0xFFFFFFFF);
    }

    // Allocates a new node from the underlying allocation, if successful the new node will be initialized
    ResourceIdMapNode* AllocNewNode(uint64_t base_driver_id)
    {
        ResourceIdMapNode* new_node = nullptr;
        if ((bytes_used + sizeof(ResourceIdMapNode)) <= allocation_size)
        {
            curr_offset = static_cast<uint8_t*>(allocation_base) + bytes_used;
            new_node    = static_cast<ResourceIdMapNode*>(curr_offset);
            bytes_used += sizeof(ResourceIdMapNode);

            new_node->base_driver_id = base_driver_id;
            new_node->unique_id      = GenUniqueId(base_driver_id);
            new_node->left           = nullptr;
            new_node->right          = nullptr;
        }
        return new_node;
    }
} ResourceIdMapAllocator;

/// A structure encapsulating a priority queue data structure.
typedef struct RmtStreamMerger
{
    RmtParser* parsers;                      ///< A pointers to an array of <c><i>RmtParser</i></c> structures.
    int32_t    parser_count;                 ///< The number of parsers.
    RmtToken   buffer[RMT_MAXIMUM_STREAMS];  ///< An array of pointers to <c><i>RmtToken</i></c> structures, one per stream.
    RmtToken*  tokens[RMT_MAXIMUM_STREAMS];  ///< An array of pointers to structures in <c><i>buffer</i></c>, the pointers are organised in min-heap order.
    size_t     current_size;                 ///< The current number of token pointers used in the heap.
    uint64_t   minimum_start_timestamp;      ///< The minimum start timestamp.
    ResourceIdMapAllocator* allocator;       ///< Allocator for a resource ID map, used to lookup unique ID based on driver provided resource ID.
    ResourceIdMapNode*      map_root;        ///< Root of the resource ID mapping tree.
} RmtStreamMerger;

/// Initialize the stream merger.
///
/// @param [in]     token_heap               An RmtStreamMerger structure defining the stream merger.
/// @param [in]     stream_parsers           An RmtParser structure defining the stream parsers.
/// @param [in]     stream_parser_count      The number of stream parsers.
/// @param [in]     file_handle              The file handle for the memory trace.
///
/// @retval
/// kRmtOk                                   The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                  The operation failed because <c><i>token_heap</i></c> was <c><i>NULL</i></c>.
/// @retval
/// kRmtErrorInvalidSize                     The operation failed because <c><i>stream_parser_count</i></c> was an invalid size.
RmtErrorCode RmtStreamMergerInitialize(RmtStreamMerger* token_heap, RmtParser* stream_parsers, int32_t stream_parser_count, FILE* file_handle);

/// Clear the heap.
///
/// @param [in]     token_heap               An RmtStreamMerger structure defining the stream merger.
/// @param [in]     file_handle              The file handle for the memory trace.
///
/// @retval
/// kRmtOk                                   The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer                  The operation failed because <c><i>token_heap</i></c> was <c><i>NULL</i></c>.
RmtErrorCode RmtStreamMergerReset(RmtStreamMerger* token_heap, FILE* file_handle);

/// Return true if the heap is empty.
///
/// @param [in]     token_heap               An RmtStreamMerger structure defining the stream merger.
/// @returns
/// true if stream merger is empty, false if not.
bool RmtStreamMergerIsEmpty(const RmtStreamMerger* token_heap);

/// Get the next token from the stream merger.
///
/// @param [in]     token_heap               An RmtStreamMerger structure defining the stream merger.
/// @param [out]    out_token                An RmtToken structure receiving the token.
///
/// @retval
/// kRmtOk                                   The operation completed successfully.
/// @retval
/// kRmtErrorOutOfMemory                     The operation failed because <c><i>token_heap</i></c> is empty.
RmtErrorCode RmtStreamMergerAdvance(RmtStreamMerger* token_heap, RmtToken* out_token);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_PARSER_RMT_TOKEN_HEAP_H_
