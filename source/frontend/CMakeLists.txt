#######################################################################################################################
### Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
### \author AMD Developer Tools Team
#######################################################################################################################

cmake_minimum_required(VERSION 3.11)

project(RadeonMemoryVisualizer)

add_definitions(-DSYSTEM_INFO_ENABLE_RDF)

IF(WIN32)
    # this warning is caused by the QT header files - use pragma to disable at source
    # disable warning C4127: conditional expression is constant
    add_compile_options(/wd4127)
    # this warning is caused by QT header files and has been introduced by VS2019 16.9.6
    # disable warning C5240: 'nodiscard': attribute is ignored in this syntactic position
    add_compile_options(/wd5240)
ENDIF(WIN32)

# switch on the autogenerators needed for Qt. Includes the MetaObject Compiler (MOC),
# the UI compiler (UIC) and the resource compiler (RCC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Add UpdateCheckAPI
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../external/update_check_api ${CMAKE_CURRENT_BINARY_DIR}/update_check_api)
include_directories(${UPDATECHECKAPI_INC_DIRS})

IF (WIN32)
    # get the location of the Visual Studio redist libraries - but don't create an install rule to install them
    # the libs will be copied manually as a post build step
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    include(InstallRequiredSystemLibraries)
ENDIF(WIN32)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(AFTER ../backend ../parser ../frontend)

IF(UNIX)
    find_package(Threads)
ENDIF(UNIX)

# List of all source files. It may be possible to have the build process call cmake to update the makefiles
# only when this file has changed (ie source files have been added or removed)


set( SOURCES
    "main.cpp"
    "managers/load_animation_manager.cpp"
    "managers/load_animation_manager.h"
    "managers/message_manager.cpp"
    "managers/message_manager.h"
    "managers/navigation_manager.cpp"
    "managers/navigation_manager.h"
    "managers/pane_manager.cpp"
    "managers/pane_manager.h"
    "managers/snapshot_manager.cpp"
    "managers/snapshot_manager.h"
    "managers/trace_manager.cpp"
    "managers/trace_manager.h"
    "models/aliased_resource_model.cpp"
    "models/aliased_resource_model.h"
    "models/allocation_bar_model.cpp"
    "models/allocation_bar_model.h"
    "models/allocation_item_model.cpp"
    "models/allocation_item_model.h"
    "models/allocation_multi_bar_model.cpp"
    "models/allocation_multi_bar_model.h"
    "models/colorizer.cpp"
    "models/colorizer.h"
    "models/colorizer_base.cpp"
    "models/colorizer_base.h"
    "models/carousel_model.cpp"
    "models/carousel_model.h"
    "models/combo_box_model.cpp"
    "models/combo_box_model.h"
    "models/heap_combo_box_model.cpp"
    "models/heap_combo_box_model.h"
    "models/resource_item_model.cpp"
    "models/resource_item_model.h"
    "models/resource_sorter.cpp"
    "models/resource_sorter.h"
    "models/resource_usage_combo_box_model.cpp"
    "models/resource_usage_combo_box_model.h"
    "models/compare/memory_leak_finder_model.h"
    "models/compare/memory_leak_finder_model.cpp"
    "models/compare/snapshot_delta_model.h"
    "models/compare/snapshot_delta_model.cpp"
    "models/proxy_models/allocation_proxy_model.h"
    "models/proxy_models/allocation_proxy_model.cpp"
    "models/proxy_models/table_proxy_model.h"
    "models/proxy_models/table_proxy_model.cpp"
    "models/proxy_models/resource_details_proxy_model.h"
    "models/proxy_models/resource_details_proxy_model.cpp"
    "models/proxy_models/resource_proxy_model.h"
    "models/proxy_models/resource_proxy_model.cpp"
    "models/proxy_models/snapshot_timeline_proxy_model.h"
    "models/proxy_models/snapshot_timeline_proxy_model.cpp"
    "models/proxy_models/memory_leak_finder_proxy_model.h"
    "models/proxy_models/memory_leak_finder_proxy_model.cpp"
    "models/timeline/device_configuration_model.h"
    "models/timeline/device_configuration_model.cpp"
    "models/timeline/snapshot_item_model.h"
    "models/timeline/snapshot_item_model.cpp"
    "models/timeline/timeline_colorizer.cpp"
    "models/timeline/timeline_colorizer.h"
    "models/timeline/timeline_model.cpp"
    "models/timeline/timeline_model.h"
    "models/snapshot/allocation_explorer_model.h"
    "models/snapshot/allocation_explorer_model.cpp"
    "models/snapshot/allocation_overview_model.h"
    "models/snapshot/allocation_overview_model.cpp"
    "models/snapshot/resource_details_model.h"
    "models/snapshot/resource_details_model.cpp"
    "models/snapshot/resource_list_model.h"
    "models/snapshot/resource_list_model.cpp"
    "models/snapshot/resource_properties_model.h"
    "models/snapshot/resource_properties_model.cpp"
    "models/snapshot/resource_overview_model.h"
    "models/snapshot/resource_overview_model.cpp"
    "models/snapshot/resource_timeline_item_model.cpp"
    "models/snapshot/resource_timeline_item_model.h"
    "models/snapshot/heap_overview_heap_model.h"
    "models/snapshot/heap_overview_heap_model.cpp"
    "settings/rmv_geometry_settings.cpp"
    "settings/rmv_geometry_settings.h"
    "settings/rmv_settings.cpp"
    "settings/rmv_settings.h"
    "settings/settings_reader.cpp"
    "settings/settings_reader.h"
    "settings/settings_writer.cpp"
    "settings/settings_writer.h"
    "util/constants.h"
    "util/definitions.h"
    "util/log_file_writer.cpp"
    "util/log_file_writer.h"
    "util/rmv_util.cpp"
    "util/rmv_util.h"
    "util/string_util.cpp"
    "util/string_util.h"
    "util/thread_controller.cpp"
    "util/thread_controller.h"
    "util/time_util.cpp"
    "util/time_util.h"
    "util/version.cpp"
    "util/version.h"
    "util/widget_util.cpp"
    "util/widget_util.h"
    "views/base_pane.cpp"
    "views/base_pane.h"
    "views/compare_pane.cpp"
    "views/compare_pane.h"
    "views/debug_window.cpp"
    "views/debug_window.h"
    "views/debug_window.ui"
    "views/keyboard_zoom_shortcuts.cpp"
    "views/keyboard_zoom_shortcuts.h"
    "views/main_window.cpp"
    "views/main_window.h"
    "views/main_window.ui"
    "views/custom_widgets/rmv_cancellable_loading_widget.cpp"
    "views/custom_widgets/rmv_cancellable_loading_widget.h"
    "views/custom_widgets/rmv_allocation_bar.cpp"
    "views/custom_widgets/rmv_allocation_bar.h"
    "views/custom_widgets/rmv_camera_snapshot_widget.cpp"
    "views/custom_widgets/rmv_camera_snapshot_widget.h"
    "views/custom_widgets/rmv_carousel.cpp"
    "views/custom_widgets/rmv_carousel.h"
    "views/custom_widgets/rmv_carousel_allocation_sizes.cpp"
    "views/custom_widgets/rmv_carousel_allocation_sizes.h"
    "views/custom_widgets/rmv_carousel_item.cpp"
    "views/custom_widgets/rmv_carousel_item.h"
    "views/custom_widgets/rmv_carousel_memory_footprint.cpp"
    "views/custom_widgets/rmv_carousel_memory_footprint.h"
    "views/custom_widgets/rmv_carousel_memory_types.cpp"
    "views/custom_widgets/rmv_carousel_memory_types.h"
    "views/custom_widgets/rmv_carousel_nav_button.cpp"
    "views/custom_widgets/rmv_carousel_nav_button.h"
    "views/custom_widgets/rmv_carousel_resource_types.cpp"
    "views/custom_widgets/rmv_carousel_resource_types.h"
    "views/custom_widgets/rmv_colored_checkbox.cpp"
    "views/custom_widgets/rmv_colored_checkbox.h"
    "views/custom_widgets/rmv_color_picker_button.cpp"
    "views/custom_widgets/rmv_color_picker_button.h"
    "views/custom_widgets/rmv_color_picker_widget.cpp"
    "views/custom_widgets/rmv_color_picker_widget.h"
    "views/custom_widgets/rmv_color_swatch_tooltip_item.cpp"
    "views/custom_widgets/rmv_color_swatch_tooltip_item.h"
    "views/custom_widgets/rmv_delta_display.cpp"
    "views/custom_widgets/rmv_delta_display.h"
    "views/custom_widgets/rmv_delta_display_widget.cpp"
    "views/custom_widgets/rmv_delta_display_widget.h"
    "views/custom_widgets/rmv_fixed_width_label.cpp"
    "views/custom_widgets/rmv_fixed_width_label.h"
    "views/custom_widgets/rmv_heap_overview_memory_bar.cpp"
    "views/custom_widgets/rmv_heap_overview_memory_bar.h"
    "views/custom_widgets/rmv_range_slider.cpp"
    "views/custom_widgets/rmv_range_slider.h"
    "views/custom_widgets/rmv_resource_details.cpp"
    "views/custom_widgets/rmv_resource_details.h"
    "views/custom_widgets/rmv_resource_timeline.cpp"
    "views/custom_widgets/rmv_resource_timeline.h"
    "views/custom_widgets/rmv_scaled_donut_widget.cpp"
    "views/custom_widgets/rmv_scaled_donut_widget.h"
    "views/custom_widgets/rmv_snapshot_marker.cpp"
    "views/custom_widgets/rmv_snapshot_marker.h"
    "views/custom_widgets/rmv_snapshot_table_view.cpp"
    "views/custom_widgets/rmv_snapshot_table_view.h"
    "views/custom_widgets/rmv_snapshot_timeline.cpp"
    "views/custom_widgets/rmv_snapshot_timeline.h"
    "views/custom_widgets/rmv_timeline_graph.cpp"
    "views/custom_widgets/rmv_timeline_graph.h"
    "views/custom_widgets/rmv_tooltip.cpp"
    "views/custom_widgets/rmv_tooltip.h"
    "views/custom_widgets/rmv_tree_map_blocks.cpp"
    "views/custom_widgets/rmv_tree_map_blocks.h"
    "views/custom_widgets/rmv_tree_map_view.cpp"
    "views/custom_widgets/rmv_tree_map_view.h"
    "views/custom_widgets/themes_and_colors_item_button.cpp"
    "views/custom_widgets/themes_and_colors_item_button.h"
    "views/compare/compare_start_pane.cpp"
    "views/compare/compare_start_pane.h"
    "views/compare/compare_start_pane.ui"
    "views/compare/memory_leak_finder_pane.h"
    "views/compare/memory_leak_finder_pane.cpp"
    "views/compare/memory_leak_finder_pane.ui"
    "views/compare/snapshot_delta_pane.h"
    "views/compare/snapshot_delta_pane.cpp"
    "views/compare/snapshot_delta_pane.ui"
    "views/delegates/rmv_compare_id_delegate.h"
    "views/delegates/rmv_compare_id_delegate.cpp"
    "views/delegates/rmv_snapshot_name_delegate.cpp"
    "views/delegates/rmv_snapshot_name_delegate.h"
    "views/delegates/rmv_regular_expression_validator.cpp"
    "views/delegates/rmv_regular_expression_validator.h"
    "views/delegates/rmv_resource_event_delegate.h"
    "views/delegates/rmv_resource_event_delegate.cpp"
    "views/settings/settings_pane.cpp"
    "views/settings/settings_pane.h"
    "views/settings/settings_pane.ui"
    "views/settings/keyboard_shortcuts_pane.cpp"
    "views/settings/keyboard_shortcuts_pane.h"
    "views/settings/keyboard_shortcuts_pane.ui"
    "views/settings/themes_and_colors_pane.cpp"
    "views/settings/themes_and_colors_pane.h"
    "views/settings/themes_and_colors_pane.ui"
    "views/snapshot/allocation_explorer_pane.h"
    "views/snapshot/allocation_explorer_pane.cpp"
    "views/snapshot/allocation_explorer_pane.ui"
    "views/snapshot/allocation_overview_pane.h"
    "views/snapshot/allocation_overview_pane.cpp"
    "views/snapshot/allocation_overview_pane.ui"
    "views/snapshot/heap_overview_heap_layout.h"
    "views/snapshot/heap_overview_heap_layout.cpp"
    "views/snapshot/heap_overview_heap_layout.ui"
    "views/snapshot/heap_overview_pane.h"
    "views/snapshot/heap_overview_pane.cpp"
    "views/snapshot/heap_overview_pane.ui"
    "views/snapshot/resource_details_pane.h"
    "views/snapshot/resource_details_pane.cpp"
    "views/snapshot/resource_details_pane.ui"
    "views/snapshot/resource_list_pane.h"
    "views/snapshot/resource_list_pane.cpp"
    "views/snapshot/resource_list_pane.ui"
    "views/snapshot/resource_event_icons.h"
    "views/snapshot/resource_event_icons.cpp"
    "views/snapshot/resource_overview_pane.h"
    "views/snapshot/resource_overview_pane.cpp"
    "views/snapshot/resource_overview_pane.ui"
    "views/snapshot/snapshot_start_pane.cpp"
    "views/snapshot/snapshot_start_pane.h"
    "views/snapshot/snapshot_start_pane.ui"
    "views/start/welcome_pane.cpp"
    "views/start/welcome_pane.h"
    "views/start/welcome_pane.ui"
    "views/start/recent_traces_pane.cpp"
    "views/start/recent_traces_pane.h"
    "views/start/recent_traces_pane.ui"
    "views/start/about_pane.cpp"
    "views/start/about_pane.h"
    "views/start/about_pane.ui"
    "views/timeline/device_configuration_pane.h"
    "views/timeline/device_configuration_pane.cpp"
    "views/timeline/device_configuration_pane.ui"
    "views/timeline/keyboard_zoom_shortcuts_timeline.h"
    "views/timeline/keyboard_zoom_shortcuts_timeline.cpp"
    "views/timeline/timeline_pane.h"
    "views/timeline/timeline_pane.cpp"
    "views/timeline/timeline_pane.ui"
    "resources.qrc"
    "dark_mode_stylesheet.qss"
    "light_mode_stylesheet.qss"
    "stylesheet.qss"
    ${UPDATECHECKAPI_SRC}
    ${UPDATECHECKAPI_INC}
    ${UPDATECHECKAPI_QT_SRC}
    ${UPDATECHECKAPI_QT_INC}
    ${UPDATECHECKAPI_QT_UI}
)

set( WINDOWS_SOURCES
    "windows/resource.h"
    "windows/rmv.rc"
    "windows/rmv_icon.ico"
)


# Filter out the UI files and get the list of generated files
set(UI_FILES ${SOURCES})
list(FILTER UI_FILES INCLUDE REGEX "\.ui$")
qt_wrap_ui (GENERATED_UI ${UI_FILES})
set(SOURCES ${SOURCES} ${GENERATED_UI})

# searching for library file

# Specify output executable name. For windows, indicate this is a Windows application
# and not a console application
IF (WIN32)
    qt_add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${WINDOWS_SOURCES})
ELSEIF (APPLE)
    IF (NO_APP_BUNDLE)
        qt_add_executable(${PROJECT_NAME} ${SOURCES})
    ELSE ()
        set(MACOSX_BUNDLE_ICON_FILE rmv.icns)
        set(MACOSX_ICON mac_osx/rmv.icns)
        set_source_files_properties(${MACOSX_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        qt_add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${MACOSX_ICON} ${SOURCES})
        set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_RPATH TRUE MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/mac_osx/MacOSXBundleInfo.plist.in)
    ENDIF ()
ELSEIF (UNIX)
    qt_add_executable(${PROJECT_NAME} ${SOURCES})
    # Setting RPATH here results in local build directory also being appended to RPATH
    # RPATH set instead by calling chrpath command line command for cleaner solution.
    # Correct cmake solution will likely require use of a separate make install
    # for packaging
    # Leaving these Commands here for future reference
    #    set_target_properties(${PROJECT_NAME} PROPERTIES
    #      BUILD_WITH_INSTALL_RPATH TRUE
    #      INSTALL_RPATH_USE_LINK_PATH TRUE
    #      INSTALL_RPATH "\$ORIGIN/qt/lib"
    #)
ENDIF (WIN32)

# Apply common developer tools target options and definitions
devtools_target_options(${PROJECT_NAME})

IF (WIN32 OR APPLE)
    SOURCE_GROUP_BY_FOLDER(${PROJECT_NAME})
ENDIF ()

# CMAKE_<CONFIG>_POSTFIX isn't applied automatically to executable targets so apply manually
IF (CMAKE_DEBUG_POSTFIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
ENDIF (CMAKE_DEBUG_POSTFIX)
IF (CMAKE_RELEASE_POSTFIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX})
ENDIF (CMAKE_RELEASE_POSTFIX)

# executable file library dependency list
IF (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE RmvBackend RmvParser system_info rdf Qt::Widgets QtCustomWidgets QtUtils wsock32 winmm ${UPDATECHECKAPI_LIBS})
ELSEIF (UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE RmvBackend RmvParser system_info rdf Qt::Widgets QtCustomWidgets QtUtils Threads::Threads ${UPDATECHECKAPI_LIBS})
ENDIF ()

if (LINUX)
    # Ensure that wayland plugin integration is enabled for deployment to work
    # TODO(mguerret): This could eventually be removed if Qt improves their deployment script logic
    qt_import_plugins(${PROJECT_NAME} INCLUDE Qt6::QWaylandIntegrationPlugin)

    target_link_libraries(${PROJECT_NAME} PRIVATE Qt::WaylandClient)
endif ()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "copying rtda to output directory"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RTDA_PATH} $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Set the RPATH properly on Linux
if (LINUX)
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
endif ()

# Install commands for CPack
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION . COMPONENT apps
        BUNDLE DESTINATION . COMPONENT apps
        ARCHIVE DESTINATION lib/ COMPONENT apps
        LIBRARY DESTINATION lib/ COMPONENT apps)

if (LINUX)
    install(FILES ${RTDA_PATH} PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE DESTINATION . COMPONENT apps)
else()
    install(FILES ${RTDA_PATH} DESTINATION . COMPONENT apps)
endif()

if (WIN32)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION . OPTIONAL COMPONENT debug)
endif()


# Deploy Qt binaries for build and install
deploy_qt(${PROJECT_NAME} apps)

