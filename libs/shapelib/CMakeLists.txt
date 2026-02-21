# ~~~
# Add linkage to the shapelib library
#
# On hosts having the package installed in system locations found by
# cmake use the system code. If not, download sources from shapelib
# github site and apply patches.
#
# Exports: ocpn::shapelib transitive link target
#
# License:      GPLv3+
# Copyright (c) 2023 Alec Leamas
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

cmake_minimum_required(VERSION 3.10.0)

if (TARGET ocpn::shapelib)
  return ()
endif ()

set(shapelib "https://github.com/OSGeo/shapelib")
set(src_url "${shapelib}/archive/refs/tags/v1.6.1.tar.gz")
set(src_hash MD5=ccdcabbc685eb575120fa3eb61003280)

list(APPEND patches
  0001-cmake-Add-guard-for-multiple-inclusions.patch
  0002-cmake-Make-BUILD_TESTING-an-option.patch
  0003-shapefil.h-Use-correct-include-gdal-cpol.h-path.patch
)

if (POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif ()

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

add_library(_SHAPELIB_IF INTERFACE)
add_library(ocpn::shapelib ALIAS _SHAPELIB_IF)

#  Check for system lib and use it if found. Done.
#
find_package(Shapelib)
if (SHAPELIB_FOUND)
  message(STATUS "Using system shapelib ")
  target_link_libraries(_SHAPELIB_IF INTERFACE shapelib::shapelib)
  return ()
endif ()

# Download sources from github
#
if (${CMAKE_VERSION} VERSION_LESS 3.20)
  message(WARNING "Using downloaded sources requires cmake >= 3.20")
endif ()

if (POLICY CMP0169)
  # FetchContent_Populate cannot be used if NEW
  cmake_policy(SET CMP0169 OLD)
endif ()

include(FetchContent)
FetchContent_Declare(shapelib_src URL ${src_url} URL_HASH ${src_hash})
FetchContent_Populate(shapelib_src)
FetchContent_GetProperties(shapelib_src SOURCE_DIR src_dir)
message(STATUS "Using downloaded shapelib at ${src_dir}")

# Apply patches
#
foreach (patch ${patches})
  execute_process(
    COMMAND ${CMAKE_COMMAND}
      -Dpatch_file=${CMAKE_CURRENT_SOURCE_DIR}/patches/${patch}
      -Dpatch_dir=${src_dir}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PatchFile.cmake
    COMMAND ${CMAKE_COMMAND} -E echo "-- Applying patch ${patch}"
  )
endforeach ()

# Configure
#
if (NOT TARGET shp)
  set(BUILD_TESTING OFF CACHE BOOL "")
  set(BUILD_APPS OFF CACHE BOOL "")
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
  set(USE_CPL OFF CACHE BOOL "")
  add_subdirectory(${src_dir} shapelib_dir)
endif ()
target_link_libraries(_SHAPELIB_IF INTERFACE shp)
target_include_directories(_SHAPELIB_IF INTERFACE "${src_dir}")
