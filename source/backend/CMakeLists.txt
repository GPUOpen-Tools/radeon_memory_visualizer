cmake_minimum_required(VERSION 3.5.1)
project(RmvBackend)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(AFTER ../backend ../parser)

IF(UNIX)
    # Remove warnings for Linux in backend
    add_compile_options(-std=c++11 -D_LINUX -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-sign-compare -Wno-uninitialized)
ENDIF(UNIX)

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
    "rmt_resource_history.cpp"
    "rmt_resource_history.h"
    "rmt_resource_list.cpp"
    "rmt_resource_list.h"
    "rmt_segment_info.h"
    "rmt_thread.cpp"
    "rmt_thread.h"
    "rmt_thread_event.cpp"
    "rmt_thread_event.h"
    "rmt_virtual_allocation_list.cpp"
    "rmt_virtual_allocation_list.h"
    "rmt_warnings.cpp"
    "rmt_warnings.h"
)

set( LINUX_SOURCES
    "../third_party/pevents/pevents.cpp"
    "../third_party/pevents/pevents.h"
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

