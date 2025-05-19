//=============================================================================
// Copyright (c) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  The system information captured in the memory trace.
//=============================================================================

#ifndef RMV_BACKEND_RMT_RDF_SYSTEM_INFO_H_
#define RMV_BACKEND_RMT_RDF_SYSTEM_INFO_H_

#include "rmt_file_format.h"

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// The maximum length for the CPU name.
#define RMT_MAX_CPU_NAME_LENGTH (64)

/// The maximum length of memory type name.
#define RMT_MAX_MEMORY_TYPE_NAME_LENGTH (16)

/// The maximum length for the driver packaging version.
#define RMT_MAX_DRIVER_PACKAGING_VERSION_NAME_LENGTH (128)

/// The maximum length for the driver software version.
#define RMT_MAX_DRIVER_SOFTWARE_VERSION_NAME_LENGTH (64)

/// The maximum length for the operating system name.
#define RMT_MAX_OS_NAME_LENGTH (64)

/// @brief A structure encapsulating the information about the system.
typedef struct RmtRdfSystemInfo
{
    char     cpu_name[RMT_MAX_CPU_NAME_LENGTH];                                            ///< The CPU name as a NULL terminated string.
    uint32_t cpu_max_clock_speed;                                                          ///< The maximum CPU clock speed in MHz.
    uint32_t num_physical_cores;                                                           ///< The number of physical cores on the CPU.
    uint32_t num_logical_cores;                                                            ///< The number of logical cores on the CPU.
    uint64_t system_physical_memory_size;                                                  ///< The amount of system memory in bytes.
    char     system_memory_type_name[RMT_MAX_MEMORY_TYPE_NAME_LENGTH];                     ///< The system memory type text string.
    char     name[RMT_MAX_ADAPTER_NAME_LENGTH];                                            ///< The name of the adapter as a NULL terminated string.
    uint32_t pcie_family_id;                                                               ///< The PCIe family ID of the adapter.
    uint32_t pcie_revision_id;                                                             ///< The PCIe revision ID of the adapter.
    uint32_t device_id;                                                                    ///< The PCIe device ID of the adapter.
    uint32_t minimum_engine_clock;                                                         ///< The minimum engine clock (in MHz).
    uint32_t maximum_engine_clock;                                                         ///< The maximum engine clock (in MHz).
    uint32_t memory_operations_per_clock;                                                  ///< The number of memory operations that can be performed per clock.
    uint32_t memory_bus_width;                                                             ///< The width of the memory bus (in bits).
    uint32_t memory_bandwidth;                                                             ///< Bandwidth of the memory system (in MB/s).
    uint32_t minimum_memory_clock;                                                         ///< The minimum memory clock (in MHz).
    uint32_t maximum_memory_clock;                                                         ///< The maximum memory clock (in MHz).
    char     video_memory_type_name[RMT_MAX_MEMORY_TYPE_NAME_LENGTH];                      ///< The video memory type text string.
    char     os_name[RMT_MAX_OS_NAME_LENGTH];                                              ///< The operating system name.
    char     driver_packaging_version_name[RMT_MAX_DRIVER_PACKAGING_VERSION_NAME_LENGTH];  ///< The driver packaging name.
    char     driver_software_version_name[RMT_MAX_DRIVER_SOFTWARE_VERSION_NAME_LENGTH];    ///< The driver software version name.
} RmtRdfSystemInfo;

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // #ifndef RMV_BACKEND_RMT_RDF_SYSTEM_INFO_H_
