# Post-build CPack script to reshape component archives to include a top-level directory

# _out_dir = _CPack_Packages/Linux/TGZ etc.
set(_out_dir "${CPACK_TEMPORARY_DIRECTORY}/..")
set(_folder_name "${CPACK_ARCHIVE_APPS_FILE_NAME}")
set(_is_tgz FALSE)
set(_is_zip FALSE)

if (WIN32)
    set(_is_zip TRUE)
    set(_apps_archive "${_out_dir}/${_folder_name}.zip")
else()
    set (_is_tgz TRUE)
    set(_apps_archive "${_out_dir}/${_folder_name}.tgz")
    if (NOT EXISTS "${_apps_archive}" )
        set(_apps_archive "${_out_dir}/${_folder_name}.tar.gz")
    endif()
endif()

if (EXISTS "${_apps_archive}")
    message(STATUS "Post-packaging: Setting archive top-level directory to '${_folder_name}'")

    # Create top-level directory
    set(_repackage_dir "${CPACK_TEMPORARY_DIRECTORY}/../${_folder_name}")
    message(STATUS "Creating ${_repackage_dir}")
    file(REMOVE_RECURSE "${_repackage_dir}")
    file(MAKE_DIRECTORY "${_repackage_dir}")

    # Extract using cmake -E tar (gz) or zip
    message(STATUS "Extracting ${CPACK_GENERATOR} to '${_repackage_dir}'")
    if (_is_zip)
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E tar xf "${_apps_archive}" --format=zip
            WORKING_DIRECTORY "${_repackage_dir}"
            RESULT_VARIABLE _x_res)
    else()
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E tar xzf "${_apps_archive}"
            WORKING_DIRECTORY "${_repackage_dir}"
            RESULT_VARIABLE _x_res)
    endif()
    if (NOT _x_res EQUAL 0)
        message(WARNING "Failed to extract ${_apps_archive}: code ${_x_res}")
        return()
    endif()

    # Remove the original archive
    message(STATUS "Removing original '${_apps_archive}'")
    file(REMOVE "${_apps_archive}")

    # Create new archive
    message(STATUS "Creating new ${CPACK_GENERATOR} '${_apps_archive}'")
    if (_is_zip)
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E tar cf "${_apps_archive}" "${_folder_name}" --format=zip
            WORKING_DIRECTORY "${CPACK_TEMPORARY_DIRECTORY}/.."
            RESULT_VARIABLE _c_res)
    else()
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E tar czf "${_apps_archive}" "${_folder_name}"
            WORKING_DIRECTORY "${CPACK_TEMPORARY_DIRECTORY}/.."
            RESULT_VARIABLE _c_res)
    endif()
    if (NOT _c_res EQUAL 0)
        message(WARNING "Failed to create reshaped archive ${_apps_archive}: code ${_c_res}")
        return()
    endif()
else()
    message(WARNING "Archive not found: ${_apps_archive}")
endif()

