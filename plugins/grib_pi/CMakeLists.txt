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

PROJECT(grib_pi)

SET(PACKAGE_NAME grib_pi)
SET(PLUGIN_SOURCE_DIR .)
MESSAGE (STATUS "*** Staging to build ${PACKAGE_NAME} ***")

#SET(CMAKE_BUILD_TYPE Debug)
#SET(CMAKE_VERBOSE_MAKEFILE ON)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

# require proper c++
#ADD_DEFINITIONS( "-Wall -ansi -pedantic -Wno-variadic-macros" )
#TODO: Should we use  -fno-stack-protector
#  IF NOT DEBUGGING CFLAGS="-O2 -march=native"
IF(NOT WIN32 AND NOT APPLE)
    ADD_DEFINITIONS( "-Wall -fexceptions -fvisibility=hidden" )
    SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")
ENDIF(NOT WIN32 AND NOT APPLE)

IF( APPLE )
   ADD_DEFINITIONS( "-fexceptions -fvisibility=hidden" )
   ADD_DEFINITIONS("-Wno-unused -Wno-overloaded-virtual")
   SET(CMAKE_SHARED_LINKER_FLAGS "-Wl -undefined dynamic_lookup")
ENDIF( APPLE )

# Add some definitions to satisfy MS
IF(MSVC)
    ADD_DEFINITIONS(-D__MSVC__)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
ENDIF(MSVC)

#  QT_ANDROID is a cross-build, so the native FIND_PACKAGE(wxWidgets...) and wxWidgets_USE_FILE are not useful.
IF(QT_ANDROID)
    SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
    SET(BUILD_SHARED_LIBS "ON")
ELSE(QT_ANDROID)
IF(NOT DEFINED wxWidgets_USE_FILE)
    SET(wxWidgets_USE_LIBS base core net xml html adv)
    SET(BUILD_SHARED_LIBS TRUE)
    FIND_PACKAGE(wxWidgets REQUIRED)
ENDIF(NOT DEFINED wxWidgets_USE_FILE)

  INCLUDE(${wxWidgets_USE_FILE})
ENDIF(QT_ANDROID)

FIND_PACKAGE(Gettext REQUIRED)


# For convenience we define the sources as a variable. You can add
# header files and cpp/c files and CMake will sort them out


SET(SRC_GRIB
            src/grib_pi.h
            src/grib_pi.cpp
            src/GribOverlayFactory.cpp
            src/GribOverlayFactory.h
            src/GribUIDialogBase.cpp
            src/GribUIDialogBase.h
            src/GrabberWin.cpp
            src/GrabberWin.h
            src/GribUIDialog.cpp
            src/GribUIDialog.h
            src/CursorData.cpp
            src/CursorData.h
            src/GribSettingsDialog.cpp
            src/GribSettingsDialog.h
            src/GribRequestDialog.cpp
            src/GribRequestDialog.h
            src/icons.h
            src/msg.h
            src/email.h
            src/email.cpp
            src/TexFont.h
            src/TexFont.cpp
            src/GribTable.h
            src/GribTable.cpp
            src/icons.cpp
            src/GribReader.cpp
            src/GribRecord.cpp
            src/GribV1Record.cpp
            src/GribV2Record.cpp
            src/zuFile.cpp
            src/IsoLine.cpp

            ../../src/wxJSON/jsonval.cpp
            ../../src/wxJSON/jsonreader.cpp
            ../../src/wxJSON/jsonwriter.cpp
)
IF(WIN32)
           SET(SRC_GRIB ${SRC_GRIB} src/smapi.h)
           SET(SRC_GRIB ${SRC_GRIB} src/smapi.cpp)
ENDIF(WIN32)

IF(NOT UNIX)
    SET(SRC_BZIP
            src/bzip2/bzlib.c
            src/bzip2/blocksort.c
            src/bzip2/compress.c
            src/bzip2/crctable.c
            src/bzip2/decompress.c
            src/bzip2/huffman.c
            src/bzip2/randtable.c
    )
    ADD_LIBRARY(LIB_BZIP ${SRC_BZIP})

    INCLUDE_DIRECTORIES(${PLUGIN_SOURCE_DIR}/src/zlib-1.2.3)
    INCLUDE_DIRECTORIES(${PLUGIN_SOURCE_DIR}/src/bzip2)
ENDIF(NOT UNIX)


IF(QT_ANDROID)
    SET(SRC_BZIP_ANDROID
            src/bzip2/bzlib.c
            src/bzip2/blocksort.c
            src/bzip2/compress.c
            src/bzip2/crctable.c
            src/bzip2/decompress.c
            src/bzip2/huffman.c
            src/bzip2/randtable.c
    )
    SET(SRC_GRIB ${SRC_GRIB} ${SRC_BZIP_ANDROID})

    INCLUDE_DIRECTORIES(${PLUGIN_SOURCE_DIR}/src/zlib-1.2.3)
    INCLUDE_DIRECTORIES(${PLUGIN_SOURCE_DIR}/src/bzip2)

    MESSAGE (STATUS ${SRC_GRIB})

ENDIF(QT_ANDROID)

    #  Stripped jasper library courtesy of https://github.com/vmarkovtsev/jasper.git
    
    file(GLOB jasper_base_sources "src/jasper/base/*.c")
    file(GLOB jasper_jp2_sources "src/jasper/jp2/*.c")
    file(GLOB jasper_jp2_headers "src/jasper/jp2/*.h")
    file(GLOB jasper_jpc_sources "src/jasper/jpc/*.c")
    file(GLOB jasper_jpc_headers "src/jasper/jpc/*.h")
    file(GLOB jasper_headers "src/jasper/include/jasper/*.h")
    
    SET(SRC_GRIB ${SRC_GRIB} ${jasper_base_sources} ${jasper_headers} ${jasper_jp2_sources} ${jasper_jp2_headers} ${jasper_jpc_sources} ${jasper_jpc_headers})

    ADD_DEFINITIONS(-DJASPER)
    ADD_DEFINITIONS(-DEXCLUDE_MIF_SUPPORT -DEXCLUDE_PNM_SUPPORT -DEXCLUDE_BMP_SUPPORT -DEXCLUDE_RAS_SUPPORT -DEXCLUDE_JPG_SUPPORT -DEXCLUDE_PGX_SUPPORT)
   
    INCLUDE_DIRECTORIES("src/jasper/include")



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

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_GRIB})

IF(WIN32)
  IF(MSVC)
    SET(OPENCPN_IMPORT_LIB "../../${CMAKE_CFG_INTDIR}/${PARENT}")
  ELSE(MSVC)
    SET(OPENCPN_IMPORT_LIB "${PARENT}.dll")
    SET( CMAKE_SHARED_LINKER_FLAGS "-L../../" )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OPENGL_LIBRARIES}
#      "-L${wxWidgets_ROOT_DIR}/lib -lwxzlib-3.1-i686-w64-mingw32")
      "-L${wxWidgets_ROOT_DIR}/lib -lwxzlib-2.8")
  ENDIF(MSVC)

  TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} LIB_BZIP ${OPENCPN_IMPORT_LIB})
  ADD_DEPENDENCIES(${PACKAGE_NAME} ${PARENT})
ENDIF(WIN32)


TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${wxWidgets_LIBRARIES} )
TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OCPN_Core_LIBRARIES} )

IF(NOT WIN32)
TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} "-lz" )
ENDIF()
IF(APPLE)
 FIND_PACKAGE(ZLIB REQUIRED)
 TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${ZLIB_LIBRARIES} )
ENDIF(APPLE)

IF(UNIX AND NOT APPLE AND NOT QT_ANDROID)
    FIND_PACKAGE(BZip2 REQUIRED)
    INCLUDE_DIRECTORIES(${BZIP2_INCLUDE_DIR})
    FIND_PACKAGE(ZLIB REQUIRED)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${BZIP2_LIBRARIES} ${ZLIB_LIBRARY})
ENDIF(UNIX AND NOT APPLE AND NOT QT_ANDROID)

IF(APPLE)
    INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/OpenCPN.app/Contents/PlugIns)

	IF(EXISTS ${PROJECT_SOURCE_DIR}/data)
		INSTALL(DIRECTORY data DESTINATION ${CMAKE_BINARY_DIR}/OpenCPN.app/Contents/SharedSupport/plugins/${PACKAGE_NAME})
	ENDIF(EXISTS ${PROJECT_SOURCE_DIR}/data)

ENDIF(APPLE)

IF(UNIX AND NOT APPLE)
    IF(NOT DEFINED PREFIX_PLUGINS)
		SET(PREFIX_PLUGINS /usr/local/lib/opencpn)
    ENDIF(NOT DEFINED PREFIX_PLUGINS)

    INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME LIBRARY DESTINATION ${PREFIX_PLUGINS})

	IF(EXISTS ${PROJECT_SOURCE_DIR}/data)
        SET(PREFIX_PARENTDATA ${PREFIX_DATA}/${PARENT})
        INSTALL(DIRECTORY data DESTINATION ${PREFIX_PARENTDATA}/plugins/${PACKAGE_NAME})
	ENDIF(EXISTS ${PROJECT_SOURCE_DIR}/data)

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
