#---------------------------------------------------------------------------
## Author:      Dave Register
## DEB config from: antonm - Anton Martchukov <anton@martchukov.com>
## Update:      sethdart (Jean-Eudes Onfray)
##              with parts from balp (Anders Arnholm)
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
## *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
## ***************************************************************************


#TODO:
# - cross platform compiling
# - test with Win & OSX
# USE_GLU_TESS
# USE_GLU_DLL
# I also find it deficient in some areas. For instance, I cannot make it output a VS project with certain compile switches set as desired, namely /MT vs /MD. This means I must manually set this and other compiler options after the CMake step. Sometimes I forget. Grrr...
# set /MT for Release build, and /MTd for debug.

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)
# This should be 2.8.0 to have FindGTK2 module
IF (COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0002 OLD)
  CMAKE_POLICY(SET CMP0003 OLD)
  CMAKE_POLICY(SET CMP0005 OLD)
  CMAKE_POLICY(SET CMP0011 OLD)
ENDIF (COMMAND cmake_policy)

PROJECT(OpenCPN)

SET(CMAKE_VERBOSE_MAKEFILE ON)

IF(APPLE)
  SET(PACKAGE_NAME OpenCPN)
ELSE(APPLE)
  SET(PACKAGE_NAME opencpn)
ENDIF(APPLE)


INCLUDE( ${CMAKE_SOURCE_DIR}/VERSION.cmake )
SET(PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

#Red Hat:   /etc/redhat-release
#Slackware: /etc/slackware-version
#Slamd64:   /etc/slamd64-version
#Gentoo: /etc/gentoo-release
#Ubuntu: /etc/debian_version

# TODO: Should PREFIX be set as well?
# Based on code from nohal
IF (NOT WIN32)
  # default
  SET (ARCH "i386")
  SET (LIB_INSTALL_DIR "lib")
  IF (EXISTS /etc/debian_version)
    SET (PACKAGE_FORMAT "DEB")
    SET (PACKAGE_DEPS "libc6, libwxgtk3.0-0, wx3.0-i18n, libglu1-mesa (>= 7.0.0), libgl1-mesa-glx (>= 7.0.0), zlib1g, bzip2, libtinyxml2.6.2, libportaudio2")
    SET (PACKAGE_RECS "xcalib,xdg-utils")
    SET (LIB_INSTALL_DIR "lib")
    IF (CMAKE_SIZEOF_VOID_P MATCHES "8")
      SET (ARCH "amd64")
#      SET (LIB_INSTALL_DIR "lib64")
    ELSE (CMAKE_SIZEOF_VOID_P MATCHES "8")
      SET (ARCH "i386")
    ENDIF (CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(TENTATIVE_PREFIX "/usr/local")
  ENDIF (EXISTS /etc/debian_version)
  IF (EXISTS /etc/redhat-release)
    SET (PACKAGE_FORMAT "RPM")
    #    SET (PACKAGE_DEPS  "wxGTK mesa-libGLU mesa-libGL gettext zlib bzip2 portaudio")
    IF (CMAKE_SIZEOF_VOID_P MATCHES "8")
      SET (ARCH "x86_64")
      SET (LIB_INSTALL_DIR "lib64")
    ELSE (CMAKE_SIZEOF_VOID_P MATCHES "8")
      SET (ARCH "i386")
      SET (LIB_INSTALL_DIR "lib")
    ENDIF (CMAKE_SIZEOF_VOID_P MATCHES "8")
  ENDIF (EXISTS /etc/redhat-release)
  IF (EXISTS /etc/suse-release OR EXISTS /etc/SuSE-release)
    SET (PACKAGE_FORMAT "RPM")
    #    SET (PACKAGE_DEPS  "libwx_baseu-2_8-0-wxcontainer MesaGLw libbz2-1 portaudio")
    IF (CMAKE_SIZEOF_VOID_P MATCHES "8")
      SET (ARCH "x86_64")
      SET (LIB_INSTALL_DIR "lib64")
    ELSE (CMAKE_SIZEOF_VOID_P MATCHES "8")
      SET (ARCH "i386")
      SET (LIB_INSTALL_DIR "lib")
    ENDIF (CMAKE_SIZEOF_VOID_P MATCHES "8")
  ENDIF (EXISTS /etc/suse-release OR EXISTS /etc/SuSE-release)
  IF (EXISTS /etc/gentoo-release)
    SET (LIB_INSTALL_DIR "lib${LIB_SUFFIX}")
  ENDIF (EXISTS /etc/gentoo-release)
  IF(APPLE)
    IF (CMAKE_SIZEOF_VOID_P MATCHES "8")
#      IF (CMAKE_OSX_64)
        SET(ARCH "x86_64")
#      ENDIF (CMAKE_OSX_64)
    ENDIF (CMAKE_SIZEOF_VOID_P MATCHES "8")
  ENDIF()
ELSE (NOT WIN32)
  # On WIN32 probably CMAKE_SIZEOF_VOID_P EQUAL 8, but we don't use it at all now...
  SET (ARCH "i386")
ENDIF (NOT WIN32)

IF (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
  SET (ARCH "armhf")
ENDIF (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")

IF (ARCH MATCHES "armhf")
  ADD_DEFINITIONS(-DARMHF)
ENDIF (ARCH MATCHES "armhf")


MESSAGE (STATUS "*** Build Architecture is ${ARCH}")
SET(PACKAGE_RELEASE "1" CACHE STRING "Package release number")

MESSAGE (STATUS "*** Staging to build ${PACKAGE_NAME} ${PACKAGE_VERSION} ***")

#If on Windows or Apple, build the monolithic package, to override use a non-sense values from the commandline (due to cmake inability to distinguish between not set and set to FALSE):
#cmake -DBUNDLE_DOCS=BLAH -DBUNDLE_TCDATA=BLAH -DBUNDLE_GSHHS=BLAH ..
IF(APPLE OR WIN32)
  IF(NOT BUNDLE_DOCS)
    SET (BUNDLE_DOCS "ON")
  ENDIF(NOT BUNDLE_DOCS)
  IF(NOT BUNDLE_GSHHS)
    SET (BUNDLE_GSHHS "CRUDE")
  ENDIF(NOT BUNDLE_GSHHS)
  IF(NOT BUNDLE_TCDATA)
    SET (BUNDLE_TCDATA "ON")
  ENDIF(NOT BUNDLE_TCDATA)
ENDIF(APPLE OR WIN32)

IF(BUNDLE_DOCS MATCHES "ON")
  MESSAGE (STATUS "*** Package will include documentation ***")
  IF(UNIX AND NOT APPLE)
    SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_docs ON")
  ENDIF(UNIX AND NOT APPLE)
ELSE(BUNDLE_DOCS MATCHES "ON")
  MESSAGE (STATUS "*** Package will NOT include documentation ***")
  IF(UNIX AND NOT APPLE)
    SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_docs OFF")
  ENDIF(UNIX AND NOT APPLE)
ENDIF(BUNDLE_DOCS MATCHES "ON")
IF(BUNDLE_GSHHS MATCHES "NONE|MIN|CRUDE|LOW|INTERMEDIATE|HIGH|FULL")
  # Possible values: NONE (default), MIN (=Only crude polygons, no borders/rivers), CRUDE, LOW, INTERMEDIATE, HIGH, FULL
  MESSAGE (STATUS "*** Package will include GSHHS basechart level: ${BUNDLE_GSHHS} ***")
  IF(UNIX AND NOT APPLE)
    IF(BUNDLE_GSHHS MATCHES "NONE")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_min OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_crude OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_low OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_intermediate OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_high OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_full OFF")
    ENDIF(BUNDLE_GSHHS MATCHES "NONE")
    IF(BUNDLE_GSHHS MATCHES "MIN")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_min ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_crude OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_low OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_intermediate OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_high OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_full OFF")
    ENDIF(BUNDLE_GSHHS MATCHES "MIN")
    IF(BUNDLE_GSHHS MATCHES "CRUDE")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_min ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_crude ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_low OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_intermediate OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_high OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_full OFF")
    ENDIF(BUNDLE_GSHHS MATCHES "CRUDE")
    IF(BUNDLE_GSHHS MATCHES "LOW")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_min ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_crude ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_low ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_intermediate OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_high OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_full OFF")
    ENDIF(BUNDLE_GSHHS MATCHES "LOW")
    IF(BUNDLE_GSHHS MATCHES "INTERMEDIATE")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_min ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_crude ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_low ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_intermediate ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_high OFF")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_full OFF")
    ENDIF(BUNDLE_GSHHS MATCHES "INTERMEDIATE")
    IF(BUNDLE_GSHHS MATCHES "HIGH")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_min ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_crude ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_low ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_intermediate ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_high ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_full OFF")
    ENDIF(BUNDLE_GSHHS MATCHES "HIGH")
    IF(BUNDLE_GSHHS MATCHES "FULL")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_min ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_crude ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_low ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_intermediate ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_high ON")
      SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_full ON")
    ENDIF(BUNDLE_GSHHS MATCHES "FULL")
  ENDIF(UNIX AND NOT APPLE)
ELSE(BUNDLE_GSHHS MATCHES "NONE|MIN|CRUDE|LOW|INTERMEDIATE|HIGH|FULL")
  SET(BUNDLE_GSHHS "NONE")
  MESSAGE (STATUS "*** Package will NOT include GSHHS data ***")
  IF(UNIX AND NOT APPLE)
    SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_min OFF")
    SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_crude OFF")
    SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_low OFF")
    SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_intermediate OFF")
    SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_high OFF")
    SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_gshhs_full OFF")
    SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_tcdata OFF")
  ENDIF(UNIX AND NOT APPLE)
ENDIF(BUNDLE_GSHHS MATCHES "NONE|MIN|CRUDE|LOW|INTERMEDIATE|HIGH|FULL")

IF(BUNDLE_TCDATA MATCHES "ON")
  MESSAGE (STATUS "*** Package will include tide and current data ***")
  IF(UNIX AND NOT APPLE)
    SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_tcdata ON")
  ENDIF(UNIX AND NOT APPLE)
ELSE(BUNDLE_TCDATA MATCHES "ON")
  MESSAGE (STATUS "*** Package will NOT include tide and current data ***")
  IF(UNIX AND NOT APPLE)
    SET(CPACK_RPM_SPEC_MORE_DEFINE "${CPACK_RPM_SPEC_MORE_DEFINE}\n%define _include_tcdata OFF")
  ENDIF(UNIX AND NOT APPLE)
ENDIF(BUNDLE_TCDATA MATCHES "ON")


#  To build for android, use this cmake invocation..
#$cmake -DUSE_GARMINHOST=OFF -D_wx_selected_config=androideabi-qt -DCMAKE_TOOLCHAIN_FILE=../buildandroid/build_android.cmake ..

IF(DEFINED _wx_selected_config)

MESSAGE (STATUS "selected config ${_wx_selected_config}")

IF(_wx_selected_config MATCHES "androideabi-qt")
  MESSAGE (STATUS "Building for wxQt-Android")
  SET(QT_ANDROID "ON")
  SET(QT_LINUX "OFF")
  SET(QT "ON")
ELSE(_wx_selected_config MATCHES "androideabi-qt")
  IF(_wx_selected_config MATCHES "qt")
    MESSAGE (STATUS "Building for wxQt-linux")
    SET(QT_ANDROID "OFF")
    SET(QT_LINUX "ON")
    SET(QT "ON")
  ENDIF(_wx_selected_config MATCHES "qt")
ENDIF(_wx_selected_config MATCHES "androideabi-qt")

ENDIF(DEFINED _wx_selected_config)

IF(UNIX)
  if(NOT QT)
   MESSAGE (STATUS "Building for wxGTK2")
   SET(GTK2 "ON")
  ENDIF(NOT QT)
ENDIF(UNIX)

IF((_wx_selected_config MATCHES "qt-armv7"))
#SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

IF((_wx_selected_config MATCHES "qt-armv7"))
SET(wxWidgets_USE_LIBS base core     xml html adv aui)
ELSE()
SET(wxWidgets_USE_LIBS base core net xml html adv aui)
ENDIF()

OPTION (USE_GL "Enable OpenGL support" ON)

# Search for opengles, short of running a program to test the speed
# of acceleration, I simply use gles on "native linux" arm systems
IF (ARCH MATCHES "arm*" AND (NOT QT_ANDROID) )
  find_path(OPENGLESv1_INCLUDE_DIR GLES/gl.h )
  IF (OPENGLESv1_INCLUDE_DIR)
    MESSAGE (STATUS "Found OpenGLESv1")
    ADD_DEFINITIONS(-DocpnUSE_GLES)
    ADD_DEFINITIONS(-DocpnUSE_GL)
    ADD_DEFINITIONS(-DUSE_GLU_TESS)

    SET(OPENGLES_FOUND "YES")
    SET(OPENGL_FOUND "YES")

    SET(wxWidgets_USE_LIBS ${wxWidgets_USE_LIBS} gl )
    add_subdirectory(src/glshim)

    add_subdirectory(src/glu)

    SET(OPENGL_LIBRARIES "GL_static" "GLU_static" "EGL" "X11" "drm" "dri2")
  ENDIF()
ENDIF()

#  Building for QT_ANDROID involves a cross-building environment,
#  So the include directories, flags, etc must be stated explicitly
#  without trying to locate them on the host build system.
IF(QT_ANDROID)
    MESSAGE (STATUS "Using GLESv1 for Android")
    ADD_DEFINITIONS(-DocpnUSE_GLES)
    ADD_DEFINITIONS(-DocpnUSE_GL)
    ADD_DEFINITIONS(-DUSE_GLU_TESS)
    ADD_DEFINITIONS(-DARMHF)
   
    SET(OPENGLES_FOUND "YES")
    SET(OPENGL_FOUND "YES")

    SET(wxWidgets_USE_LIBS ${wxWidgets_USE_LIBS} gl )
    add_subdirectory(src/glshim)

    add_subdirectory(src/glu)

ENDIF(QT_ANDROID)


IF ((NOT OPENGLES_FOUND) AND (NOT QT_ANDROID))

IF(USE_GL)
FIND_PACKAGE(OpenGL)
ELSE(USE_GL)
    MESSAGE (STATUS "OpenGL disabled by option..." )
ENDIF(USE_GL)

IF(OPENGL_FOUND)

    SET(wxWidgets_USE_LIBS ${wxWidgets_USE_LIBS} gl )
    INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})

    MESSAGE (STATUS "Found OpenGL..." )
    MESSAGE (STATUS "    Lib: " ${OPENGL_LIBRARIES})
    MESSAGE (STATUS "    Include: " ${OPENGL_INCLUDE_DIR})
    ADD_DEFINITIONS(-DocpnUSE_GL)
    ADD_DEFINITIONS(-DUSE_GLU_TESS)

ELSE(OPENGL_FOUND)
    MESSAGE (STATUS "OpenGL not found..." )
ENDIF(OPENGL_FOUND)

ENDIF()

IF(NOT QT_ANDROID)
# Find wxWidgets here, and the setting get inherited by all plugins.
# These options can be used to set the linux widgets build type
SET( wxWidgets_USE_DEBUG OFF)
SET( wxWidgets_USE_UNICODE ON)
SET( wxWidgets_USE_UNIVERSAL OFF)
SET( wxWidgets_USE_STATIC OFF)

FIND_PACKAGE(wxWidgets REQUIRED)
INCLUDE(${wxWidgets_USE_FILE})
    MESSAGE (STATUS "Found wxWidgets..." )
    MESSAGE (STATUS "${wxWidgets_LIBRARIES}")
ENDIF(NOT QT_ANDROID)

MESSAGE (STATUS "")
MESSAGE (STATUS "*** Staging to build ${PACKAGE_NAME}  ***")

#SET(CMAKE_BUILD_TYPE Debug)
IF( NOT CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE Release CACHE STRING
       "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
       FORCE )
ENDIF( NOT CMAKE_BUILD_TYPE )

MESSAGE (STATUS "Build type: ${CMAKE_BUILD_TYPE}")

IF (PREFIX)
    SET(CMAKE_INSTALL_PREFIX ${PREFIX})
ELSE (PREFIX)
    IF (NOT CMAKE_INSTALL_PREFIX)
        SET(CMAKE_INSTALL_PREFIX ${TENTATIVE_PREFIX})
    ENDIF (NOT CMAKE_INSTALL_PREFIX)
ENDIF (PREFIX)

MESSAGE (STATUS "*** Will install to ${CMAKE_INSTALL_PREFIX}  ***")
SET(PREFIX_BIN bin)
SET(PREFIX_INCLUDE include)
SET(PREFIX_DATA share)
SET(PREFIX_PKGDATA ${PREFIX_DATA}/${PACKAGE_NAME})
SET(PREFIX_LIB "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

# require proper c++
#ADD_DEFINITIONS( "-Wall -ansi -pedantic -Wno-variadic-macros" )
#TODO: Should we use  -fno-stack-protector
#  IF NOT DEBUGGING CFLAGS="-O2 -march=native"
IF(NOT WIN32 AND NOT APPLE )
  ADD_DEFINITIONS( "-Wall -Wno-unused -fexceptions -rdynamic" )
  ADD_DEFINITIONS( " -g -fno-strict-aliasing")

  ADD_DEFINITIONS( " -DPREFIX=\\\"${CMAKE_INSTALL_PREFIX}\\\"")
  # profiling with gprof
#    ADD_DEFINITIONS( -pg )
#    SET(CMAKE_EXE_LINKER_FLAGS -pg)
  # profiling with gcov
#    ADD_DEFINITIONS( "-fprofile-arcs -ftest-coverage" )
#    SET(EXTRA_LIBS ${EXTRA_LIBS} "gcov")
ENDIF(NOT WIN32 AND NOT APPLE)

IF( APPLE )
  ADD_DEFINITIONS( "-Wall -Wno-unused -fexceptions -Wno-overloaded-virtual" )
  ADD_DEFINITIONS( " -g -fno-strict-aliasing")
  ADD_DEFINITIONS( " -Wno-deprecated -Wno-deprecated-declarations" )
  ADD_DEFINITIONS( " -D_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_" )
ENDIF(APPLE)

#Enable CrashReporting for Windows and Linux
IF(WIN32)
    ADD_DEFINITIONS(-DOCPN_USE_CRASHRPT)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/buildwin/crashrpt)
    SET(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/CrashRpt1402.lib)
    SET(CMAKE_EXE_LINKER_FLAGS /DEBUG)
ELSE(WIN32)
    IF(NOT APPLE AND NOT QT_ANDROID)
        IF(CMAKE_BUILD_TYPE MATCHES "Rel*")
            ADD_DEFINITIONS(-DLINUX_CRASHRPT)
        ENDIF(CMAKE_BUILD_TYPE MATCHES "Rel*")
    ENDIF(NOT APPLE AND NOT QT_ANDROID)
ENDIF(WIN32)

# Add some definitions to satisfy MS
IF(WIN32)
    ADD_DEFINITIONS(-D__MSVC__)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
    ADD_DEFINITIONS(-DPSAPI_VERSION=1)
ENDIF(WIN32)

# Location where cmake first looks for cmake modules.
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/build)

# Let cmake find additional modules private to opencpn
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/")

IF (NOT WIN32 AND NOT APPLE)

IF(GTK2)
FIND_PACKAGE(GTK2 2.6)
IF (GTK2_FOUND)
    MESSAGE (STATUS "Found GTK2")
    ADD_DEFINITIONS(-DocpnUSE_GTK_OPTIMIZE)
    ADD_DEFINITIONS(-DocpnHAS_GTK)
    INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS})
    SET(EXTRA_LIBS ${EXTRA_LIBS} ${GTK2_LIBRARIES})

    # Add a definition to satisfy some linux system builds, avoiding Apple builds
    IF (NOT WIN32 AND NOT APPLE)
       ADD_DEFINITIONS("`pkg-config --cflags --libs gtk+-2.0`")
    ENDIF (NOT WIN32 AND NOT APPLE)

ELSE(GTK2_FOUND)
    MESSAGE (STATUS "GTK2 Not found...")
ENDIF (GTK2_FOUND)
ENDIF(GTK2)

IF(QT_ANDROID)
  SET(CMAKE_BUILD_TYPE Debug)
  ADD_DEFINITIONS(-D__WXQT__)
  ADD_DEFINITIONS(-DOCPN_USE_WRAPPER)
  ADD_DEFINITIONS(-D__OCPN__ANDROID__)
  ADD_DEFINITIONS(-DANDROID)

  SET(CMAKE_CXX_FLAGS "-pthread -fPIC -s")

  ## Compiler flags
 #   if(CMAKE_COMPILER_IS_GNUCXX)
 #       set(CMAKE_CXX_FLAGS "-O2")        ## Optimize
        set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary
 #   endif()
    
#  INCLUDE_DIRECTORIES("${Qt_Base}/5.2.1/android_armv7/include/QtCore")
#  INCLUDE_DIRECTORIES("${Qt_Base}/5.2.1/android_armv7/include")
#  INCLUDE_DIRECTORIES("${Qt_Base}/5.2.1/android_armv7/include/QtWidgets")
#  INCLUDE_DIRECTORIES("${Qt_Base}/5.2.1/android_armv7/include/QtGui")
#  INCLUDE_DIRECTORIES("${Qt_Base}/5.2.1/android_armv7/include/QtOpenGL")
#  INCLUDE_DIRECTORIES("${Qt_Base}/5.2.1/android_armv7/include/QtTest")

  INCLUDE_DIRECTORIES("${Qt_Base}/5.3/android_armv7/include/QtCore")
  INCLUDE_DIRECTORIES("${Qt_Base}/5.3/android_armv7/include")
  INCLUDE_DIRECTORIES("${Qt_Base}/5.3/android_armv7/include/QtWidgets")
  INCLUDE_DIRECTORIES("${Qt_Base}/5.3/android_armv7/include/QtGui")
  INCLUDE_DIRECTORIES("${Qt_Base}/5.3/android_armv7/include/QtOpenGL")
  INCLUDE_DIRECTORIES("${Qt_Base}/5.3/android_armv7/include/QtTest")

  ADD_DEFINITIONS(-DQT_WIDGETS_LIB)

  #  For shared library build
  
  
  SET(OCPN_Core_LIBRARIES

        /home/dsr/Projects/opencpn_android/build_5.3_static/libopencpn.so
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_baseu-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_core-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_html-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_baseu_xml-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_qa-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_adv-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_aui-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_baseu_net-3.1-arm-linux-androideabi.a
        ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_gl-3.1-arm-linux-androideabi.a
        ${Qt_Base}/5.3/android_armv7/lib/libQt5Core.so
        ${Qt_Base}/5.3/android_armv7/lib/libQt5OpenGL.so
        ${Qt_Base}/5.3/android_armv7/lib/libQt5Widgets.so
        ${Qt_Base}/5.3/android_armv7/lib/libQt5Gui.so
        ${Qt_Base}/5.3/android_armv7/lib/libQt5AndroidExtras.so
        ${NDK_Base}/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/libgnustl_shared.so
 
        
        )
    
  SET(ASHARE OFF)

  IF(ASHARE)
    SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
    INCLUDE_DIRECTORIES( "${wxQt_Base}/${wxQt_Build}/lib/wx/include/arm-linux-androideabi-qt-unicode-3.1")
  ELSE(ASHARE)
    #  For static build
    INCLUDE_DIRECTORIES( "${wxQt_Base}/${wxQt_Build}/lib/wx/include/arm-linux-androideabi-qt-unicode-static-3.1")
  ENDIF(ASHARE)
  
  INCLUDE_DIRECTORIES("${wxQt_Base}/include")

ENDIF()



IF(UNIX AND NOT QT_ANDROID)
INCLUDE("FindPortaudio.cmake")
IF (PORTAUDIO_FOUND)
    MESSAGE (STATUS "Portaudio Found")
    INCLUDE_DIRECTORIES(${PORTAUDIO_INCLUDE_DIRS})
    SET(EXTRA_LIBS ${EXTRA_LIBS} ${PORTAUDIO_LIBRARIES})
    ADD_DEFINITIONS(${PORTAUDIO_DEFINITIONS})
    ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
ELSE (PORTAUDIO_FOUND)
    MESSAGE (STATUS "Portaudio Not Found...")
ENDIF (PORTAUDIO_FOUND)

#SET(EXTRA_LIBS ${EXTRA_LIBS} "/usr/lib/i386-linux-gnu/libportaudio.so.2.0.0")
#ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
ENDIF(UNIX AND NOT QT_ANDROID)

ENDIF (NOT WIN32 AND NOT APPLE)

FIND_PACKAGE(Gettext REQUIRED)

IF(UNIX)
  IF(NOT QT_ANDROID) # this should be detected by the grib plugin
    FIND_PACKAGE(BZip2 REQUIRED)
    INCLUDE_DIRECTORIES(${BZIP2_INCLUDE_DIR})
    FIND_PACKAGE(ZLIB REQUIRED)
    FIND_PACKAGE(X11)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
    FIND_PACKAGE(TinyXML)
  ENDIF(NOT QT_ANDROID)  
ENDIF(UNIX)

# For convenience we define the sources as a variable. You can add
# header files and cpp/c files and CMake will sort them out

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
		src/nmea0183/GPwpl.cpp
		src/nmea0183/GPwpl.hpp
        src/nmea0183/apb.cpp
        src/nmea0183/apb.hpp
        src/nmea0183/xte.cpp
        src/nmea0183/xte.hpp
	)
ADD_LIBRARY(NMEA0183 ${SRC_NMEA0183})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/nmea0183)


#IF(NOT WIN32)
#OPTION (USE_GPSD "Enable GPSD Library" ON)
#IF (USE_GPSD)
#      message (STATUS "GPSD Library support: enabled")
#
##     FIND_PACKAGE(gps QUIET)
#     FIND_PACKAGE(gps)
#      IF (GPS_FOUND)
#            message (STATUS "Building with libgps includes")
#            INCLUDE_DIRECTORIES(${GPS_INCLUDE_DIR})
#            ADD_DEFINITIONS(${GPS_DEFINITIONS})
#            ADD_DEFINITIONS(-DBUILD_WITH_LIBGPS)
#            message (STATUS "GPS library include location is ${GPS_INCLUDE_DIR}")
##            SET(EXTRA_LIBS ${EXTRA_LIBS} ${GPS_LIBRARY})
#      ELSE (GPS_FOUND)
#            message (STATUS "Gps library not found")
#      ENDIF (GPS_FOUND)
#ENDIF (USE_GPSD)
#ENDIF (NOT WIN32)




SET(HDRS
                include/ocpn_types.h
                include/ocpndc.h
		include/chart1.h
		include/bbox.h
		include/ocpn_pixel.h
		include/chartdb.h
		include/chartdbs.h
		include/chartimg.h
		include/chcanv.h
		include/timers.h
		include/emboss_data.h
		include/AISTargetQueryDialog.h
		include/AIS_Bitstring.h
		include/AISTargetListDialog.h
		include/OCPNListCtrl.h
		include/AISTargetAlertDialog.h
		include/AIS_Decoder.h
		include/AIS_Target_Data.h
		include/CM93DSlide.h
		include/GoToPositionDialog.h
		include/RolloverWin.h
		include/S57QueryDialog.h
		include/S57ObjectDesc.h
		include/S57Light.h
		include/s57RegistrarMgr.h
		include/S57ClassRegistrar.h
		include/MarkIcon.h
		include/SendToGpsDlg.h
		include/TCWin.h
		include/tide_time.h
		include/ChInfoWin.h
		include/ocpCursor.h
		include/Quilt.h
		include/Hyperlink.h
		include/NavObjectCollection.h
		include/RoutePoint.h
		include/Route.h
		include/SelectItem.h
		include/Select.h
		include/FontMgr.h
		include/FontDesc.h
		include/vector2D.h
		include/OCPNSoundData.h
		include/OCPN_Sound.h
		include/NMEALogWindow.h
		include/WindowDestroyListener.h
		include/TTYWindow.h
		include/TTYScroll.h
		include/Layer.h
		include/concanv.h
		include/cutil.h
		include/georef.h
		include/navutil.h
		include/routeman.h
		include/routemanagerdialog.h
		include/routeprop.h
		include/routeprintout.h
		include/PositionParser.h
		include/printtable.h
		include/chartbarwin.h
		include/tcmgr.h
		include/Station_Data.h
		include/IDX_entry.h
		include/TC_Error_Code.h
		include/TCDataFactory.h
		include/TCDS_Ascii_Harmonic.h
		include/TCDS_Binary_Harmonic.h
		include/TCDataSource.h
		include/thumbwin.h
		include/options.h
		include/gshhs.h
		include/kml.h
		include/undo.h
		include/about.h
		include/ais.h
		include/pluginmanager.h
		include/ocpn_plugin.h
		include/gpxdocument.h
		include/wx/json_defs.h
		include/wx/jsonwriter.h
		include/wx/jsonreader.h
		include/styles.h
		include/toolbar.h
		include/compass.h
		include/geodesic.h
		include/datastream.h
		include/OCPN_DataStreamEvent.h
		include/ConnectionParams.h
		include/OCP_DataStreamInput_Thread.h
		include/dsPortType.h
                include/multiplexer.h
                include/OCPNRegion.h
                include/TrackPropDlg.h
                include/LinkPropDlg.h
                include/viewport.h
                include/canvasMenu.h
                include/OCPNPlatform.h
                include/S57Sector.h
)

SET(SRCS
		src/chart1.cpp
		src/bbox.cpp
		src/ocpn_pixel.cpp
                src/ocpndc.cpp
		src/chartdb.cpp
		src/chartdbs.cpp
		src/chartimg.cpp
		src/chcanv.cpp
		src/ocpCursor.cpp
		src/TCWin.cpp
		src/S57QueryDialog.cpp
		src/s57RegistrarMgr.cpp
		src/GoToPositionDialog.cpp
		src/CM93DSlide.cpp
		src/RolloverWin.cpp
		src/ChInfoWin.cpp
		src/AISTargetQueryDialog.cpp
		src/AIS_Bitstring.cpp
		src/AISTargetListDialog.cpp
		src/AISTargetAlertDialog.cpp
		src/AIS_Decoder.cpp
		src/AIS_Target_Data.cpp
		src/OCPNListCtrl.cpp
		src/Quilt.cpp
		src/Hyperlink.cpp
		src/NavObjectCollection.cpp
		src/RoutePoint.cpp
		src/Route.cpp
		src/SelectItem.cpp
		src/Select.cpp
		src/FontMgr.cpp
		src/FontDesc.cpp
		src/OCPNSoundData.cpp
		src/OCPN_Sound.cpp
		src/NMEALogWindow.cpp
		src/TTYWindow.cpp
		src/TTYScroll.cpp
		src/SendToGpsDlg.cpp
		src/Layer.cpp
		src/concanv.cpp
		src/cutil.cpp
		src/georef.cpp
		src/navutil.cpp
		src/routeman.cpp
		src/routemanagerdialog.cpp
		src/routeprop.cpp
		src/routeprintout.cpp
		src/PositionParser.cpp
		src/printtable.cpp
		src/chartbarwin.cpp
		src/tcmgr.cpp
		src/Station_Data.cpp
		src/IDX_entry.cpp
		src/TCDataFactory.cpp
		src/TCDS_Ascii_Harmonic.cpp
		src/TCDS_Binary_Harmonic.cpp
		src/TCDataSource.cpp
		src/thumbwin.cpp
		src/options.cpp
		src/gshhs.cpp
		src/kml.cpp
		src/undo.cpp
		src/about.cpp
		src/ais.cpp
		src/pluginmanager.cpp
		src/gpxdocument.cpp
		src/wxJSON/jsonwriter.cpp
		src/wxJSON/jsonreader.cpp
		src/wxJSON/jsonval.cpp
		src/styles.cpp
		src/toolbar.cpp
		src/compass.cpp
		src/geodesic.cpp
		src/datastream.cpp
		src/OCPN_DataStreamEvent.cpp
		src/ConnectionParams.cpp
		src/OCP_DataStreamInput_Thread.cpp
                src/multiplexer.cpp
                src/pugixml.cpp
                src/OCPNRegion.cpp
                src/TrackPropDlg.cpp
                src/LinkPropDlg.cpp
                src/ssl/sha1.c
                src/viewport.cpp
                src/canvasMenu.cpp
                src/OCPNPlatform.cpp
    )

IF(NOT WIN32 AND NOT APPLE AND NOT QT_ANDROID)
  SET(HDRS ${HDRS} include/crashprint.h )
  SET(SRCS ${SRCS} src/crashprint.cpp)
ENDIF(NOT WIN32 AND NOT APPLE AND NOT QT_ANDROID)

IF(QT_ANDROID)
    SET(HDRS ${HDRS} include/androidUTIL.h)
    SET(SRCS ${SRCS} src/androidUTIL.cpp)
ENDIF(QT_ANDROID)

#  Search for wxWidgets 2.9 or newer
STRING(REGEX MATCHALL "-2.9|-3." wx_Version_2.9+ "${wxWidgets_INCLUDE_DIRS}")
IF(wx_Version_2.9+)
ELSE(wx_Version_2.9+)
#  SET(HDRS ${HDRS} include/scrollingdialog.h )
#  SET(SRCS ${SRCS} src/scrollingdialog.cpp)
ENDIF(wx_Version_2.9+)

SET( PACKAGE_HEADERS "" )

SET(SRC_TINYXML
            src/tinyxml.cpp
            src/tinyxmlerror.cpp
            src/tinyxmlparser.cpp
            include/tinyxml.h
      )

IF(OPENGL_FOUND)
  SET(HDRS ${HDRS} include/glChartCanvas.h
                   include/glTextureDescriptor.h
                   include/glTexCache.h
                   include/TexFont.h
        )
  SET(SRCS ${SRCS} src/glChartCanvas.cpp
                   src/glTextureDescriptor.cpp
                   src/glTexCache.cpp
                   src/TexFont.cpp
        )
ENDIF(OPENGL_FOUND)


IF(TINYXML_FOUND)
    message (STATUS "Building with tinyxml includes")
    INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
    ADD_DEFINITIONS(-DTIXML_USE_STL)
ELSE(TINYXML_FOUND)
    message (STATUS "tinyxml library not found")
    SET(SRCS ${SRCS} ${SRC_TINYXML})
    ADD_DEFINITIONS(-DTIXML_USE_STL)
ENDIF(TINYXML_FOUND)



OPTION (USE_S57 "Enable S57 ENC support" ON)
IF (USE_S57)
	message (STATUS "S57 ENC support: enabled")
	ADD_DEFINITIONS(-DUSE_S57)

	SET(SRC_S57ENC
                include/TexFont.h
		include/s52plib.h
		include/s52utils.h
		include/s57chart.h
		include/mygeom.h
                src/TexFont.cpp
		src/s52cnsy.cpp
		src/s52plib.cpp
		src/s52utils.cpp
		src/s57chart.cpp
		src/cm93.cpp
		src/mygeom.cpp
		include/cm93.h
		src/tri.c
		src/myiso8211/ddffielddefn.cpp
		src/myiso8211/ddfmodule.cpp
		src/myiso8211/ddfrecord.cpp
		src/myiso8211/ddfsubfielddefn.cpp
		src/myiso8211/ddffield.cpp
		src/myiso8211/ddfutils.cpp
		src/mygdal/cpl_config.h
		src/mygdal/gdal_priv.h
		src/mygdal/cpl_conv.h
		src/mygdal/iso8211.h
		src/mygdal/cpl_csv.h
		src/mygdal/ogr_api.h
		src/mygdal/cpl_error.h
		src/mygdal/ogr_attrind.h
		src/mygdal/cpl_list.h
		src/mygdal/cpl_minixml.h
		src/mygdal/ogr_feature.h
		src/mygdal/cpl_multiproc.h
		src/mygdal/ogr_geometry.h
		src/mygdal/cpl_odbc.h
		src/mygdal/ogr_p.h
		src/mygdal/ogr_core.h
		src/mygdal/cpl_port.h
		src/mygdal/ogr_s57.h
		src/mygdal/cpl_string.h
		src/mygdal/ogr_spatialref.h
		src/mygdal/cpl_vsi.h
		src/mygdal/ogrsf_frmts.h
		src/mygdal/gdal.h
		src/mygdal/s57.h
		src/mygdal/gdal_frmts.h
		src/mygdal/ddfrecordindex.cpp
		src/mygdal/cpl_conv.cpp
		src/mygdal/cpl_csv.cpp
		src/mygdal/cpl_error.cpp
		src/mygdal/cpl_findfile.cpp
		src/mygdal/cpl_minixml.cpp
		src/mygdal/cpl_path.cpp
		src/mygdal/cpl_string.cpp
		src/mygdal/cpl_vsisimple.cpp
		src/mygdal/cplgetsymbol.cpp
		src/mygdal/ograssemblepolygon.cpp
		src/mygdal/ogrcurve.cpp
		src/mygdal/ogrfeature.cpp
		src/mygdal/ogrfeaturedefn.cpp
		src/mygdal/ogrfielddefn.cpp
		src/mygdal/ogrgeometry.cpp
		src/mygdal/ogrgeometrycollection.cpp
		src/mygdal/ogrgeometryfactory.cpp
		src/mygdal/ogrlinearring.cpp
		src/mygdal/ogrlinestring.cpp
		src/mygdal/ogrmultilinestring.cpp
		src/mygdal/ogrmultipoint.cpp
		src/mygdal/ogrmultipolygon.cpp
		src/mygdal/ogrpoint.cpp
		src/mygdal/ogrpolygon.cpp
		src/mygdal/ogrs57datasource.cpp
		src/mygdal/ogrlayer.cpp
		src/mygdal/ogrs57layer.cpp
		src/mygdal/ogrutils.cpp
		src/mygdal/s57classregistrar.cpp
		src/mygdal/s57featuredefns.cpp
		src/mygdal/s57reader.cpp
		src/mygdal/gdal_misc.cpp
	)
	ADD_LIBRARY(S57ENC ${SRC_S57ENC})
	INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/mygdal)
	SET(EXTRA_LIBS ${EXTRA_LIBS} S57ENC)
ELSE (USE_S57)
	message (STATUS "S57 ENC support: disabled")
ENDIF (USE_S57)

OPTION (USE_GARMINHOST "Enable Garmin Host Mode support" ON)
IF (USE_GARMINHOST)
	message (STATUS "Garmin Host Mode support: enabled")
	ADD_DEFINITIONS(-DUSE_GARMINHOST)

	SET(SRC_GARMINHOST
                  src/garmin/jeeps/gpsapp.c
                  src/garmin/jeeps/gpscom.c
                  src/garmin/jeeps/gpsmath.c
                  src/garmin/jeeps/gpsmem.c
	          src/garmin/jeeps/gpsprot.c
                  src/garmin/jeeps/gpsread.c
	          src/garmin/jeeps/gpsdevice.c
                  src/garmin/jeeps/gpsdevice_ser.c
                  src/garmin/jeeps/gpsdevice_usb.c
                  src/garmin/jeeps/gpsrqst.c
                  src/garmin/jeeps/gpssend.c
                  src/garmin/jeeps/gpsserial.c
                  src/garmin/jeeps/gpsutil.c
                  src/garmin/jeeps/gpsusbread.c
                  src/garmin/jeeps/gpsusbsend.c
                  src/garmin/jeeps/gpsusbcommon.c
                  src/garmin/jeeps/garmin_wrapper_utils.c
                  src/garmin/jeeps/garmin_wrapper.cpp
                  src/garmin/jeeps/garmin_gps.h
                  src/garmin/jeeps/garmin_wrapper.h
                  src/garmin/jeeps/gpsapp.h
                  src/garmin/jeeps/gpsdatum.h
                  src/garmin/jeeps/gpsfmt.h
                  src/garmin/jeeps/gpsmath.h
                  src/garmin/jeeps/gpsport.h
                  src/garmin/jeeps/gpsprot.h
                  src/garmin/jeeps/gpsrqst.h
                  src/garmin/jeeps/gpsserial.h
                  src/garmin/jeeps/gpsusbint.h
                  src/garmin/jeeps/garminusb.h
                  src/garmin/jeeps/garmin_wrapper_utils.h
                  src/garmin/jeeps/gpscom.h
                  src/garmin/jeeps/gpsdevice.h
                  src/garmin/jeeps/gpsinput.h
                  src/garmin/jeeps/gpsmem.h
                  src/garmin/jeeps/gpsproj.h
                  src/garmin/jeeps/gpsread.h
                  src/garmin/jeeps/gpssend.h
                  src/garmin/jeeps/gpsusbcommon.h
                  src/garmin/jeeps/gpsutil.h
	)
        IF(WIN32)
           SET(SRC_GARMINHOST ${SRC_GARMINHOST} src/garmin/jeeps/gpsusbwin.c)
        ENDIF(WIN32)

	ADD_LIBRARY(GARMINHOST ${SRC_GARMINHOST})
	INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/garmin/jeeps)
	SET(EXTRA_LIBS ${EXTRA_LIBS} GARMINHOST)
        IF(WIN32)
           ADD_DEFINITIONS(-D__WIN32__)
        ENDIF(WIN32)
ELSE (USE_GARMINHOST)
	message (STATUS "Garmin Host Mode support: disabled")
ENDIF (USE_GARMINHOST)

#Chart Symbols library
SET(SRC_SYMBOLS
    src/chartsymbols.cpp
    include/chartsymbols.h
    src/razdsparser.cpp
    include/razdsparser.h
    )

ADD_LIBRARY(SYMBOLS ${SRC_SYMBOLS})
SET(EXTRA_LIBS ${EXTRA_LIBS} SYMBOLS)




# Compile texcompression support library
IF (OPENGL_FOUND)

  SET(SRC_TEXCMP
    src/texcmp/lz4/lz4.c
    src/texcmp/lz4/lz4hc.c
    src/texcmp/squish/alpha.cpp
    src/texcmp/squish/clusterfit.cpp
    src/texcmp/squish/colourblock.cpp
    src/texcmp/squish/colourfit.cpp
    src/texcmp/squish/colourset.cpp
    src/texcmp/squish/maths.cpp
    src/texcmp/squish/rangefit.cpp
    src/texcmp/squish/singlecolourfit.cpp
    src/texcmp/squish/squish.cpp
    src/texcmp/etcpak.cpp)

  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/texcmp/lz4)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/texcmp/squish)

  ADD_LIBRARY(TEXCMP ${SRC_TEXCMP})
  SET(EXTRA_LIBS ${EXTRA_LIBS} TEXCMP)

  # always use optimizations for the texture compression which is much faster
  IF ( NOT MSVC )
      # try to use sse on x86 based systems
      IF (( ARCH MATCHES "i386" OR ARCH MATCHES "amd64" OR ARCH MATCHES "x86_64") AND NOT QT_ANDROID)
        # using sse which makes the compression run about 50% faster
        message (STATUS "Compiling texture compression library with sse support")
        set_property(TARGET TEXCMP PROPERTY COMPILE_FLAGS "-O3 -msse2 -DSQUISH_USE_SSE=2")
      ELSE ()
        # use standard optimizations for other architectures
        set_property(TARGET TEXCMP PROPERTY COMPILE_FLAGS "-O3")
      ENDIF ()
  ELSE (NOT MSVC)
        # try to use sse on x86 based systems
      IF ( ARCH MATCHES "i386" OR ARCH MATCHES "amd64" OR ARCH MATCHES "x86_64")
        # using sse which makes the compression run about 50% faster
        message (STATUS "Compiling texture compression library with sse support")
        set_property(TARGET TEXCMP PROPERTY COMPILE_FLAGS "/arch:SSE2 -DSQUISH_USE_SSE=2")
      ENDIF ()
  ENDIF (NOT MSVC)

ENDIF (OPENGL_FOUND)

#TODO
#dnl
#dnl Use OpenGL tesselator or Internal tesselator
#dnl
#tess_internal=false
#tess_glu=true
#AC_ARG_WITH(tess_internal,
#          [[  --with-tess-internal    use Internal Polygon Tesselator]],
#          [tess_internal=true]
#           )
#
#if [[ "$tess_internal" = "false" ]] ; then
#        dnl  Look for and qualify an external GLU library
#        echo "checking for useable OpenGLU Library"
#        AC_CHECK_LIB(GLU, gluNewTess, true, dnl here true is just a nop
#           AC_MSG_ERROR([*** libGLU not found.  Run configure using --with-tess-internal.]))
#
#        GL_LIBS="-lGL -lGLU"
#        AC_SUBST(GL_LIBS)
#
#        GL_CFLAGS="-DUSE_GLU_TESS"
#        AC_SUBST(GL_CFLAGS)
#fi


# If we build for windows systems, we also include the resource file
# containing the manifest, icon and other resources
IF(WIN32)
  SET(SRCS ${SRCS} src/opencpn.rc)
ENDIF(WIN32)

SET(gshhs_MIN
        data/gshhs/poly-c-1.dat
  )

SET(gshhs_CRUDE ${gshhs_MIN}
        data/gshhs/wdb_borders_c.b
        data/gshhs/wdb_rivers_c.b
  )

SET(gshhs_LOW ${gshhs_CRUDE}
        data/gshhs/poly-l-1.dat
        data/gshhs/wdb_borders_l.b
        data/gshhs/wdb_rivers_l.b
  )

SET(gshhs_INTERMEDIATE ${gshhs_LOW}
        data/gshhs/poly-i-1.dat
        data/gshhs/wdb_borders_i.b
        data/gshhs/wdb_rivers_i.b
  )

SET(gshhs_HIGH ${gshhs_INTERMEDIATE}
        data/gshhs/poly-h-1.dat
        data/gshhs/wdb_borders_h.b
        data/gshhs/wdb_rivers_h.b
  )

SET(gshhs_FULL ${gshhs_HIGH}
        data/gshhs/poly-f-1.dat
        data/gshhs/wdb_borders_f.b
        data/gshhs/wdb_rivers_f.b
  )

# Various data files
IF(BUNDLE_GSHHS MATCHES "MIN")
  SET(gshhs ${gshhs_MIN})
ENDIF(BUNDLE_GSHHS MATCHES "MIN")

IF(BUNDLE_GSHHS MATCHES "CRUDE")
  SET(gshhs ${gshhs_CRUDE})
ENDIF(BUNDLE_GSHHS MATCHES "CRUDE")

IF(BUNDLE_GSHHS MATCHES "LOW")
  SET(gshhs ${gshhs_LOW})
ENDIF(BUNDLE_GSHHS MATCHES "LOW")

IF(BUNDLE_GSHHS MATCHES "INTERMEDIATE")
  SET(gshhs ${gshhs_INTERMEDIATE})
ENDIF(BUNDLE_GSHHS MATCHES "INTERMEDIATE")

IF(BUNDLE_GSHHS MATCHES "HIGH")
  SET(gshhs ${gshhs_HIGH})
ENDIF(BUNDLE_GSHHS MATCHES "HIGH")

IF(BUNDLE_GSHHS MATCHES "FULL")
  SET(gshhs ${gshhs_FULL})
ENDIF(BUNDLE_GSHHS MATCHES "FULL")

SET(uiData
	src/bitmaps/styles.xml
	src/bitmaps/toolicons_journeyman.png
	src/bitmaps/toolicons_journeyman_flat.png
	src/bitmaps/toolicons_traditional.png
	)

IF (USE_S57)
      SET(s57data
            data/s57data/attdecode.csv
            data/s57data/S52RAZDS.RLE
            data/s57data/s57attributes.csv
            data/s57data/s57expectedinput.csv
            data/s57data/s57objectclasses.csv
            data/s57data/rastersymbols-dark.png
            data/s57data/rastersymbols-day.png
            data/s57data/rastersymbols-dusk.png
            data/s57data/chartsymbols.xml
             )
ENDIF (USE_S57)

IF(BUNDLE_TCDATA MATCHES "ON")
SET(tcData
        data/tcdata/HARMONIC
        data/tcdata/HARMONIC.IDX
        data/tcdata/README.harmonics
   )
ENDIF(BUNDLE_TCDATA MATCHES "ON")


IF(APPLE)
  INCLUDE_DIRECTORIES(/usr/X11/include /usr/X11/include/GL)

  SET(CMAKE_C_FLAGS "-O2 -arch ${ARCH}")
  SET(CMAKE_C_FLAGS_DEBUG "-g -O0 -arch ${ARCH}")
  SET(CMAKE_C_FLAGS_MINSIZEREL "-O2 -arch ${ARCH}")
  SET(CMAKE_C_FLAGS_RELEASE "-O3 -arch ${ARCH}")
  SET(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -arch ${ARCH}")

  SET(CMAKE_CXX_FLAGS "-O2 -arch ${ARCH}")
  SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -arch ${ARCH}")
  SET(CMAKE_CXX_FLAGS_MINSIZEREL "-O2 -arch ${ARCH}")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -arch ${ARCH}")
  SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3 -arch ${ARCH}")

  SET(CMAKE_EXE_LINKER_FLAGS "-O2 -arch ${ARCH}")
  SET(CMAKE_SHARED_LINKER_FLAGS "-O2 -arch ${ARCH}")
  SET(CMAKE_MODULE_LINKER_FLAGS "-O2 -arch ${ARCH}")

  SET(SRCS ${SRCS}
    include/macutils.h
    src/macutils.c
    )
ENDIF(APPLE)

#  Mac has trouble finding libgobject-2.0.dylib
#  We look for it explicitely
IF(xAPPLE)
#       Assume pkg-config is available.
    INCLUDE( FindPkgConfig )
    PKG_SEARCH_MODULE( GOBJECT REQUIRED gobject-2.0 )
#    MESSAGE (STATUS "    GOBJECT: " ${GOBJECT_LDFLAGS})

    SET(EXTRA_LIBS ${EXTRA_LIBS} ${GOBJECT_LDFLAGS})
    SET(EXTRA_LIBS ${EXTRA_LIBS} "iconv")
ENDIF(xAPPLE)

IF(WIN32)
    IF(MSVC)
        SET(CMAKE_C_FLAGS_DEBUG               "/MDd /Ob0 /Od  /D_DEBUG  /Zi /RTC1" )
        SET(CMAKE_C_FLAGS_MINSIZEREL          "/MD  /O1  /Ob1 /D NDEBUG")
        SET(CMAKE_C_FLAGS_RELEASE             "/MD  /O2  /Ob2 /D NDEBUG")
        SET(CMAKE_C_FLAGS_RELWITHDEBINFO      "/MD  /O2  /Ob1 /D NDEBUG /Zi")
        SET(CMAKE_CXX_FLAGS_DEBUG             "/MDd /Ob0 /Od  /D_DEBUG  /Zi /RTC1 /EHa")
        SET(CMAKE_CXX_FLAGS_MINSIZEREL        "/MD  /O1  /Ob1 /D NDEBUG /EHa")
        SET(CMAKE_CXX_FLAGS_RELEASE           "/MD  /O2  /Ob2 /D NDEBUG /EHa")
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO    "/MD  /O2  /Ob1 /D NDEBUG /Zi /EHa" )
    ENDIF(MSVC)
ADD_EXECUTABLE(${PACKAGE_NAME} WIN32 ${HDRS} ${SRCS})
ENDIF(WIN32)


IF(APPLE)

IF(NOT BUNDLE_GSHHS MATCHES "NONE")
  SET_SOURCE_FILES_PROPERTIES(
           ${gshhs} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/gshhs  )
ENDIF(NOT BUNDLE_GSHHS MATCHES "NONE")

SET_SOURCE_FILES_PROPERTIES(
           ${uiData} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/uidata  )
SET_SOURCE_FILES_PROPERTIES(
           ${s57data} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/s57data  )
IF(BUNDLE_TCDATA MATCHES "ON")
SET_SOURCE_FILES_PROPERTIES(
           ${tcData} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/tcdata  )
ENDIF(BUNDLE_TCDATA MATCHES "ON")

SET_SOURCE_FILES_PROPERTIES(
           data/license.txt PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport )

FILE(GLOB SOUND_FILES ${CMAKE_SOURCE_DIR}/data/sounds/*)
FOREACH (_currentSoundFile ${SOUND_FILES})
  SET_SOURCE_FILES_PROPERTIES(
           ${_currentSoundFile} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/sounds )
ENDFOREACH (_currentSoundFile )

SET_SOURCE_FILES_PROPERTIES(
           data/doc/help_web.html PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/doc )
IF(BUNDLE_DOCS MATCHES "ON")
  SET_SOURCE_FILES_PROPERTIES(
           data/doc/help_en_US.html PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/doc )

  FILE(GLOB DOC_FILES "${CMAKE_SOURCE_DIR}/data/doc/images/*")
  FOREACH (_currentDocFile ${DOC_FILES})
    SET_SOURCE_FILES_PROPERTIES(
           ${_currentDocFile}
           PROPERTIES MACOSX_PACKAGE_LOCATION "SharedSupport/doc/images" )
  ENDFOREACH (_currentDocFile )
ENDIF(BUNDLE_DOCS MATCHES "ON")

SET_SOURCE_FILES_PROPERTIES(
           buildosx/opencpn.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources )

SET_SOURCE_FILES_PROPERTIES(
           buildosx/MainMenu.xib PROPERTIES MACOSX_PACKAGE_LOCATION Resources/English.lproj )

#  These variables get substituted into the Info.plist template file at build time
SET(MACOSX_BUNDLE_ICON_FILE opencpn.icns)
SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PACKAGE_VERSION}")
SET(MACOSX_BUNDLE_BUNDLE_NAME "OpenCPN")
SET(MACOSX_BUNDLE_GUI_IDENTIFIER "org.opencpn")

ADD_EXECUTABLE(${PACKAGE_NAME} MACOSX_BUNDLE
      ${HDRS}
      ${SRCS}
      ${gshhs}
      ${uiData}
      ${s57data}
      ${tcData}
      data/license.txt
      ${SOUND_FILES}
      buildosx/opencpn.icns
      buildosx/MainMenu.xib
      )

IF(BUNDLE_DOCS MATCHES "ON")
  ADD_EXECUTABLE(${PACKAGE_NAME} MACOSX_BUNDLE
      data/doc/help_web.html
      data/doc/help_en_US.html
      ${DOC_FILES}
  )
ELSE(BUNDLE_DOCS MATCHES "ON")
  ADD_EXECUTABLE(${PACKAGE_NAME} MACOSX_BUNDLE
      data/doc/help_web.html
  )
ENDIF(BUNDLE_DOCS MATCHES "ON")

  INSTALL(TARGETS
    ${PACKAGE_NAME}
    BUNDLE DESTINATION ${PREFIX_BIN} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)


TARGET_LINK_LIBRARIES(${PACKAGE_NAME}
      ${wxWidgets_LIBRARIES}
      NMEA0183
      ${EXTRA_LIBS}
      )

set_target_properties(${PACKAGE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST
                       "${CMAKE_SOURCE_DIR}/buildosx/Info.plist.in")

ENDIF(APPLE)

IF(UNIX AND NOT APPLE)
ADD_EXECUTABLE(${PACKAGE_NAME} ${HDRS} ${SRCS})
ENDIF(UNIX AND NOT APPLE)

IF(QT_ANDROID)
ADD_LIBRARY(gorp ${HDRS} ${SRCS}
        ${SRC_NMEA0183}
        ${SRC_S57ENC}
#        ${SRC_GARMINHOST}
        ${SRC_TEXCMP}
        ${SRC_SYMBOLS}
        )
        
TARGET_LINK_LIBRARIES( gorp ${wxWidgets_LIBRARIES} )
      
ENDIF()






If(NOT APPLE AND NOT QT_ANDROID)
TARGET_LINK_LIBRARIES(${PACKAGE_NAME}
      ${wxWidgets_LIBRARIES}
      NMEA0183
      ${EXTRA_LIBS}
      )
ENDIF(NOT APPLE AND NOT QT_ANDROID)

IF(WIN32)
TARGET_LINK_LIBRARIES(${PACKAGE_NAME}
	setupapi.lib
	gdiplus.lib
	glu32.lib
        Psapi.lib
)
ENDIF(WIN32)

IF(UNIX)
TARGET_LINK_LIBRARIES(${PACKAGE_NAME}
      ${OPENGL_LIBRARIES}
      ${GETTEXT_LIBRARY}
      ${BZIP2_LIBRARIES}
      ${ZLIB_LIBRARY}
      )

IF(X11_FOUND AND NOT APPLE)
ADD_DEFINITIONS(-DOCPN_HAVE_X11)
TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${X11_LIBRARIES} )
ENDIF(X11_FOUND AND NOT APPLE)

IF(TINYXML_FOUND)
   TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${TINYXML_LIBRARIES} )
ENDIF(TINYXML_FOUND)
ENDIF(UNIX)

#   Certain older Cmake FindGTK2 modules ( e.g. cmake-2.8.0-2) do not yield all of the required link libraries
#   So, add them manually.  These declarations may be redundant in some architectures, but do no harm.
IF(UNIX)
TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} dl )
ENDIF(UNIX)

IF(GTK2_FOUND)
TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} gobject-2.0 )
ENDIF(GTK2_FOUND)

IF(QT_LINUX)
  TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} Qt5Widgets Qt5OpenGL Qt5Gui Qt5Test Qt5Core )
ENDIF(QT_LINUX)

IF(NOT APPLE)

IF(WIN32)
INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION ".")
ENDIF(WIN32)

IF(UNIX AND NOT APPLE AND NOT QT_ANDROID)
INSTALL(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION ${PREFIX_BIN} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ENDIF(UNIX AND NOT APPLE AND NOT QT_ANDROID)


IF(WIN32)
SET(PREFIX_PKGDATA ".")
ENDIF(WIN32)

IF(NOT BUNDLE_GSHHS MATCHES "NONE")
  INSTALL(FILES ${gshhs} DESTINATION ${PREFIX_PKGDATA}/gshhs PERMISSIONS OWNER_READ  OWNER_WRITE GROUP_READ WORLD_READ)
ENDIF(NOT BUNDLE_GSHHS MATCHES "NONE")

INSTALL(FILES ${uiData} DESTINATION ${PREFIX_PKGDATA}/uidata PERMISSIONS OWNER_READ  OWNER_WRITE GROUP_READ WORLD_READ)

IF (USE_S57)
	INSTALL(FILES ${s57data} DESTINATION ${PREFIX_PKGDATA}/s57data PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
ENDIF (USE_S57)

IF(BUNDLE_TCDATA MATCHES "ON")
  INSTALL(FILES ${tcData} DESTINATION ${PREFIX_PKGDATA}/tcdata PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
ENDIF(BUNDLE_TCDATA MATCHES "ON")

INSTALL(FILES data/license.txt DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
INSTALL(FILES data/copyright DESTINATION ${PREFIX_DATA}/doc/${PACKAGE_NAME} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
INSTALL(FILES data/changelog.Debian.gz DESTINATION ${PREFIX_DATA}/doc/${PACKAGE_NAME} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

INSTALL(DIRECTORY data/sounds/ DESTINATION ${PREFIX_PKGDATA}/sounds )

IF(BUNDLE_DOCS MATCHES "ON")
  INSTALL(DIRECTORY data/doc/ DESTINATION ${PREFIX_PKGDATA}/doc )
ELSE(BUNDLE_DOCS MATCHES "ON")
  INSTALL(FILES data/doc/help_web.html DESTINATION ${PREFIX_PKGDATA}/doc )
ENDIF(BUNDLE_DOCS MATCHES "ON")

IF(UNIX)
  INSTALL(FILES data/opencpn.png DESTINATION ${PREFIX_DATA}/icons/hicolor/48x48/apps PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
  INSTALL(FILES src/bitmaps/other_svg_src/opencpn.svg DESTINATION ${PREFIX_DATA}/icons/hicolor/scalable/apps PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
  INSTALL(FILES data/opencpn.desktop DESTINATION ${PREFIX_DATA}/applications PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ )
ENDIF(UNIX)

ENDIF(NOT APPLE)
#  Fix up time_t definition
# Checks for 32-bit version. And always use 32-bit time_t for compatibility


#uninstall-local:
#        rm -fr $(DESTDIR)$(pkgdatadir)/wvsdata/*
#        rm -fr $(DESTDIR)$(pkgdatadir)/gshhs/*
#        rm -fr $(DESTDIR) $(pkgdatadir)/s57data/*
#        rm -fr $(DESTDIR)$(pkgdatadir)/tcdata/*
#        rmdir $(DESTDIR)$(pkgdatadir)/wvsdata
#        rmdir $(DESTDIR)$(pkgdatadir)/gshhs
#        rmdir $(DESTDIR)$(pkgdatadir)/s57data
#        rmdir $(DESTDIR)$(pkgdatadir)/tcdata
#        rmdir $(DESTDIR)$(pkgdatadir)
#

FIND_PROGRAM(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
IF (GETTEXT_XGETTEXT_EXECUTABLE)

ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot.dummy
   COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} --from-code=iso-8859-1 --force-po --package-name=${PACKAGE_NAME} --package-version="${PACKAGE_VERSION}" --output=${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot  --keyword=_ --width=80 --files-from=${CMAKE_SOURCE_DIR}/po/POTFILES.in
   DEPENDS ${CMAKE_SOURCE_DIR}/po/POTFILES.in ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
   COMMENT "pot-update [${PACKAGE_NAME}]: Generated pot file."
)
ADD_CUSTOM_TARGET(pot-update COMMENT "pot-update: Done." DEPENDS ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot.dummy)

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
         COMMENT "po-update [${_poBasename}]: Updated po file."
      )

      SET(_poFiles ${_poFiles} ${_absFile}.dummy)

   ENDFOREACH (_currentPoFile )

   ADD_CUSTOM_TARGET(po-update COMMENT "po-update: Done." DEPENDS ${_poFiles})
ENDMACRO(GETTEXT_UPDATE_PO)

IF (GETTEXT_MSGMERGE_EXECUTABLE)
FILE(GLOB PACKAGE_PO_FILES ${CMAKE_SOURCE_DIR}/po/*.po)
GETTEXT_UPDATE_PO(${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot ${PACKAGE_PO_FILES})
ENDIF(GETTEXT_MSGMERGE_EXECUTABLE)

SET(_gmoFiles)
MACRO(GETTEXT_BUILD_MO _poFile _lang)

   GET_FILENAME_COMPONENT(_absFile ${_poFile} ABSOLUTE)
   GET_FILENAME_COMPONENT(_poBasename ${_absFile} NAME_WE)
   SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_poBasename}.mo)

   ADD_CUSTOM_COMMAND(
      OUTPUT ${_gmoFile}
      COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_absFile}
      DEPENDS ${_absFile}
      COMMENT "i18n [${_poBasename}]: Created mo file."
   )
IF(APPLE)
   INSTALL(FILES ${_gmoFile} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/Resources/${_lang}.lproj RENAME ${PACKAGE_NAME}.mo )
ELSE(APPLE)
   INSTALL(FILES ${_gmoFile} DESTINATION ${PREFIX_DATA}/locale/${_lang}/LC_MESSAGES RENAME ${PACKAGE_NAME}.mo PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
ENDIF(APPLE)

   SET(_gmoFiles ${_gmoFiles} ${_gmoFile})
ENDMACRO(GETTEXT_BUILD_MO)

if(GETTEXT_MSGFMT_EXECUTABLE)
GETTEXT_BUILD_MO(po/opencpn_cs_CZ.po cs)
GETTEXT_BUILD_MO(po/opencpn_da_DK.po da)
GETTEXT_BUILD_MO(po/opencpn_de_DE.po de)
GETTEXT_BUILD_MO(po/opencpn_es_ES.po es)
GETTEXT_BUILD_MO(po/opencpn_fr_FR.po fr)
GETTEXT_BUILD_MO(po/opencpn_it_IT.po it)
GETTEXT_BUILD_MO(po/opencpn_nl_NL.po nl)
GETTEXT_BUILD_MO(po/opencpn_pl_PL.po pl)
GETTEXT_BUILD_MO(po/opencpn_ru_RU.po ru)
GETTEXT_BUILD_MO(po/opencpn_sv_SE.po sv)
GETTEXT_BUILD_MO(po/opencpn_et_EE.po et)
GETTEXT_BUILD_MO(po/opencpn_pt_PT.po pt_PT)
GETTEXT_BUILD_MO(po/opencpn_pt_BR.po pt_BR)
GETTEXT_BUILD_MO(po/opencpn_nb_NO.po nb_NO)
GETTEXT_BUILD_MO(po/opencpn_tr_TR.po tr_TR)
GETTEXT_BUILD_MO(po/opencpn_fi_FI.po fi_FI)
GETTEXT_BUILD_MO(po/opencpn_el_GR.po el_GR)
GETTEXT_BUILD_MO(po/opencpn_zh_TW.po zh_TW)
GETTEXT_BUILD_MO(po/opencpn_hu_HU.po hu_HU)
GETTEXT_BUILD_MO(po/opencpn_gl_ES.po gl_ES)
GETTEXT_BUILD_MO(po/opencpn_ca_ES.po ca_ES)
ADD_CUSTOM_TARGET(i18n COMMENT "i18n: Done." DEPENDS ${_gmoFiles})

IF(NOT QT_ANDROID)
ADD_DEPENDENCIES(${PACKAGE_NAME} i18n)
ENDIF(NOT QT_ANDROID)

ENDIF(GETTEXT_MSGFMT_EXECUTABLE)


#       Install wxstd.mo in Windows and Mac
IF(WIN32 OR APPLE)

    FILE(GLOB WXWIDGETS_MO_FILES ${CMAKE_SOURCE_DIR}/wxWidgets/locale/*.mo)

    FOREACH ( _current_wx_mofile ${WXWIDGETS_MO_FILES})
      GET_FILENAME_COMPONENT(_absFile ${_current_wx_mofile} ABSOLUTE)
      GET_FILENAME_COMPONENT(_mlang ${_absFile} NAME_WE)

      IF(APPLE)
        INSTALL(FILES ${_current_wx_mofile} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/Resources/${_mlang}.lproj RENAME wxstd.mo )
      ELSE(APPLE)
        INSTALL(FILES ${_current_wx_mofile} DESTINATION ${PREFIX_DATA}/locale/${_mlang}/LC_MESSAGES RENAME wxstd.mo PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
      ENDIF(APPLE)

    ENDFOREACH (_current_wx_mofile )

ENDIF(WIN32 OR APPLE)


# On Windows builds, copy the wxWidgets dlls and helpers to the executable directory
IF(WIN32 AND NOT UNIX)
FILE(GLOB wxdll_files "${CMAKE_CURRENT_SOURCE_DIR}/buildwin/wxWidgets/*.dll")
INSTALL(FILES ${wxdll_files} DESTINATION ".")
FILE(GLOB vcdll_files "${CMAKE_CURRENT_SOURCE_DIR}/buildwin/vc/*.dll")
INSTALL(FILES ${vcdll_files} DESTINATION ".")

INSTALL(FILES ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/CrashRpt1402.dll DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/CrashSender1402.exe DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/crashrpt_lang.ini DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/dbghelp.dll DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/PrivacyPolicy.txt DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/buildwin/ocpn_gltest1.exe DESTINATION ${PREFIX_PKGDATA} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

ENDIF(WIN32 AND NOT UNIX)

MESSAGE (STATUS "")

IF(NOT SKIP_PLUGINS)
 add_subdirectory (plugins)
ENDIF(NOT SKIP_PLUGINS)

# build a CPack driven installer package
#include (InstallRequiredSystemLibraries)
SET(CPACK_PACKAGE_NAME "OpenCPN")
SET(CPACK_PACKAGE_VENDOR "opencpn.org")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenCPN ${PACKAGE_VERSION}")
SET(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_CURRENT_BINARY_DIR};${PACKAGE_NAME};ALL;/")
SET(CPACK_PACKAGE_EXECUTABLES ${PACKAGE_NAME} "OpenCPN")



IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.

  SET(CPACK_NSIS_INSTALLED_ICON_NAME "opencpn.exe")
  SET(CPACK_NSIS_PACKAGE_NAME_LC "opencpn")

#  These lines set the name of the Windows Start Menu shortcut and the icon that goes with it
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME} ${PACKAGE_VERSION}")
  SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/src/bitmaps\\\\opencpn.ico")

  SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}_setup" )

  SET(CPACK_NSIS_DIR "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode")  #Gunther
  SET(CPACK_BUILDWIN_DIR "${CMAKE_SOURCE_DIR}/buildwin")  #Gunther

ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY ${PACKAGE_NAME})
  SET(CPACK_STRIP_FILES "bin/opencpn.exe")
  SET(CPACK_SOURCE_STRIP_FILES "")
  SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${PACKAGE_VERSION}-${PACKAGE_RELEASE}_${ARCH}" )

ENDIF(WIN32 AND NOT UNIX)

#SET(CPACK_PACKAGE_EXECUTABLES ${PACKAGE_NAME})
#SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${PACKAGE_NAME} ${PACKAGE_VERSION}")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/data/license.txt")
#SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/data/README")
#SET(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/data/Welcome.txt")
IF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README")
#    MESSAGE(STATUS "Using generic cpack package description file.")
    SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
    SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")
ENDIF ()
SET(CPACK_SOURCE_GENERATOR "TGZ")

# The following components are regex's to match anywhere (unless anchored)
# in absolute path + filename to find files or directories to be excluded
# from source tarball.
set(CPACK_SOURCE_IGNORE_FILES
"\\\\.cvsignore$"
"^${CMAKE_CURRENT_SOURCE_DIR}.*/CVS/"
"^${CMAKE_CURRENT_SOURCE_DIR}/build*"
"^${CPACK_PACKAGE_INSTALL_DIRECTORY}/*"
)

IF(APPLE)
#  SET(CPACK_GENERATOR "Bundle")
#  SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
#  SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/buildosx/opencpn.icns")
#  SET(CPACK_BUNDLE_NAME "${PACKAGE_NAME}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
#  SET(CPACK_BUNDLE_ICON "${CMAKE_SOURCE_DIR}/buildosx/opencpn.icns")
#  SET(CPACK_BUNDLE_PLIST "${CMAKE_SOURCE_DIR}/buildosx/Info.plist")

#  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/buildosx/README.txt")
#  SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/buildosx/README.txt")

#  SET(CPACK_PACKAGING_INSTALL_PREFIX "/")

ENDIF(APPLE)


IF(UNIX AND NOT APPLE)
#    INCLUDE(UseRPMTools)
#    IF(RPMTools_FOUND)
#        RPMTools_ADD_RPM_TARGETS(packagename ${CMAKE_SOURCE_DIR}/package.spec)
#    ENDIF(RPMTools_FOUND)

    SET(CPACK_STRIP_FILES ON)
    SET(CPACK_GENERATOR ${PACKAGE_FORMAT})
    SET(CPACK_PACKAGE_CONTACT "David S. Register ")
    SET(CPACK_DEBIAN_PACKAGE_DEPENDS ${PACKAGE_DEPS})
    SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS ${PACKAGE_RECS})
    SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCH})
    SET(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")
    SET(CPACK_DEBIAN_PACKAGE_SECTION "misc")
    SET(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
    SET(CPACK_RPM_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")
    SET(CPACK_RPM_PACKAGE_ARCHITECTURE  ${ARCH})
    SET(CPACK_RPM_PACKAGE_REQUIRES  ${PACKAGE_DEPS})
    SET(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_SOURCE_DIR}/opencpn.spec.in")
    SET(CPACK_OPENCPN_RPM_BINDIR "${CMAKE_INSTALL_PREFIX}/${PREFIX_BIN}")
    SET(CPACK_OPENCPN_RPM_LIBDIR "${PREFIX_LIB}")
    SET(CPACK_OPENCPN_RPM_DATADIR "${CMAKE_INSTALL_PREFIX}/${PREFIX_DATA}")
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenSource Chart Plotter/Navigator")
    SET(CPACK_PACKAGE_DESCRIPTION "OpenCPN is a concise ChartPlotter/Navigator. The application supports: GPS/GPDS Position Input, BSB Raster Chart Display, S57 Vector ENChart Display, AIS Input Decoding, Waypoint Autopilot Navigation .")
    SET(CPACK_SET_DESTDIR ON)
    SET(CPACK_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
ENDIF(UNIX AND NOT APPLE)

IF(WIN32 AND NOT UNIX)
IF(NOT SKIP_VERSION_CONFIG)
 configure_file(${CMAKE_SOURCE_DIR}/src/opencpn.rc.in ${CMAKE_SOURCE_DIR}/src/opencpn.rc)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_GERMAN.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_GERMAN.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_FRENCH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_FRENCH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_CZECH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_CZECH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_DANISH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_DANISH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_SPANISH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_SPANISH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_ITALIAN.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_ITALIAN.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_DUTCH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_DUTCH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_POLISH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_POLISH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_PORTUGUESEBR.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_PORTUGUESEBR.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_PORTUGUESE.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_PORTUGUESE.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_RUSSIAN.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_RUSSIAN.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_SWEDISH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_SWEDISH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_FINNISH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_FINNISH.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_NORWEGIAN.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_NORWEGIAN.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_CHINESETW.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_CHINESETW.nsh" @ONLY)
 configure_file("${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_TURKISH.nsh.in" "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_TURKISH.nsh" @ONLY)
ENDIF(NOT SKIP_VERSION_CONFIG)
ENDIF(WIN32 AND NOT UNIX)

IF(NOT SKIP_VERSION_CONFIG)
   configure_file(version.h.in ${CMAKE_SOURCE_DIR}/include/version.h)
ENDIF(NOT SKIP_VERSION_CONFIG)

# this dummy target is necessary to make sure the ADDITIONAL_MAKE_CLEAN_FILES directive is executed.
# apparently, the base CMakeLists.txt file must have "some" target to activate all the clean steps.
ADD_CUSTOM_TARGET(dummy COMMENT "dummy: Done." DEPENDS ${PACKAGE_NAME})

SET_DIRECTORY_PROPERTIES(dummy ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_SOURCE_DIR}/include/version.h)

INCLUDE(CPack)

IF(APPLE)
# -- Run the BundleUtilities cmake code
  SET(CPACK_BUNDLE_PLIST "${CMAKE_SOURCE_DIR}/buildosx/Info.plist.in")

  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app")
  SET(DIRS "")

  SET(LIBS "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/PlugIns/libdashboard_pi.dylib")
  SET(LIBS
      ${LIBS} "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/PlugIns/libgrib_pi.dylib")

  INSTALL(CODE "
   include(BundleUtilities)
   fixup_bundle(\"${APPS}\"   \"${LIBS}\"   \"${DIRS}\")
   " COMPONENT Runtime)

 ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.dmg
   COMMAND ${CMAKE_SOURCE_DIR}/buildosx/create-dmg ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.dmg ${CMAKE_INSTALL_PREFIX}/bin/
   DEPENDS ${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
   COMMENT "create-dmg [${PACKAGE_NAME}]: Generated dmg file."
)

 ADD_CUSTOM_TARGET(create-dmg COMMENT "create-dmg: Done." DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.dmg)


ENDIF(APPLE)

MESSAGE(STATUS "Writing spec file...")


