# ~~~
# Add linkage to the rapidjson header-only library
#
# On hosts having the package installed in system locations found by
# cmake use the system code. If not, download sources from rapidjson
# github site and apply patches.
#
# Exports: ocpn::rapidjson transitive link target
#
# License:      GPLv3+
# Copyright (c) 2023 Alec Leamas
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

cmake_minimum_required(VERSION 3.5.0)

if (TARGET ocpn::rapidjson)
  return ()
endif ()

set(rapidjson_url "https://github.com/Tencent/rapidjson")
set(src_url "${rapidjson_url}/archive/refs/tags/v1.1.0.tar.gz")
set(src_hash MD5=badd12c511e081fec6c89c43a7027bce)

list(APPEND patches
  0010-rapidjson-1.1.0-c++20.patch
  0011-do_not_include_gtest_src_dir.patch
  0012-Fix-Wclass-memaccess-warnings-errors.patch
  0013-gcc7.patch
)

if (POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif ()

# Use system or downloaded sources.
#
find_path(RAPIDJSON NAME rapidjson.h PATH_SUFFIXES rapidjson)
if (RAPIDJSON)
  # Drop rapidjson/include from path
  if (${CMAKE_VERSION} VERSION_LESS 3.20.0)
    get_filename_component(RAPIDJSON ${RAPIDJSON} DIRECTORY)
    get_filename_component(RAPIDJSON ${RAPIDJSON} DIRECTORY)
  else ()
    cmake_path(GET RAPIDJSON PARENT_PATH RAPIDJSON)
    cmake_path(GET RAPIDJSON PARENT_PATH RAPIDJSON)
  endif ()
  message(STATUS "Using system rapidjson at ${RAPIDJSON}/include")
  set(src_dir ${RAPIDJSON})
else ()
  if (${CMAKE_VERSION} VERSION_LESS 3.20)
    message(WARNING "Using downloaded sources requires cmake >= 3.20")
  endif ()
  include(FetchContent)
  FetchContent_Declare(rapidjson_src URL ${src_url} URL_HASH ${src_hash})
  FetchContent_Populate(rapidjson_src)
  FetchContent_GetProperties(rapidjson_src SOURCE_DIR src_dir)
  message(STATUS "Using downloaded rapidjson at ${src_dir}/include")

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
endif ()

# Main target and linkage.
#
add_library(_RAPIDJSON_IF INTERFACE)
add_library(ocpn::rapidjson ALIAS _RAPIDJSON_IF)
target_include_directories(_RAPIDJSON_IF INTERFACE "${src_dir}/include")
target_compile_definitions(_RAPIDJSON_IF INTERFACE RAPIDJSON_HAS_STDSTRING=1)
