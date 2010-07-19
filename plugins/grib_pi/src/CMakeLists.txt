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

PROJECT(grib_pi)

SET(PACKAGE_NAME grib_pi)
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
ENDIF(NOT WIN32)

# Add some definitions to satisfy MS
IF(WIN32)
    ADD_DEFINITIONS(-D__MSVC__)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
ENDIF(WIN32)

#  Fix up time_t definition
# Checks for 32-bit version. And always use 32-bit time_t for compatibility
IF(CMAKE_GENERATOR MATCHES "Visual Studio" AND CMAKE_SIZEOF_VOID_P MATCHES 4)
  ADD_DEFINITIONS(-D_USE_32BIT_TIME_T)
ENDIF(CMAKE_GENERATOR MATCHES "Visual Studio" AND CMAKE_SIZEOF_VOID_P MATCHES 4)

 
SET(wxWidgets_USE_LIBS base core net xml html adv)
SET(BUILD_SHARED_LIBS TRUE)
FIND_PACKAGE(wxWidgets REQUIRED)

INCLUDE(${wxWidgets_USE_FILE})

FIND_PACKAGE(Gettext REQUIRED)

# For convenience we define the sources as a variable. You can add 
# header files and cpp/c files and CMake will sort them out


SET(SRC_GRIB
                grib_pi.h
 		grib_pi.cpp
                grib.h 
                grib.cpp
		GribReader.cpp 
		GribRecord.cpp  
		zuFile.cpp 
		IsoLine.cpp 
		bzip2/bzlib.c 
		bzip2/blocksort.c 
		bzip2/compress.c 
		bzip2/crctable.c 
		bzip2/decompress.c 
		bzip2/huffman.c 
		bzip2/randtable.c
	)
ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_GRIB})

INCLUDE_DIRECTORIES(${PLUGIN_SOURCE_DIR}/zlib-1.2.3)
INCLUDE_DIRECTORIES(${PLUGIN_SOURCE_DIR}/bzip2)

IF(WIN32)
SET(OPENCPN_IMPORT_LIB "../../${CMAKE_CFG_INTDIR}/${PARENT}")
TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OPENCPN_IMPORT_LIB} )
ENDIF(WIN32)

TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${wxWidgets_LIBRARIES} )

IF(UNIX)
INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME LIBRARY DESTINATION ${PREFIX_PARENTDATA}/plugins)
ENDIF(UNIX)

IF(WIN32)
INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION "plugins")
ENDIF(WIN32)
 	  	 
