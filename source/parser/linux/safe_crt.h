//==============================================================================
// Copyright (c) 2016-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Linux definition of Windows safe CRT functions.
//==============================================================================

#ifndef RMV_PARSER_LINUX_SAFE_CRT_H_
#define RMV_PARSER_LINUX_SAFE_CRT_H_

#if !defined(_WIN32)

/// errno_t defined so that the function prototypes match the Windows function prototypes.
typedef int errno_t;

/// fopen_s secure version of fopen.
/// \param file A pointer to the file pointer that will receive the pointer to the opened file.
/// \param filename The filename of the file to be opened.
/// \param mode Type of access permitted.
/// \return Zero if successful; an error code on failure.
errno_t fopen_s(FILE** file, const char* filename, const char* mode);

/// sprintf_s secure version of sprintf.
/// \param buffer Storage location for output.
/// \param size_of_buffer Maximum number of characters to store.
/// \param format Format-control string.
/// \param ... Optional arguments to be formatted.
/// \return The number of characters written or -1 if an error occurred.
int sprintf_s(char* buffer, size_t size_of_buffer, const char* format, ...);

/// fprintf_s secure version of fprintf.
/// \param stream Pointer to FILE structure.
/// \param format Format-control string.
/// \param ... Optional arguments to be formatted.
/// \return The number of bytes written, or a negative value when an output error occurs.
int fprintf_s(FILE* stream, const char* format, ...);

/// fread_s secure version of fread.
/// \param buffer Storage location for data.
/// \param buffer_size Size of the destination buffer in bytes.
/// \param element_size Size of the item to read in bytes.
/// \param count Maximum number of items to be read.
/// \param stream Pointer to FILE structure.
/// \return The number of (whole) items that were read into the buffer, which may be less than count if a
///         read error or the end of the file is encountered before count is reached. Use the feof or ferror
///         function to distinguish an error from an end-of-file condition. If size or count is 0, fread_s
///         returns 0 and the buffer contents are unchanged.
size_t fread_s(void* buffer, size_t buffer_size, size_t element_size, size_t count, FILE* stream);

/// strcpy_s secure version of strcpy.
/// \param destination The destination (output) string.
/// \param size The maximum number of bytes to copy.
/// \param source The source (input) string.
/// \return 0 on success, non-zero on error.
errno_t strcpy_s(char* destination, size_t size, const char* source);

/// strcat_s secure version of strcat.
/// \param destination The destination (output) string.
/// \param size The maximum number of bytes to copy.
/// \param source The source (input) string.
/// \return 0 on success, non-zero on error.
errno_t strcat_s(char* destination, size_t size, const char* source);

#endif  // !_WIN32

#endif  // RMV_PARSER_LINUX_SAFE_CRT_H_
