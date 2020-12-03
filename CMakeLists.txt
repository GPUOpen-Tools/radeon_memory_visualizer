cmake_minimum_required(VERSION 3.11)

## Specify the top level name of the project - this will define the solution name for Visual Studio
project(RMV)

## For RMV we only care about the Debug and Release configuration types
set(CMAKE_CONFIGURATION_TYPES Debug Release)

## Determine build suffixes based on configuration, bitness and internal status
## These values will be inherited by all child projects
set(ADT_PLATFORM_POSTFIX "-x86")
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ADT_PLATFORM_POSTFIX "-x64")
ENDIF()

# As default for RMV, include the debug & internal status in filename - but not the platform bitness
set (CMAKE_DEBUG_POSTFIX -d${ADT_INTERNAL_POSTFIX})
set (CMAKE_RELEASE_POSTFIX ${ADT_INTERNAL_POSTFIX})

IF(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../release${ADT_INTERNAL_POSTFIX})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../debug${ADT_INTERNAL_POSTFIX})
ELSE(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../../release${ADT_INTERNAL_POSTFIX})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../../debug${ADT_INTERNAL_POSTFIX})
ENDIF(WIN32)

# Add for CentOS compiler warning
add_definitions(-DJSON_SKIP_UNSUPPORTED_COMPILER_CHECK)

include_directories("${PROJECT_SOURCE_DIR}/external/qt_common/")
include_directories("${PROJECT_SOURCE_DIR}/external/")

# Global compiler options
IF(WIN32)
    add_compile_options(/W4 /WX /MP)
    # disable warning C4201: nonstandard extension used: nameless struct/union
    add_compile_options(/wd4201)
    # this warning is caused by the QT header files - use pragma to disable at source
    # disable warning C4127: conditional expression is constant
    add_compile_options(/wd4127)
    # bump the stack size
    add_link_options(/STACK:16777216)
ELSEIF(UNIX)
    # Use -Wno-missing-field-initializers for CentOS compiler warning
    add_compile_options(-std=c++11 -D_LINUX -Wall -Wextra -Werror -Wno-missing-field-initializers)
    # Use _DEBUG on Unix for Debug Builds (defined automatically on Windows)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
ENDIF(WIN32)

IF(WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")
ENDIF(WIN32)

# Macro to build source groups to match directory structure
MACRO(SOURCE_GROUP_BY_FOLDER target)
    SET(SOURCE_GROUP_DELIMITER "/")
    SET(last_dir "")
    SET(files "")
    FOREACH(file ${SOURCES})
        GET_FILENAME_COMPONENT(dir "${file}" PATH)
        IF (NOT "${dir}" STREQUAL "${last_dir}")
            IF (files)
            SOURCE_GROUP("${last_dir}" FILES ${files})
            ENDIF (files)
            SET(files "")
        ENDIF (NOT "${dir}" STREQUAL "${last_dir}")
        SET(files ${files} ${file})
        SET(last_dir "${dir}")
    ENDFOREACH(file)
    IF (files)
        SOURCE_GROUP("${last_dir}" FILES ${files})
    ENDIF (files)
ENDMACRO(SOURCE_GROUP_BY_FOLDER)

add_subdirectory(external/qt_common/custom_widgets QtCommon/custom_widgets)
add_subdirectory(external/qt_common/utils QtCommon/utils)
add_subdirectory(source/parser parser)
add_subdirectory(source/backend backend)
add_subdirectory(source/frontend frontend)

# Group external dependency targets into folder
IF(WIN32)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(QtCustomWidgets
                      QtUtils
    PROPERTIES
    FOLDER Dependencies
)
ELSEIF(APPLE)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(QtCustomWidgets
                      QtUtils
    PROPERTIES
    FOLDER Dependencies
)
ENDIF()

IF(WIN32)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RadeonMemoryVisualizer)
ENDIF(WIN32)

## Copy Documentation and Samples to output directory
add_custom_target(Documentation ALL)
add_custom_command(TARGET Documentation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copying documentation to output directory"
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:RadeonMemoryVisualizer>/docs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/documentation/License.htm     $<TARGET_FILE_DIR:RadeonMemoryVisualizer>/docs/.
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/README.md                     $<TARGET_FILE_DIR:RadeonMemoryVisualizer>/.
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/Release_Notes.txt             $<TARGET_FILE_DIR:RadeonMemoryVisualizer>/.
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/NOTICES.txt                   $<TARGET_FILE_DIR:RadeonMemoryVisualizer>/.
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/License.txt                   $<TARGET_FILE_DIR:RadeonMemoryVisualizer>/.
    COMMAND ${CMAKE_COMMAND} -E echo "copying samples to output directory"
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:RadeonMemoryVisualizer>/samples
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/samples/sampleTrace.rmv       $<TARGET_FILE_DIR:RadeonMemoryVisualizer>/samples/.
)
