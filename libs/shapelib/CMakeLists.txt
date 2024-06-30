# Top-level CMakeLists.txt for the CMake-based build and test system
# of the shapelib software.
#
# Copyright (C) 2012-2013, Alan W. Irwin
#
# SPDX-License-Identifier: MIT OR LGPL-2.0-or-later
#
# See README.CMake

# Version 3.7 or above of cmake is currently required for all platforms.
cmake_minimum_required(VERSION 3.7)
project(shapelib C CXX)

message(STATUS "CMake version = ${CMAKE_VERSION}")
message(STATUS "CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 6)
set(PROJECT_VERSION_PATCH 0)
set(shp_LIB_VERSIONINFO "4:0:0")
set(PROJECT_VERSION 
  "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# libraries are all shared by default.
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Option to build contributed utilities
option(BUILD_SHAPELIB_CONTRIB "Build utilities (from contrib)" OFF)

# Use rpath?
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # No rpath on Darwin. Setting it will only cause trouble.
else()
  option(USE_RPATH "Use -rpath when linking libraries, executables" ON)
endif()

# In windows all created dlls are gathered in the dll directory
# if you add this directory to your PATH all shared libraries are available
if(BUILD_SHARED_LIBS AND WIN32 AND NOT CYGWIN)
  set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/dll)
endif()

set(PACKAGE shp)

# Set up install locations.
set(
  CMAKE_INSTALL_BINDIR bin
  CACHE PATH "install location for user executables"
)

set(
  CMAKE_INSTALL_LIBDIR lib
  CACHE PATH "install location for object code libraries"
)

set(
  CMAKE_INSTALL_INCLUDEDIR include
  CACHE PATH "install location for C header files"
)

set(
  CMAKE_INSTALL_CMAKEDIR share/${PROJECT_NAME}
  CACHE PATH "install location for read-only architecture-independent shp data"
)

file(RELATIVE_PATH RELATIVE_LIBDIR
  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
)

message(STATUS "CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR}")

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  # Set a default build type for single-configuration cmake generators
  # if no build type is set.
  set(CMAKE_BUILD_TYPE Release)
endif()

# Export build information to help other projects link installed
# shapelib software.  Only one of these signatures is required
# for the export_shp name.
#install(EXPORT targets
#  NAMESPACE ${PROJECT_NAME}::
#  DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
#  FILE "${PROJECT_NAME}-targets.cmake"
#)

# Initial boilerplate done, now build library and executables.

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(lib_SRC
  shpopen.c
  dbfopen.c
  safileio.c
  shptree.c
  sbnsearch.c
  shapefil.h
)

add_library(${PACKAGE} ${lib_SRC})

target_include_directories(${PACKAGE}
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

if(NOT MSVC)
  target_compile_options(${PACKAGE} PRIVATE -Wall -Wextra -pedantic)
endif()

if(WIN32 AND NOT CYGWIN)
  set_target_properties(${PACKAGE} PROPERTIES
    COMPILE_DEFINITIONS SHAPELIB_DLLEXPORT
  )
endif()

if(UNIX)
  find_library(M_LIB m)
  if(M_LIB)
    target_link_libraries(${PACKAGE} -lm)
  endif()
endif()

# Convert shp_LIB_VERSIONINFO libtool version format into SOVERSION
# Convert from ":" separated into CMake list format using ";"
string(REPLACE ":" ";" shp_LIB_VERSIONINFO ${shp_LIB_VERSIONINFO})
list(GET shp_LIB_VERSIONINFO 0 shp_LIB_VERSION_CURRENT)
list(GET shp_LIB_VERSIONINFO 2 shp_LIB_VERSION_AGE)
math(EXPR shp_SOVERSION "${shp_LIB_VERSION_CURRENT} - ${shp_LIB_VERSION_AGE}")
set(shp_VERSION ${PROJECT_VERSION})
set_target_properties(${PACKAGE} PROPERTIES
  SOVERSION ${shp_SOVERSION}
  VERSION ${shp_VERSION}
  INSTALL_NAME_DIR "${CMAKE_INSTALL_LIBDIR}"
)

if(USE_RPATH)
  set_target_properties(${PACKAGE} PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_LIBDIR}"
  )
endif()

#install(TARGETS ${PACKAGE}
#  EXPORT targets
#  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#)

# executables to be built and installed.
set(executables
  shpcreate
  shpadd
  shpdump
  shprewind
  dbfcreate
  dbfadd
  dbfdump
  shptreedump
)

if(MSVC)
  # TODO(schwehr): How to test on Windows?
  set(BUILD_TESTING OFF CACHE BOOL "")
else()
  # TODO(schwehr): Temporary work around.
  cmake_policy(SET CMP0026 OLD)
endif()

find_program(BASH_EXECUTABLE bash)
find_program(SED_EXECUTABLE sed)
if(BASH_EXECUTABLE AND SED_EXECUTABLE)
  set(BUILD_TESTING ON CACHE BOOL "")
else()
  message(STATUS "WARNING: sed or bash not available so disabling testing")
endif()

set(BUILD_TESTING OFF)

# For the first series of tests, the user needs to have downloaded
# from http://dl.maptools.org/dl/shapelib/shape_eg_data.zip, unpacked
# that file, and specified the location of that directory with
# the cmake option, -DEG_DATA:PATH=whatever
if(BUILD_TESTING)
  if(EG_DATA)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/script.sed "s?/u/www/projects/shapelib/eg_data?${EG_DATA}?\n")
  else()
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/script.sed "")
    message(STATUS "WARNING: EG_DATA:PATH not set to point to downloaded eg_data directory so the eg_data part of testing will be ignored.")
  endif()
endif()

if(NOT MSVC)
  # Set the run time path for shared libraries for non-Windows machines.
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # See also INSTALL_RPATH property on the tools.
    set(CMAKE_MACOSX_RPATH ON)
  else()
    # Use relative path so that package is relocatable
    set(CMAKE_INSTALL_RPATH "\$ORIGIN/${RELATIVE_LIBDIR}")
  endif()
endif()

#foreach(executable ${executables})
#  add_executable(${executable} ${executable}.c)
#  target_link_libraries(${executable} ${PACKAGE})
#  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#    # Ensure that the package is relocatable
#    set_target_properties(${TOOLS} PROPERTIES
#      INSTALL_RPATH "@loader_path/${RELATIVE_LIBDIR}")
#  endif()
#  if(BUILD_TESTING)
#    get_target_property(${executable}_LOC ${executable} LOCATION)
#    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/script.sed "s?\\./${executable}?${${executable}_LOC}?\n")
#  endif()
#  if(NOT MSVC)
#    target_compile_options(${executable} PRIVATE -Wall -Wextra)
#  endif()
#endforeach(executable ${executables})

#install(TARGETS ${executables}
#  EXPORT targets DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install header
#install(FILES shapefil.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# For compatibility with older installation
#install(FILES shapefil.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/shapelib)

if(BUILD_TESTING)
  # Set up tests:

  enable_testing()

  # Other executables to be built to facilitate tests.
  foreach(executable shptest shputils)
    add_executable(${executable} ${executable}.c)
    target_link_libraries(${executable} ${PACKAGE})
    get_target_property(${executable}_LOC ${executable} LOCATION)
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/script.sed "s?\\./${executable}?${${executable}_LOC}?\n")
  endforeach(executable shptest shputils)

  # Write this as a shell script since execute_process cannot handle
  # anything like redirection.
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/script.sh "${SED_EXECUTABLE} -f script.sed < $1 >| $2")
  execute_process(
    COMMAND
    ${BASH_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/script.sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/test1.sh ${CMAKE_CURRENT_BINARY_DIR}/sed_scripted_test1.sh
  )

  execute_process(
    COMMAND
    ${BASH_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/script.sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/test2.sh ${CMAKE_CURRENT_BINARY_DIR}/sed_scripted_test2.sh
  )

  execute_process(
    COMMAND
    ${BASH_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/script.sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/test3.sh ${CMAKE_CURRENT_BINARY_DIR}/sed_scripted_test3.sh
  )

  if(EG_DATA)
    # These tests correspond to everything in test1.sh
    add_test(
      NAME test1
      COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/sed_scripted_test1.sh
    )
  endif()
  # These tests correspond to everything in test2.sh
  add_test(
    NAME test2
    COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/sed_scripted_test2.sh
  )

  # These tests correspond to everything in test3.sh
  add_test(
    NAME test3
    COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/sed_scripted_test3.sh
  )
endif()

include(cmake/contrib.cmake)

add_subdirectory (cmake)

add_library(shapelib::shapelib ALIAS ${PACKAGE})
