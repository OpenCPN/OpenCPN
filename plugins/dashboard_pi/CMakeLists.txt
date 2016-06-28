##---------------------------------------------------------------------------
## Author:      Dave Register
## ***************************************************************************
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
## *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
## ***************************************************************************


# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)
# This should be 2.8.0 to have FindGTK2 module
IF (COMMAND cmake_policy)
    CMAKE_POLICY(SET CMP0003 OLD)
    CMAKE_POLICY(SET CMP0005 OLD)
    CMAKE_POLICY(SET CMP0011 OLD)
ENDIF (COMMAND cmake_policy)

PROJECT(dashboard_pi)

SET(PACKAGE_NAME dashboard_pi)
SET(PLUGIN_SOURCE_DIR .)
MESSAGE (STATUS "")
MESSAGE (STATUS "*** Staging to build ${PACKAGE_NAME} ***")

#SET(CMAKE_BUILD_TYPE Debug)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

# require proper c++
#ADD_DEFINITIONS( "-Wall -ansi -pedantic -Wno-variadic-macros" )
#TODO: Should we use  -fno-stack-protector
#  IF NOT DEBUGGING CFLAGS="-O2 -march=native"
IF(NOT WIN32 AND NOT APPLE)
    ADD_DEFINITIONS( "-Wall -O2 -fexceptions -fvisibility=hidden" )
    SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")
ENDIF(NOT WIN32 AND NOT APPLE)

IF( APPLE )
   ADD_DEFINITIONS( "-O3 -fexceptions -fvisibility=hidden" )
   ADD_DEFINITIONS("-Wno-unused -Wno-overloaded-virtual")
   SET(CMAKE_SHARED_LINKER_FLAGS "-Wl -undefined dynamic_lookup")
ENDIF( APPLE )

# Add some definitions to satisfy MSVC
IF(MSVC)
    ADD_DEFINITIONS(-D__MSVC__)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
ENDIF(MSVC)



#  QT_ANDROID is a cross-build, so the native FIND_PACKAGE(wxWidgets...) and wxWidgets_USE_FILE is not useful.
IF(NOT QT_ANDROID)
IF(NOT DEFINED wxWidgets_USE_FILE)
  SET(wxWidgets_USE_LIBS base core net xml html adv)
  SET(BUILD_SHARED_LIBS TRUE)
  FIND_PACKAGE(wxWidgets REQUIRED)
ENDIF(NOT DEFINED wxWidgets_USE_FILE)

  INCLUDE(${wxWidgets_USE_FILE})
ENDIF(NOT QT_ANDROID)

FIND_PACKAGE(Gettext REQUIRED)

# For convenience we define the sources as a variable. You can add
# header files and cpp/c files and CMake will sort them out


SET(SRC_DASHBOARD
    src/dashboard_pi.h
    src/dashboard_pi.cpp
    src/instrument.cpp
    src/instrument.h
    src/iirfilter.h
    src/iirfilter.cpp
    src/compass.cpp
    src/compass.h
    src/dial.cpp
    src/dial.h
    src/gps.cpp
    src/gps.h
    src/rudder_angle.cpp
    src/rudder_angle.h
    src/speedometer.cpp
    src/speedometer.h
    src/wind.cpp
    src/wind.h
    src/depth.cpp
    src/depth.h
    src/clock.cpp
    src/clock.h
    src/icons.cpp
    src/icons.h
    src/wind_history.cpp
    src/wind_history.h
    src/baro_history.cpp
    src/baro_history.h
    src/from_ownship.cpp
    src/from_ownship.h
    src/wxJSON/jsonval.cpp
    src/wxJSON/jsonreader.cpp

)

SET(SRC_NMEA0183
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
    src/nmea0183/dbt.cpp
    src/nmea0183/dbt.hpp
    src/nmea0183/dpt.cpp
    src/nmea0183/dpt.hpp
    src/nmea0183/mtw.cpp
    src/nmea0183/mtw.hpp
    src/nmea0183/mwv.cpp
    src/nmea0183/mwv.hpp
    src/nmea0183/vhw.hpp
    src/nmea0183/vhw.cpp
    src/nmea0183/vwr.cpp
    src/nmea0183/vwr.hpp
    src/nmea0183/zda.cpp
    src/nmea0183/zda.hpp
    src/nmea0183/rsa.hpp
    src/nmea0183/rsa.cpp
    src/nmea0183/SatInfo.h
    src/nmea0183/mwd.cpp
    src/nmea0183/mwd.hpp
    src/nmea0183/vwt.cpp
    src/nmea0183/vwt.hpp
    src/nmea0183/mta.cpp
    src/nmea0183/mta.hpp
    src/nmea0183/vlw.cpp
    src/nmea0183/vlw.hpp
    src/nmea0183/mda.cpp
    src/nmea0183/mda.hpp
    src/nmea0183/xdr.cpp
    src/nmea0183/xdr.hpp
)
INCLUDE_DIRECTORIES(src/nmea0183)


SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
SET(BUILD_SHARED_LIBS "ON")
ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_DASHBOARD} ${SRC_NMEA0183} )


IF(WIN32)
  IF(MSVC)
    SET(OPENCPN_IMPORT_LIB "../../${CMAKE_CFG_INTDIR}/${PARENT}")
  ELSE(MSVC)
    SET(OPENCPN_IMPORT_LIB "${PARENT}.dll")
    SET( CMAKE_SHARED_LINKER_FLAGS "-L../../" )
  ENDIF(MSVC)

  TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OPENCPN_IMPORT_LIB} )
  ADD_DEPENDENCIES(${PACKAGE_NAME} ${PARENT})
ENDIF(WIN32)


# On Android, PlugIns need a specific linkage set....
IF (QT_ANDROID )
  # These libraries are needed to create PlugIns on Android.
  
  SET(OCPN_Core_LIBRARIES
        # Presently, Android Plugins are built in the core tree, so the variables {wxQT_BASE}, etc.
        # flow to this module from above.  If we want to build Android plugins out-of-core, this will need improvement.

        # TODO This is pretty ugly, but there seems no way to avoid specifying a full path in a cross build....
        /home/dsr/Projects/opencpn_sf/opencpn/build-opencpn-Android_for_armeabi_v7a_GCC_4_8_Qt_5_5_0-Debug/libopencpn.so

        ${wxQt_Base}/${wxQt_Build}/lib/libwx_baseu-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_core-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_html-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_baseu_xml-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_qa-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_adv-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_aui-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_baseu_net-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_gl-3.1-arm-linux-androideabi.a
        ${Qt_Base}/android_armv7/lib/libQt5Core.so
        ${Qt_Base}/android_armv7/lib/libQt5OpenGL.so
        ${Qt_Base}/android_armv7/lib/libQt5Widgets.so
        ${Qt_Base}/android_armv7/lib/libQt5Gui.so
        ${Qt_Base}/android_armv7/lib/libQt5AndroidExtras.so
        
        ${NDK_Base}/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/libgnustl_shared.so
        )

ENDIF(QT_ANDROID)

TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${wxWidgets_LIBRARIES} )
TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OCPN_Core_LIBRARIES} )


IF(APPLE)
    INSTALL(TARGETS ${PACKAGE_NAME} BUNDLE LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/OpenCPN.app/Contents/PlugIns)
   
   IF(EXISTS ${PROJECT_SOURCE_DIR}/data)
    INSTALL(DIRECTORY data DESTINATION ${CMAKE_BINARY_DIR}/OpenCPN.app/Contents/SharedSupport/plugins/${PACKAGE_NAME})
    ENDIF()

ENDIF(APPLE)

IF(UNIX AND NOT APPLE)
    INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME LIBRARY DESTINATION ${PREFIX_PLUGINS})

    IF(EXISTS ${PROJECT_SOURCE_DIR}/data)
        SET(PREFIX_PARENTDATA ${PREFIX_DATA}/${PARENT})
        INSTALL(DIRECTORY data DESTINATION ${PREFIX_PARENTDATA}/plugins/${PACKAGE_NAME})
    ENDIF()
ENDIF(UNIX AND NOT APPLE)

IF(WIN32)
    INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION "plugins")
  
    SET(INSTALL_DIRECTORY "plugins\\\\${PACKAGE_NAME}")

    IF(EXISTS ${PROJECT_SOURCE_DIR}/data)
    INSTALL(DIRECTORY data DESTINATION "${INSTALL_DIRECTORY}")
    ENDIF(EXISTS ${PROJECT_SOURCE_DIR}/data)
ENDIF(WIN32)

INCLUDE("../PluginLocalization.cmake")
PLUGIN_LOCALIZATION()
