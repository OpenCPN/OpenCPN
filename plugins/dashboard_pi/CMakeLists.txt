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
IF(NOT WIN32)
 ADD_DEFINITIONS( "-Wall -O3 -fexceptions -fvisibility=hidden" )

 IF(NOT APPLE)
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")
 ELSE(NOT APPLE)
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl -undefined dynamic_lookup")
 ENDIF(NOT APPLE)

ENDIF(NOT WIN32)

#SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")

# Add some definitions to satisfy MS
IF(WIN32)
    ADD_DEFINITIONS(-D__MSVC__)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
ENDIF(WIN32)


#SET(wxWidgets_USE_LIBS base core net xml html adv aui)
#SET(BUILD_SHARED_LIBS TRUE)
#SET( wxWidgets_USE_DEBUG OFF)
#SET( wxWidgets_USE_UNICODE ON)
#FIND_PACKAGE(wxWidgets REQUIRED)

INCLUDE(${wxWidgets_USE_FILE})

FIND_PACKAGE(Gettext REQUIRED)

# For convenience we define the sources as a variable. You can add
# header files and cpp/c files and CMake will sort them out


SET(SRC_DASHBOARD
            src/dashboard_pi.h
 		src/dashboard_pi.cpp
 		src/instrument.cpp
 		src/instrument.h
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

	)
INCLUDE_DIRECTORIES(src/nmea0183)

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_DASHBOARD} ${SRC_NMEA0183} )

IF(WIN32)
  SET(OPENCPN_IMPORT_LIB "../../${CMAKE_CFG_INTDIR}/${PARENT}")
  TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OPENCPN_IMPORT_LIB} )
  ADD_DEPENDENCIES(${PACKAGE_NAME} ${PARENT})
ENDIF(WIN32)

TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${wxWidgets_LIBRARIES} )


IF(APPLE)
      INSTALL(TARGETS ${PACKAGE_NAME} BUNDLE LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/OpenCPN.app/Contents/PlugIns)
ENDIF(APPLE)

IF(UNIX AND NOT APPLE)
INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME LIBRARY DESTINATION ${PREFIX_PLUGINS})
ENDIF(UNIX AND NOT APPLE)

IF(WIN32)
INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION "plugins")
ENDIF(WIN32)

# find src/ -name \*.cpp -or -name \*.h -or -name \*.hpp | sort>po/POTFILES.in
FIND_PROGRAM(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
IF (GETTEXT_XGETTEXT_EXECUTABLE)
ADD_CUSTOM_COMMAND(
   OUTPUT po/${PACKAGE_NAME}.pot.dummy
   COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} --force-po --package-name=${PACKAGE_NAME} --package-version="${PACKAGE_VERSION}" --output=po/${PACKAGE_NAME}.pot  --keyword=_ --width=80 --files-from=${CMAKE_CURRENT_SOURCE_DIR}/po/POTFILES.in
   DEPENDS po/POTFILES.in po/${PACKAGE_NAME}.pot
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   COMMENT "dashboard-pot-update [${PACKAGE_NAME}]: Generated pot file."
)
ADD_CUSTOM_TARGET(dashboard-pot-update COMMENT "dashboard-pot-update: Done." DEPENDS po/${PACKAGE_NAME}.pot.dummy)

ENDIF(GETTEXT_XGETTEXT_EXECUTABLE )

MACRO(GETTEXT_UPDATE_PO _potFile)
   SET(_poFiles ${_potFile})
   GET_FILENAME_COMPONENT(_absPotFile ${_potFile} ABSOLUTE)

   FOREACH (_currentPoFile ${ARGN})
      GET_FILENAME_COMPONENT(_absFile ${_currentPoFile} ABSOLUTE)
      GET_FILENAME_COMPONENT(_poBasename ${_absFile} NAME_WE)

      ADD_CUSTOM_COMMAND(
         OUTPUT ${_absFile}.dummy
         COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --width=80 --strict --quiet --update --backup=none --no-location -s ${_absFile} ${_absPotFile}
         DEPENDS ${_absPotFile} ${_absFile}
         COMMENT "dashboard-po-update [${_poBasename}]: Updated po file."
      )

      SET(_poFiles ${_poFiles} ${_absFile}.dummy)

   ENDFOREACH (_currentPoFile )

   ADD_CUSTOM_TARGET(dashboard-po-update COMMENT "dashboard-po-update: Done." DEPENDS ${_poFiles})
ENDMACRO(GETTEXT_UPDATE_PO)

IF (GETTEXT_MSGMERGE_EXECUTABLE)
FILE(GLOB PACKAGE_PO_FILES po/*.po)
GETTEXT_UPDATE_PO(po/${PACKAGE_NAME}.pot ${PACKAGE_PO_FILES})
ENDIF(GETTEXT_MSGMERGE_EXECUTABLE)

SET(_gmoFiles)
MACRO(GETTEXT_BUILD_MO)
   FOREACH (_poFile ${ARGN})
      GET_FILENAME_COMPONENT(_absFile ${_poFile} ABSOLUTE)
      GET_FILENAME_COMPONENT(_poBasename ${_absFile} NAME_WE)
      SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_poBasename}.mo)

      ADD_CUSTOM_COMMAND(
         OUTPUT ${_gmoFile}
         COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_absFile}
         DEPENDS ${_absFile}
         COMMENT "dashboard-i18n [${_poBasename}]: Created mo file."
      )
IF(APPLE)
      INSTALL(FILES ${_gmoFile} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/Resources/${_poBasename}.lproj RENAME opencpn-${PACKAGE_NAME}.mo )
ELSE(APPLE)
      INSTALL(FILES ${_gmoFile} DESTINATION ${PREFIX_DATA}/locale/${_poBasename}/LC_MESSAGES RENAME opencpn-${PACKAGE_NAME}.mo )
ENDIF(APPLE)
      SET(_gmoFiles ${_gmoFiles} ${_gmoFile})
   ENDFOREACH (_poFile )
ENDMACRO(GETTEXT_BUILD_MO)

if(GETTEXT_MSGFMT_EXECUTABLE)
   FILE(GLOB PACKAGE_PO_FILES po/*.po)
   GETTEXT_BUILD_MO(${PACKAGE_PO_FILES})
   ADD_CUSTOM_TARGET(dashboard-i18n COMMENT "dashboard-i18n: Done." DEPENDS ${_gmoFiles})
   #ADD_DEPENDENCIES(dashboard-i18n ${PACKAGE_PO_FILES})
   ADD_DEPENDENCIES(${PACKAGE_NAME} dashboard-i18n)
ENDIF(GETTEXT_MSGFMT_EXECUTABLE)

