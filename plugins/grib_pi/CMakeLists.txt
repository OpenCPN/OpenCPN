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
CMAKE_MINIMUM_REQUIRED(VERSION 3.1.1)

IF (COMMAND cmake_policy)
  if (POLICY CMP0043)
    CMAKE_POLICY(SET CMP0043 NEW)
  endif (POLICY CMP0043)
ENDIF (COMMAND cmake_policy)

if (NOT CMAKE_MODULE_PATH)
  set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
endif ()

PROJECT(grib_pi)

SET(PACKAGE_NAME grib_pi)
SET(PLUGIN_SOURCE_DIR .)
MESSAGE (STATUS "*** Staging to build ${PACKAGE_NAME} ***")

#SET(CMAKE_BUILD_TYPE Debug)
#SET(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_MACOSX_RPATH "ON")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

# require proper c++
#ADD_DEFINITIONS( "-Wall -ansi -pedantic -Wno-variadic-macros" )
#TODO: Should we use  -fno-stack-protector
#  IF NOT DEBUGGING CFLAGS="-O2 -march=native"
IF(NOT WIN32 AND NOT APPLE)
    ADD_DEFINITIONS( "-Wall -fexceptions -fvisibility=hidden" )
    IF(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
      SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic")
    ENDIF(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
ENDIF(NOT WIN32 AND NOT APPLE)

IF( APPLE )
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
    ADD_DEFINITIONS("-Wno-inconsistent-missing-override -Wno-potentially-evaluated-expression")
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
            src/pi_TexFont.h
            src/pi_TexFont.cpp
            src/GribTable.h
            src/GribTable.cpp
            src/CustomGrid.h
            src/CustomGrid.cpp
            src/icons.cpp
            src/GribReader.cpp
            src/GribRecord.cpp
            src/GribV1Record.cpp
            src/GribV2Record.cpp
            src/zuFile.cpp
            src/IsoLine.cpp
            src/pi_ocpndc.cpp
            src/pi_ocpndc.h
)
IF(WIN32)
           SET(SRC_GRIB ${SRC_GRIB} src/smapi.h)
           SET(SRC_GRIB ${SRC_GRIB} src/smapi.cpp)
ENDIF(WIN32)

IF(QT_ANDROID)
           SET(SRC_GRIB ${SRC_GRIB} src/pi_shaders.h)
           SET(SRC_GRIB ${SRC_GRIB} src/pi_shaders.cpp)
ENDIF(QT_ANDROID)

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
    ADD_LIBRARY(LIB_BZIP STATIC ${SRC_BZIP})

    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/buildwin/include)
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

    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/buildwin/include)
    INCLUDE_DIRECTORIES(${PLUGIN_SOURCE_DIR}/src/bzip2)

    MESSAGE (STATUS ${SRC_GRIB})

ENDIF(QT_ANDROID)

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_GRIB})

find_package(Jasper)
USE_BUNDLED_LIB(USE_BUNDLED_JASPER jasper)
if (JASPER_FOUND AND NOT USE_BUNDLED_JASPER)
  message(STATUS "Using system jasper library...")
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${JASPER_LIBRARIES})
  target_include_directories(${PACKAGE_NAME} BEFORE PRIVATE  ${JASPER_INCLUDE_DIR})
else ()
  message(STATUS "Using bundled jasper library...")
  add_subdirectory("libs/jasper")
  target_link_libraries(${PACKAGE_NAME} PRIVATE JASPER)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang")
    # FIXME: Need better handling after 5.0.0
    target_compile_options(${PACKAGE_NAME} PRIVATE -Wno-format)
endif ()


IF(WIN32)
  IF(MSVC)
      TARGET_LINK_LIBRARIES(${PACKAGE_NAME}
      PRIVATE "${CMAKE_SOURCE_DIR}/buildwin/zlib1.lib"
          "GLU_static"
      )
  ELSE(MSVC)
    SET( CMAKE_SHARED_LINKER_FLAGS "-L../../" )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE
    ${OPENGL_LIBRARIES}
#      "-L${wxWidgets_ROOT_DIR}/lib -lwxzlib-3.1-i686-w64-mingw32")
#      "-L${wxWidgets_ROOT_DIR}/lib -lwxzlib-2.8"
       "-lz"
    )
  ENDIF(MSVC)

  TARGET_LINK_LIBRARIES( ${PACKAGE_NAME}
      PRIVATE LIB_BZIP
  )
  ADD_DEPENDENCIES(${PACKAGE_NAME} ${PARENT})
ENDIF(WIN32)

IF (NOT QT_ANDROID )
TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE ${wxWidgets_LIBRARIES} )
TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE ${OCPN_Core_LIBRARIES} )
    target_link_libraries( ${PACKAGE_NAME} PRIVATE ocpn::wxsvg)
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::opencpn)
ELSE (NOT QT_ANDROID )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE "-lz" )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE "-lGLESv1_CM" )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE "-lGLESv2" )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE  ${Qt_Base}/${Qt_Build}/lib/libQt5Gui.so )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE  ${Qt_Base}/${Qt_Build}/lib/libQt5Core.so )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE  ${Qt_Base}/${Qt_Build}/lib/libQt5Widgets.so )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE  ${Qt_Base}/${Qt_Build}/lib/libQt5AndroidExtras.so )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE  ${CMAKE_CURRENT_BINARY_DIR}/../../libgorp.so )
    SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-soname,lib${PACKAGE_NAME}.so ")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-s")
ENDIF(NOT QT_ANDROID)

TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE ocpn::wxjson )

IF(NOT WIN32)
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE "-lz" )
ENDIF()

IF(APPLE)
 FIND_PACKAGE(ZLIB REQUIRED)
 TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE ${ZLIB_LIBRARIES} )
ENDIF(APPLE)

if  (NOT WIN32 AND NOT APPLE AND NOT QT_ANDROID)
  include(OcpnFindGtk)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::gtk)
  find_package(PANGO REQUIRED)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::pango)
endif ()

IF(UNIX AND NOT APPLE AND NOT QT_ANDROID)
    FIND_PACKAGE(BZip2 REQUIRED)
    INCLUDE_DIRECTORIES(${BZIP2_INCLUDE_DIR})
    FIND_PACKAGE(ZLIB REQUIRED)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE
        ${BZIP2_LIBRARIES} ${ZLIB_LIBRARY}
    )
ENDIF(UNIX AND NOT APPLE AND NOT QT_ANDROID)

target_link_libraries(${PACKAGE_NAME} PRIVATE ${OPENCPN_LINK_TARGET} )

IF( APPLE )
    target_compile_options(${PACKAGE_NAME} PRIVATE
        "-Wall" "-Wno-unused" "-fexceptions"
        "-Wno-overloaded-virtual"
        "-Wno-deprecated" "-Wno-deprecated-declarations"
        "-D_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_"
    )
ENDIF(APPLE)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang|GNU")
  target_compile_options(${PACKAGE_NAME} PRIVATE "-Wno-unknown-pragmas")
endif ()


IF(APPLE)
    INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME LIBRARY DESTINATION
            ${CMAKE_BINARY_DIR}/OpenCPN.app/Contents/PlugIns)

    IF(EXISTS ${PROJECT_SOURCE_DIR}/data)
       INSTALL(DIRECTORY data DESTINATION
           ${CMAKE_BINARY_DIR}/OpenCPN.app/Contents/SharedSupport/plugins/${PACKAGE_NAME})
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

    SET(INSTALL_DIRECTORY "plugins/${PACKAGE_NAME}")

    IF(EXISTS ${PROJECT_SOURCE_DIR}/data)
        INSTALL(DIRECTORY data DESTINATION "${INSTALL_DIRECTORY}")
    ENDIF(EXISTS ${PROJECT_SOURCE_DIR}/data)

ENDIF(WIN32)

# Copy shared library to execution folder for debugging if not CI build
if (NOT OCPN_CI_BUILD)
  if (MSVC)
    add_custom_command(
      TARGET ${PACKAGE_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "Copying dll file"
      COMMAND ${CMAKE_COMMAND} -E echo "$(TargetPath)" "${CMAKE_BINARY_DIR}/$(configuration)/plugins"
      COMMAND ${CMAKE_COMMAND} -E copy "$(TargetPath)" "${CMAKE_BINARY_DIR}/$(configuration)/plugins"
    )
      if (CMAKE_BUILD_TYPE MATCHES "Debug")
        add_custom_command(
          COMMAND ${CMAKE_COMMAND} -E echo "Copying pdb file"
          COMMAND ${CMAKE_COMMAND} -E echo "$(OutputPath)/$(TargetName).pdb" "${CMAKE_BINARY_DIR}/$(configuration)/plugins"
          COMMAND ${CMAKE_COMMAND} -E copy "$(OutputPath)/$(TargetName).pdb" "${CMAKE_BINARY_DIR}/$(configuration)/plugins"
        )
      endif (CMAKE_BUILD_TYPE)
  endif (MSVC)
endif (NOT OCPN_CI_BUILD)

INCLUDE("../PluginLocalization.cmake")
PLUGIN_LOCALIZATION()
