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
CMAKE_MINIMUM_REQUIRED(VERSION 3.1.1)
CMAKE_POLICY(SET CMP0043 NEW)

PROJECT(dashboard_pi)

SET(PACKAGE_NAME dashboard_pi)
SET(PLUGIN_SOURCE_DIR .)
MESSAGE (STATUS "")
MESSAGE (STATUS "*** Staging to build ${PACKAGE_NAME} ***")

#SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_MACOSX_RPATH "ON")


# require proper c++
#ADD_DEFINITIONS( "-Wall -ansi -pedantic -Wno-variadic-macros" )
#TODO: Should we use  -fno-stack-protector
#  IF NOT DEBUGGING CFLAGS="-O2 -march=native"
IF(NOT WIN32 AND NOT APPLE)
    ADD_DEFINITIONS( "-Wall -O2 -fexceptions -fvisibility=hidden" )
    IF(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
      SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic")
    ENDIF(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
ENDIF(NOT WIN32 AND NOT APPLE)

IF( APPLE )
    ADD_COMPILE_OPTIONS( "-O3" "-fexceptions" "-fvisibility=hidden" )
    ADD_COMPILE_OPTIONS("-Wno-unused" "-Wno-overloaded-virtual")
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
    src/altitude.cpp
    src/altitude.h
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
    include/wx/json_defs.h
    include/wx/jsonreader.h
    include/wx/jsonval.h
    include/wx/jsonwriter.h
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
    src/nmea0183/xte.cpp
    src/nmea0183/xte.hpp
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
##INCLUDE_DIRECTORIES(src/nmea0183)


SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
SET(BUILD_SHARED_LIBS "ON")
ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_DASHBOARD} ${SRC_NMEA0183} )
target_include_directories(${PACKAGE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/wx
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nmea0183
)
target_link_libraries( ${PACKAGE_NAME} ocpn::opencpn )

IF (WIN32 AND NOT MSVC)
  SET( CMAKE_SHARED_LINKER_FLAGS "-L../../" )
ENDIF()

# On Android, PlugIns need a specific linkage set....
IF (QT_ANDROID )
    ADD_DEFINITIONS("-Wno-inconsistent-missing-override -Wno-potentially-evaluated-expression")
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME}  ${CMAKE_CURRENT_BINARY_DIR}/../../libgorp.so )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME}  ${Qt_Base}/${Qt_Build}/lib/libQt5Gui.so )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME}  ${Qt_Base}/${Qt_Build}/lib/libQt5Core.so )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME}  ${Qt_Base}/${Qt_Build}/lib/libQt5Widgets.so )
    SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-soname,lib${PACKAGE_NAME}.so ")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-s")
ELSE (QT_ANDROID )
    target_link_libraries( ${PACKAGE_NAME} ocpn::opencpn )
ENDIF(QT_ANDROID)

TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${wxWidgets_LIBRARIES} )
TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OCPN_Core_LIBRARIES} )
if (TARGET ocpn::wxsvg)
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ocpn::wxsvg )
endif()


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
  if (NOT OCPN_CI_BUILD)
    INSTALL(FILES $<TARGET_PDB_FILE:${PACKAGE_NAME}> DESTINATION "plugins")
  endif (NOT OCPN_CI_BUILD)

    SET(INSTALL_DIRECTORY "plugins/${PACKAGE_NAME}")

    IF(EXISTS ${PROJECT_SOURCE_DIR}/data)
    INSTALL(DIRECTORY data DESTINATION "${INSTALL_DIRECTORY}")
    ENDIF(EXISTS ${PROJECT_SOURCE_DIR}/data)
ENDIF(WIN32)

# Copy shared library to execution folder for debugging if not CI build
if (NOT OCPN_CI_BUILD)
  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG /OPT:REF /OPT:ICF")
    add_custom_command(
      TARGET ${PACKAGE_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "Copying dll file"
      COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_FILE:${PACKAGE_NAME}>"
                "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/plugins/${PACKAGE_NAME}.dll"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${PACKAGE_NAME}>"
                "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/plugins/${PACKAGE_NAME}.dll"
    )
    add_custom_command(
      TARGET ${PACKAGE_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "Copying pdb file"
      COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_PDB_FILE:${PACKAGE_NAME}>"
                "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/plugins/${PACKAGE_NAME}.pdb"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_PDB_FILE:${PACKAGE_NAME}>"
                "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/plugins/${PACKAGE_NAME}.pdb"
    )
  endif (MSVC)
endif (NOT OCPN_CI_BUILD)

INCLUDE("../PluginLocalization.cmake")
PLUGIN_LOCALIZATION()
