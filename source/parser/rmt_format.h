//=============================================================================
// Copyright (c) 2019-2022 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definitions of the RMT format.
//=============================================================================

#ifndef RMV_PARSER_RMT_FORMAT_H_
#define RMV_PARSER_RMT_FORMAT_H_

#include <string.h>  // for memcpy()
#include <stdint.h>

#include "rmt_types.h"

#ifndef _WIN32
#include <stdlib.h>
#endif

/// The maximum number of bytes it will take to hold the maximum number of pages
/// that a <c><i>kRmtTokenTypePageReference</i></c> token can encode.
///
/// NOTE: The real value is 620, this is rounded up to hit a byte boundary.
#define RMT_PAGE_REF_COUNT (624 / 8)

/// There are only 3 bits for the pool size count, so that sets the max number.
#define RMT_MAX_POOLS (1ull << 3)

/// There are 4 preferred heaps.
#define RMT_NUM_HEAP_PREFERENCES (4)

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// An enumeration of all token types supported by RMT.
typedef enum RmtTokenType
{
    kRmtTokenTypeTimestamp         = 0,   ///< The token is a timestamp token.
    kRmtTokenTypeReserved0         = 1,   ///< The token is reserved
    kRmtTokenTypeReserved1         = 2,   ///< The token is reserved
    kRmtTokenTypePageTableUpdate   = 3,   ///< The token is a page table update token.
    kRmtTokenTypeUserdata          = 4,   ///< The token is a user data token.
    kRmtTokenTypeMisc              = 5,   ///< The token is a miscellaneous token.
    kRmtTokenTypeResourceReference = 6,   ///< The token is a resource reference token.
    kRmtTokenTypeResourceBind      = 7,   ///< The token is a resource bind token.
    kRmtTokenTypeProcessEvent      = 8,   ///< The token is a process event token.
    kRmtTokenTypePageReference     = 9,   ///< The token is a page reference token.
    kRmtTokenTypeCpuMap            = 10,  ///< The token is a CPU map token.
    kRmtTokenTypeVirtualFree       = 11,  ///< The token is a virtual free token.
    kRmtTokenTypeVirtualAllocate   = 12,  ///< The token is a virtual allocation token.
    kRmtTokenTypeResourceCreate    = 13,  ///< The token is a resource create token.
    kRmtTokenTypeTimeDelta         = 14,  ///< The token is a time delta token.
    kRmtTokenTypeResourceDestroy   = 15,  ///< The token is a resource destroy token.

    // Add above here.
    kRmtTokenTypeCount
} RmtTokenType;

/// An enumeration of user data types.
typedef enum RmtUserdataType
{
    kRmtUserdataTypeName                 = 0,  ///< The user data contains a string which is relevant to the following sequence of tokens.
    kRmtUserdataTypeSnapshot             = 1,  ///< The user data contains a string which names a specific momemnt in time.
    kRmtUserdataTypeBinary               = 2,  ///< The user data contains a blind binary payload.
    kRmtUserdataTypeReserved             = 3,  ///< Reserved for future expansion.
    kRmtUserdataTypeCorrelation          = 4,  ///< The user data contains 2 32-bit handles for matching resource names to DX12 resources.
    kRmtUserdataTypeMarkImplicitResource = 5   ///< The user data contains a 32-bit resource ID that should be filtered because it was created implicitly.
} RmtUserdataType;

/// An enumeration of miscellaenous events.
typedef enum RmtMiscType
{
    kRmtMiscTypeSubmitGfx        = 0,  ///< An event occurred where work was submited to graphics.
    kRmtMiscTypeSubmitCompute    = 1,  ///< An event occurred where work was submitted to compute.
    kRmtMiscTypeSubmitCopy       = 2,  ///< An event occurred where work was submitted to copy.
    kRmtMiscTypePresent          = 3,  ///< An event occurred where a present happened.
    kRmtMiscTypeInvalidateRanges = 4,  ///< An event occurred where host memory was invalidated.
    kRmtMiscTypeFlushMappedRange = 5,  ///< An event occurred where we flushed local memory.
    kRmtMiscTypeTrimMemory       = 6,  ///< An event occurred where the host operating system asked the driver to trim resident memory.
    kRmtMiscTypeReserved0        = 7,  ///< Reserved for future expansion.
    kRmtMiscTypeReserved1        = 8   ///< Reserved for future expansion.
} RmtMiscType;

/// An enumeration of residency update types.
typedef enum RmtResidencyUpdateType
{
    kRmtResidencyUpdateTypeAdd    = 0,  ///< A reference was added to the resource.
    kRmtResidencyUpdateTypeRemove = 1   ///< A reference was removed from the resource.
} RmtResidencyUpdateType;

/// An enumeration of resource types.
typedef enum RmtResourceType
{
    kRmtResourceTypeImage    = 0,  ///< An image.
    kRmtResourceTypeBuffer   = 1,  ///< A buffer.
    kRmtResourceTypeGpuEvent = 2,  ///< Used by the driver for finer-grained synchronization control.
    kRmtResourceTypeBorderColorPalette =
        3,  ///< When indexing out of bounds of an image from a shader this palette image represents the color values during texture coordinate clamping.
    kRmtResourceTypeIndirectCmdGenerator =
        4,  ///< The resource is used to support indirectly generating command buffers from the GPU. Used to support some variants of ExecuteIndirect.
    kRmtResourceTypeMotionEstimator = 5,   ///< Motion estimator video technology.
    kRmtResourceTypePerfExperiment  = 6,   ///< Used to perform performance experiments in the driver, including SQTT, SPM and traditional, sampled counters.
    kRmtResourceTypeQueryHeap       = 7,   ///< The resource is a heap which contains query results.
    kRmtResourceTypeVideoDecoder    = 8,   ///< The resource is used as the source or destination for a video decoder.
    kRmtResourceTypeVideoEncoder    = 9,   ///< The resource is used as the source or destination for a video encoder.
    kRmtResourceTypeTimestamp       = 10,  ///< The resource is one or more timestamps.
    kRmtResourceTypeHeap            = 11,  ///< The resource is a heap, a larger allocation which can be used for sub-allocations.
    kRmtResourceTypePipeline =
        12,  ///< The resource is a pipeline, i.e.: the combination of GCN/RDNA ISA code together with the state required to compile & use it.
    kRmtResourceTypeDescriptorHeap   = 13,  ///< A specialised type of heap which holds descriptors only. [DX12 only]
    kRmtResourceTypeDescriptorPool   = 14,  ///< A specialised pool containing multiple heaps which hold descriptors only. [Vulkan only]
    kRmtResourceTypeCommandAllocator = 15,  ///< A command allocator resource.
    kRmtResourceTypeMiscInternal     = 16,  ///< A micellaneous internal type of resource.

    // add above this.
    kRmtResourceTypeCount
} RmtResourceType;

/// An enumeration of process events.
typedef enum RmtProcessEventType
{
    kRmtProcessEventTypeStart = 0,  ///< A process was started.
    kRmtProcessEventTypeStop  = 1   ///< A process was stopped.
} RmtProcessEventType;

/// An enumeration of all owner types.
typedef enum RmtOwnerType
{
    kRmtOwnerTypeApplication  = 0,  ///< The owner of the object is the application.
    kRmtOwnerTypePal          = 1,  ///< The owner of the object is PAL.
    kRmtOwnerTypeClientDriver = 2,  ///< The owner of the object is the client driver.
    kRmtOwnerTypeKmd          = 3,  ///< The owner of the object is KMD.

    // add above this.
    kRmtOwnerTypeCount
} RmtOwnerType;

/// An enumeraetion of the various commit types for a resource.
typedef enum RmtCommitType
{
    kRmtCommitTypeCommitted = 0,  ///< The resource was requested to be committed. This means the resource was created with an implicit
                                  ///< heap large enough to contain it. i.e.: The driver stack is being requested to create a full
                                  ///< chain of memory mappings down to the physical page.

    kRmtCommitTypePlaced = 1,  ///< The resource was requested to be a placed resource. This means the resource is bound to a
                               ///< previously allocated heap of memory.

    kRmtCommitTypeVirtual = 2,  ///< The resource was requested to be a virtual resource. This means that there is no physical memory
                                ///< backing the resource. The resource will have its own virtual address range allocated at the time
                                ///< the resource is created, but that virtual address space will not be mapped to an underlaying
                                ///< physical address range.

    kRmtCommitTypeReserved = 3,  ///< Reserved for future expansion.

    // add above this.
    kRmtCommitTypeCount
} RmtCommitType;

/// An enumeration of formats.
typedef enum RmtFormat
{
    kRmtFormatUndefined             = 0,    ///< Format is UNDEFINED.
    kRmtFormatX1Unorm               = 1,    ///< Format is X1 UNORM.
    kRmtFormatX1Uscaled             = 2,    ///< Format is X1 USCALED.
    kRmtFormatX4Y4Unorm             = 3,    ///< Format is X4Y4 UNORM.
    kRmtFormatX4Y4Uscaled           = 4,    ///< Format is X4Y4 USCALED.
    kRmtFormatL4A4Unorm             = 5,    ///< Format is L4A4 UNORM.
    kRmtFormatX4Y4Z4W4Unorm         = 6,    ///< Format is X4Y4Z4W4 UNORM.
    kRmtFormatX4Y4Z4W4Uscaled       = 7,    ///< Format is X4Y4Z4W4 USCALED.
    kRmtFormatX5Y6Z5Unorm           = 8,    ///< Format is X5Y6Z5 UNORM.
    kRmtFormatX5Y6Z5Uscaled         = 9,    ///< Format is X5Y6Z5 USCALED.
    kRmtFormatX5Y5Z5W1Unorm         = 10,   ///< Format is X5Y5Z5W1 UNORM.
    kRmtFormatX5Y5Z5W1Uscaled       = 11,   ///< Format is X5Y5Z5W1 USCALED.
    kRmtFormatX1Y5Z5W5Unorm         = 12,   ///< Format is X1Y5Z5W5 UNORM.
    kRmtFormatX1Y5Z5W5Uscaled       = 13,   ///< Format is X1Y5Z5W5 USCALED.
    kRmtFormatX8Xnorm               = 14,   ///< Format is X8 XNORM.
    kRmtFormatX8Snorm               = 15,   ///< Format is X8 SNORM.
    kRmtFormatX8Uscaled             = 16,   ///< Format is X8 USCALED.
    kRmtFormatX8Sscaled             = 17,   ///< Format is X8 SSCALED.
    kRmtFormatX8Uint                = 18,   ///< Format is X8 UINT.
    kRmtFormatX8Sint                = 19,   ///< Format is X8 SINT.
    kRmtFormatX8Srgb                = 20,   ///< Format is X8 SRGB.
    kRmtFormatA8Unorm               = 21,   ///< Format is A8 UNORM.
    kRmtFormatL8Unorm               = 22,   ///< Format is L8 UNORM.
    kRmtFormatP8Uint                = 23,   ///< Format is P8 UINT.
    kRmtFormatX8Y8Unorm             = 24,   ///< Format is X8Y8 UNORM.
    kRmtFormatX8Y8Snorm             = 25,   ///< Format is X8Y8 SNORM.
    kRmtFormatX8Y8Uscaled           = 26,   ///< Format is X8Y8 USCALED.
    kRmtFormatX8Y8Sscaled           = 27,   ///< Format is X8Y8 SSCALED.
    kRmtFormatX8Y8Uint              = 28,   ///< Format is X8Y8 UINT.
    kRmtFormatX8Y8Sint              = 29,   ///< Format is X8Y8 SINT.
    kRmtFormatX8Y8Srgb              = 30,   ///< Format is X8Y8 SRGB.
    kRmtFormatL8A8Unorm             = 31,   ///< Format is L8A8 UNORM.
    kRmtFormatX8Y8Z8W8Unorm         = 32,   ///< Format is X8Y8Z8W8 UNORM.
    kRmtFormatX8Y8Z8W8Snorm         = 33,   ///< Format is X8Y8Z8W8 SNORM.
    kRmtFormatX8Y8Z8W8Uscaled       = 34,   ///< Format is X8Y8Z8W8 USCALED.
    kRmtFormatX8Y8Z8W8Sscaled       = 35,   ///< Format is X8Y8Z8W8 SSCALED.
    kRmtFormatX8Y8Z8W8Uint          = 36,   ///< Format is X8Y8Z8W8 UINT.
    kRmtFormatX8Y8Z8W8Sint          = 37,   ///< Format is X8Y8Z8W8 SINT.
    kRmtFormatX8Y8Z8W8Srgb          = 38,   ///< Format is X8Y8Z8W8 SRGB.
    kRmtFormatU8V8SnormL8W8Unorm    = 39,   ///< Format is U8V8 SNORM L8W8 UNORM.
    kRmtFormatX10Y11Z11Float        = 40,   ///< Format is X10Y11Z11 FLOAT.
    kRmtFormatX11Y11Z10Float        = 41,   ///< Format is X11Y11Z10 FLOAT.
    kRmtFormatX10Y10Z10W2Unorm      = 42,   ///< Format is X10Y10Z10W2 UNORM.
    kRmtFormatX10Y10Z10W2Snorm      = 43,   ///< Format is X10Y10Z10W2 SNORM.
    kRmtFormatX10Y10Z10W2Uscaled    = 44,   ///< Format is X10Y10Z10W2 USCALED.
    kRmtFormatX10Y10Z10W2Sscaled    = 45,   ///< Format is X10Y10Z10W2 SSCALED.
    kRmtFormatX10Y10Z10W2Uint       = 46,   ///< Format is X10Y10Z10W2 UINT.
    kRmtFormatX10Y10Z10W2Sint       = 47,   ///< Format is X10Y10Z10W2 SINT.
    kRmtFormatX10Y10Z10W2BiasUnorm  = 48,   ///< Format is X10Y10Z10W2BIAS UNORM.
    kRmtFormatU10V10W10SnormA2Unorm = 49,   ///< Format is U10V10W10 SNORM A2 UNORM.
    kRmtFormatX16Unorm              = 50,   ///< Format is X16 UNORM.
    kRmtFormatX16Snorm              = 51,   ///< Format is X16 SNORM.
    kRmtFormatX16Uscaled            = 52,   ///< Format is X16 USCALED.
    kRmtFormatX16Sscaled            = 53,   ///< Format is X16 SSCALED.
    kRmtFormatX16Uint               = 54,   ///< Format is X16 UINT.
    kRmtFormatX16Sint               = 55,   ///< Format is X16 SINT.
    kRmtFormatX16Float              = 56,   ///< Format is X16 FLOAT.
    kRmtFormatL16Unorm              = 57,   ///< Format is L16 UNORM.
    kRmtFormatX16Y16Unorm           = 58,   ///< Format is X16Y16 UNORM.
    kRmtFormatX16Y16Snorm           = 59,   ///< Format is X16Y16 SNORM.
    kRmtFormatX16Y16Uscaled         = 60,   ///< Format is X16Y16 USCALED.
    kRmtFormatX16Y16Sscaled         = 61,   ///< Format is X16Y16 SSCALED.
    kRmtFormatX16Y16Uint            = 62,   ///< Format is X16Y16 UINT.
    kRmtFormatX16Y16Sint            = 63,   ///< Format is X16Y16 SINT.
    kRmtFormatX16Y16Float           = 64,   ///< Format is X16Y16 FLOAT.
    kRmtFormatX16Y16Z16W16Unorm     = 65,   ///< Format is X16Y16Z16W16 UNORM.
    kRmtFormatX16Y16Z16W16Snorm     = 66,   ///< Format is X16Y16Z16W16 SNORM.
    kRmtFormatX16Y16Z16W16Uscaled   = 67,   ///< Format is X16Y16Z16W16 USCALED.
    kRmtFormatX16Y16Z16W16Sscaled   = 68,   ///< Format is X16Y16Z16W16 SSCALED.
    kRmtFormatX16Y16Z16W16Uint      = 69,   ///< Format is X16Y16Z16W16 UINT.
    kRmtFormatX16Y16Z16W16Sint      = 70,   ///< Format is X16Y16Z16W16 SINT.
    kRmtFormatX16Y16Z16W16Float     = 71,   ///< Format is X16Y16Z16W16 FLOAT.
    kRmtFormatX32Uint               = 72,   ///< Format is X32 UINT.
    kRmtFormatX32Sint               = 73,   ///< Format is X32 SINT.
    kRmtFormatX32Float              = 74,   ///< Format is X32 FLOAT.
    kRmtFormatX32Y32Uint            = 75,   ///< Format is X32Y32 UINT.
    kRmtFormatX32Y32Sint            = 76,   ///< Format is X32Y32 SINT.
    kRmtFormatX32Y32Float           = 77,   ///< Format is X32Y32 FLOAT.
    kRmtFormatX32Y32Z32Uint         = 78,   ///< Format is X32Y32Z32 UINT.
    kRmtFormatX32Y32Z32Sint         = 79,   ///< Format is X32Y32Z32 SINT.
    kRmtFormatX32Y32Z32Float        = 80,   ///< Format is X32Y32Z32 FLOAT.
    kRmtFormatX32Y32Z32W32Uint      = 81,   ///< Format is X32Y32Z32W32 UINT.
    kRmtFormatX32Y32Z32W32Sint      = 82,   ///< Format is X32Y32Z32W32 SINT.
    kRmtFormatX32Y32Z32W32Float     = 83,   ///< Format is X32Y32Z32W32 FLOAT.
    kRmtFormatD16UnormS8Uint        = 84,   ///< Format is D16 UNORM S8 UINT.
    kRmtFormatD32UnormS8Uint        = 85,   ///< Format is D32 UNORM S8 UINT.
    kRmtFormatX9Y9Z9E5Float         = 86,   ///< Format is X9Y9Z9E5 FLOAT.
    kRmtFormatBC1Unorm              = 87,   ///< Format is BC1 UNORM.
    kRmtFormatBC1Srgb               = 88,   ///< Format is BC1 SRGB.
    kRmtFormatBC2Unorm              = 89,   ///< Format is BC2 UNORM.
    kRmtFormatBC2Srgb               = 90,   ///< Format is BC2 SRGB.
    kRmtFormatBC3Unorm              = 91,   ///< Format is BC3 UNORM.
    kRmtFormatBC3Srgb               = 92,   ///< Format is BC3 SRGB.
    kRmtFormatBC4Unorm              = 93,   ///< Format is BC4 UNORM.
    kRmtFormatBC4Srgb               = 94,   ///< Format is BC4 SRGB.
    kRmtFormatBC5Unorm              = 95,   ///< Format is BC5 UNORM.
    kRmtFormatBC5Srgb               = 96,   ///< Format is BC5 SRGB.
    kRmtFormatBC6Unorm              = 97,   ///< Format is BC6 UNORM.
    kRmtFormatBC6Srgb               = 98,   ///< Format is BC6 SRGB.
    kRmtFormatBC7Unorm              = 99,   ///< Format is BC7 UNORM.
    kRmtFormatBC7Srgb               = 100,  ///< Format is BC7 SRGB.
    kRmtFormatEtC2X8Y8Z8Unorm       = 101,  ///< Format is ETC2X8Y8Z8 UNORM.
    kRmtFormatEtC2X8Y8Z8Srgb        = 102,  ///< Format is ETC2X8Y8Z8 SRGB.
    kRmtFormatEtC2X8Y8Z8W1Unorm     = 103,  ///< Format is ETC2X8Y8Z8W1 UNORM.
    kRmtFormatEtC2X8Y8Z8W1Srgb      = 104,  ///< Format is ETC2X8Y8Z8W1 SRGB.
    kRmtFormatEtC2X8Y8Z8W8Unorm     = 105,  ///< Format is ETC2X8Y8Z8W8 UNORM.
    kRmtFormatEtC2X8Y8Z8W8Srgb      = 106,  ///< Format is ETC2X8Y8Z8W8 SRGB.
    kRmtFormatEtC2X11Unorm          = 107,  ///< Format is ETC2X11 UNORM.
    kRmtFormatEtC2X11Snorm          = 108,  ///< Format is ETC2X11 SNORM.
    kRmtFormatEtC2X11Y11Unorm       = 109,  ///< Format is ETC2X11Y11 UNORM.
    kRmtFormatEtC2X11Y11Snorm       = 110,  ///< Format is ETC2X11Y11 SNORM.
    kRmtFormatAstcldR4X4Unorm       = 111,  ///< Format is ASTCLDR4X4 UNORM.
    kRmtFormatAstcldR4X4Srgb        = 112,  ///< Format is ASTCLDR4X4 SRGB.
    kRmtFormatAstcldR5X4Unorm       = 113,  ///< Format is ASTCLDR5X4 UNORM.
    kRmtFormatAstcldR5X4Srgb        = 114,  ///< Format is ASTCLDR5X4 SRGB.
    kRmtFormatAstcldR5X5Unorm       = 115,  ///< Format is ASTCLDR5X5 UNORM.
    kRmtFormatAstcldR5X5Srgb        = 116,  ///< Format is ASTCLDR5X5 SRGB.
    kRmtFormatAstcldR6X5Unorm       = 117,  ///< Format is ASTCLDR6X5 UNORM.
    kRmtFormatAstcldR6X5Srgb        = 118,  ///< Format is ASTCLDR6X5 SRGB.
    kRmtFormatAstcldR6X6Unorm       = 119,  ///< Format is ASTCLDR6X6 UNORM.
    kRmtFormatAstcldR6X6Srgb        = 120,  ///< Format is ASTCLDR6X6 SRGB.
    kRmtFormatAstcldR8X5Unorm       = 121,  ///< Format is ASTCLDR8X5 UNORM.
    kRmtFormatAstcldR8X5Srgb        = 122,  ///< Format is ASTCLDR8X5 SRGB.
    kRmtFormatAstcldR8X6Unorm       = 123,  ///< Format is ASTCLDR8X6 UNORM.
    kRmtFormatAstcldR8X6Srgb        = 124,  ///< Format is ASTCLDR8X6 SRGB.
    kRmtFormatAstcldR8X8Unorm       = 125,  ///< Format is ASTCLDR8X8 UNORM.
    kRmtFormatAstcldR8X8Srgb        = 126,  ///< Format is ASTCLDR8X8 SRGB.
    kRmtFormatAstcldR10X5Unorm      = 127,  ///< Format is ASTCLDR10X5 UNORM.
    kRmtFormatAstcldR10X5Srgb       = 128,  ///< Format is ASTCLDR10X5 SRGB.
    kRmtFormatAstcldR10X6Unorm      = 129,  ///< Format is ASTCLDR10X6 UNORM.
    kRmtFormatAstcldR10X6Srgb       = 130,  ///< Format is ASTCLDR10X6 SRGB.
    kRmtFormatAstcldR10X8Unorm      = 131,  ///< Format is ASTCLDR10X8 UNORM.
    kRmtFormatAstcldR10X10Unorm     = 132,  ///< Format is ASTCLDR10X10 UNORM.
    kRmtFormatAstcldR12X10Unorm     = 133,  ///< Format is ASTCLDR12X10 UNORM.
    kRmtFormatAstcldR12X10Srgb      = 134,  ///< Format is ASTCLDR12X10 SRGB.
    kRmtFormatAstcldR12X12Unorm     = 135,  ///< Format is ASTCLDR12X12 UNORM.
    kRmtFormatAstcldR12X12Srgb      = 136,  ///< Format is ASTCLDR12X12 SRGB.
    kRmtFormatAstchdR4x4Float       = 137,  ///< Format is ASTCHDR4x4 FLOAT.
    kRmtFormatAstchdR5x4Float       = 138,  ///< Format is ASTCHDR5x4 FLOAT.
    kRmtFormatAstchdR5x5Float       = 139,  ///< Format is ASTCHDR5x5 FLOAT.
    kRmtFormatAstchdR6x5Float       = 140,  ///< Format is ASTCHDR6x5 FLOAT.
    kRmtFormatAstchdR6x6Float       = 141,  ///< Format is ASTCHDR6x6 FLOAT.
    kRmtFormatAstchdR8x5Float       = 142,  ///< Format is ASTCHDR8x5 FLOAT.
    kRmtFormatAstchdR8x6Float       = 143,  ///< Format is ASTCHDR8x6 FLOAT.
    kRmtFormatAstchdR8x8Float       = 144,  ///< Format is ASTCHDR8x8 FLOAT.
    kRmtFormatAstchdR10x5Float      = 145,  ///< Format is ASTCHDR10x5 FLOAT.
    kRmtFormatAstchdR10x6Float      = 146,  ///< Format is ASTCHDR10x6 FLOAT.
    kRmtFormatAstchdR10x8Float      = 147,  ///< Format is ASTCHDR10x8 FLOAT.
    kRmtFormatAstchdR10x10Float     = 148,  ///< Format is ASTCHDR10x10 FLOAT.
    kRmtFormatAstchdR12x10Float     = 149,  ///< Format is ASTCHDR12x10 FLOAT.
    kRmtFormatAstchdR12x12Float     = 150,  ///< Format is ASTCHDR12x12 FLOAT.
    kRmtFormatX8Y8Z8Y8Unorm         = 151,  ///< Format is X8Y8 Z8Y8 UNORM.
    kRmtFormatX8Y8Z8Y8Uscaled       = 152,  ///< Format is X8Y8 Z8Y8 USCALED.
    kRmtFormatY8X8Y8Z8Unorm         = 153,  ///< Format is Y8X8 Y8Z8 UNORM.
    kRmtFormatY8X8Y8Z8Uscaled       = 154,  ///< Format is Y8X8 Y8Z8 USCALED.
    kRmtFormatAyuv                  = 155,  ///< Format is AYUV.
    kRmtFormatUyvy                  = 156,  ///< Format is UYVY.
    kRmtFormatVyuy                  = 157,  ///< Format is VYUY.
    kRmtFormatYuY2                  = 158,  ///< Format is YUY2.
    kRmtFormatYvY2                  = 159,  ///< Format is YVY2.
    kRmtFormatYV12                  = 160,  ///< Format is YV12.
    kRmtFormatNV11                  = 161,  ///< Format is NV11.
    kRmtFormatNV12                  = 162,  ///< Format is NV12.
    kRmtFormatNV21                  = 163,  ///< Format is NV21.
    kRmtFormatP016                  = 164,  ///< Format is P016.
    kRmtFormatP010                  = 165,  ///< Format is P010.

    // add above this.
    kRmtFormatCount
} RmtFormat;

/// An enumeration of the harware engine types
typedef enum RmtEngineType
{
    kRmtEngineTypeUniversal        = 0,   ///< Engine type is universal.
    kRmtEngineTypeCompute          = 1,   ///< Engine type is compute.
    kRmtEngineTypeExclusiveCompute = 2,   ///< Engine type is exclusive compute.
    kRmtEngineTypeDma              = 3,   ///< Engine type is DMA.
    kRmtEngineTypeTimer            = 4,   ///< Engine type is timer.
    kRmtEngineTypeVceEncode        = 5,   ///< Engine type is VCE encode.
    kRmtEngineTypeUvdDecode        = 6,   ///< Engine type is UVD decode.
    kRmtEngineTypeUvdEncode        = 7,   ///< Engine type is UVD encode.
    kRmtEngineTypeVcnDecode        = 8,   ///< Engine type is VCN decode.
    kRmtEngineTypeVcnEncode        = 9,   ///< Engine type is VCN encode.
    kRmtEngineTypeHP3D             = 10,  ///< Engine type is HP3D.
} RmtEngineType;

/// An enumeration of video decoder types
typedef enum RmtVideoDecoderType
{
    kRmtVideoDecoderTypeH264      = 0,   ///< Decoder type H.264.
    kRmtVideoDecoderTypeVC1       = 1,   ///< Decoder type VC1.
    kRmtVideoDecoderTypeMpeG2Idct = 2,   ///< Decoder type MPEG2 IDCT.
    kRmtVideoDecoderTypeMpeG2Vld  = 3,   ///< Decoder type MPEG2 VLD.
    kRmtVideoDecoderTypeMpeG4     = 4,   ///< Decoder type MPEG4.
    kRmtVideoDecoderTypeWmV9      = 5,   ///< Decoder type WMV9.
    kRmtVideoDecoderTypeMjpeg     = 6,   ///< Decoder type MJPEG.
    kRmtVideoDecoderTypeHvec      = 7,   ///< Decoder type HVEC.
    kRmtVideoDecoderTypeVP9       = 8,   ///< Decoder type VP9.
    kRmtVideoDecoderTypeHevC10Bit = 9,   ///< Decoder type HEVC 10bit.
    kRmtVideoDecoderTypeVP910Bit  = 10,  ///< Decoder type VP 910bit.
    kRmtVideoDecoderTypeAV1       = 11,  ///< Decoder type AV1.
    kRmtVideoDecoderTypeAV1_12Bit = 12,  ///< Decoder type AV1_12BIT.
} RmtVideoDecoderType;

/// An enumeration of video encoder types
typedef enum RmtVideoEncoderType
{
    kRmtVideoEncoderTypeH264 = 0,  ///< Encoder type H.264.
    kRmtVideoEncoderTypeH265 = 1,  ///< Encoder type H.265.
} RmtVideoEncoderType;

/// An enumeration of descriptor types
typedef enum RmtDescriptorType
{
    kRmtDescriptorTypeCsvSrvUav             = 0,   ///< Descriptor type CSV, SRV UAV.
    kRmtDescriptorTypeSampler               = 1,   ///< Descriptor type sampler.
    kRmtDescriptorTypeRtv                   = 2,   ///< Descriptor type RTV.
    kRmtDescriptorTypeDsv                   = 3,   ///< Descriptor type DSV.
    kRmtDescriptorTypeCombinedImageSampler  = 4,   ///< Descriptor type combined image sampler.
    kRmtDescriptorTypeSampledImage          = 5,   ///< Descriptor type sampled image.
    kRmtDescriptorTypeStorageImage          = 6,   ///< Descriptor type storage image.
    kRmtDescriptorTypeUniformTexelBuffer    = 7,   ///< Descriptor type uniform texel buffer.
    kRmtDescriptorTypeStorageTexelBuffer    = 8,   ///< Descriptor type storage texel buffer.
    kRmtDescriptorTypeUniformBuffer         = 9,   ///< Descriptor type uniform buffer.
    kRmtDescriptorTypeStorageBuffer         = 10,  ///< Descriptor type storage buffer.
    kRmtDescriptorTypeUniformBufferDynamic  = 11,  ///< Descriptor type uniform buffer dynamic.
    kRmtDescriptorTypeStorageBufferDynamic  = 12,  ///< Descriptor type storage buffer dynamic.
    kRmtDescriptorTypeInputAttachment       = 13,  ///< Descriptor type input attachment.
    kRmtDescriptorTypeInlineUniformBlock    = 14,  ///< Descriptor type inline uniform block.
    kRmtDescriptorTypeAccelerationStructure = 15,  ///< Descriptor type acceleration structure.
} RmtDescriptorType;

/// An structure encapsulating a descriptor pool description.
typedef struct RmtDescriptorPool
{
    RmtDescriptorType type;             ///< The type of descriptor this pool holds.
    uint32_t          num_descriptors;  ///< The number of descriptors to be allocated by this pool.
} RmtDescriptorPool;

/// An enumeration of flag bits for command allocators.
typedef enum RmtCmdAllocatorFlagsBits
{
    kRmtCmdAllocatorAutoMemoryReuse          = (1 << 0),
    kRmtCmdAllocatorDisableBusyChunkTracking = (1 << 1),
    kRmtCmdAllocatorThreadSafe               = (1 << 2),
} RmtCmdAllocatorFlagsBits;

/// An enumeration of image types.
typedef enum RmtImageType
{
    kRmtImageType1D       = 0,  ///< The iamge is 1 dimensional.
    kRmtImageType2D       = 1,  ///< The iamge is 2 dimensional.
    kRmtImageType3D       = 2,  ///< The iamge is 3 dimensional.
    kRmtImageTypeReserved = 3   ///< Reserved for future expansion.
} RmtImageType;

/// An enumeration of image creation flags.
typedef enum RmtImageCreationFlagBits
{
    kRmtImageCreationFlagInvariant                = (1 << 0),   ///<
    kRmtImageCreationFlagCloneable                = (1 << 1),   ///<
    kRmtImageCreationFlagShareable                = (1 << 2),   ///< Set if the image is owned by another process.
    kRmtImageCreationFlagFlippable                = (1 << 3),   ///<
    kRmtImageCreationFlagStereo                   = (1 << 4),   ///<
    kRmtImageCreationFlagCubemap                  = (1 << 5),   ///<
    kRmtImageCreationFlagPrt                      = (1 << 6),   ///< The image is a partially resident texture.
    kRmtImageCreationFlagReserved0                = (1 << 7),   ///<
    kRmtImageCreationFlagReadSwizzleEquations     = (1 << 8),   ///<
    kRmtImageCreationFlagPerSubresourceInit       = (1 << 9),   ///<
    kRmtImageCreationFlagSeparateDepthAspectRatio = (1 << 10),  ///<
    kRmtImageCreationFlagCopyFormatsMatch         = (1 << 11),  ///<
    kRmtImageCreationFlagRepetitiveResolve        = (1 << 12),  ///<
    kRmtImageCreationFlagPreferSwizzleEquations   = (1 << 13),  ///<
    kRmtImageCreationFlagFixedTileSwizzle         = (1 << 14),  ///<
    kRmtImageCreationFlagVideoReferenceOnly       = (1 << 15),  ///<
    kRmtImageCreationFlagOptimalShareable         = (1 << 16),  ///<
    kRmtImageCreationFlagSampleLocationsKnown     = (1 << 17),  ///<
    kRmtImageCreationFlagFullResolveDestOnly      = (1 << 18),  ///<
    kRmtImageCreationFlagExternalShared           = (1 << 19),  ///< Set if an image is owned by another process.
} RmtImageCreationFlagBits;

/// An enumeration of image usage flags.
typedef enum RmtImageUsageFlagBits
{
    kRmtImageUsageFlagsShaderRead         = (1 << 0),  ///< Image will be read from shader (i.e., texture).
    kRmtImageUsageFlagsShaderWrite        = (1 << 1),  ///< Image will be written from a shader (i.e., UAV).
    kRmtImageUsageFlagsResolveSource      = (1 << 2),  ///< Image will be used as resolve source image.
    kRmtImageUsageFlagsResolveDestination = (1 << 3),  ///< Image will be used as resolve destination image.
    kRmtImageUsageFlagsColorTarget        = (1 << 4),  ///< Image will be bound as a color target.
    kRmtImageUsageFlagsDepthStencil       = (1 << 5),  ///< Image will be bound as a depth/stencil target.
    kRmtImageUsageFlagsNoStencilShaderRead =
        (1
         << 6),  ///< Image will be neither read as stencil nor resolved on stencil aspect. Note that if RESOLVE_SOURCE bit has been set to indicate that the image could be adopted as a resolve source image and there could be stencil resolve, NO_STENCIL_SHADER_READ must be set to 0, since shader-read based stencil resolve might be performed.
    kRmtImageUsageFlagsHiZNeverInvalid =
        (1
         << 7),  ///< Hint to PAL indicating the client will guarantee that no operations performed on this Image while it is in a decompressed state will cause Hi-Z metadata to become invalid. This allows PAL to avoid an expensive re-summarization blit in some resource barriers.
    kRmtImageUsageFlagsDepthAsZ24 =
        (1 << 8),  ///< Use a 24-bit format for HW programming of a native 32-bit surface. If set, border color and Z-reference values are treated as Z-24.
    kRmtImageUsageFlagsFirstShaderWritableMip =
        (1
         << 9),  ///< Only relevant if the SHADER_WRITE flag is set. Typically set to 0 so entire image is writable. If non-zero, such as an image where only level 0 is used as a color target and compute is used to generate mipmaps, PAL may be able to enable additional compression on the baseLevels which are used exclusively as color target and shader read.
    kRmtImageUsageFlagsCornerSampling =
        (1
         << 10),  ///< Set if this image will use corner sampling in image-read scenarios.  With corner sampling, the extent refers to the number of pixel corners which will be one more than the number of pixels.  Border color is ignored when corner sampling is enabled.
    kRmtImageUsageFlagsVrsDepth = (1 << 11)  ///< Set if this depth image will be bound when VRS rendering is enabled.
} RmtImageUsageFlagBits;

/// An enumeration of tiling types.
typedef enum RmtTilingType
{
    kRmtTilingTypeLinear          = 0,  ///< Image is linear.
    kRmtTilingTypeOptimal         = 1,  ///< Image is tiled optimally for hardware.
    kRmtTilingTypeStandardSwizzle = 2,  ///< Imgae is tiled using API specified swizzling pattern.
    kRmtTilingTypeReserved        = 3   ///< Reserved for future expansion.
} RmtTilingType;

/// An enumeration of optimization tiling modes.
typedef enum RmtTilingOptimizationMode
{
    kRmtTilingOptimizationModeBalanced = 0,  ///< Tiling mode is balanced.
    kRmtTilingOptimizationModeSpace    = 1,  ///< Tiling mode is optimized for space.
    kRmtTilingOptimizationModeSpeed    = 2,  ///< Tiling mode is optimized for speed.
    kRmtTilingOptimizationModeReserved = 3   ///< Reserved for future expansion.
} RmtTilingOptimizationMode;

/// An enumeration of metadata modes.
typedef enum RmtMetadataMode
{
    kRmtMetadataModeDefault           = 0,  ///< The default metadata mode.
    kRmtMetadataModeOptForTexPrefetch = 1,  ///< The metadata mode is optimized for texture prefetch.
    kRmtMetadataModeDisabled          = 2,  ///< The metadata mode is disabled.
    kRmtMetadataModeReserved          = 3   ///< Reserved for future expansion.
} RmtMetadataMode;

/// An enumeration of channel swizzle values.
typedef enum RmtChannelSwizzle
{
    kRmtSwizzleZero      = 0,  ///< Hardwired zero value.
    kRmtSwizzleOne       = 1,  ///< Hardwired one value.
    kRmtSwizzleX         = 2,  ///< Read X channel from source image.
    kRmtSwizzleY         = 3,  ///< Read Y channel from source image.
    kRmtSwizzleZ         = 4,  ///< Read Z channel from source image.
    kRmtSwizzleW         = 5,  ///< Read W channel from source image.
    kRmtSwizzleReserved0 = 6,  ///< Reserved for future expansion.
    kRmtSwizzleReserved1 = 7   ///< Reserved for future expansion.
} RmtChannelSwizzle;

/// An enumeration of all creation flags for buffers.
typedef enum RmtBufferCreationFlagBits
{
    kRmtBufferCreationFlagSparseBinding              = (1 << 0),
    kRmtBufferCreationFlagSparseResidency            = (1 << 1),
    kRmtBufferCreationFlagSparseAliasing             = (1 << 2),
    kRmtBufferCreationFlagProtected                  = (1 << 3),
    kRmtBufferCreationFlagDeviceAddressCaptureReplay = (1 << 4)
} RmtBufferCreationFlagBits;

/// An enumeration of all usage flags for buffers.
typedef enum RmtBufferUsageFlagBits
{
    kRmtBufferUsageFlagTransferSource                 = (1 << 0),   ///< The buffer is used as a transfer source.
    kRmtBufferUsageFlagTransferDestination            = (1 << 1),   ///< The buffer is used as a transafer destination.
    kRmtBufferUsageFlagUniformTexelBuffer             = (1 << 2),   ///<
    kRmtBufferUsageFlagStorageTexelBuffer             = (1 << 3),   ///<
    kRmtBufferUsageFlagUniformBuffer                  = (1 << 4),   ///< The buffer is used a uniform/constant buffer.
    kRmtBufferUsageFlagStorageBuffer                  = (1 << 5),   ///<
    kRmtBufferUsageFlagIndexBuffer                    = (1 << 6),   ///< The buffer is used as an index buffer.
    kRmtBufferUsageFlagVertexBuffer                   = (1 << 7),   ///< The buffer is used as a vertex buffer.
    kRmtBufferUsageFlagIndirectBuffer                 = (1 << 8),   ///< The buffer is used to store indirect arguments for draw calls/dispatches.
    kRmtBufferUsageFlagTransformFeedbackBuffer        = (1 << 9),   ///<
    kRmtBufferUsageFlagTransformFeedbackCounterBuffer = (1 << 10),  ///<
    kRmtBufferUsageFlagConditionalRendering           = (1 << 11),  ///<
    kRmtBufferUsageFlagRayTracing                     = (1 << 12),  ///<
    kRmtBufferUsageFlagShaderDeviceAddress            = (1 << 13)   ///<
} RmtBufferUsageFlagBits;

/// An enumeration of all query heap types.
typedef enum RmtQueryHeapType
{
    kRmtQueryHeapTypeOcclusion        = 0,  ///< The heap is used for occlusion queries.
    kRmtQueryHeapTypePipelineStats    = 1,  ///< The heap is used for pipeline stats.
    kRmtQueryHeapTypeStreamoutStats   = 2,  ///< The heap is used for streamout stats.
    kRmtQueryHeapTypeVideoDecodeStats = 3   ///< The heap is used for video decoder stats.
} RmtQueryHeapType;

/// An enumeration of all the pipeline stage bits.
typedef enum RmtPipelineStageBits
{
    kRmtPipelineStageMaskPs = (1 << 0),  ///< The pipeline ran on the PS stage.
    kRmtPipelineStageMaskHs = (1 << 1),  ///< The pipeline ran on the HS stage.
    kRmtPipelineStageMaskDs = (1 << 2),  ///< The pipeline ran on the DS stage.
    kRmtPipelineStageMaskVs = (1 << 3),  ///< The pipeline ran on the VS stage.
    kRmtPipelineStageMaskGs = (1 << 4),  ///< The pipeline ran on the GS stage.
    kRmtPipelineStageMaskCs = (1 << 5),  ///< The pipeline ran on the CS stage.
    kRmtPipelineStageMaskTs = (1 << 6),  ///< The pipeline ran on the TS stage.
    kRmtPipelineStageMaskMs = (1 << 7)   ///< The pipeline ran on the MS stage.
} RmtPipelineStageBits;

/// An enumeration of all pipeline creation flags.
typedef enum RmtPipelineCreateFlagBits
{
    kRmtPipelineCreateFlagInternal        = (1 << 0),  ///< The pipeline was created for internal use.
    kRmtPipelineCreateFlagOverrideGpuHeap = (1 << 1),  ///< The pipeline should override the default heap.
    kRmtPipelineCreateFlagReserved0       = (1 << 2),  ///< Reserved for future expansion.
    kRmtPipelineCreateFlagReserved1       = (1 << 3),  ///< Reserved for future expansion.
    kRmtPipelineCreateFlagReserved2       = (1 << 4),  ///< Reserved for future expansion.
    kRmtPipelineCreateFlagReserved3       = (1 << 5),  ///< Reserved for future expansion.
    kRmtPipelineCreateFlagReserved4       = (1 << 6),  ///< Reserved for future expansion.
    kRmtPipelineCreateFlagReserved5       = (1 << 7)   ///< Reserved for future expansion.
} RmtPipelineCreateFlagBits;

/// An enumeration of flag bits for GPU events.
typedef enum RmtGpuEventFlagBits
{
    kRmtGpuEventFlagGpuOnly = (1 << 0)  ///< Event is only used from the GPU.
} RmtGpuEventFlagBits;

/// An enumeration of all internal resource types.
typedef enum RmtResourceMiscInternalType
{
} RmtResourceMiscInternalType;

/// A structure encapsulating the full image format.
typedef struct RmtImageFormat
{
    RmtChannelSwizzle swizzle_x;  ///< The swizzle applied to the X channel.
    RmtChannelSwizzle swizzle_y;  ///< The swizzle applied to the Y channel.
    RmtChannelSwizzle swizzle_z;  ///< The swizzle applied to the Z channel.
    RmtChannelSwizzle swizzle_w;  ///< The swizzle applied to the W channel.
    RmtFormat         format;     ///< The format of the channels.
} RmtImageFormat;

/// A structure encapsulating the resource description for an <c><i>kRmtResourceTypeImage</i></c>.
typedef struct RmtResourceDescriptionImage
{
    uint32_t                  create_flags;              ///< Flags describing how the image was created.
    uint32_t                  usage_flags;               ///< Flags deccribing how the image is used.
    RmtImageType              image_type;                ///< The type of the image.
    int32_t                   dimension_x;               ///< The width of the image [1..4096].
    int32_t                   dimension_y;               ///< The height of the image [1...4096].
    int32_t                   dimension_z;               ///< The depth of the image [1...4096].
    RmtImageFormat            format;                    ///< The image format.
    int32_t                   mip_levels;                ///< The number of mip-map levels the image contains.
    int32_t                   slices;                    ///< The number of slices the image contains.
    int32_t                   sample_count;              ///< The number of samples.
    int32_t                   fragment_count;            ///< The number of fragments.
    RmtTilingType             tiling_type;               ///< The tiling type used to create the image.
    RmtTilingOptimizationMode tiling_optimization_mode;  ///< The tiling optimization mode.
    RmtMetadataMode           metadata_mode;             ///< The metadata mode for the image.
    uint64_t                  max_base_alignment;        ///< The alignment of the image resource.
    uint64_t                  image_offset;              ///< The offset (in bytes) from the base virtual address of the resource to the core image data.
    uint64_t                  image_size;                ///< The size (in bytes) of the core image data inside the resource.
    uint64_t                  image_alignment;           ///< The alignment of the core image data within the resource's virtual address allocation.
    uint64_t metadata_head_offset;     ///< The offset (in bytes) from the base virtual address of the resource to the metadata header of the image.
    uint64_t metadata_head_size;       ///< The size (in bytes) of the metadata header inside the resource.
    uint64_t metadata_head_alignment;  ///< The alignment of the metadata header within the resource's virtual address allocation.
    uint64_t metadata_tail_offset;     ///< The offset (in bytes) from the base virtual address of the resource to the metadata tail.
    uint64_t metadata_tail_size;       ///< The size (in bytes) of the metadata tail inside the resource.
    uint64_t metadata_tail_alignment;  ///< The alignment of the metadata tail within the resource's virtual address allocation.
    bool     presentable;              ///< The image is able to be presented by the display controller.
    bool     fullscreen;               ///< The image is fullscreen, only valid when <c><i>presentable</i></c> is <c><i>true</i></c>.
} RmtResourceDescriptionImage;

/// A structure encapsulating the resource description for an <c><i>kRmtResourceTypeBuffer</i></c>.
typedef struct RmtResourceDescriptionBuffer
{
    uint32_t create_flags;   ///< Flags describing how the buffer was created.
    uint32_t usage_flags;    ///< Flags deccribing how the image is used.
    uint64_t size_in_bytes;  ///< The size (in bytes) of the buffer.
} RmtResourceDescriptionBuffer;

/// A structure encapsulating the resource description for an <c><i>kRmtResourceTypeGpuEvent</i></c>.
typedef struct RmtResourceDescriptionGpuEvent
{
    uint32_t flags;  ///< Flags describing the GPU event.
} RmtResourceDescriptionGpuEvent;

/// A structure encapsulating the resource description for an <c><i>kRmtResourceTypeBorderColorPalette</i></c>.
typedef struct RmtResourceDescriptionBorderColorPalette
{
    uint32_t size_in_entries;  ///< The number of entries in the palette.
} RmtResourceDescriptionBorderColorPalette;

/// A structure encapsulating the resource description for an <c><i>kRmtResourceTypePerfExperiment</i></c>.
typedef struct RmtResourceDescriptionPerfExperiment
{
    uint64_t spm_size;      ///< The size of the perf experiment memory used for SPM counters.
    uint64_t sqtt_size;     ///< The size of the perf experiment memory used for SQTT counters.
    uint64_t counter_size;  ///< The size of the perf experiment memory used for regular counters.
} RmtResourceDescriptionPerfExperiment;

/// A structure encapsulating the resource description for an <c><i>kRmtResourceTypeQueryHeap</i></c>.
typedef struct RmtResourceDescriptionQueryHeap
{
    RmtQueryHeapType heap_type;          ///< The type of the query heap.
    bool             enable_cpu_access;  ///< Set to true if CPU access is allowed.
} RmtResourceDescriptionQueryHeap;

/// A structure encapsulating a resource description for an <c><i>kRmtResourceTypeVideoDecoder</i></c>.
typedef struct RmtResourceDescriptionVideoDecoder
{
    RmtEngineType       engine_type;   ///< The hardware engine type.
    RmtVideoDecoderType decoder_type;  ///< The video decoder type.
    uint32_t            width;         ///< The width of the video.
    uint32_t            height;        ///< The height of the video.
} RmtResourceDescription;

/// A structure encapsulating a resource description for an <c><i>kRmtResourceTypeVideoEncoder</i></c>.
typedef struct RmtResourceDescriptionVideoEncoder
{
    RmtEngineType       engine_type;   ///< The hardware engine type.
    RmtVideoEncoderType encoder_type;  ///< The video encoder type.
    uint16_t            width;         ///< The width of the video.
    uint16_t            height;        ///< The height of the video.
    RmtImageFormat      format;        ///< The video image format.
} RmtResourceDescriptionVideoEncoder;

/// A structure encapsulating a resource description for an <c><i>kRmtResourceTypeDescriptorHeap</i></c>.
typedef struct RmtResourceDescriptionHeap
{
    uint8_t     flags;          ///< The heap creation flags.
    uint64_t    size;           ///< The size of the heap, in bytes.
    RmtPageSize alignment;      ///< The alignment of the heap, expressed as a page size.
    uint8_t     segment_index;  ///< The segment index where the heap was requested to be created.
} RmtResourceDescriptionHeap;

/// A structure encapsulating the resource description for an <c><i>kRmtResourceTypePipeline</i></c>.
typedef struct RmtResourceDescriptionPipeline
{
    uint64_t internal_pipeline_hash_hi;  ///< Hi bits of the internal pipeline hash.
    uint64_t internal_pipeline_hash_lo;  ///< Lo bits of the internal pipeline hash.
    uint32_t create_flags;               ///< Create flags for the pipeline.
    uint32_t stage_mask;                 ///< A bitfield representing which stages are active for this pipeline.
    bool     is_ngg;                     ///< Set to true if NGG is enabled.
} RmtResourceDescriptionPipeline;

typedef struct RmtResourceDescriptionDescriptorHeap
{
    RmtDescriptorType descriptor_type;  ///< The type of descriptors in the descriptor heap.
    bool              shader_visible;   ///< Set to true if the heap is shader-visible.
    uint8_t           gpu_mask;         ///< For multiple adapter mode, this is a bitmask indicating which adapters the heap applies to.
    uint16_t          num_descriptors;  ///< The number of descriptors in the heap.
} RmtResourceDescriptionDescriptorHeap;

typedef struct RmtResourceDescriptionDescriptorPool
{
    uint16_t          max_sets;              ///< Maximum number of descriptor sets that can be allocated from the pool.
    uint8_t           pools_count;           ///< The number of pool descriptions in the pools[] array.
    RmtDescriptorPool pools[RMT_MAX_POOLS];  ///< Array of pool descriptions.
} RmtResourceDescriptionDescriptorPool;

typedef struct RmtResourceDescriptionCommandAllocator
{
    uint8_t     flags;          ///< Create flags for the command allocator.
    RmtHeapType cmd_data_heap;  ///< The preferred allocation heap for executable command data.
    uint64_t
        cmd_data_size;  ///< The size, in bytes, of the base memory allocations the command allocator will make for executable command data. Expressed as 4Kb chunks.
    uint64_t
                cmd_data_suballoc_size;  ///< The size, in bytes, of the chunks the command allocator will give to command buffers for executable command data. Expressed as 4Kb chunks.
    RmtHeapType embed_data_heap;  ///< The preferred allocation heap for embedded command data.
    uint64_t
        embed_data_size;  ///< The size, in bytes, of the base memory allocations the command allocator will make for embedded command data. Expressed as 4Kb chunks.
    uint64_t
                embed_data_suballoc_size;  ///< The size, in bytes, of the chunks the command allocator will give to command buffers for embedded command data. Expressed as 4Kb chunks.
    RmtHeapType gpu_scratch_heap;  ///< The preferred allocation heap for GPU scratch memory.
    uint64_t
        gpu_scratch_size;  ///< The size, in bytes, of the base memory allocations the command allocator will make for GPU scratch memory. Expressed as 4Kb chunks.
    uint64_t
        gpu_scratch_suballoc_size;  ///< The size, in bytes, of the chunks the command allocator will give to command buffers for GPU scratch memory. Expressed as 4Kb chunks.
} RmtResourceDescriptionCommandAllocator;

/// A structure encapsulating the resource description for a miscellaneous internal resource.
typedef struct RmtResourceDescriptionMiscInternal
{
    RmtResourceMiscInternalType type;  ///< The type of the miscellaneous internal resource.
} RmtResourceDescriptionMiscInternal;

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
    /// @brief Constructor for the RmtTokenUserdata.
    RmtTokenUserdata();

    /// @brief Destructor for the RmtTokenUserdata.
    ~RmtTokenUserdata();

    /// @brief RmtTokenUserdata assignment operator.
    ///
    /// @param [in] object The source RmtTokenUserdata.
    ///
    /// @return A reference to the updated RmtTokenUserdata.
    RmtTokenUserdata& operator=(const RmtTokenUserdata& object);

    RmtTokenCommon  common;         ///< Fields common to all tokens.
    RmtUserdataType userdata_type;  ///< The type of the user data in the payload.
    int32_t         size_in_bytes;  ///< The size (in bytes) of the payload. The largest we can encode is 1MB.
    uint8_t*        payload_cache;  ///< Pointer to the payload of the user data.

    RmtResourceIdentifier
        resource_identifier;  ///< The identifier used to match a name to a non-DX resource, only valid when usedataType is RMT_USERDATA_TYPE_NAME.
    RmtCorrelationIdentifier correlation_identifier;        ///< The identifier used to match correlation ID for DX traces.
    RmtResourceIdentifier    original_resource_identifier;  ///< The Original Resource ID contained in the RESOURCE_CREATE token.
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
        RmtResourceDescriptionMiscInternal     misc_internal;      ///< Valid when <c><i>resourceType</i></c> is <c><i>RMT_RESOURCE_TYPE_MISC_INTERNAL</i></c>.
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
} RmtTokenResourceUnbind;

/// @brief Allocate memory for the USERDATA token payload cache.
///
/// @param [in] size The number of bytes to allocate.
///
/// @return A pointer to the allocated memory.
uint8_t* AllocatePayloadCache(size_t size);

/// @brief Deallocate memory previously allocated for the USERDATA token payload cache.
///
/// @param [in] payload_cache A pointer to the memory to be deallocated.
///
/// @return The number of rows.
void DeallocatePayloadCache(uint8_t* payload_cache);

/// A structure encapsulating the token.
typedef struct RmtToken
{
    /// @brief RmtToken constructor.
    RmtToken();

    /// @brief RmtToken destructor.
    ~RmtToken();

    /// @brief RmtToken assignment operator.
    ///
    /// @param [in] object The source RmtToken.
    ///
    /// @return A reference to the updated RmtToken.
    RmtToken& operator=(const RmtToken& object);

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
    };

} RmtToken;

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_PARSER_RMT_FORMAT_H_
