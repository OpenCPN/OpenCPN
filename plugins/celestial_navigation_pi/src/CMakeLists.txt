##---------------------------------------------------------------------------
## Author:      Sean D'Epagnier
## Copyright:   
## License:     GPL
##---------------------------------------------------------------------------
 

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)
# This should be 2.8.0 to have FindGTK2 module
IF (COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 OLD)
  CMAKE_POLICY(SET CMP0005 OLD)
  CMAKE_POLICY(SET CMP0011 OLD)
ENDIF (COMMAND cmake_policy)

PROJECT(celestial_navigation_pi)

SET(PACKAGE_NAME celestial_navigation_pi)
SET(PLUGIN_SOURCE_DIR .)
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
ADD_DEFINITIONS( "-Wall -g -fexceptions" )
SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")
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


SET(SRC_CELESTIAL_NAVIGATION
          astrolabe/calendar.cpp
          astrolabe/dicts.cpp
          astrolabe/dynamical.cpp
          astrolabe/elp2000.cpp
          astrolabe/equinox.cpp
          astrolabe/globals.cpp
          astrolabe/nutation.cpp
          astrolabe/riseset.cpp
          astrolabe/sun.cpp
          astrolabe/util.cpp
          astrolabe/vsop87d.cpp
          geomag/geomag.c
	  celestialnavigationdialog.cpp
	  sightdialog.cpp 
          sight.cpp
          celestial_navigation_pi.cpp
	)

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_CELESTIAL_NAVIGATION})

INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(${PLUGIN_SOURCE_DIR}/include/astrolabe)


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
 	  	 
IF(WIN32)
  INSTALL(DIRECTORY ../data DESTINATION "plugins\\\\celestial_navigation")
ELSE(WIN32)
  INSTALL(DIRECTORY ../data DESTINATION ${PREFIX_PARENTDATA}/plugins/celestial_navigation)
ENDIF(WIN32)
