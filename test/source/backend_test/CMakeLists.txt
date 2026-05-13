#######################################################################################################################
### Copyright (c) 2019-2026 Advanced Micro Devices, Inc. All rights reserved.
### \author AMD Developer Tools Team
#######################################################################################################################

cmake_minimum_required(VERSION 3.10)
project(RmvBackendTest)

find_package (Threads)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(AFTER
    ../../../source/backend
    ../../../source/parser
    ../../../external/third_party/catch2/single_include/catch2
    ../../../source/backend/core)

if(WIN32)
    set(ADDITIONAL_COMPILE_OPTIONS ${ADDITIONAL_COMPILE_OPTIONS}
                                     -wd4389)
    # the test fails to run when built with incremental linking -- disable it for debug builds (where by default it is enabled)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
endif()

# specify output executable name
add_executable(${PROJECT_NAME})

# List of all source files. It may be possible to have the build process call cmake to update the makefiles
# only when this file has changed (ie source files have been added or removed)
target_sources(${PROJECT_NAME} PRIVATE
    "main.cpp"
    "log.cpp"
    "log.h"
    "rmv_test_cases.cpp"
    "rmv_test_cases.h"
    "rmv_test_system_info.cpp"
    "rmv_test_system_info.h"
    "rmv_test_allocations.cpp"
    "rmv_test_allocations.h"
    "rmv_test_resources.cpp"
    "rmv_test_resources.h"
    "rmv_test_segment_info.cpp"
    "rmv_test_segment_info.h"
    "rmv_test_timestamps.cpp"
    "rmv_test_timestamps.h"
    "tests.cpp"
)

if (WIN32)
    target_sources (${PROJECT_NAME} PRIVATE
        "windows/backend_test.rc"
        "windows/resource.h"
    )
endif()

if (WIN32)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION . OPTIONAL COMPONENT debug)
endif()

# Apply common developer tools target options and definitions
devtools_target_options(${PROJECT_NAME})

# CMAKE_<CONFIG>_POSTFIX isn't applied automatically to executable targets so apply manually
IF(CMAKE_DEBUG_POSTFIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
ENDIF(CMAKE_DEBUG_POSTFIX)
IF(CMAKE_RELEASE_POSTFIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX})
ENDIF(CMAKE_RELEASE_POSTFIX)

IF (UNIX)
    target_link_libraries(${PROJECT_NAME} RmvBackend RmvParser Threads::Threads dl)
ELSE()
    target_link_libraries(${PROJECT_NAME} RmvParser RmvBackend Threads::Threads)
ENDIF()

IF (WIN32)
SOURCE_GROUP_BY_FOLDER(${PROJECT_NAME})
ENDIF()
