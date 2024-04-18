# config file support for find_package (shapelib).

# Variables needed by ${PROJECT_NAME}-config-version.cmake
if (MSVC)
  # For checking the compatibility of MSVC_TOOLSET_VERSION; see
  # https://docs.microsoft.com/en-us/cpp/porting/overview-of-potential-upgrade-issues-visual-cpp
  # Assume major version number is obtained by dropping the last decimal
  # digit.
  math (EXPR MSVC_TOOLSET_MAJOR "${MSVC_TOOLSET_VERSION}/10")
else ()
  set (MSVC_TOOLSET_VERSION 0)
  set (MSVC_TOOLSET_MAJOR 0)
endif ()
if (CMAKE_CROSSCOMPILING)
  # Ensure that all "true" (resp. "false") settings are represented by
  # the same string.
  set (CMAKE_CROSSCOMPILING_STR "ON")
else ()
  set (CMAKE_CROSSCOMPILING_STR "OFF")
endif ()

# Find root of install tree relative to CMAKE_INSTALL_CMAKEDIR
file (RELATIVE_PATH PROJECT_ROOT_DIR
  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_CMAKEDIR}" "${CMAKE_INSTALL_PREFIX}")
# strip trailing slash
get_filename_component (PROJECT_ROOT_DIR "${PROJECT_ROOT_DIR}/." PATH)


configure_file (project-config.cmake.in project-config.cmake @ONLY)
configure_file (project-config-version.cmake.in
  project-config-version.cmake @ONLY)
install (FILES
  "${CMAKE_CURRENT_BINARY_DIR}/project-config.cmake"
  DESTINATION "${CMAKE_INSTALL_CMAKEDIR}"
  RENAME "${PROJECT_NAME}-config.cmake")
install (FILES
  "${CMAKE_CURRENT_BINARY_DIR}/project-config-version.cmake"
  DESTINATION "${CMAKE_INSTALL_CMAKEDIR}"
  RENAME "${PROJECT_NAME}-config-version.cmake")
