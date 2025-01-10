//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of printing helper functions for RMT.
//=============================================================================

#include "rmt_print.h"

#include <stdarg.h>
#include <stdio.h>   // for sprintf
#include <string.h>  // for strcat

#ifndef _WIN32
#include "linux/safe_crt.h"
#else
#include <Windows.h>
#endif

// The printing callback function.
static RmtPrintingCallback printing_func       = nullptr;
static bool                is_printing_enabled = true;

const char* RmtGetPageSizeNameFromPageSize(RmtPageSize page_size)
{
    switch (page_size)
    {
    case kRmtPageSizeUnmapped:
        return "UNMAPPED";
    case kRmtPageSize4Kb:
        return "4KB";
    case kRmtPageSize64Kb:
        return "64KB";
    case kRmtPageSize256Kb:
        return "256KB";
    case kRmtPageSize1Mb:
        return "1MB";
    case kRmtPageSize2Mb:
        return "2MB";
    default:
        return "Unknown";
    }
}

const char* RmtGetResourceTypeNameFromResourceType(RmtResourceType resource_type)
{
    switch (resource_type)
    {
    case kRmtResourceTypeImage:
        return "IMAGE";
    case kRmtResourceTypeBuffer:
        return "BUFFER";
    case kRmtResourceTypeGpuEvent:
        return "GPU_EVENT";
    case kRmtResourceTypeBorderColorPalette:
        return "BORDER_COLOR_PALETTE";
    case kRmtResourceTypeIndirectCmdGenerator:
        return "INDIRECT_CMD_GENERATOR";
    case kRmtResourceTypeMotionEstimator:
        return "MOTION_ESTIMATOR";
    case kRmtResourceTypePerfExperiment:
        return "PERF_EXPERIMENT";
    case kRmtResourceTypeQueryHeap:
        return "QUERY_HEAP";
    case kRmtResourceTypeVideoDecoder:
        return "VIDEO_DECODER";
    case kRmtResourceTypeVideoEncoder:
        return "VIDEO_ENCODER";
    case kRmtResourceTypeTimestamp:
        return "TIMESTAMP";
    case kRmtResourceTypeHeap:
        return "HEAP";
    case kRmtResourceTypePipeline:
        return "PIPELINE";
    case kRmtResourceTypeDescriptorHeap:
        return "DESCRIPTOR_HEAP";
    case kRmtResourceTypeDescriptorPool:
        return "DESCRIPTOR_POOL";
    case kRmtResourceTypeCommandAllocator:
        return "CMD_ALLOCATOR";
    case kRmtResourceTypeMiscInternal:
        return "MISC_INTERNAL";
    default:
        return "Unknown";
    }
}

// Get a resource's usage type as a string.
const char* RmtGetResourceUsageTypeNameFromResourceUsageType(const RmtResourceUsageType usage_type)
{
    switch (usage_type)
    {
    case kRmtResourceUsageTypeDepthStencil:
        return "Depth stencil texture";

    case kRmtResourceUsageTypeRenderTarget:
        return "Render target";

    case kRmtResourceUsageTypeTexture:
        return "Texture";

    case kRmtResourceUsageTypeRayTracingBuffer:
        return "Acceleration structure";

    case kRmtResourceUsageTypeShaderPipeline:
        return "Shader pipeline";

    case kRmtResourceUsageTypeCommandBuffer:
        return "Command buffer";

    case kRmtResourceUsageTypeHeap:
        return "Heap";

    case kRmtResourceUsageTypeDescriptors:
        return "Descriptors";

    case kRmtResourceUsageTypeBuffer:
        return "Buffer";

    case kRmtResourceUsageTypeFree:
        return "Unbound";

    case kRmtResourceUsageTypeGpuEvent:
        return "Event";

    case kRmtResourceUsageTypeInternal:
        return "Internal";

    default:
        return "Unknown";
    }
}

const char* RmtGetCommitTypeNameFromCommitType(RmtCommitType commit_type)
{
    switch (commit_type)
    {
    case kRmtCommitTypeCommitted:
        return "COMMITTED";
    case kRmtCommitTypePlaced:
        return "PLACED";
    case kRmtCommitTypeVirtual:
        return "VIRTUAL";
    default:
        return "Unknown";
    }
}

const char* RmtGetOwnerTypeNameFromOwnerType(RmtOwnerType owner_type)
{
    switch (owner_type)
    {
    case kRmtOwnerTypeApplication:
        return "Application";
    case kRmtOwnerTypePal:
        return "PAL";
    case kRmtOwnerTypeClientDriver:
        return "ClientDriver";
    case kRmtOwnerTypeKmd:
        return "KMD";
    default:
        return "Unknown";
    }
}

const char* RmtGetMiscTypeNameFromMiscType(RmtMiscType misc_type)
{
    switch (misc_type)
    {
    case kRmtMiscTypeFlushMappedRange:
        return "FLUSH_MAPPED_RAGE";
    case kRmtMiscTypeInvalidateRanges:
        return "INVALIDATE_RANGES";
    case kRmtMiscTypePresent:
        return "PRESENT";
    case kRmtMiscTypeSubmitCompute:
        return "SUBMIT_COMPUTE";
    case kRmtMiscTypeSubmitCopy:
        return "SUBMIT_COPY";
    case kRmtMiscTypeSubmitGfx:
        return "SUBMIT_GFX";
    case kRmtMiscTypeTrimMemory:
        return "TRIM_MEMORY";
    default:
        return "Unknown";
    }
}

const char* RmtGetProcessEventNameFromProcessEvent(RmtProcessEventType process_event)
{
    switch (process_event)
    {
    case kRmtProcessEventTypeStart:
        return "START";
    case kRmtProcessEventTypeStop:
        return "STOP";
    default:
        return "Unknown";
    }
}

const char* RmtGetHeapTypeNameFromHeapType(RmtHeapType heap_type)
{
    switch (heap_type)
    {
    case kRmtHeapTypeLocal:
        return "Local";
    case kRmtHeapTypeInvisible:
        return "Invisible";
    case kRmtHeapTypeSystem:
        return "Host";
    case kRmtHeapTypeNone:
        return "Unspecified";
    default:
        return "Unknown";
    }
}

const char* RmtGetPageTableUpdateTypeNameFromPageTableUpdateType(RmtPageTableUpdateType update_type)
{
    switch (update_type)
    {
    case kRmtPageTableUpdateTypeDiscard:
        return "DISCARD";
    case kRmtPageTableUpdateTypeUpdate:
        return "UPDATE";
    case kRmtPageTableUpdateTypeTransfer:
        return "TRANSFER";
    case kRmtPageTableUpdateTypeReserved:
        return "RESERVED";
    default:
        return "Unknown";
    }
}

const char* RmtGetTokenNameFromTokenType(RmtTokenType token_type)
{
    switch (token_type)
    {
    case kRmtTokenTypeCpuMap:
        return "CPU_MAP";
    case kRmtTokenTypeMisc:
        return "MISC";
    case kRmtTokenTypePageReference:
        return "PAGE_REFERENCE";
    case kRmtTokenTypePageTableUpdate:
        return "PAGE_TABLE_UPDATE";
    case kRmtTokenTypeProcessEvent:
        return "PROCESS_EVENT";
    case kRmtTokenTypeResourceBind:
        return "RESOURCE_BIND";
    case kRmtTokenTypeResourceCreate:
        return "RESOURCE_CREATE";
    case kRmtTokenTypeResourceDestroy:
        return "RESOURCE_DESTROY";
    case kRmtTokenTypeResourceReference:
        return "RESOURCE_REFERENCE";
    case kRmtTokenTypeTimestamp:
        return "TIMESTAMP";
    case kRmtTokenTypeTimeDelta:
        return "TIME_DELTA";
    case kRmtTokenTypeUserdata:
        return "USERDATA";
    case kRmtTokenTypeVirtualAllocate:
        return "VIRTUAL_ALLOCATE";
    case kRmtTokenTypeVirtualFree:
        return "VIRTUAL_FREE";
    default:
        return "Unknown";
    }
}

// get the name of a format.
const char* RmtGetFormatNameFromFormat(RmtFormat format)
{
    switch (format)
    {
    case kRmtFormatUndefined:
        return "UNDEFINED";
    case kRmtFormatX1Unorm:
        return "X1_UNORM";
    case kRmtFormatX1Uscaled:
        return "X1_USCALED";
    case kRmtFormatX4Y4Unorm:
        return "X4Y4_UNORM";
    case kRmtFormatX4Y4Uscaled:
        return "X4Y4_USCALED";
    case kRmtFormatL4A4Unorm:
        return "L4A4_UNORM";
    case kRmtFormatX4Y4Z4W4Unorm:
        return "X4Y4Z4W4_UNORM";
    case kRmtFormatX4Y4Z4W4Uscaled:
        return "X4Y4Z4W4_USCALED";
    case kRmtFormatX5Y6Z5Unorm:
        return "X5Y6Z5_UNORM";
    case kRmtFormatX5Y6Z5Uscaled:
        return "X5Y6Z5_USCALED";
    case kRmtFormatX5Y5Z5W1Unorm:
        return "X5Y5Z5W1_UNORM";
    case kRmtFormatX5Y5Z5W1Uscaled:
        return "X5Y5Z5W1_USCALED";
    case kRmtFormatX1Y5Z5W5Unorm:
        return "X1Y5Z5W5_UNORM";
    case kRmtFormatX1Y5Z5W5Uscaled:
        return "X1Y5Z5W5_USCALED";
    case kRmtFormatX8Unorm:
        return "X8_UNORM";
    case kRmtFormatX8Snorm:
        return "X8_SNORM";
    case kRmtFormatX8Uscaled:
        return "X8_USCALED";
    case kRmtFormatX8Sscaled:
        return "X8_SSCALED";
    case kRmtFormatX8Uint:
        return "X8_UINT";
    case kRmtFormatX8Sint:
        return "X8_SINT";
    case kRmtFormatX8Srgb:
        return "X8_SRGB";
    case kRmtFormatA8Unorm:
        return "A8_UNORM";
    case kRmtFormatL8Unorm:
        return "L8_UNORM";
    case kRmtFormatP8Unorm:
        return "P8_UNORM";
    case kRmtFormatX8Y8Unorm:
        return "X8Y8_UNORM";
    case kRmtFormatX8Y8Snorm:
        return "X8Y8_SNORM";
    case kRmtFormatX8Y8Uscaled:
        return "X8Y8_USCALED";
    case kRmtFormatX8Y8Sscaled:
        return "X8Y8_SSCALED";
    case kRmtFormatX8Y8Uint:
        return "X8Y8_UINT";
    case kRmtFormatX8Y8Sint:
        return "X8Y8_SINT";
    case kRmtFormatX8Y8Srgb:
        return "X8Y8_SRGB";
    case kRmtFormatL8A8Unorm:
        return "L8A8_UNORM";
    case kRmtFormatX8Y8Z8W8Unorm:
        return "X8Y8Z8W8_UNORM";
    case kRmtFormatX8Y8Z8W8Snorm:
        return "X8Y8Z8W8_SNORM";
    case kRmtFormatX8Y8Z8W8Uscaled:
        return "X8Y8Z8W8_USCALED";
    case kRmtFormatX8Y8Z8W8Sscaled:
        return "X8Y8Z8W8_SSCALED";
    case kRmtFormatX8Y8Z8W8Uint:
        return "X8Y8Z8W8_UINT";
    case kRmtFormatX8Y8Z8W8Sint:
        return "X8Y8Z8W8_SINT";
    case kRmtFormatX8Y8Z8W8Srgb:
        return "X8Y8Z8W8_SRGB";
    case kRmtFormatU8V8SnormL8W8Unorm:
        return "U8V8_SNORM_L8W8_UNORM";
    case kRmtFormatX10Y11Z11Float:
        return "X10Y11Z11_FLOAT";
    case kRmtFormatX11Y11Z10Float:
        return "X11Y11Z10_FLOAT";
    case kRmtFormatX10Y10Z10W2Unorm:
        return "X10Y10Z10W2_UNORM";
    case kRmtFormatX10Y10Z10W2Snorm:
        return "X10Y10Z10W2_SNORM";
    case kRmtFormatX10Y10Z10W2Uscaled:
        return "X10Y10Z10W2_USCALED";
    case kRmtFormatX10Y10Z10W2Sscaled:
        return "X10Y10Z10W2_SSCALED";
    case kRmtFormatX10Y10Z10W2Uint:
        return "X10Y10Z10W2_UINT";
    case kRmtFormatX10Y10Z10W2Sint:
        return "X10Y10Z10W2_SINT";
    case kRmtFormatX10Y10Z10W2BiasUnorm:
        return "X10Y10Z10W2BIAS_UNORM";
    case kRmtFormatU10V10W10SnormA2Unorm:
        return "U10V10W10_SNORM_A2_UNORM";
    case kRmtFormatX16Unorm:
        return "X16_UNORM";
    case kRmtFormatX16Snorm:
        return "X16_SNORM";
    case kRmtFormatX16Uscaled:
        return "X16_USCALED";
    case kRmtFormatX16Sscaled:
        return "X16_SSCALED";
    case kRmtFormatX16Uint:
        return "X16_UINT";
    case kRmtFormatX16Sint:
        return "X16_SINT";
    case kRmtFormatX16Float:
        return "X16_FLOAT";
    case kRmtFormatL16Unorm:
        return "L16_UNORM";
    case kRmtFormatX16Y16Unorm:
        return "X16Y16_UNORM";
    case kRmtFormatX16Y16Snorm:
        return "X16Y16_SNORM";
    case kRmtFormatX16Y16Uscaled:
        return "X16Y16_USCALED";
    case kRmtFormatX16Y16Sscaled:
        return "X16Y16_SSCALED";
    case kRmtFormatX16Y16Uint:
        return "X16Y16_UINT";
    case kRmtFormatX16Y16Sint:
        return "X16Y16_SINT";
    case kRmtFormatX16Y16Float:
        return "X16Y16_FLOAT";
    case kRmtFormatX16Y16Z16W16Unorm:
        return "X16Y16Z16W16_UNORM";
    case kRmtFormatX16Y16Z16W16Snorm:
        return "X16Y16Z16W16_SNORM";
    case kRmtFormatX16Y16Z16W16Uscaled:
        return "X16Y16Z16W16_USCALED";
    case kRmtFormatX16Y16Z16W16Sscaled:
        return "X16Y16Z16W16_SSCALED";
    case kRmtFormatX16Y16Z16W16Uint:
        return "X16Y16Z16W16_UINT";
    case kRmtFormatX16Y16Z16W16Sint:
        return "X16Y16Z16W16_SINT";
    case kRmtFormatX16Y16Z16W16Float:
        return "X16Y16Z16W16_FLOAT";
    case kRmtFormatX32Uint:
        return "X32_UINT";
    case kRmtFormatX32Sint:
        return "X32_SINT";
    case kRmtFormatX32Float:
        return "X32_FLOAT";
    case kRmtFormatX32Y32Uint:
        return "X32Y32_UINT";
    case kRmtFormatX32Y32Sint:
        return "X32Y32_SINT";
    case kRmtFormatX32Y32Float:
        return "X32Y32_FLOAT";
    case kRmtFormatX32Y32Z32Uint:
        return "X32Y32Z32_UINT";
    case kRmtFormatX32Y32Z32Sint:
        return "X32Y32Z32_SINT";
    case kRmtFormatX32Y32Z32Float:
        return "X32Y32Z32_FLOAT";
    case kRmtFormatX32Y32Z32W32Uint:
        return "X32Y32Z32W32_UINT";
    case kRmtFormatX32Y32Z32W32Sint:
        return "X32Y32Z32W32_SINT";
    case kRmtFormatX32Y32Z32W32Float:
        return "X32Y32Z32W32_FLOAT";
    case kRmtFormatD16UnormS8Uint:
        return "D16_UNORM_S8_UINT";
    case kRmtFormatD32FloatS8Uint:
        return "D32_FLOAT_S8_UINT";
    case kRmtFormatX9Y9Z9E5Float:
        return "X9Y9Z9E5_FLOAT";
    case kRmtFormatBC1Unorm:
        return "BC1_UNORM";
    case kRmtFormatBC1Srgb:
        return "BC1_SRGB";
    case kRmtFormatBC2Unorm:
        return "BC2_UNORM";
    case kRmtFormatBC2Srgb:
        return "BC2_SRGB";
    case kRmtFormatBC3Unorm:
        return "BC3_UNORM";
    case kRmtFormatBC3Srgb:
        return "BC3_SRGB";
    case kRmtFormatBC4Unorm:
        return "BC4_UNORM";
    case kRmtFormatBC4Snorm:
        return "BC4_SNORM";
    case kRmtFormatBC5Unorm:
        return "BC5_UNORM";
    case kRmtFormatBC5Snorm:
        return "BC5_SNORM";
    case kRmtFormatBC6UFloat:
        return "BC6_UFLOAT";
    case kRmtFormatBC6SFloat:
        return "BC6_SFLOAT";
    case kRmtFormatBC7Unorm:
        return "BC7_UNORM";
    case kRmtFormatBC7Srgb:
        return "BC7_SRGB";
    case kRmtFormatEtC2X8Y8Z8Unorm:
        return "ETC2X8Y8Z8_UNORM";
    case kRmtFormatEtC2X8Y8Z8Srgb:
        return "ETC2X8Y8Z8_SRGB";
    case kRmtFormatEtC2X8Y8Z8W1Unorm:
        return "ETC2X8Y8Z8W1_UNORM";
    case kRmtFormatEtC2X8Y8Z8W1Srgb:
        return "ETC2X8Y8Z8W1_SRGB";
    case kRmtFormatEtC2X8Y8Z8W8Unorm:
        return "ETC2X8Y8Z8W8_UNORM";
    case kRmtFormatEtC2X8Y8Z8W8Srgb:
        return "ETC2X8Y8Z8W8_SRGB";
    case kRmtFormatEtC2X11Unorm:
        return "ETC2X11_UNORM";
    case kRmtFormatEtC2X11Snorm:
        return "ETC2X11_SNORM";
    case kRmtFormatEtC2X11Y11Unorm:
        return "ETC2X11Y11_UNORM";
    case kRmtFormatEtC2X11Y11Snorm:
        return "ETC2X11Y11_SNORM";
    case kRmtFormatAstcldR4X4Unorm:
        return "ASTCLDR4X4_UNORM";
    case kRmtFormatAstcldR4X4Srgb:
        return "ASTCLDR4X4_SRGB";
    case kRmtFormatAstcldR5X4Unorm:
        return "ASTCLDR5X4_UNORM";
    case kRmtFormatAstcldR5X4Srgb:
        return "ASTCLDR5X4_SRGB";
    case kRmtFormatAstcldR5X5Unorm:
        return "ASTCLDR5X5_UNORM";
    case kRmtFormatAstcldR5X5Srgb:
        return "ASTCLDR5X5_SRGB";
    case kRmtFormatAstcldR6X5Unorm:
        return "ASTCLDR6X5_UNORM";
    case kRmtFormatAstcldR6X5Srgb:
        return "ASTCLDR6X5_SRGB";
    case kRmtFormatAstcldR6X6Unorm:
        return "ASTCLDR6X6_UNORM";
    case kRmtFormatAstcldR6X6Srgb:
        return "ASTCLDR6X6_SRGB";
    case kRmtFormatAstcldR8X5Unorm:
        return "ASTCLDR8X5_UNORM";
    case kRmtFormatAstcldR8X5Srgb:
        return "ASTCLDR8X5_SRGB";
    case kRmtFormatAstcldR8X6Unorm:
        return "ASTCLDR8X6_UNORM";
    case kRmtFormatAstcldR8X6Srgb:
        return "ASTCLDR8X6_SRGB";
    case kRmtFormatAstcldR8X8Unorm:
        return "ASTCLDR8X8_UNORM";
    case kRmtFormatAstcldR8X8Srgb:
        return "ASTCLDR8X8_SRGB";
    case kRmtFormatAstcldR10X5Unorm:
        return "ASTCLDR10X5_UNORM";
    case kRmtFormatAstcldR10X5Srgb:
        return "ASTCLDR10X5_SRGB";
    case kRmtFormatAstcldR10X6Unorm:
        return "ASTCLDR10X6_UNORM";
    case kRmtFormatAstcldR10X6Srgb:
        return "ASTCLDR10X6_SRGB";
    case kRmtFormatAstcldR10X8Unorm:
        return "ASTCLDR10X8_UNORM";
    case kRmtFormatAstcldR10X8Srgb:
        return "ASTCLDR10X8_SRGB";
    case kRmtFormatAstcldR10X10Unorm:
        return "ASTCLDR10X10_UNORM";
    case kRmtFormatAstcldR10X10Srgb:
        return "ASTCLDR10X10_SRGB";
    case kRmtFormatAstcldR12X10Unorm:
        return "ASTCLDR12X10_UNORM";
    case kRmtFormatAstcldR12X10Srgb:
        return "ASTCLDR12X10_SRGB";
    case kRmtFormatAstcldR12X12Unorm:
        return "ASTCLDR12X12_UNORM";
    case kRmtFormatAstcldR12X12Srgb:
        return "ASTCLDR12X12_SRGB";
    case kRmtFormatAstchdR4x4Float:
        return "ASTCHDR4x4_FLOAT";
    case kRmtFormatAstchdR5x4Float:
        return "ASTCHDR5x4_FLOAT";
    case kRmtFormatAstchdR5x5Float:
        return "ASTCHDR5x5_FLOAT";
    case kRmtFormatAstchdR6x5Float:
        return "ASTCHDR6x5_FLOAT";
    case kRmtFormatAstchdR6x6Float:
        return "ASTCHDR6x6_FLOAT";
    case kRmtFormatAstchdR8x5Float:
        return "ASTCHDR8x5_FLOAT";
    case kRmtFormatAstchdR8x6Float:
        return "ASTCHDR8x6_FLOAT";
    case kRmtFormatAstchdR8x8Float:
        return "ASTCHDR8x8_FLOAT";
    case kRmtFormatAstchdR10x5Float:
        return "ASTCHDR10x5_FLOAT";
    case kRmtFormatAstchdR10x6Float:
        return "ASTCHDR10x6_FLOAT";
    case kRmtFormatAstchdR10x8Float:
        return "ASTCHDR10x8_FLOAT";
    case kRmtFormatAstchdR10x10Float:
        return "ASTCHDR10x10_FLOAT";
    case kRmtFormatAstchdR12x10Float:
        return "ASTCHDR12x10_FLOAT";
    case kRmtFormatAstchdR12x12Float:
        return "ASTCHDR12x12_FLOAT";
    case kRmtFormatX8Y8Z8Y8Unorm:
        return "X8Y8_Z8Y8_UNORM";
    case kRmtFormatX8Y8Z8Y8Uscaled:
        return "X8Y8_Z8Y8_USCALED";
    case kRmtFormatY8X8Y8Z8Unorm:
        return "Y8X8_Y8Z8_UNORM";
    case kRmtFormatY8X8Y8Z8Uscaled:
        return "Y8X8_Y8Z8_USCALED";
    case kRmtFormatAyuv:
        return "AYUV";
    case kRmtFormatUyvy:
        return "UYVY";
    case kRmtFormatVyuy:
        return "VYUY";
    case kRmtFormatYuY2:
        return "YUY2";
    case kRmtFormatYvY2:
        return "YVY2";
    case kRmtFormatYV12:
        return "YV12";
    case kRmtFormatNV11:
        return "NV11";
    case kRmtFormatNV12:
        return "NV12";
    case kRmtFormatNV21:
        return "NV21";
    case kRmtFormatP016:
        return "P016";
    case kRmtFormatP010:
        return "P010";
    case kRmtFormatP210:
        return "P210";
    case kRmtFormatX8MMUnorm:
        return "X8 MM UNORM";
    case kRmtFormatX8MMUint:
        return "X8 MM UINT";
    case kRmtFormatX8Y8MMUnorm:
        return "X8Y8 MM UNORM";
    case kRmtFormatX8Y8MMUint:
        return "X8Y8 MM UINT";
    case kRmtFormatX16MM10Unorm:
        return "X16 MM10 UNORM";
    case kRmtFormatX16MM10Uint:
        return "X16 MM10 UINT";
    case kRmtFormatX16Y16MM10Unorm:
        return "X16Y16 MM10 UNORM";
    case kRmtFormatX16Y16MM10Uint:
        return "X16Y16 MM10 UINT";
    case kRmtFormatP208:
        return "P208";
    case kRmtFormatX16MM12Unorm:
        return "X16 MM12 UNORM";
    case kRmtFormatX16MM12Uint:
        return "X16 MM12 UINT";
    case kRmtFormatX16Y16MM12Unorm:
        return "X16Y16 MM12 UNORM";
    case kRmtFormatX16Y16MM12Uint:
        return "X16Y16 MM12 UINT";
    case kRmtFormatP012:
        return "P012";
    case kRmtFormatP212:
        return "P212";
    case kRmtFormatP412:
        return "P412";
    case kRmtFormatX10Y10Z10W2Float:
        return "X10Y10Z10W2 FLOAT";
    case kRmtFormatY216:
        return "Y216";
    case kRmtFormatY210:
        return "Y210";
    case kRmtFormatY416:
        return "Y416";
    case kRmtFormatY410:
        return "Y410";

    default:
        return "Unknown";
    }
}

const char* RmtGetChannelSwizzleNameFromChannelSwizzle(RmtChannelSwizzle channel_swizzle)
{
    switch (channel_swizzle)
    {
    case kRmtSwizzleZero:
        return "0";
    case kRmtSwizzleOne:
        return "1";
    case kRmtSwizzleX:
        return "X";
    case kRmtSwizzleY:
        return "Y";
    case kRmtSwizzleZ:
        return "Z";
    case kRmtSwizzleW:
        return "W";
    default:
        return "Unknown";
    }
}

const char* RmtGetSwizzlePatternFromImageFormat(const RmtImageFormat* image_format, char* out_string, int32_t max_length)
{
    sprintf_s(out_string,
              max_length,
              "%s%s%s%s",
              RmtGetChannelSwizzleNameFromChannelSwizzle(image_format->swizzle_x),
              RmtGetChannelSwizzleNameFromChannelSwizzle(image_format->swizzle_y),
              RmtGetChannelSwizzleNameFromChannelSwizzle(image_format->swizzle_z),
              RmtGetChannelSwizzleNameFromChannelSwizzle(image_format->swizzle_w));
    return out_string;
}

const char* RmtGetTilingNameFromTilingType(RmtTilingType tiling_type)
{
    switch (tiling_type)
    {
    case kRmtTilingTypeLinear:
        return "Linear";
    case kRmtTilingTypeOptimal:
        return "Optimal";
    case kRmtTilingTypeStandardSwizzle:
        return "Standard Swizzle";
    default:
        return "Unknown";
    }
}

const char* RmtGetImageTypeNameFromImageType(RmtImageType image_type)
{
    switch (image_type)
    {
    case kRmtImageType1D:
        return "1D";
    case kRmtImageType2D:
        return "2D";
    case kRmtImageType3D:
        return "3D";
    default:
        return "Unknown";
    }
}

const char* RmtGetTilingOptimizationModeNameFromTilingOptimizationMode(RmtTilingOptimizationMode tiling_optimization_mode)
{
    switch (tiling_optimization_mode)
    {
    case kRmtTilingOptimizationModeBalanced:
        return "Balanced";
    case kRmtTilingOptimizationModeSpace:
        return "Space";
    case kRmtTilingOptimizationModeSpeed:
        return "Speed";
    default:
        return "Unknown";
    }
}

/// Get the image creation flag text based on the bitfield parameter
static const char* GetImageCreationNameFromImageCreationFlagBits(int32_t bitfield)
{
    switch (bitfield)
    {
    case kRmtImageCreationFlagInvariant:
        return "INVARIANT";
    case kRmtImageCreationFlagCloneable:
        return "CLONEABLE";
    case kRmtImageCreationFlagShareable:
        return "SHAREABLE";
    case kRmtImageCreationFlagFlippable:
        return "FLIPPABLE";
    case kRmtImageCreationFlagStereo:
        return "STEREO";
    case kRmtImageCreationFlagCubemap:
        return "CUBEMAP";
    case kRmtImageCreationFlagPrt:
        return "PRT";
    case kRmtImageCreationFlagReserved0:
        return "RESERVED_0";
    case kRmtImageCreationFlagReadSwizzleEquations:
        return "READ_SWIZZLE_EQUATIONS";
    case kRmtImageCreationFlagPerSubresourceInit:
        return "PER_SUBRESOURCE_INIT";
    case kRmtImageCreationFlagSeparateDepthAspectRatio:
        return "SEPARATE_DEPTH_ASPECT_RATIO";
    case kRmtImageCreationFlagCopyFormatsMatch:
        return "COPY_FORMATS_MATCH";
    case kRmtImageCreationFlagRepetitiveResolve:
        return "REPETITIVE_RESOLVE";
    case kRmtImageCreationFlagPreferSwizzleEquations:
        return "PREFER_SWIZZLE_EQUATIONS";
    case kRmtImageCreationFlagFixedTileSwizzle:
        return "FIXED_TILE_SWIZZLE";
    case kRmtImageCreationFlagVideoReferenceOnly:
        return "VIDEO_REFERENCE_ONLY";
    case kRmtImageCreationFlagOptimalShareable:
        return "OPTIMAL_SHAREABLE";
    case kRmtImageCreationFlagSampleLocationsKnown:
        return "SAMPLE_LOCATIONS_KNOWN";
    case kRmtImageCreationFlagFullResolveDestOnly:
        return "FULL_RESOLVE_DEST_ONLY";
    case kRmtImageCreationFlagExternalShared:
        return "EXTERNAL_SHARED";
    default:
        return "";
    }
}

/// Get the image usage flag text based on the bitfield parameter
static const char* GetImageUsageNameFromImageUsageFlagBits(int32_t bitfield)
{
    switch (bitfield)
    {
    case kRmtImageUsageFlagsShaderRead:
        return "SHADER_READ";
    case kRmtImageUsageFlagsShaderWrite:
        return "SHADER_WRITE";
    case kRmtImageUsageFlagsResolveSource:
        return "RESOLVE_SOURCE";
    case kRmtImageUsageFlagsResolveDestination:
        return "RESOLVE_DESTINATION";
    case kRmtImageUsageFlagsColorTarget:
        return "COLOR_TARGET";
    case kRmtImageUsageFlagsDepthStencil:
        return "DEPTH_STENCIL";
    case kRmtImageUsageFlagsNoStencilShaderRead:
        return "NO_STENCIL_SHADER_READ";
    case kRmtImageUsageFlagsHiZNeverInvalid:
        return "HI_Z_NEVER_INVALID";
    case kRmtImageUsageFlagsDepthAsZ24:
        return "DEPTH_AS_Z24";
    case kRmtImageUsageFlagsFirstShaderWritableMip:
        return "FIRST_SHADER_WRITABLE_MIP";
    case kRmtImageUsageFlagsCornerSampling:
        return "CORNER_SAMPLING";
    case kRmtImageUsageFlagsVrsDepth:
        return "VRS_DEPTH";
    default:
        return "";
    }
}

/// Get the buffer creation flag text based on the bitfield parameter
static const char* GetBufferCreationNameFromBufferCreationFlagBits(int32_t bitfield)
{
    switch (bitfield)
    {
    case kRmtBufferCreationFlagSparseBinding:
        return "SPARSE_BINDING";
    case kRmtBufferCreationFlagSparseResidency:
        return "SPARSE_RESIDENCY";
    case kRmtBufferCreationFlagSparseAliasing:
        return "SPARSE_ALIASING";
    case kRmtBufferCreationFlagProtected:
        return "PROTECTED";
    case kRmtBufferCreationFlagDeviceAddressCaptureReplay:
        return "DEVICE_ADDRESS_CAPTURE_REPLAY";
    default:
        return "";
    }
}

/// Get the buffer usage flag text based on the bitfield parameter
static const char* GetBufferUsageNameFromBufferUsageFlagBits(int32_t bitfield)
{
    switch (bitfield)
    {
    case kRmtBufferUsageFlagTransferSource:
        return "TRANSFER_SOURCE";
    case kRmtBufferUsageFlagTransferDestination:
        return "TRANSFER_DESTINATION";
    case kRmtBufferUsageFlagUniformTexelBuffer:
        return "UNIFORM_TEXEL_BUFFER";
    case kRmtBufferUsageFlagStorageTexelBuffer:
        return "STORAGE_TEXEL_BUFFER";
    case kRmtBufferUsageFlagUniformBuffer:
        return "UNIFORM_BUFFER";
    case kRmtBufferUsageFlagStorageBuffer:
        return "STORAGE_BUFFER";
    case kRmtBufferUsageFlagIndexBuffer:
        return "INDEX_BUFFER";
    case kRmtBufferUsageFlagVertexBuffer:
        return "VERTEX_BUFFER";
    case kRmtBufferUsageFlagIndirectBuffer:
        return "INDIRECT_BUFFER";
    case kRmtBufferUsageFlagTransformFeedbackBuffer:
        return "TRANSFORM_FEEDBACK_BUFFER";
    case kRmtBufferUsageFlagTransformFeedbackCounterBuffer:
        return "TRANSFORM_FEEDBACK_COUNTER_BUFFER";
    case kRmtBufferUsageFlagConditionalRendering:
        return "CONDITIONAL_RENDERING";
    case kRmtBufferUsageFlagRayTracing:
        return "RAY_TRACING";
    case kRmtBufferUsageFlagShaderDeviceAddress:
        return "SHADER_DEVICE_ADDRESS";
    default:
        return "";
    }
}

/// Get the GPU event flag text based on the bitfield parameter
static const char* GetGpuEventNameFromGpuEventFlagBits(int32_t bitfield)
{
    switch (bitfield)
    {
    case kRmtGpuEventFlagGpuOnly:
        return "GPU_ONLY";
    default:
        return "";
    }
}

/// Get the pipeline creation flag text based on the bitfield parameter
static const char* GetPipelineCreationNameFromPipelineCreationFlagBits(int32_t bitfield)
{
    switch (bitfield)
    {
    case kRmtPipelineCreateFlagInternal:
        return "CLIENT_INTERNAL";
    case kRmtPipelineCreateFlagOverrideGpuHeap:
        return "OVERRIDE_GPU_HEAP";
    case kRmtPipelineCreateFlagReserved0:
        return "RESERVED_0";
    case kRmtPipelineCreateFlagReserved1:
        return "RESERVED_1";
    case kRmtPipelineCreateFlagReserved2:
        return "RESERVED_2";
    case kRmtPipelineCreateFlagReserved3:
        return "RESERVED_3";
    case kRmtPipelineCreateFlagReserved4:
        return "RESERVED_4";
    case kRmtPipelineCreateFlagReserved5:
        return "RESERVED_5";
    default:
        return "";
    }
}

/// Get the command allocator flag text based on the bitfield parameter
static const char* GetCmdAllocatorNameFromCmdAllocatorFlagBits(int32_t bitfield)
{
    switch (bitfield)
    {
    case kRmtCmdAllocatorAutoMemoryReuse:
        return "AUTO_MEMORY_REUSE";
    case kRmtCmdAllocatorDisableBusyChunkTracking:
        return "DISABLE_BUSY_CHUNK_TRACKING";
    case kRmtCmdAllocatorThreadSafe:
        return "THREAD_SAFE";
    default:
        return "";
    }
}

/// Get the pipeline stage flag text based on the bitfield parameter
static const char* GetPipelineStageNameFromPipelineStageBits(int32_t bitfield)
{
    switch (bitfield)
    {
    case kRmtPipelineStageMaskPs:
        return "PS";
    case kRmtPipelineStageMaskHs:
        return "HS";
    case kRmtPipelineStageMaskDs:
        return "DS";
    case kRmtPipelineStageMaskVs:
        return "VS";
    case kRmtPipelineStageMaskGs:
        return "GS";
    case kRmtPipelineStageMaskCs:
        return "CS";
    case kRmtPipelineStageMaskTs:
        return "TS";
    case kRmtPipelineStageMaskMs:
        return "MS";
    default:
        return "";
    }
}

/// Get a text string based on the flags passed in. Each flag is separated by a '|'
/// If there are no text strings corresponding to the flags, the raw value is shown
/// The raw flags value is shown after the flag text string in parentheses
/// @param [in]     flags                       The buffer usage flags to convert to text
/// @param [out]    flag_text                   String to accept the flag text
/// @param [in]     text_length                 The length of the flag text string
/// @param [in]     function                    Pointer to a function to get the text from a bitfield.
static void GetFlagsNameFromFlags(int32_t flags, char* flag_text, int text_length, const char* (*function)(int32_t))
{
    int32_t       mask = 0x01;
    const int32_t size = sizeof(int32_t) * 8;

    flag_text[0] = '\0';

    int flag_count = 0;
    for (int i = 0; i < size; i++)
    {
        if ((flags & mask) != 0)
        {
            if (flag_count > 0)
            {
                strcat_s(flag_text, text_length, " | ");
            }
            const char* flag_name = function(mask);
            strcat_s(flag_text, text_length, flag_name);
            flag_count++;
        }
        mask <<= 1;
    }

    if (flag_count == 0)
    {
        // no flags text set so display "None"
        sprintf_s(flag_text, text_length, "None");
    }
    else
    {
        // append the value of the flags after the text
        char value_text[32];
        sprintf_s(value_text, 32, " (%d)", flags);
        strcat_s(flag_text, text_length, value_text);
    }
}

void RmtGetImageCreationNameFromImageCreationFlags(int32_t flags, char* flag_text, int text_length)
{
    GetFlagsNameFromFlags(flags, flag_text, text_length, &GetImageCreationNameFromImageCreationFlagBits);
}

void RmtGetImageUsageNameFromImageUsageFlags(int32_t flags, char* flag_text, int text_length)
{
    GetFlagsNameFromFlags(flags, flag_text, text_length, &GetImageUsageNameFromImageUsageFlagBits);
}

void RmtGetBufferCreationNameFromBufferCreationFlags(int32_t flags, char* flag_text, int text_length)
{
    GetFlagsNameFromFlags(flags, flag_text, text_length, &GetBufferCreationNameFromBufferCreationFlagBits);
}

void RmtGetBufferUsageNameFromBufferUsageFlags(int32_t flags, char* flag_text, int text_length)
{
    GetFlagsNameFromFlags(flags, flag_text, text_length, &GetBufferUsageNameFromBufferUsageFlagBits);
}

void RmtGetGpuEventNameFromGpuEventFlags(int32_t flags, char* flag_text, int text_length)
{
    GetFlagsNameFromFlags(flags, flag_text, text_length, &GetGpuEventNameFromGpuEventFlagBits);
}

void RmtGetPipelineCreationNameFromPipelineCreationFlags(int32_t flags, char* flag_text, int text_length)
{
    GetFlagsNameFromFlags(flags, flag_text, text_length, &GetPipelineCreationNameFromPipelineCreationFlagBits);
}

void RmtGetCmdAllocatorNameFromCmdAllocatorFlags(int32_t flags, char* flag_text, int text_length)
{
    GetFlagsNameFromFlags(flags, flag_text, text_length, &GetCmdAllocatorNameFromCmdAllocatorFlagBits);
}

void RmtGetPipelineStageNameFromPipelineStageFlags(int32_t flags, char* flag_text, int text_length)
{
    GetFlagsNameFromFlags(flags, flag_text, text_length, &GetPipelineStageNameFromPipelineStageBits);
}

void RmtSetPrintingCallback(RmtPrintingCallback callback_func, bool enable_printing)
{
    printing_func       = callback_func;
    is_printing_enabled = enable_printing;
}

void RmtPrint(const char* format, ...)
{
    if (!is_printing_enabled)
    {
        return;
    }

    va_list args;
    va_start(args, format);

    if (printing_func == nullptr)
    {
        char buffer[2048];
        vsnprintf(buffer, 2048, format, args);
#ifdef _WIN32
        const size_t len = strlen(buffer);
        buffer[len]      = '\n';
        buffer[len + 1]  = '\0';
        OutputDebugString(buffer);
#else
        printf("%s\n", buffer);
#endif
    }
    else
    {
        char buffer[2048];
        vsnprintf(buffer, 2048, format, args);
        printing_func(buffer);
    }

    va_end(args);
}
