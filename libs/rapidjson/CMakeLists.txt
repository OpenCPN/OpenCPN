#
# Add linkage to the rapidjson header-only library
#
# On hosts having the package installed in system locations found by
# cmake use the system code. If not, download sources from rapidjson
# github site.
#
# Exports: ocpn::rapidjson link target

cmake_minimum_required(VERSION 3.1.20)

if (TARGET ocpn::rapidjson)
    return ()
endif ()

set(src_url "https://github.com/Tencent/rapidjson/archive/refs/tags/v1.1.0.tar.gz")
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
find_path(RAPIDJSON_BASE NAME rapidjson.h PATH_SUFFIXES rapidjson)
if (RAPIDJSON_BASE)
  # Drop include/rapidjson
  if (${CMAKE_VERSION} VERSION_LESS 3.20.0)
    get_filename_component(RAPIDJSON_BASE ${RAPIDJSON_BASE} DIRECTORY)
    get_filename_component(RAPIDJSON_BASE ${RAPIDJSON_BASE} DIRECTORY)
  else ()
    cmake_path(GET RAPIDJSON_BASE PARENT_PATH RAPIDJSON_BASE)
    cmake_path(GET RAPIDJSON_BASE PARENT_PATH RAPIDJSON_BASE)
  endif ()
  message(STATUS "Using system rapidjson at ${RAPIDJSON_BASE}/include")
  set(SRC_DIR ${RAPIDJSON_BASE})
else ()
  include(FetchContent)
  FetchContent_Declare(rapidjson URL ${src_url} URL_HASH ${src_hash})
  FetchContent_Populate(rapidjson)
  FetchContent_GetProperties(rapidjson SOURCE_DIR SRC_DIR)
  message(STATUS "Using downloaded rapidjson at ${SRC_DIR}/include")
endif ()

if (NOT RAPIDJSON_BASE)
  # Patches setup, creates targets patch1, patch2, patch3, ...
  #
  set(num 0)
  foreach (patch ${patches})
    math(EXPR num "${num} + 1")
    add_custom_target(patch${num})
    add_custom_command(
      TARGET patch${num}
      COMMAND ${CMAKE_COMMAND}
        -Dpatch_file=${CMAKE_CURRENT_SOURCE_DIR}/patches/${patch}
        -Dpatch_dir=${SRC_DIR}
        -Dpatch_stamp="${CMAKE_BINARY_DIR}/patch${num}.stamp"
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PatchFile.cmake
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endforeach ()
endif ()

# main target and linkage.
#
add_library(_RAPIDJSON INTERFACE)
add_library(ocpn::rapidjson ALIAS _RAPIDJSON)
target_include_directories(_RAPIDJSON INTERFACE "${SRC_DIR}/include")


# Apply all defined patches. No ordering, problematic if patches
# depends on each other
foreach(num RANGE 10)
  if (TARGET patch${num})
    add_dependencies(_RAPIDJSON patch${num})
  endif ()
endforeach ()

