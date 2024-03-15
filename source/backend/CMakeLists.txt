#######################################################################################################################
### Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
### \author AMD Developer Tools Team
#######################################################################################################################

cmake_minimum_required(VERSION 3.10)
project(RmvBackend)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(AFTER ../backend ../parser ../../external/rdf/rdf/inc ../third_party)

add_definitions(-DSYSTEM_INFO_ENABLE_RDF)
add_definitions(-DRDF_CXX_BINDINGS)

# List of all source files. It may be possible to have the build process call cmake to update the makefiles
# only when this file has changed (ie source files have been added or removed)

set( SOURCES
    "rmt_adapter_info.cpp"
    "rmt_adapter_info.h"
    "rmt_atomic.cpp"
    "rmt_atomic.h"
    "rmt_configuration.h"
    "rmt_data_profile.h"
    "rmt_data_set.cpp"
    "rmt_data_set.h"
    "rmt_data_set.h"
    "rmt_data_snapshot.cpp"
    "rmt_data_snapshot.h"
    "rmt_data_timeline.cpp"
    "rmt_data_timeline.h"
    "rmt_job_system.cpp"
    "rmt_job_system.h"
    "rmt_linear_buffer.cpp"
    "rmt_linear_buffer.h"
    "rmt_legacy_snapshot_writer.cpp"
    "rmt_legacy_snapshot_writer.h"
    "rmt_memory_aliasing_timeline.cpp"
    "rmt_memory_aliasing_timeline.h"
    "rmt_memory_event_history.cpp"
    "rmt_memory_event_history.h"
    "rmt_memory_event_history_impl.cpp"
    "rmt_memory_event_history_impl.h"
    "rmt_mutex.cpp"
    "rmt_mutex.h"
    "rmt_page_table.cpp"
    "rmt_page_table.h"
    "rmt_physical_allocation_list.cpp"
    "rmt_physical_allocation_list.h"
    "rmt_pool.cpp"
    "rmt_pool.h"
    "rmt_process_map.cpp"
    "rmt_process_map.h"
    "rmt_process_start_info.h"
    "rmt_rdf_file_parser.h"
    "rmt_rdf_file_parser.cpp"
    "rmt_rdf_snapshot_writer.cpp"
    "rmt_rdf_snapshot_writer.h"
    "rmt_rdf_system_info.h"
    "rmt_resource_history.cpp"
    "rmt_resource_history.h"
    "rmt_resource_list.cpp"
    "rmt_resource_list.h"
    "rmt_resource_userdata.cpp"
    "rmt_resource_userdata.h"
    "rmt_segment_info.h"
    "rmt_snapshot_writer.cpp"
    "rmt_snapshot_writer.h"
    "rmt_thread.cpp"
    "rmt_thread.h"
    "rmt_thread_event.cpp"
    "rmt_thread_event.h"
    "rmt_trace_loader.cpp"
    "rmt_trace_loader.h"
    "rmt_virtual_allocation_list.cpp"
    "rmt_virtual_allocation_list.h"
    "rmt_warnings.cpp"
    "rmt_warnings.h"
)

set( LINUX_SOURCES
    "rmt_include_pevents.cpp"
)


# specify output library name
IF(WIN32)
    add_library(${PROJECT_NAME} ${SOURCES})
ELSEIF(UNIX)
    add_library(${PROJECT_NAME} ${SOURCES} ${LINUX_SOURCES})
ENDIF(WIN32)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

IF(WIN32)
# Create Visual Studio filters so that the source files in the project match the directory structure
foreach(source IN LISTS SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
ENDIF(WIN32)

devtools_target_options(${PROJECT_NAME})

