##---------------------------------------------------------------------------
## Author:      Mauro Calvi
##***************************************************************************
## *   Copyright (C) 2010 by Mauro Calvi                               *
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
## *   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx,                                       *
## *   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx          *
## ***************************************************************************
 

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(squiddio_pi)

SET(PACKAGE_NAME squiddio_pi)
SET(VERBOSE_NAME Squiddio)
SET(TITLE_NAME SQUIDDIO)
SET(CPACK_PACKAGE_CONTACT "Mauro Calvi")

SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "7")

#SET(CMAKE_BUILD_TYPE Debug)

INCLUDE("cmake/PluginConfigure.cmake")

SET(SRC_SQUIDDIO
    src/squiddio_pi.h
    src/squiddio_pi.cpp
    src/squiddioPrefsDialogBase.cpp
    src/squiddioPrefsDialogBase.h
    src/squiddio_pi_thread.h
    src/squiddio_pi_thread.cpp
    src/icons.h
    src/icons.cpp
    src/logs.cpp
    src/logs.h
    src/NavObjectCollection.h
    src/NavObjectCollection.cpp
    src/Poi.h
    src/Poi.cpp
    src/PoiMan.h
    src/PoiMan.cpp
    src/Layer.h
    src/pugixml.hpp
    src/pugixml.cpp
    src/ocpn_types.h
    src/pugiconfig.hpp
    src/Hyperlink.h
    src/gpxdocument.h
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
    )
INCLUDE_DIRECTORIES(nmea0183)
INCLUDE_DIRECTORIES(src/wxJSON)

ADD_DEFINITIONS(-DTIXML_USE_STL)
IF(UNIX)
 INCLUDE("cmake/FindTinyXML.cmake")
 FIND_PACKAGE(TinyXML QUIET)
ENDIF(UNIX)

IF(TINYXML_FOUND)
 message (STATUS "Building with system tinyxml")
 INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
 ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_SQUIDDIO} ${SRC_NMEA0183})
 TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${TINYXML_LIBRARIES} )
ELSE(TINYXML_FOUND)
 message (STATUS "Building with embedded tinyxml")
 SET(SRC_LTINYXML
    src/tinyxml.h
    src/tinystr.h
 )
 ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_SQUIDDIO} ${SRC_NMEA0183} ${SRC_LTINYXML})
ENDIF(TINYXML_FOUND)

IF (QT_ANDROID )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OCPN_Core_LIBRARIES} )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} "-lgnustl_shared")
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} "-lz" )
ENDIF(QT_ANDROID)


INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")

