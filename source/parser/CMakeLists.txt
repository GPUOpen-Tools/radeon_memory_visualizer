cmake_minimum_required(VERSION 3.10)
project(RmvParser)

add_definitions(-DRDF_CXX_BINDINGS)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(AFTER ../parser)

IF(WIN32)
    # Warnings as errors for Windows
    add_compile_options(/W4 /WX)
ELSEIF(UNIX)
    # Remove warnings for Linux in backend
    add_compile_options(-D_LINUX -Wall -Wextra -Werror)
ENDIF(WIN32)

# List of all source files. It may be possible to have the build process call cmake to update the makefiles
# only when this file has changed (ie source files have been added or removed)
set( SOURCES
    "rmt_address_helper.cpp"
    "rmt_address_helper.h"
    "rmt_assert.cpp"
    "rmt_assert.h"
    "rmt_constants.h"
    "rmt_error.h"
    "rmt_file_format.h"
    "rmt_file_format.cpp"
    "rmt_format.h"
    "rmt_parser.cpp"
    "rmt_parser.h"
    "rmt_platform.cpp"
    "rmt_platform.h"
    "rmt_print.cpp"
    "rmt_print.h"
    "rmt_profile.h"
    "rmt_rdf_data_stream.cpp"
    "rmt_rdf_data_stream.h"
    "rmt_rdf_dd_event_info.cpp"
    "rmt_rdf_dd_event_info.h"
    "rmt_rdf_gpu_mem_segment_info.cpp"
    "rmt_rdf_gpu_mem_segment_info.h"
    "rmt_rdf_snapshot_index.cpp"
    "rmt_rdf_snapshot_index.h"
    "rmt_rdf_snapshot_info.cpp"
    "rmt_rdf_snapshot_info.h"
    "rmt_token.cpp"
    "rmt_token.h"
    "rmt_token_heap.cpp"
    "rmt_token_heap.h"
    "rmt_types.h"
    "rmt_util.h"
)

set( LINUX_SOURCES
    "linux/safe_crt.cpp"
    "linux/safe_crt.h"
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

