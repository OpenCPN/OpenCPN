# ~~~
# Include the libdnet library from https://github.com/ofalk/libdnet
#
# Exports: ocpn::dnet transitive link target
#
# License:      GPLv3+
# Copyright (c) 2023 Alec Leamas
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

cmake_minimum_required(VERSION 3.5.0)

if (TARGET ocpn::dnet)
  return ()
endif ()

cmake_policy(SET CMP0026 NEW)
if (POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif ()

include(ExternalProject)
set(LIBDNET_SRC ${CMAKE_BINARY_DIR}/libdnet-src)
if (APPLE)
  if (DEFINED CMAKE_OSX_ARCHITECTURES
      AND NOT CMAKE_OSX_ARCHITECTURES STREQUAL ""
  )
    string(REPLACE ";" "$<SEMICOLON>" DNET_ARCHS "${CMAKE_OSX_ARCHITECTURES}")
    set(CMAKE_ADDITIONAL_PARAMS "-DCMAKE_OSX_ARCHITECTURES='${DNET_ARCHS}'")
  endif ()
endif ()

set(
  LIBDNET_PATCH
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in-Fix-wrong-return-type.patch
)
ExternalProject_Add(
  _libdnet EXCLUDE_FROM_ALL
  DOWNLOAD_COMMAND
      ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/libdnet-libdnet-1.18.0  ${LIBDNET_SRC}
  UPDATE_COMMAND ""
  PATCH_COMMAND
      ${CMAKE_COMMAND} -Dpatch:FILEPATH=${LIBDNET_PATCH}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PatchFile.cmake
  CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/libdnet-src/installed
      -DCMAKE_INSTALL_LIBDIR=lib
      -DCMAKE_INSTALL_INCLUDEDIR=include
      -DCMAKE_INSTALL_DATADIR=share
      ${CMAKE_ADDITIONAL_PARAMS}
  SOURCE_DIR "${LIBDNET_SRC}"
  TEST_COMMAND ""
  INSTALL_COMMAND
      cmake --build . --target install --config $<CONFIG>
)
message(STATUS "Using patched libdnet at ${CMAKE_BINARY_DIR}/libdnet")

add_library(_DNET_IF INTERFACE)
add_library(ocpn::dnet ALIAS _DNET_IF)
add_dependencies(_DNET_IF _libdnet)
target_include_directories(
  _DNET_IF INTERFACE ${LIBDNET_SRC}/installed/include
  _DNET_IF INTERFACE ${LIBDNET_SRC}/installed/include/dnet
)

if (WIN32)
  target_link_libraries(
    _DNET_IF INTERFACE ${LIBDNET_SRC}/installed/lib/dnet.lib
  )
else ()
  target_link_libraries(
    _DNET_IF INTERFACE ${LIBDNET_SRC}/installed/lib/libdnet.a
  )
endif ()
