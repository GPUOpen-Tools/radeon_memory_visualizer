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
    # this warning is caused by QT header files and has been introduced by VS2019 16.9.6
    # disable warning C5240: 'nodiscard': attribute is ignored in this syntactic position
    add_compile_options(/wd5240)
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

## Copy Documentation and Samples to output directory. Note - this target is intentionally not included in
## the default project build. It needs to be explicitly built as a separate project

# Determine where the build process will be placing the binary files
# This is evaluated at project build time - not at CMake generation time
set(BUILD_ROOT $<$<CONFIG:debug>:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}>$<$<CONFIG:release>:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}>)

# Define the option to pass to the sphinx documentation job
set(SPHINX_OPTION public)

find_program(SPHINX_EXECUTABLE sphinx-build)
if(NOT SPHINX_EXECUTABLE)
  message(FATAL_ERROR "SPHINX_EXECUTABLE (sphinx-build) is not found!")
endif()

# group sphinx source files into a sphinx folder
file(GLOB SPHINX_DOC_FILES ${SPHINX_DOC_FILES} ${CMAKE_SOURCE_DIR}/documentation/source/*.rst)
set (SPHINX_DOC_MAIN ${CMAKE_SOURCE_DIR}/documentation/source/conf.py)
source_group("sphinx" FILES ${SPHINX_DOC_FILES} ${SPHINX_DOC_MAIN})

# group release documents into a release_docs folder
set (RELEASE_DOCS_IN_ROOT
     ${CMAKE_SOURCE_DIR}/README.md
     ${CMAKE_SOURCE_DIR}/Release_Notes.txt
     ${CMAKE_SOURCE_DIR}/NOTICES.txt
     ${CMAKE_SOURCE_DIR}/License.txt
)
set (RELEASE_DOCS ${RELEASE_DOCS_IN_ROOT})
source_group("release_docs" FILES ${RELEASE_DOCS})

# hang the sphinx build on the conf.py file and specify a dummy output ("sphinx_output")
# this ensures the sphinx docs are built everytime you ask to build the Documentation target
# Sphinx has proper dependency checking, so this works as expected.
# Once built, clean up any unneeded files.
add_custom_target(Documentation SOURCES ${SPHINX_DOC_FILES} ${RELEASE_DOCS} DEPENDS sphinx_output)
add_custom_command(MAIN_DEPENDENCY ${SPHINX_DOC_MAIN} OUTPUT sphinx_output
    COMMAND ${CMAKE_COMMAND} -E echo "building Sphinx documentation"
    COMMAND ${SPHINX_EXECUTABLE} ${CMAKE_SOURCE_DIR}/documentation/source ${BUILD_ROOT}/docs/help/rmv/html/. -t ${SPHINX_OPTION}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${BUILD_ROOT}/docs/help/rmv/html/.doctrees
    COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_ROOT}/docs/help/rmv/html/allocation_explorer.html
    COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_ROOT}/docs/help/rmv/html/allocation_overview.html
    COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_ROOT}/docs/help/rmv/html/capture.html
    COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_ROOT}/docs/help/rmv/html/carousel.html
    COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_ROOT}/docs/help/rmv/html/heap_overview.html
    COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_ROOT}/docs/help/rmv/html/memory_leak_finder.html
    COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_ROOT}/docs/help/rmv/html/resource_details.html
    COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_ROOT}/docs/help/rmv/html/resource_list.html
    COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_ROOT}/docs/help/rmv/html/resource_overview.html
    COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_ROOT}/docs/help/rmv/html/settings.html
    COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_ROOT}/docs/help/rmv/html/snapshot_delta.html
    COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_ROOT}/docs/help/rmv/html/timeline.html
)

add_custom_command(TARGET Documentation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copying Documentation to output directory"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_ROOT}/docs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RELEASE_DOCS_IN_ROOT}           ${BUILD_ROOT}/.
    COMMAND ${CMAKE_COMMAND} -E echo "copying Samples to output directory"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_ROOT}/samples
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/samples/sampleTrace.rmv  ${BUILD_ROOT}/samples/.
)
