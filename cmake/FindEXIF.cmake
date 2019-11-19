# - Find libexif
# Find the native EXIF includes and library.
# Once done this will define:
#
#  EXIF_ROOT_DIR       - The base directory of EXIF library.
#  EXIF_INCLUDE_DIRS   - Where to find exif-*.h header files.
#  EXIF_LIBRARIES      - Libraries to be linked with executable.
#  EXIF_FOUND          - True if EXIF library is found.
#
#  EXIF_VERSION_STRING - The version of EXIF library found (x.y.z)
#  EXIF_VERSION_MAJOR  - The major version of EXIF library
#  EXIF_VERSION_MINOR  - The minor version of EXIF library
#  EXIF_VERSION_PATCH  - The patch version of EXIF library
#
# Optional input variables:
#
#  EXIF_USE_STATIC_LIB - This can be set to ON to use the static version of
#      the library. If it does not exist in the system, a warning is printed
#      and the shared library is used.
#
# If libexif is installed in a custom folder (this usually happens on Windows)
# the environment variable EXIF_ROOT_DIR can be set to the base folder of
# the library.
#
#=============================================================================
# Copyright (C) 2011 Simone Rossetto <simros85@gmail.com>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#=============================================================================

### FIND section ###
find_path(EXIF_ROOT_DIR
    NAMES "include/libexif/exif-data.h"
    PATHS ENV EXIF_ROOT_DIR)

find_path(EXIF_INCLUDE_DIR
    NAMES "libexif/exif-data.h"
    HINTS ${EXIF_ROOT_DIR}
    PATHS ENV EXIF_ROOT_DIR
    PATH_SUFFIXES "include")

find_library(EXIF_LIBRARY
    NAMES "exif"
    HINTS ${EXIF_ROOT_DIR}
    PATHS ENV EXIF_ROOT_DIR
    PATH_SUFFIXES "lib")

find_file(_exif_pkg_config_file
    NAMES "libexif.pc"
    HINTS ${EXIF_ROOT_DIR}
    PATHS ENV EXIF_ROOT_DIR
    PATH_SUFFIXES "lib/pkgconfig" "pkgconfig")

if(EXISTS ${_exif_pkg_config_file})
    file(STRINGS "${_exif_pkg_config_file}" _exif_version REGEX "^Version: .*$")
    string(REGEX REPLACE "^.*Version: ([0-9]+).*$" "\\1" EXIF_VERSION_MAJOR "${_exif_version}")
    string(REGEX REPLACE "^.*Version: [0-9]+\\.([0-9]+).*$" "\\1" EXIF_VERSION_MINOR  "${_exif_version}")
    string(REGEX REPLACE "^.*Version: [0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" EXIF_VERSION_PATCH "${_exif_version}")
    set(EXIF_VERSION_STRING "${EXIF_VERSION_MAJOR}.${EXIF_VERSION_MINOR}.${EXIF_VERSION_PATCH}")
else(EXISTS ${_exif_pkg_config_file})
    set(EXIF_VERSION_STRING "")
endif(EXISTS ${_exif_pkg_config_file})


### OPTIONS and COMPONENTS section ###
if(EXIF_USE_STATIC_LIB)
    set(_exif_shared_lib ${EXIF_LIBRARY})
    if(WIN32 AND NOT MSVC)
        string(REPLACE ".dll.a" ".a" _exif_static_lib ${_exif_shared_lib})
    elseif(APPLE)
        string(REPLACE ".dylib" ".a" _exif_static_lib ${_exif_shared_lib})
    elseif(UNIX)
        string(REPLACE ".so" ".a" _exif_static_lib ${_exif_shared_lib})
    endif()

    if(EXISTS ${_exif_static_lib})
        set(EXIF_LIBRARY ${_exif_static_lib})
    else(EXISTS ${_exif_static_lib})
        message(WARNING "EXIF static library does not exist, using the shared one.")
        set(EXIF_LIBRARY ${_exif_shared_lib})
    endif(EXISTS ${_exif_static_lib})
endif(EXIF_USE_STATIC_LIB)

mark_as_advanced(
    EXIF_LIBRARY
    EXIF_INCLUDE_DIR
    _exif_pkg_config_file
    _exif_shared_lib
    _exif_static_lib)


### final check ###
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EXIF
    REQUIRED_VARS EXIF_LIBRARY EXIF_INCLUDE_DIR
    VERSION_VAR EXIF_VERSION_STRING)

if(EXIF_FOUND)
    set(EXIF_INCLUDE_DIRS ${EXIF_INCLUDE_DIR} "${EXIF_INCLUDE_DIR}/libexif")
    set(EXIF_LIBRARIES ${EXIF_LIBRARY})
endif(EXIF_FOUND)
