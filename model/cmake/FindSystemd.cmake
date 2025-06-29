# FindSystemd
# -------
#
# Find Systemd library
#
# Try to find Systemd library on UNIX systems. The following values are defined
#
#  Exports:
#   systemd::systemd transitive link target
#   SYSTEMD_FOUND         - True if Systemd is available
#
#
#   SYSTEMD_INCLUDE_DIRS  - Include directories for Systemd
#   SYSTEMD_LIBRARIES     - List of libraries for Systemd
#   SYSTEMD_DEFINITIONS   - List of definitions for Systemd
#
#
#
#=============================================================================
# Copyright (c) 2015 Jari Vetoniemi
#
# Distributed under the OSI-approved BSD License (the "License");
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

if (TARGET systemd::systemd)
  return ()
endif ()

include(FeatureSummary)
set_package_properties(Systemd PROPERTIES
   URL "http://freedesktop.org/wiki/Software/systemd/"
   DESCRIPTION "System and Service Manager")

find_package(PkgConfig)
pkg_check_modules(PC_SYSTEMD QUIET libsystemd)
find_library(SYSTEMD_LIBRARIES NAMES systemd ${PC_SYSTEMD_LIBRARY_DIRS})
find_path(SYSTEMD_INCLUDE_DIRS systemd/sd-login.h HINTS ${PC_SYSTEMD_INCLUDE_DIRS})

set(SYSTEMD_DEFINITIONS ${PC_SYSTEMD_CFLAGS_OTHER})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SYSTEMD DEFAULT_MSG SYSTEMD_INCLUDE_DIRS SYSTEMD_LIBRARIES)

add_library(_systemd_if INTERFACE)
add_library(systemd::systemd ALIAS _systemd_if)
target_include_directories(_systemd_if INTERFACE ${SYSTEMD_INCLUDE_DIRS})
target_link_libraries(_systemd_if INTERFACE ${SYSTEMD_LIBRARIES})
target_compile_definitions(_systemd_if INTERFACE ${SYSTEMD_DEFINITIONS})

mark_as_advanced(SYSTEMD_INCLUDE_DIRS SYSTEMD_LIBRARIES SYSTEMD_DEFINITIONS)
