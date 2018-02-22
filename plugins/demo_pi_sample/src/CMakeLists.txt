##---------------------------------------------------------------------------
## Author:      bdbcat aka. dsr (Dave Register)
## Copyright:   
## License:     wxWidgets License
##---------------------------------------------------------------------------
 

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)
# This should be 2.8.0 to have FindGTK2 module
IF (COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 OLD)
  CMAKE_POLICY(SET CMP0005 OLD)
  CMAKE_POLICY(SET CMP0011 OLD)
ENDIF (COMMAND cmake_policy)

PROJECT(demo_pi)

SET(PACKAGE_NAME demo_pi)
SET(PLUGIN_SOURCE_DIR ..)
MESSAGE (STATUS "*** Building ${PACKAGE_NAME} ***")

#SET(CMAKE_BUILD_TYPE Debug)

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

IF(WIN32)
  SET(OPENCPN_IMPORT_LIB "../../../${CMAKE_CFG_INTDIR}/${PARENT}")
  TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OPENCPN_IMPORT_LIB} )
  ADD_DEPENDENCIES(${PACKAGE_NAME} ${PARENT})
ENDIF(WIN32)

TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${wxWidgets_LIBRARIES} )

IF(UNIX)
INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME LIBRARY DESTINATION ${PREFIX_PARENTDATA}/plugins)
ENDIF(UNIX)

IF(WIN32)
INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION "plugins")
ENDIF(WIN32)
 	  	 
