//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a priority queue data structure.
//=============================================================================

#include "rmt_token_heap.h"
#include "rmt_platform.h"
#include <rmt_print.h>
#include "rmt_format.h"
#include "rmt_parser.h"
#include "rmt_assert.h"
#include <string.h>  // for memcpy()

// NOTE: if its a KMD stream, bias the timestamp backwards. The reason for this
// is to compensate for the latency of the data output from KMD being shorter
// than that of the UMD. This gives rise to KMD tokens sometimes arriving for PTE
// tokens before the corresponding VA is recorded as being allocated by the UMD.
// by biasing it backwards we get a more accurate accounting of mapped memory
// per VA (and per process) at the expense of some accuracy of when memory is mapped.
// this seems like a reaonable tradeoff as unless the user generates a snapshot
// inside this buffer offset (relative to the start of a VA) it should be benign.
#define KMD_TIMESTAMP_BIAS (0)

// helper function to compare to RmtToken timestamp values.
static RMT_FORCEINLINE bool ElementCompareLess(const RmtToken* a, const RmtToken* b)
{
    return a->common.timestamp <= b->common.timestamp;
}

// helper function to compare to RmtToken timestamp values.
#ifdef VALIDATE_HEAP
static RMT_FORCEINLINE bool elementCompareLessByIndex(const RmtStreamMerger* token_heap, int32_t a, int32_t b)
{
    return token_heap->tokens[a]->common.timestamp <= token_heap->tokens[b]->common.timestamp;
}
#endif  // VALIDATE_HEAP

// heap function to swap two tokens
static RMT_FORCEINLINE void ElementSwap(RmtToken** a, RmtToken** b)
{
    RmtToken* temp = *a;
    *a             = *b;
    *b             = temp;
}

#ifdef VALIDATE_HEAP
static bool validateHeap(RmtStreamMerger* token_heap, int32_t element_index)
{
    if (!token_heap)
    {
        return false;
    }

    if ((size_t)element_index >= token_heap->current_size)
    {
        return true;
    }

    const int32_t left_element_index  = (element_index << 1) + 1;
    const int32_t right_element_index = (element_index << 1) + 2;

    if ((size_t)left_element_index < token_heap->current_size && !elementCompareLessByIndex(token_heap, element_index, left_element_index))
    {
        return false;
    }

    if ((size_t)right_element_index < token_heap->current_size && !elementCompareLessByIndex(token_heap, element_index, right_element_index))
    {
        return false;
    }

    return validateHeap(token_heap, left_element_index) && validateHeap(token_heap, right_element_index);
}
#endif  // VALIDATE_HEAP

// helper function to move an element to its correct place in the heap from the bttom of the heap.
static void ElementMoveUp(RmtStreamMerger* token_heap, size_t element_index)
{
    if (element_index == 0U)
    {
        return;
    }

    size_t parent_index = (element_index - 1) >> 1;

    while (element_index > 0 && ElementCompareLess(token_heap->tokens[element_index], token_heap->tokens[parent_index]))
    {
        ElementSwap(&token_heap->tokens[parent_index], &token_heap->tokens[element_index]);
        element_index = parent_index;
        parent_index  = (element_index - 1) >> 1;
    }
}

// helper function to move an element to its correct place from the top of the heap.
static void ElementMoveDown(RmtStreamMerger* token_heap, size_t element_index)
{
    if (element_index >= token_heap->current_size)
    {
        return;
    }

    RmtToken** current_element = &token_heap->tokens[element_index];

    // traverse the heap until we hit the bottom.
    for (;;)
    {
        const size_t left_child_index  = (element_index << 1) + 1;
        const size_t right_child_index = (element_index << 1) + 2;

        // get the token values.
        RmtToken** smallest_element = &token_heap->tokens[left_child_index];
        element_index               = left_child_index;

        RmtToken** right_element = &token_heap->tokens[right_child_index];

        if (right_child_index < token_heap->current_size && ElementCompareLess(*right_element, *smallest_element))
        {
            smallest_element = &token_heap->tokens[right_child_index];
            element_index    = right_child_index;
        }

        if (left_child_index >= token_heap->current_size || ElementCompareLess(*current_element, *smallest_element))
        {
            break;
        }

        ElementSwap(smallest_element, current_element);
        current_element = smallest_element;
    }
}

static RmtErrorCode Peek(RmtStreamMerger* token_heap, RmtToken* out_token)
{
    RMT_ASSERT(token_heap);
    RMT_ASSERT(out_token);

    memcpy(out_token, (const void*)token_heap->tokens[0], sizeof(RmtToken));
    return kRmtOk;
}

static RmtErrorCode Poll(RmtStreamMerger* token_heap, RmtToken* out_token)
{
    RMT_ASSERT(token_heap);
    RMT_ASSERT(out_token);

    Peek(token_heap, out_token);

    // remove the element
    if (token_heap->current_size == 0)
    {
        return kRmtOk;
    }

    ElementSwap(&token_heap->tokens[0], &token_heap->tokens[token_heap->current_size - 1]);
    token_heap->current_size--;
    ElementMoveDown(token_heap, 0);

#ifdef VALIDATE_HEAP
    RMT_ASSERT(validateHeap(token_heap, 0));
#endif  // #ifdef VALIDATE_HEAP

    return kRmtOk;
}

static bool IsFull(const RmtStreamMerger* token_heap)
{
    RMT_ASSERT(token_heap);
    return (token_heap->current_size == RMT_MAXIMUM_STREAMS);
}

static RmtErrorCode Insert(RmtStreamMerger* token_heap, RmtToken* token)
{
    RMT_ASSERT(token_heap);
    RMT_ASSERT(token);
    RMT_RETURN_ON_ERROR(token_heap, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(token, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(!IsFull(token_heap), kRmtErrorOutOfMemory);

    token_heap->tokens[token_heap->current_size] = token;
    ElementMoveUp(token_heap, token_heap->current_size);
    token_heap->current_size++;

#ifdef VALIDATE_HEAP
    RMT_ASSERT(validateHeap(token_heap, 0));
#endif  // #ifdef VALIDATE_HEAP
    return kRmtOk;
}

RmtErrorCode RmtStreamMergerInitialize(RmtStreamMerger* token_heap, RmtParser* stream_parsers, int32_t stream_parser_count)
{
    RMT_RETURN_ON_ERROR(token_heap, kRmtErrorInvalidPointer);
    RMT_RETURN_ON_ERROR(stream_parser_count, kRmtErrorInvalidSize);
    RMT_RETURN_ON_ERROR(stream_parser_count < RMT_MAXIMUM_STREAMS, kRmtErrorInvalidSize);

    token_heap->parser_count            = stream_parser_count;
    token_heap->current_size            = 0;
    token_heap->parsers                 = stream_parsers;
    token_heap->minimum_start_timestamp = UINT64_MAX;

    RmtStreamMergerReset(token_heap);
    return kRmtOk;
}

RmtErrorCode RmtStreamMergerReset(RmtStreamMerger* token_heap)
{
    RMT_RETURN_ON_ERROR(token_heap, kRmtErrorInvalidPointer);

    token_heap->current_size = 0;

    for (int32_t current_rmt_stream_index = 0; current_rmt_stream_index < token_heap->parser_count; ++current_rmt_stream_index)
    {
        // reset each parser.
        RmtErrorCode error_code = RmtParserReset(&token_heap->parsers[current_rmt_stream_index]);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

        // insert first token of each parser
        RmtToken* current_token = &token_heap->buffer[current_rmt_stream_index];
        error_code              = RmtParserAdvance(&token_heap->parsers[current_rmt_stream_index], current_token, NULL);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

        // NOTE: Only apply biasing of the KMD tokens in the advance, its unlikely to
        // cause a problem for the first token out of the trap, and avoids the issue
        // of the start time going negative due to the biasing.

        error_code = Insert(token_heap, current_token);
        RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

        // track the minimum timestamp.
        token_heap->minimum_start_timestamp = RMT_MINIMUM(token_heap->minimum_start_timestamp, current_token->common.timestamp);
    }

    // reset the resource count for deterministic IDs.
    if (token_heap->allocator != NULL)
    {
        token_heap->allocator->resource_count = 0;
    }

    return kRmtOk;
}

bool RmtStreamMergerIsEmpty(const RmtStreamMerger* token_heap)
{
    RMT_ASSERT(token_heap);
    return (token_heap->current_size == 0);
}

// recursive function to find a node by base driver ID
static ResourceIdMapNode* FindResourceNode(ResourceIdMapNode* root, uint64_t base_driver_id)
{
    if (root == nullptr)
    {
        return NULL;
    }

    if (root->base_driver_id == base_driver_id)
    {
        return root;
    }

    if (base_driver_id < root->base_driver_id)
    {
        return FindResourceNode(root->left, base_driver_id);
    }

    return FindResourceNode(root->right, base_driver_id);
}

// recursive function to insert a new node, returns the newly added node
static ResourceIdMapNode* InsertNode(ResourceIdMapAllocator* allocator,
                                     ResourceIdMapNode*      node,
                                     uint64_t                base_driver_id,
                                     RmtResourceIdentifier*  new_resource_id)
{
    if ((node == nullptr) && (allocator != nullptr))
    {
        // create a new node
        ResourceIdMapNode* new_node = allocator->AllocNewNode(base_driver_id);
        RMT_ASSERT(new_node != nullptr);
        if (new_resource_id != nullptr)
        {
            (*new_resource_id) = new_node->unique_id;
        }

        return new_node;
    }

    if (base_driver_id < node->base_driver_id)
    {
        node->left = InsertNode(allocator, node->left, base_driver_id, new_resource_id);
    }
    else if (base_driver_id > node->base_driver_id)
    {
        node->right = InsertNode(allocator, node->right, base_driver_id, new_resource_id);
    }
    else
    {
        // In this case, it means we're replacing a driver resource Id with a new unique Id
        node->unique_id = allocator->GenUniqueId(base_driver_id);
        if (new_resource_id != nullptr)
        {
            (*new_resource_id) = node->unique_id;
        }
    }

    return node;
}

static uint64_t HashId(uint64_t base_driver_id)
{
    return Fnv1aHash(reinterpret_cast<uint8_t*>(&base_driver_id), sizeof(base_driver_id));
}

RmtErrorCode RmtStreamMergerAdvance(RmtStreamMerger* token_heap, RmtToken* out_token)
{
    if (RmtStreamMergerIsEmpty(token_heap))
    {
        return kRmtErrorOutOfMemory;
    }

    // grab the next token from the heap.
    RmtErrorCode error_code = Poll(token_heap, out_token);
    RMT_ASSERT(error_code == kRmtOk);
    RMT_RETURN_ON_ERROR(error_code == kRmtOk, error_code);

    // rebase against the minimum timestamp seen on all heaps.
    out_token->common.timestamp -= token_heap->minimum_start_timestamp;
    if (out_token->common.stream_index == 1)
    {
        out_token->common.timestamp += KMD_TIMESTAMP_BIAS;
    }

    if (token_heap->allocator != nullptr)
    {
        switch (out_token->type)
        {
        case kRmtTokenTypeTimeDelta:
        {
            break;
        }

        case kRmtTokenTypeResourceCreate:
        {
            // When we see a new resource create, we want to create a new map node which will generate a unique resource ID based on our driver
            // provided ID.
            out_token->resource_create_token.original_resource_identifier = out_token->resource_create_token.resource_identifier;
            uint64_t              base_driver_id                 = HashId(out_token->resource_create_token.resource_identifier);
            RmtResourceIdentifier unique_id                      = 0;
            token_heap->map_root                                 = InsertNode(token_heap->allocator, token_heap->map_root, base_driver_id, &unique_id);
            out_token->resource_create_token.resource_identifier = unique_id;
            break;
        }
        case kRmtTokenTypeResourceBind:
        {
            uint64_t           base_driver_id                  = HashId(out_token->resource_bind_token.resource_identifier);
            ResourceIdMapNode* node                            = FindResourceNode(token_heap->map_root, base_driver_id);
            out_token->resource_bind_token.resource_identifier = node != nullptr ? node->unique_id : base_driver_id;
            break;
        }
        case kRmtTokenTypeResourceDestroy:
        {
            uint64_t           base_driver_id                     = HashId(out_token->resource_destroy_token.resource_identifier);
            ResourceIdMapNode* node                               = FindResourceNode(token_heap->map_root, base_driver_id);
            out_token->resource_destroy_token.resource_identifier = node != nullptr ? node->unique_id : base_driver_id;
            break;
        }

        case kRmtTokenTypeUserdata:
        {
            // If an associated ResourceCreate token has been parsed, update the UserData tokens's resource ID to match the
            // unique ID that was generated.

            switch (out_token->userdata_token.userdata_type)
            {
            case kRmtUserdataTypeName:
            case kRmtUserdataTypeCorrelation:
            case kRmtUserdataTypeMarkImplicitResource:
            {
                break;
            }

            default:
                break;
            }
        }

        default:
            // Do nothing for other token types
            break;
        }
    }

    // now get the next token (if there is one) from the stream we just processed a token from, this
    // will ensure there is always 1 token from each stream with outstanding tokens available in the
    // heap for consideration.
    RmtToken* next_token_from_stream = &token_heap->buffer[out_token->common.stream_index];
    *next_token_from_stream          = {};
    error_code                       = RmtParserAdvance(&token_heap->parsers[out_token->common.stream_index], next_token_from_stream, NULL);
    if (error_code == kRmtOk)
    {
        error_code = Insert(token_heap, next_token_from_stream);
        RMT_ASSERT(error_code == kRmtOk);
    }

    // EOF is a valid error code, as that's just a stream ending, anything else is probably bad.
    if (error_code == kRmtEof)
    {
        return kRmtOk;
    }

    return error_code;
}
