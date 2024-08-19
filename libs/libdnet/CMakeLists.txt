# ~~~
# Include, somehow, the libdnet library
#
# On hosts having the package installed in system locations found by
# cmake use the system code. If not, download sources from libdnet
# github site and link to build results
#
# Exports: ocpn::libdnet transitive link target
#
# License:      GPLv3+
# Copyright (c) 2023 Alec Leamas
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

cmake_policy(SET CMP0026 NEW)
if (POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif ()

cmake_minimum_required(VERSION 3.5.0)

if (TARGET ocpn::libdnet)
  return ()
endif ()

add_library(_DNET_IF INTERFACE)
add_library(ocpn::dnet ALIAS _DNET_IF)

find_path(DNET_HEADERS NAME dnet.h PATH_SUFFIXES dnet)
find_library(DNET_LIBS NAMES dnet)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBDNET DEFAULT_MSG DNET_LIBS DNET_HEADERS)
if (LIBDNET_FOUND)
  message(STATUS "Using system libdnet at ${DNET_HEADERS}")
  target_include_directories(_DNET_IF INTERFACE ${DNET_HEADERS})
  target_link_libraries(_DNET_IF INTERFACE ${DNET_LIBS})
else ()
  if (${CMAKE_VERSION} VERSION_LESS 3.20)
    message(WARNING "Using downloaded sources requires cmake >= 3.20")
  endif ()
  include(ExternalProject)
  set(LIBDNET_SRC ${CMAKE_BINARY_DIR}/libdnet)
  if (APPLE)
    if (DEFINED CMAKE_OSX_ARCHITECTURES AND NOT CMAKE_OSX_ARCHITECTURES STREQUAL "")
      string(REPLACE ";" "$<SEMICOLON>" DNET_ARCHS "${CMAKE_OSX_ARCHITECTURES}")
      set(CMAKE_ADDITIONAL_PARAMS "-DCMAKE_OSX_ARCHITECTURES='${DNET_ARCHS}'")
    endif ()
 endif ()
  ExternalProject_Add(
    libdnet

    GIT_REPOSITORY "https://github.com/ofalk/libdnet.git"
    GIT_TAG "libdnet-1.18.0"

    UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_INSTALL_LIBDIR=/lib -DCMAKE_INSTALL_INCLUDEDIR=/include ${CMAKE_ADDITIONAL_PARAMS}
    SOURCE_DIR "${LIBDNET_SRC}"
    TEST_COMMAND ""
    INSTALL_COMMAND make DESTDIR=${LIBDNET_SRC}/installed install
  )

  message(STATUS "Using downloaded libdnet at ${CMAKE_BINARY_DIR}/libdnet")
  target_include_directories(
    _DNET_IF INTERFACE ${LIBDNET_SRC}/installed/include/dnet
  )
  target_link_libraries(
    _DNET_IF INTERFACE ${LIBDNET_SRC}/installed/lib/libdnet.a
  )
endif ()
