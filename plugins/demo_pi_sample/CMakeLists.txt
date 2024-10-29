# ---------------------------------------------------------------------------
# Author:      Dave Register
# ***************************************************************************
# - Copyright (C) 2010 by David S. Register                               *
# - This program is free software; you can redistribute it and/or modify  *
# - it under the terms of the GNU General Public License as published by  *
# - the Free Software Foundation; either version 2 of the License, or     *
# - (at your option) any later version.                                   *
# - *
# - This program is distributed in the hope that it will be useful,       *
# - but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# - MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# - GNU General Public License for more details.                          *
# - *
# - You should have received a copy of the GNU General Public License     *
# - along with this program; if not, write to the                         *
# - Free Software Foundation, Inc.,                                       *
# - 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
# ***************************************************************************

# define minimum cmake version
cmake_minimum_required(VERSION 3.1.1)

project(demo_pi)

set(PACKAGE_NAME demo_pi)
set(VERBOSE_NAME demo)
set(TITLE_NAME demo)
set(CPACK_PACKAGE_CONTARCT "Pavel Kalian")

message(STATUS "*** Building ${PACKAGE_NAME} ***")

set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_PATCH "3")
set(VERSION_DATE "07/01/2018")

if (NOT CMAKE_BUILD_TYPE)
  set(
    CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose build type [None Debug Release RelWithDebInfo MinSizeRel]: " FORCE
  )
endif ()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include("cmake/PluginConfigure.cmake")

if (PREFIX)
  set(CMAKE_INSTALL_PREFIX ${PREFIX})
endif ()

set(PARENT opencpn)
set(PREFIX_BIN bin)
set(PREFIX_INCLUDE include)
set(PREFIX_DATA share)
set(PREFIX_PARENTDATA ${PREFIX_DATA}/${PARENT})

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

# require proper c++ ADD_DEFINITIONS( "-Wall -ansi -pedantic
# -Wno-variadic-macros" ) TODO: Should we use  -fno-stack-protector IF NOT
# DEBUGGING CFLAGS="-O2 -march=native"

if (WIN32)
  add_definitions(-D__MSVC__)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang")
  string(APPEND
    CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS " -undefined dynamic_lookup"
  )
  add_compile_options("-Wall" "-g" "-fexceptions" "-fvisibility=hidden")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL GNU)
  add_compile_options("-Wl,-Bsymbolic")
  add_compile_options("-Wl,--unresolved-symbols=ignore-in-shared-libs")
  add_compile_options("-Wall" "-g" "-fexceptions" "-fvisibility=hidden")
endif ()

set(BUILD_SHARED_LIBS TRUE)

if ($ENV{CI} STREQUAL "")
  set(wxWidgets_USE_LIBS base core net xml html adv)
  find_package(wxWidgets REQUIRED)
  include(${wxWidgets_USE_FILE})
endif ()

find_package(Gettext REQUIRED)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include("cmake/PluginConfigure.cmake")

if (PREFIX)
  set(CMAKE_INSTALL_PREFIX ${PREFIX})
endif ()

set(PARENT opencpn)
set(PREFIX_BIN bin)
set(PREFIX_INCLUDE include)
set(PREFIX_DATA share)
set(PREFIX_PARENTDATA ${PREFIX_DATA}/${PARENT})

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

# require proper c++
# ADD_DEFINITIONS( "-Wall -ansi -pedantic -Wno-variadic-macros" )
# TODO: Should we use  -fno-stack-protector IF NOT
# DEBUGGING CFLAGS="-O2 -march=native"


# For convenience we define the sources as a variable. You can add header files
# and cpp/c files and CMake will sort them out

set(SRC_DEMO src/demo_pi.h src/demo_pi.cpp)

set(SRC_NMEA0183
    src/nmea0183/LatLong.hpp
    src/nmea0183/latlong.cpp
    src/nmea0183/long.cpp
    src/nmea0183/nmea0183.cpp
    src/nmea0183/nmea0183.hpp
    src/nmea0183/Response.hpp
    src/nmea0183/response.cpp
    src/nmea0183/RMB.hpp
    src/nmea0183/rmb.cpp
    src/nmea0183/Sentence.hpp
    src/nmea0183/sentence.cpp
    src/nmea0183/talkerid.cpp
    src/nmea0183/RMC.HPP
    src/nmea0183/rmc.cpp
    src/nmea0183/hexvalue.cpp
    src/nmea0183/lat.cpp
    src/nmea0183/expid.cpp
    src/nmea0183/wpl.hpp
    src/nmea0183/wpl.cpp
    src/nmea0183/rte.hpp
    src/nmea0183/rte.cpp
    src/nmea0183/hdt.hpp
    src/nmea0183/hdt.cpp
    src/nmea0183/hdg.hpp
    src/nmea0183/hdg.cpp
    src/nmea0183/hdm.hpp
    src/nmea0183/hdm.cpp
    src/nmea0183/gll.hpp
    src/nmea0183/gll.cpp
    src/nmea0183/vtg.hpp
    src/nmea0183/vtg.cpp
    src/nmea0183/gga.hpp
    src/nmea0183/gga.cpp
    src/nmea0183/gsv.hpp
    src/nmea0183/gsv.cpp
)
include_directories(nmea0183)

add_library(${PACKAGE_NAME} SHARED ${SRC_DEMO} ${SRC_NMEA0183})

target_link_libraries(${PACKAGE_NAME} ocpn::opencpn)

include("cmake/PluginInstall.cmake")
include("cmake/PluginLocalization.cmake")
include("cmake/PluginPackage.cmake")
