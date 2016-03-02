##---------------------------------------------------------------------------
## Author:      Dave Register
##***************************************************************************
## *   Copyright (C) 2010 by David S. Register                               *
## *   This program is free software; you can redistribute it and/or modify  *
## *   it under the terms of the GNU General Public License as published by  *
## *   the Free Software Foundation; either version 2 of the License, or     *
## *   (at your option) any later version.                                   *
## *                                                                         *
## *   This program is distributed in the hope that it will be useful,       *
## *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
## *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
## *   GNU General Public License for more details.                          *
## *                                                                         *
## *   You should have received a copy of the GNU General Public License     *
## *   along with this program; if not, write to the                         *
## *   Free Software Foundation, Inc.,                                       *
## *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
## ***************************************************************************
 

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)
# This should be 2.8.0 to have FindGTK2 module

PROJECT(demo_pi)

SET(PACKAGE_NAME demo_pi)
SET(VERBOSE_NAME demo)
SET(TITLE_NAME DEMO)
SET(CPACK_PACKAGE_CONTACT "Pavel Kalian")

MESSAGE (STATUS "*** Building ${PACKAGE_NAME} ***")

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "0")
SET(VERSION_PATCH "2")
SET(VERSION_DATE "01/03/2016")

IF( NOT CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE Release CACHE STRING
       "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
       FORCE )
ENDIF( NOT CMAKE_BUILD_TYPE )

MESSAGE (STATUS "Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE("cmake/PluginConfigure.cmake")

IF (PREFIX)
	SET(CMAKE_INSTALL_PREFIX ${PREFIX})
ENDIF (PREFIX)

SET(PARENT opencpn)
SET(PREFIX_BIN bin)
SET(PREFIX_INCLUDE include)
SET(PREFIX_DATA share)
SET(PREFIX_PARENTDATA ${PREFIX_DATA}/${PARENT})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

# require proper c++
#ADD_DEFINITIONS( "-Wall -ansi -pedantic -Wno-variadic-macros" )
#TODO: Should we use  -fno-stack-protector
#  IF NOT DEBUGGING CFLAGS="-O2 -march=native"
IF(NOT WIN32)
 ADD_DEFINITIONS( "-Wall -g -fexceptions -fvisibility=hidden" )

 IF(NOT APPLE)
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")
 ELSE(NOT APPLE)
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl")
 ENDIF(NOT APPLE)

ENDIF(NOT WIN32)

# Add some definitions to satisfy MS
IF(WIN32)
    ADD_DEFINITIONS(-D__MSVC__)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
ENDIF(WIN32)


SET(wxWidgets_USE_LIBS base core net xml html adv)
SET(BUILD_SHARED_LIBS TRUE)
FIND_PACKAGE(wxWidgets REQUIRED)

INCLUDE(${wxWidgets_USE_FILE})

FIND_PACKAGE(Gettext REQUIRED)

# For convenience we define the sources as a variable. You can add 
# header files and cpp/c files and CMake will sort them out


SET(SRC_DEMO
                demo_pi.h
 		demo_pi.cpp
 	)

SET(SRC_NMEA0183
		nmea0183/LatLong.hpp
		nmea0183/latlong.cpp
		nmea0183/long.cpp
		nmea0183/nmea0183.cpp
		nmea0183/nmea0183.hpp
		nmea0183/Response.hpp
		nmea0183/response.cpp
		nmea0183/RMB.hpp
		nmea0183/rmb.cpp
		nmea0183/Sentence.hpp
		nmea0183/sentence.cpp
		nmea0183/talkerid.cpp
		nmea0183/RMC.HPP
		nmea0183/rmc.cpp
		nmea0183/hexvalue.cpp
		nmea0183/lat.cpp
		nmea0183/expid.cpp
		nmea0183/wpl.hpp
		nmea0183/wpl.cpp
		nmea0183/rte.hpp
		nmea0183/rte.cpp
		nmea0183/hdt.hpp
		nmea0183/hdt.cpp
		nmea0183/hdg.hpp
		nmea0183/hdg.cpp
		nmea0183/hdm.hpp
		nmea0183/hdm.cpp
		nmea0183/gll.hpp
		nmea0183/gll.cpp
		nmea0183/vtg.hpp
		nmea0183/vtg.cpp
		nmea0183/gga.hpp
		nmea0183/gga.cpp
		nmea0183/gsv.hpp
		nmea0183/gsv.cpp
	)
INCLUDE_DIRECTORIES(nmea0183)

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_DEMO} ${SRC_NMEA0183} )


INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")
	  	 
