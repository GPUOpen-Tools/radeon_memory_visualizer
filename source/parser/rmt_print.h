//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// $brief  Printing helper functions for RMT.
//=============================================================================

#ifndef RMV_PARSER_RMT_PRINT_H_
#define RMV_PARSER_RMT_PRINT_H_

#include "rmt_error.h"
#include "rmt_format.h"

/// Callback function for printing.
typedef void (*RmtPrintingCallback)(const char* msg);

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// Get the page size as a string from the page size ID.
///
/// @param [in]     page_size               An RmtPageSize defining the page size.
/// @returns
/// Pointer to the page size string.
const char* RmtGetPageSizeNameFromPageSize(RmtPageSize page_size);

/// Get the resource type as a string from the resource type ID.
///
/// @param [in]     resource_type           An RmtResourceType defining the resource type.
/// @returns
/// Pointer to the resource type string.
const char* RmtGetResourceTypeNameFromResourceType(RmtResourceType resource_type);

/// Get the commit type as a string from the commit type ID.
///
/// @param [in]     commit_type             An RmtCommitType defining the commit type.
/// @returns
/// Pointer to the commit type string.
const char* RmtGetCommitTypeNameFromCommitType(RmtCommitType commit_type);

/// Get the owner type as a string from the owner type ID.
///
/// @param [in]     owner_type              An RmtOwnerType defining the owner type.
/// @returns
/// Pointer to the owner type string.
const char* RmtGetOwnerTypeNameFromOwnerType(RmtOwnerType owner_type);

/// Get the miscellaneous type as a string from the miscellaneous type ID.
///
/// @param [in]     misc_type               An RmtMiscType defining the miscellaneous type.
/// @returns
/// Pointer to the miscellaneous type string.
const char* RmtGetMiscTypeNameFromMiscType(RmtMiscType misc_type);

/// Get the process event type as a string from the process event type ID.
///
/// @param [in]     process_event           An RmtProcessEventType defining the process event type.
/// @returns
/// Pointer to the process event type string.
const char* RmtGetProcessEventNameFromProcessEvent(RmtProcessEventType process_event);

/// Get the heap type as a string from the heap type ID.
///
/// @param [in]     heap_type               An RmtHeapType defining the heap type.
/// @returns
/// Pointer to the heap type string.
const char* RmtGetHeapTypeNameFromHeapType(RmtHeapType heap_type);

/// Get the page table update type as a string from the page table update type ID.
///
/// @param [in]     update_type             An RmtPageTableUpdateType defining the page table update type.
/// @returns
/// Pointer to the page table update type string.
const char* RmtGetPageTableUpdateTypeNameFromPageTableUpdateType(RmtPageTableUpdateType update_type);

/// Get the token type as a string from the token type ID.
///
/// @param [in]     token_type              An RmtTokenType defining the token type.
/// @returns
/// Pointer to the token type string.
const char* RmtGetTokenNameFromTokenType(RmtTokenType token_type);

/// Get the format as a string from the format ID.
///
/// @param [in]     format                  An RmtFormat defining the format.
/// @returns
/// Pointer to the format string.
const char* RmtGetFormatNameFromFormat(RmtFormat format);

/// Get the channel swizzle name as a string from the channel swizzle struct.
///
/// @param [in]     channel_swizzle         An RmtChannelSwizzle defining the token type.
/// @returns
/// Pointer to the channel swizzle string.
const char* RmtGetChannelSwizzleNameFromChannelSwizzle(RmtChannelSwizzle channel_swizzle);

/// Get swizzle pattern name as a string from the image format struct.
///
/// @param [in]     format                  An RmtImageFormat defining the image format.
/// @param [out]    out_string              A pointer to a string to receive the name string.
/// @param [in]     max_length              The length of out_string, in bytes.
/// @returns
/// Pointer to the swizzel pattern string (the same pointer as out_string).
const char* RmtGetSwizzlePatternFromImageFormat(const RmtImageFormat* format, char* out_string, int32_t max_length);

/// Get the tiling type as a string from the tiling type ID.
///
/// @param [in]     tiling_type             An RmtTilingType defining the tiling type.
/// @returns
/// Pointer to the tiling type string.
const char* RmtGetTilingNameFromTilingType(RmtTilingType tiling_type);

/// Get the image type as a string from the image type ID.
///
/// @param [in]     image_type              An RmtImageType defining the image type.
/// @returns
/// Pointer to the image type string.
const char* RmtGetImageTypeNameFromImageType(RmtImageType image_type);

/// Get the tiling optimization mode as a string from the tiling optimization mode ID.
///
/// @param [in]     tiling_optimization_mode    An RmtTilingOptimizationMode defining the tiling optimization mode.
/// @returns
/// Pointer to the tiling optimization mode string.
const char* RmtGetTilingOptimizationModeNameFromTilingOptimizationMode(RmtTilingOptimizationMode tiling_optimization_mode);

/// Get the image creation flags.
/// @param [in]     flags                       The image creation flags to convert to text.
/// @param [out]    flag_text                   String to accept the flag text.
/// @param [in]     text_length                 The length of the flag text string.
void RmtGetImageCreationNameFromImageCreationFlags(int32_t flags, char* flag_text, int text_length);

/// Get the image usage flags.
/// @param [in]     flags                       The image usage flags to convert to text.
/// @param [out]    flag_text                   String to accept the flag text.
/// @param [in]     text_length                 The length of the flag text string.
void RmtGetImageUsageNameFromImageUsageFlags(int32_t flags, char* flag_text, int text_length);

/// Get the buffer creation flags.
/// @param [in]     flags                       The buffer creation flags to convert to text.
/// @param [out]    flag_text                   String to accept the flag text.
/// @param [in]     text_length                 The length of the flag text string.
void RmtGetBufferCreationNameFromBufferCreationFlags(int32_t flags, char* flag_text, int text_length);

/// Get the buffer usage flags.
/// @param [in]     flags                       The buffer usage flags to convert to text.
/// @param [out]    flag_text                   String to accept the flag text.
/// @param [in]     text_length                 The length of the flag text string.
void RmtGetBufferUsageNameFromBufferUsageFlags(int32_t flags, char* flag_text, int text_length);

/// Get the GPU event flags.
/// @param [in]     flags                       The GPU event flags to convert to text.
/// @param [out]    flag_text                   String to accept the flag text.
/// @param [in]     text_length                 The length of the flag text string.
void RmtGetGpuEventNameFromGpuEventFlags(int32_t flags, char* flag_text, int text_length);

/// Get the pipeline creation flags.
/// @param [in]     flags                       The pipeline creation flags to convert to text.
/// @param [out]    flag_text                   String to accept the flag text.
/// @param [in]     text_length                 The length of the flag text string.
void RmtGetPipelineCreationNameFromPipelineCreationFlags(int32_t flags, char* flag_text, int text_length);

/// Get the command allocator flags.
/// @param [in]     flags                       The command allocator flags to convert to text.
/// @param [out]    flag_text                   String to accept the flag text.
/// @param [in]     text_length                 The length of the flag text string.
void RmtGetCmdAllocatorNameFromCmdAllocatorFlags(int32_t flags, char* flag_text, int text_length);

/// Get the pipeline stage flags.
/// @param [in]     flags                       The pipeline stage flags to convert to text.
/// @param [out]    flag_text                   String to accept the flag text.
/// @param [in]     text_length                 The length of the flag text string.
void RmtGetPipelineStageNameFromPipelineStageFlags(int32_t flags, char* flag_text, int text_length);

/// @brief Set the printing callback for backend functions to do logging.
///
/// @param [in] callback_func   The callback function to use for printing.
/// @param [in] enable_printing Enable the print function that prints to stdout if no callback specified.
///                             May be useful to disable in case the backend is being used outside of RMV.
void RmtSetPrintingCallback(RmtPrintingCallback callback_func, bool enable_printing = true);

/// @brief Printing function to use. Will use printing function set with
/// <c><i>RgpSetPrintingCallback</i></c>. If nothing is set, then
/// printf will be used.
///
/// @param [in] format The formatting string.
/// @param [in] ...    Variable parameters determined by <c><i>format</i></c>.
void RmtPrint(const char* format, ...);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_PARSER_RMT_PRINT_H_
