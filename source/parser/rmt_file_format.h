//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of structures and functions for the RMT file format.
//=============================================================================

#ifndef RMV_PARSER_RMT_FILE_FORMAT_H_
#define RMV_PARSER_RMT_FILE_FORMAT_H_

#include "rmt_error.h"
#include "rmt_types.h"
#include <stdio.h>

/// Magic number for all RMT files.
#define RMT_FILE_MAGIC_NUMBER (0x494e494d)

/// The maximum number of separate RMT streams in a file.
#define RMT_MAXIMUM_STREAMS (256)

/// The maximum length of an adapter name.
#define kRmtMaxAdapterNameLength (128)

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// Structure encapsulating the file header of a RMT file.
typedef struct RmtFileHeader
{
    uint32_t magic_number;         ///< Magic number, always set to <c><i>RMT_FILE_MAGIC_NUMBER</i></c>.
    uint32_t version_major;        ///< The major version number of the file.
    uint32_t version_minor;        ///< The minor version number of the file.
    uint32_t flags;                ///< Bitfield of flags set with information about the file.
    int32_t  chunk_offset;         ///< The offset in bytes to the first chunk contained in the file.
    int32_t  second;               ///< The second in the minute that the RMT file was created.
    int32_t  minute;               ///< The minute in the hour that the RMT file was created.
    int32_t  hour;                 ///< The hour in the day that the RMT file was created.
    int32_t  day_in_month;         ///< The day in the month that the RMT file was created.
    int32_t  month;                ///< The month in the year that the RMT file was created.
    int32_t  year;                 ///< The year that the RMT file was created.
    int32_t  day_in_week;          ///< The day in the week that the RMT file was created.
    int32_t  day_in_year;          ///< The day in the year that the RMT file was created.
    int32_t  is_daylight_savings;  ///< Set to 1 if the time is subject to daylight savings.
} RmtFileHeader;

/// An enumeration of all chunk types used in the file format.
typedef enum RmtFileChunkType
{
    kRmtFileChunkTypeAsicInfo     = 0,  ///< A chunk containing information about the ASIC on which the RMT file was generated.
    kRmtFileChunkTypeApiInfo      = 1,  ///< A chunk containing information about the API that the application generating the RMT file was using.
    kRmtFileChunkTypeSystemInfo   = 2,  ///< A chunk containing the description of the system on which the trace was made.
    kRmtFileChunkTypeRmtData      = 3,  ///< A chunk containing the RMT data.
    kRmtFileChunkTypeSegmentInfo  = 4,  ///< A chunk containing segment information for the main process.
    kRmtFileChunkTypeProcessStart = 5,  ///< A chunk containing process state information at the start of the RMT trace.
    kRmtFileChunkTypeSnapshotInfo = 6,  ///< A chunk containing snapshot info.
    kRmtFileChunkTypeAdapterInfo  = 7,  ///< A chunk containing adapter info.

    // NOTE: Add new chunks above this.
    kRmtFileChunkTypeCount  ///< The number of different chunk types.
} RmtFileChunkType;

/// An enumeration of flags about the file header.
typedef enum RmtFileChunkFileHeaderFlags
{
    kRmtFileHeaderFlagReserved = (1 << 0),  ///< Get the queue timing source
} RmtFileChunkFileHeaderFlags;

/// An enumeration of the API types.
typedef enum RmtApiType
{
    kRmtApiTypeDirectx12 = 0,  ///< The trace contains data from a DirectX 12 application.
    kRmtApiTypeVulkan    = 1,  ///< The trace contains data from a Vulkan application.
    kRmtApiTypeGeneric   = 2,  ///< The API of the application is not known.
    kRmtApiTypeOpencl    = 3,  ///< The API of the application is OpenCL.
    kRmtApiTypeCount           ///< The number of APIs supported.
} RmtApiType;

typedef struct ChunkInfo
{
    RmtFileChunkType chunk_type : 8;   ///< The type of chunk.
    int32_t          chunk_index : 8;  ///< The index of the chunk.
    int32_t          reserved : 16;    ///< Reserved, set to 0.
} ChunkInfo;

/// A structure encapsulating a single chunk identifier.
typedef struct RmtFileChunkIdentifier
{
    union
    {
        ChunkInfo chunk_info;
        uint32_t  value;  ///< 32bit value containing all the above fields.
    };
} RmtFileChunkIdentifier;

/// A structure encapsulating common fields of a chunk in the RMT file format.
typedef struct RmtFileChunkHeader
{
    RmtFileChunkIdentifier chunk_identifier;  ///< A unique identifier for the chunk.
    int16_t                version_minor;     ///< The minor version of the chunk. Please see above note on ordering of minor and major version
    int16_t                version_major;     ///< The major version of the chunk.
    int32_t                size_in_bytes;     ///< The size of the chunk in bytes.
    int32_t                padding;           ///< Reserved padding dword.
} RmtFileChunkHeader;

/// A structure encapsulating information about the location of the RMT data within the RMT file itself.
typedef struct RmtFileChunkRmtData
{
    uint64_t process_id;  ///< The process ID that generated this RMT data. If unknown program to 0.
    uint64_t thread_id;   ///< The CPU thread ID of the thread in the application that generated this RMT data.
} RmtFileChunkRmtData;

/// A structure encapsulating system information.
typedef struct RmtFileChunkSystemInfo
{
    char     vendor_id[16];        ///< For x86 CPUs this is based off the 12 character ASCII string retreived via CPUID instruction.
    char     processor_brand[48];  ///< For x86 CPUs this is based off the 48 byte null-terminated ASCII processor brand using CPU instruction.
    uint64_t padding;              ///< Padding after 48 byte string.
    uint64_t
             timestamp_frequency;  ///< The frequency of the timestamp clock (in Hz). For windows this is the same as reported by the <c><i>QueryPerformanceFrequency</i></c> API.
    uint32_t clock_speed;       ///< The maximum clock frequency of the CPU (in MHz).
    int32_t  logic_cores;       ///< The number of logical cores.
    int32_t  physical_cores;    ///< The number of physical cores.
    int32_t  system_ram_in_mb;  ///< The amount of system RAM expressed in MB.
} RmtFileChunkSystemInfo;

/// A structure encapsulating segment info.
typedef struct RmtFileChunkSegmentInfo
{
    uint64_t    base_address;   ///< The physical address for the segment.
    uint64_t    size_in_bytes;  ///< The size (in bytes) of the segment.
    RmtHeapType heap_type;      ///< The type of heap that the segment implements.
    int32_t     memory_index;   ///< The memory index exposed by the Vulkan software stack.
} RmtFileChunkSegmentInfo;

/// A structure encapsulating adapter info.
typedef struct RmtFileChunkAdapterInfo
{
    char     name[kRmtMaxAdapterNameLength];  ///< The name of the adapter as a NULL terminated string.
    uint32_t pcie_family_id;                  ///< The PCIe family ID of the adapter.
    uint32_t pcie_revision_id;                ///< The PCIe revision ID of the adapter.
    uint32_t device_id;                       ///< The PCIe device ID of the adapter.
    uint32_t minimum_engine_clock;            ///< The minimum engine clock (in MHz).
    uint32_t maximum_engine_clock;            ///< The maximum engine clock (in MHz).
    uint32_t memory_type;                     ///< The memory type.
    uint32_t memory_operations_per_clock;     ///< The number of memory operations that can be performed per clock.
    uint32_t memory_bus_width;                ///< The width of the memory bus (in bits).
    uint32_t memory_bandwidth;                ///< Bandwidth of the memory system (in MB/s).
    uint32_t minimum_memory_clock;            ///< The minimum memory clock (in MHz).
    uint32_t maximum_memory_clock;            ///< The maximum memory clock (in MHz).
} RmtFileChunkAdapterInfo;

/// A structure encapsulating snapshot info.
typedef struct RmtFileChunkSnapshotInfo
{
    uint64_t snapshot_time;         ///< The time (in RMT clocks) when the snapshot was taken.
    int32_t  name_length_in_bytes;  ///< The length of the name in bytes.

    // NOTE: The name follows this structure.
} RmtFileChunkSnapshotInfo;

/// A structure encapsulating the state of the RMT file parser.
typedef struct RmtFileParser
{
    FILE*              file_handle;        ///< The file handle.
    RmtFileHeader      header;             ///< The RMT file header read from the buffer.
    RmtFileChunkHeader current_chunk;      ///< Storage for a <c><i>RmtFileChunkHeader</i></c> structure.
    int32_t            next_chunk_offset;  ///< The offset to the next chunk to read.
    size_t             file_size;          ///< The size of the file.
} RmtFileParser;

/// Create an RMT file parser from a buffer.
///
/// @param [in, out]    file_parser                 A pointer to a <c><i>RmtFileParser</i></c> structure to populate.
/// @param [in]         file_handle                 A pointer to a <c><i>FILE</i></c> object.
///
/// @retval
/// kRmtOk                          The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer         The operation failed because <c><i>fileParser</i></c> or <c><i>fileBuffer</i></c> was NULL.
/// @retval
/// kRmtErrorInvalidSize            The operation failed because <c><i>fileBufferSize</i></c> was not large enough to contain the RMT file header.
/// @retval
/// kRmtErrorMalformedData          The operation failed because the data pointed to by <c><i>fileBuffer</i></c> didn't begin with a valid RMT file header.
///
RmtErrorCode RmtFileParserCreateFromHandle(RmtFileParser* file_parser, FILE* file_handle);

/// Get a pointer to the next chunk in the file.
///
/// @param [in, out]    file_parser                 A pointer to a <c><i>RmtFileParser</i></c> structure.
/// @param [out]        parsed_chunk                A pointer to a <c><i>RmtFileChunkHeader</i></c> structure to populate with the chunk information.
///
/// @retval
/// kRmtOk                          The operation completed successfully.
/// @retval
/// kRmtErrorInvalidPointer         The operation failed because <c><i>fileParser</i></c> or <c><i>parsedChunk</i></c> was NULL.
/// @retval
/// kRmtEndOfFile                   The operation failed because the end of the buffer was reached.
///
RmtErrorCode RmtFileParserParseNextChunk(RmtFileParser* file_parser, RmtFileChunkHeader** parsed_chunk);

/// Check if the current chunk can be processed by this build of RMT. Only checks major versions.
/// See implementation comment for directions on how to express the versions presently supported.
///
/// @param [in]     header      A pointer to a Chunk header
/// @retval
/// kRmtOk                                  The chunk is supported.
/// @retval
/// RMT_ERROR_UNSUPPORTED_LOW_SPEC_VERSION  Incompatible version
/// @retval
/// RMT_ERROR_UNSUPPORTED_HIGH_SPEC_VERSION Incompatible version
///
RmtErrorCode RmtFileParserIsChunkSupported(const RmtFileChunkHeader* header);

/// Check if the RMT file can be processed by this build of RMT.
///
/// @param [in]     header      A pointer to a file header
/// @retval
/// kRmtOk                                  The file is compatible.
/// @retval
/// RMT_ERROR_UNSUPPORTED_LOW_SPEC_VERSION  Incompatible version
/// @retval
/// RMT_ERROR_UNSUPPORTED_HIGH_SPEC_VERSION Incompatible version
///
RmtErrorCode RmtFileParserIsFileSupported(const RmtFileHeader* header);

#ifdef __cplusplus
}
#endif
#endif  // #ifndef RMV_PARSER_RMT_FILE_FORMAT_H_
