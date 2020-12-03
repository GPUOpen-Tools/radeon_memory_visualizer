//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author
/// \brief  Global configuration values for the RMT backend.
//=============================================================================

#ifndef RMV_BACKEND_RMT_CONFIGURATION_H_
#define RMV_BACKEND_RMT_CONFIGURATION_H_

/// The maximum length of a name.
#define RMT_MAXIMUM_NAME_LENGTH (1024)

/// The maximum number of segments per process.
#define RMT_MAXIMUM_SEGMENTS (8)

/// The maximum number of entries for a 64GiB memory map at 4KiB page size.
#define RMT_PAGE_TABLE_MAX_SIZE (16777216)

/// The maximum number of processes that a single RMT file will contain.
#define RMT_MAXIMUM_PROCESS_COUNT (1024)

/// The maximum number of snapshot points a single RMT file will contain.
#define RMT_MAXIMUM_SNAPSHOT_POINTS (1024)

/// The maximum nubmer of file path.
#define RMT_MAXIMUM_FILE_PATH (8192)

/// The maximum number of resource history events that can be analyzed.
#define RMT_MAXIMUM_RESOURCE_HISTORY_EVENTS (32768)

/// The maximum number of physical addresses that can be associated with a single resource history.
#define RMT_MAXIMUM_RESOURCE_PHYSICAL_ADDRESSES (32768)

#endif  // #ifndef RMV_BACKEND_RMT_CONFIGURATION_H_
