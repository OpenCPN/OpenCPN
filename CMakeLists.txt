# ---------------------------------------------------------------------------
# Author:      Dave Register DEB 
# config from: antonm - Anton Martchukov <anton@martchukov.com> 
# Update:      sethdart (Jean-Eudes Onfray) 
#                 with parts from balp (Anders Arnholm)
# ***************************************************************************
# - Copyright (C) 2010 by David S. Register                               *
# - This program is free software; you can redistribute it and/or modify  *
# - it under the terms of the GNU General Public License as published by  *
# - the Free Software Foundation; either version 2 of the License, or     *
# - (at your option) any later version.                                   *
# - *
# - This program is distributed in the hope that it will be useful,       *
# - but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# - MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# - GNU General Public License for more details.                          *
# - *
# - You should have received a copy of the GNU General Public License     *
# - along with this program; if not, write to the                         *
# - Free Software Foundation, Inc.,                                       *
# - 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
# ***************************************************************************
#[[

Configuring opencpn
===================

Help:
    Opencpn configuration options can be listed using
        cmake -LH  | grep -B1 OCPN_

Renamed options:
    From version 5.0.0 all OpenCPN configuration options have a OCPN_ prefix
    Many older options are renamed, some examples:

        ENABLE_SYSTEM_CMD_SOUND -> OCPN_ENABLE_SYSTEM_CMD_SOUND
        USE_BUNDLED_LIBS -> OCPN_USE_BUNDLED_LIBS
        BUNDLE_DOCS ->  OCPN_BUNDLE_DOCS
        BUNDLE_TCDATA -> OCPN_BUNDLE_TCDATA
        BUNDLE_GSHHS ->OCPN_BUNDLE_GSHHS
        ENABLE_PORTAUDIO -> OCPN_ENABLE_PORTAUDIO
        USE_GARMINHOST -> OCPN_USE_GARMINHOST
        ENABLE_CPPCHECK -> OCPN_ENABLE_CPPCHECK


Installation paths:
  - Since 5.0.0 OpenCPN supports the GNU install paths, see
    https://cmake.org/cmake/help/v3.0/module/GNUInstallDirs.html
  - Use CMAKE_INSTALL_LIBDIR to change the base directory for plugins;
    defaults to library location on most platforms, but is 'lib' on
    Debian.

Sound:
  - Without options, defaults to MswSound on Windows, PortAudioSound
    on Linux and WxSound on OSX.
  - With OCPN_ENABLE_SYSTEM_CMD_SOUND enabled SystemCmdSound will be the
    backend used on all platforms.
  - With OCPN_ENABLE_PORTAUDIO activated PortAudioSound can be used on all
    platforms if the necessary headers and libraries are available.
  - OCPN_ENABLE_SNDFILE adds libsndfile support to portaudio. libsndfile
    supports a wide range of audio formats instead of .wav only.

Bundled libraries:
  - OCPN_USE_BUNDLED_LIBS forces use of external/bundled libraries including
    wxsvg and liblz4. It's either:
      - ON => use all bundled libraries.
      - OFF => Always use external (system) libraries if available.
      - A comma-separated list of libraries where the bundled variant
        should be used. For other libraries, system libs is preferred.
  - If found, the system tinyxml library is used unconditionally.

Documentation:
  - OCPN_BUNDLE_DOCS governs inclusion of docs in the package.
  - Note that documentation is available in a separate download from
    opencpn website.


Tidal/current and GSHHS wallpaper
  - OCPN_BUNDLE_TCDATA (boolean) includes tide/current harmonics data in
    package.
  - BUNDLE_GSHHS (NONE,MIN,[CRUDE],LOW,INTERMEDIATE,HIGH,FULL) governs the
    wallpaper map used as last resort and world overview.

To build for android, use something like:
    $cmake -DUSE_GARMINHOST=OFF
        -D_wx_selected_config=androideabi-qt
        -DCMAKE_TOOLCHAIN_FILE=../buildandroid/build_android.cmake
        -DwxQt_Build=build_android_53
       	-DwxQt_Base=/home/dsr/Projects/wxqt/wxWidgets
        -DQt_Base /home/dsr/Qt/5.3)
        ..

Silent/Verbose builds:
    cmake's handling of these issues is underwhelming. On most platforms the
    builds are by default verbose. Use 'cmake OCPN_VERBOSE=OFF' to make 
    a silent build. The OCPN_VERBOSE value can be overridden using
    'make VERBOSE=1' or 'make -s'
#]]
#[[
TODO:
- Profiling opt
- test with Win & OSX
USE_GLU_TESS
USE_GLU_DLL
I also find it deficient in some areas. For instance, I cannot make it
output a VS project with certain compile switches set as desired,
namely /MT vs /MD. This means I must manually set this and other compiler
options after the CMake step. Sometimes I forget. Grrr...
set /MT for Release build, and /MTd for debug.
Is this still problem? If so, generator expressions seem to be the solution.
#]]

cmake_minimum_required(VERSION 3.1.1)
cmake_policy(SET CMP0043 NEW)
cmake_policy(SET CMP0025 NEW)
if (POLICY CMP0057)
  cmake_policy(SET CMP0057 NEW)
endif ()
if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif ()

# Prefer libGL.so to libOpenGL.so, see CMP0072
set(OpenGL_GL_PREFERENCE "LEGACY")

project(OpenCPN)

if (NOT DEFINED OCPN_VERBOSE OR OCPN_VERBOSE)
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif ()

message(STATUS "cmake version: ${CMAKE_VERSION}")

if (APPLE)
  set(PACKAGE_NAME OpenCPN)
else (APPLE)
  set(PACKAGE_NAME opencpn)
endif (APPLE)

# Locations where cmake looks for cmake modules.
set(
  CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/build
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_SOURCE_DIR}/cmake
)

IF(DEFINED _wx_selected_config)
  MESSAGE (STATUS "selected config ${_wx_selected_config}")
  IF(_wx_selected_config MATCHES "androideabi-qt")
    MESSAGE (STATUS "Building for wxQt-Android")
    SET(QT_ANDROID "ON")
  ENDIF(_wx_selected_config MATCHES "androideabi-qt")  
ENDIF(DEFINED _wx_selected_config)

include(FindPkgConfig)
include(CMakeDependentOption)

# Per default, use CMAKE_INSTALL_LIBDIR=lib on Debian (and derivatives)
# to avoid multilib plugin paths. After all, we do not install any
# "real" libraries.
if (EXISTS /etc/debian_version AND NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR "lib")
endif ()
include(GNUInstallDirs)

#
# Options
#

macro (BUNDLE_DOCS_OPT onoff)
  option(OCPN_BUNDLE_DOCS "Include documentation in package" ${onoff})
endmacro (BUNDLE_DOCS_OPT)

macro (BUNDLE_TCDATA_OPT onoff)
  option(
    OCPN_BUNDLE_TCDATA "Include tide/current harmonics data in package"
    ${onoff}
  )
endmacro (BUNDLE_TCDATA_OPT)

macro (BUNDLE_GSHHS_OPT value)
  set(
    OCPN_BUNDLE_GSHHS "${value}"
    CACHE
      STRING
      "Include GSHHS wallpaper map in package (NONE,MIN,[CRUDE],LOW,INTERMEDIATE,HIGH,FULL)"
  )
endmacro (BUNDLE_GSHHS_OPT)

# If on Windows or Apple, build the monolithic package, to override use a non-
# sense values from the commandline (due to cmake inability to distinguish
# between not set and set to FALSE): cmake -DOCPN_BUNDLE_DOCS=BLAH
# -DOCPN_BUNDLE_TCDATA=BLAH -DOCPN_BUNDLE_GSHHS=BLAH ..
if (APPLE OR WIN32)
  bundle_docs_opt("ON")
  bundle_tcdata_opt("ON")
  bundle_gshhs_opt("CRUDE")
else (APPLE OR WIN32)
  bundle_docs_opt("OFF")
  bundle_tcdata_opt("OFF")
  bundle_gshhs_opt("NONE")
endif (APPLE OR WIN32)

option(OCPN_VERBOSE "Make verbose builds"  ON)
option(OCPN_PEDANTIC "Enable more compiler warnings" OFF)

set(OCPN_PACKAGE_RELEASE "1" CACHE STRING "Package release number")

option(OCPN_USE_GL "Enable OpenGL support" ON)

if (MSVC OR (NOT APPLE AND NOT QT_ANDROID AND NOT MINGW))
  option(OCPN_USE_CRASHREPORT "Enable crash reporting" ON)
else ()
  set(OCPN_USE_CRASHREPORT FALSE)
endif ()

if (NOT WIN32 AND NOT APPLE)
  set(PA_DEFAULT "ON")
endif ()
option(
  OCPN_ENABLE_PORTAUDIO "Use portaudio(3) to play sounds if available"
  ${PA_DEFAULT}
)
cmake_dependent_option(
  OCPN_ENABLE_SNDFILE
  "Use libsndfile for portaudio if available."
  ON
  "OCPN_ENABLE_PORTAUDIO"
  ON
)

option(OCPN_USE_GARMINHOST "Enable Garmin Host Mode support" ON)

set(
  OCPN_WXWIDGETS_FORCE_VERSION
  CACHE STRING "Force usage of a specific wxWidgets version."
)

set(
  OCPN_WXWIDGETS_OPTIONS
  CACHE STRING "Additional flags to wxWidgets_CONFIG_OPTIONS"
)

if (WIN32 AND NOT UNIX)
  option(OCPN_BUNDLE_WXDLLS "Bundle the prebuilt WX DLLs" ON)
  option(OCPN_BUNDLE_VCDLLS "Bundle the VC redistributable libraries" ON)
endif ()

if (CMAKE_VERSION VERSION_GREATER 3.4)
  option(OCPN_ENABLE_CLANG_TIDY "Add clang-tidy automatically to builds" OFF)
  set(ENABLE_SANITIZER "none" CACHE STRING "Add clang sanitizer to the build")
endif ()

if (NOT WIN32 AND NOT APPLE AND NOT QT_ANDROID)
    if (EXISTS "/usr/lib64")
        set(_LIBDIRS "~/.local/lib:/usr/local/lib64:/usr/lib64")
    elseif (EXISTS "/usr/lib/arm-linux-gnueabihf") 
        set(_LIBDIRS "~/.local/lib:/usr/local/lib:/usr/lib/arm-linux-gnueabihf:/usr/lib")
    else ()
        set(_LIBDIRS "~/.local/lib:/usr/local/lib:/usr/lib")	
    endif ()
    set(OCPN_LINUX_LOAD_PATH "${_LIBDIRS}"
        CACHE STRING "Plugin libraries directories (${_LIBDIRS})"
    )
endif ()

if (CMAKE_VERSION VERSION_GREATER 3.9)
  option(OCPN_ENABLE_CPPCHECK "Add cppcheck automatically to builds" OFF)
endif ()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(
    CMAKE_BUILD_TYPE RelWithDebInfo
    CACHE
      STRING
      "Choose type of build: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE
  )
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)

if (APPLE)
  option(OCPN_USE_LIBCPP "Use libc++ instead of libstdc++ on macOS" ON)
endif ()

option(OCPN_USE_EXTERN_CURL "Use external libcurl" OFF)

if (NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${TENTATIVE_PREFIX})
endif (NOT CMAKE_INSTALL_PREFIX)

if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(BUNDLED_LIBS_DEFAULT "ON")
else ()
  set(BUNDLED_LIBS_DEFAULT "OFF")
endif ()
set(
  OCPN_USE_BUNDLED_LIBS ${BUNDLED_LIBS_DEFAULT}
  CACHE
    STRING
    "Use bundled libraries instead of system's [ON, OFF or comma-sep list]"
)

option(OCPN_USE_NEWSERIAL "Use new serial communication implementation" ON)
option(OCPN_USE_CURL "Use Curl libraries" ON)
option(OCPN_USE_SVG "Use SVG graphics" ON)
option(OCPN_USE_WEBVIEW "Use wxWidget's webview addon if available" ON)
option(OCPN_USE_LZMA "Use LZMA for chart compression" ON)
option(OCPN_CI_BUILD "Use CI build versioning rules" OFF)

option(
  OCPN_ENABLE_SYSTEM_CMD_SOUND
  "Use aplay(1), afplay(1) etc. to play sounds if available" ON
)

IF (QT_ANDROID)
  SET(OCPN_NEW_SERIAL OFF)
  SET(OCPN_USE_NEWSERIAL OFF)
  SET(OCPN_USE_CURL OFF)
  SET(ENABLE_PORTAUDIO OFF)
  SET(ENABLE_SYSTEM_CMD_SOUND OFF)
  SET(ENABLE_SNDFILE OFF) 
  SET(OCPN_USE_LZMA OFF) 
ENDIF (QT_ANDROID)

# Check if a given library should use the bundled source
macro (USE_BUNDLED_LIB _result library)
  if ("${OCPN_USE_BUNDLED_LIBS}" STREQUAL "")
    set(OCPN_USE_BUNDLED_LIBS "ON")
  endif ()
  if (${OCPN_USE_BUNDLED_LIBS} STREQUAL "OFF")
    set(${_result} "OFF")
  elseif (${OCPN_USE_BUNDLED_LIBS} STREQUAL "ON")
    set(${_result} "ON")
  elseif (",${OCPN_USE_BUNDLED_LIBS}," MATCHES ",${library},")
    set(${_result} "ON")
  else ()
    set(${_result} "OFF")
  endif ()
endmacro (USE_BUNDLED_LIB)

#
# Language, compiler and static checkers setup
#

include(GetArch)
getarch()
message(STATUS "*** Build Architecture is ${ARCH}")

set(CMAKE_CXX_STANDARD 11)
message(STATUS "Setting C++11 standard via cmake standard mechanism")
if (NOT MSVC)
  set(OBJ_VISIBILITY "-fvisibility=hidden")
endif ()

if (CMAKE_VERSION VERSION_GREATER 3.4)
  if (OCPN_ENABLE_CLANG_TIDY)
    find_program(
      CLANG_TIDY_EXE
      NAMES "clang-tidy"
      PATHS /usr/local/opt/llvm/bin
    )
    if (CLANG_TIDY_EXE)
      message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
      # For more, see http://clang.llvm.org/extra/clang-tidy/
      # set(CLANG_TIDY_CHECKS "-*,modernize-*")
      set(CLANG_TIDY_CHECKS "-*,performance-*")
      set(
        CMAKE_CXX_CLANG_TIDY
        "${CLANG_TIDY_EXE};-checks=${CLANG_TIDY_CHECKS};-header-filter='${CMAKE_SOURCE_DIR}/*'"
        CACHE STRING ""
        FORCE
      )
    else ()
      message(AUTHOR_WARNING "clang-tidy not found!")
      set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE) # delete it
    endif ()
  endif ()
endif ()

if (CMAKE_VERSION VERSION_GREATER 3.4)
  # Add support for address etc sanitizers, part 1/2 (other half after
  # ADD_EXECUTABLE)
  set_property(
    CACHE ENABLE_SANITIZER
    PROPERTY
      STRINGS
      none
      address
      memory
      thread
      undefined
  )
  if (NOT "${ENABLE_SANITIZER}" MATCHES "none")
    add_compile_options(-fsanitize=${ENABLE_SANITIZER})
  endif ()
endif ()

if (CMAKE_VERSION VERSION_GREATER 3.9)
  if (OCPN_ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXECUTABLE NAMES "cppcheck")
    set(CMAKE_CXX_CPPCHECK ${CPPCHECK_EXECUTABLE})
  endif ()
endif ()

message(STATUS "Default compiler options:")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_MINSIZEREL: ${CMAKE_CXX_FLAGS_MINSIZEREL}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(
  STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
)
# ADD_COMPILE_OPTIONS( "-Wall" "-ansi" "-pedantic" "-Wno-variadic-macros" )
# TODO: Should we use  -fno-stack-protector IF NOT DEBUGGING CFLAGS="-O2
# -march=native"
if (NOT WIN32 AND NOT APPLE)
  add_compile_options("-Wall")
  if (NOT OCPN_PEDANTIC)
    add_compile_options(
      "-Wno-unused"
      "-fexceptions"
      "-rdynamic"
      "-fno-strict-aliasing"
      "-Wno-deprecated-declarations"
    )
  endif ()
  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_options("-O0")
  endif ()

  add_definitions(" -DPREFIX=\\\"${CMAKE_INSTALL_PREFIX}\\\"")
  # profiling with gprof ADD_COMPILE_OPTIONS( -pg ) SET(CMAKE_EXE_LINKER_FLAGS
  # -pg) profiling with gcov ADD_COMPILE_OPTIONS( "-fprofile-arcs -ftest-
  # coverage" ) SET(EXTRA_LIBS ${EXTRA_LIBS} "gcov")
endif (NOT WIN32 AND NOT APPLE)

if (MINGW)
  add_compile_options("-Wall")
  if (NOT OCPN_PEDANTIC)
    add_compile_options(
      "-Wno-unused"
      "-Wno-cpp"
      "-fexceptions"
      "-fno-strict-aliasing"
    )
  endif ()
  add_definitions("-DPSAPI_VERSION=1")
  add_definitions("-DUNICODE" "-D_UNICODE")
endif (MINGW)

if (APPLE)
  add_compile_options("-Wall")
  if (NOT OCPN_PEDANTIC)
    add_compile_options(
      "-Wno-unused"
      "-fexceptions"
      "-Wno-overloaded-virtual"
      "-fno-strict-aliasing"
      "-Wno-deprecated"
      "-Wno-deprecated-declarations"
      "-Wno-unknown-pragmas"
      "-D_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_"
    )
  endif ()
endif (APPLE)

if (APPLE)
  set(CMAKE_C_FLAGS "-O2 -arch ${ARCH}")
  set(CMAKE_C_FLAGS_DEBUG "-g -O0 -arch ${ARCH}")
  set(CMAKE_C_FLAGS_MINSIZEREL "-O2 -arch ${ARCH}")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -arch ${ARCH}")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -arch ${ARCH}")

  if (OCPN_USE_LIBCPP)
    set(OCPN_LIBCPP "-stdlib=libc++")
  endif (OCPN_USE_LIBCPP)
  set(CMAKE_CXX_FLAGS "-O2 ${OCPN_LIBCPP} -arch ${ARCH}")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 ${OCPN_LIBCPP} -arch ${ARCH}")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "-O2 ${OCPN_LIBCPP} -arch ${ARCH}")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 ${OCPN_LIBCPP} -arch ${ARCH}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3 ${OCPN_LIBCPP} -arch ${ARCH}")

  set(
    CMAKE_EXE_LINKER_FLAGS
    "-O2 ${OCPN_LIBCPP} -arch ${ARCH} ${CMAKE_EXE_LINKER_FLAGS}"
  )
  set(CMAKE_SHARED_LINKER_FLAGS "-O2 ${OCPN_LIBCPP} -arch ${ARCH}")
  set(CMAKE_MODULE_LINKER_FLAGS "-O2 ${OCPN_LIBCPP} -arch ${ARCH}")
endif (APPLE)

if (MSVC)
  add_definitions(-D__MSVC__)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
  add_definitions(-DPSAPI_VERSION=1)
endif (MSVC)

if (MSVC)
  set(CMAKE_C_FLAGS_DEBUG "/MP /MDd /Ob0 /Od  /D_DEBUG  /Zi /RTC1")
  set(CMAKE_C_FLAGS_MINSIZEREL "/MP /MD  /O1  /Ob1 /D NDEBUG")
  set(CMAKE_C_FLAGS_RELEASE "/MP /MD  /O2  /Ob2 /D NDEBUG /Zi /wd8051 /wd4068")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "/MP /MD  /O2  /Ob1 /D NDEBUG /Zi")
  set(CMAKE_CXX_FLAGS_DEBUG "/MP /MDd /Ob0 /Od  /D_DEBUG  /Zi /RTC1 /EHa")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "/MP /MD  /O1  /Ob1 /D NDEBUG /EHa")
  set(CMAKE_CXX_FLAGS_RELEASE "/MP /MD  /O2  /Ob2 /D NDEBUG /Zi /EHa")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MP /MD  /O2  /Ob1 /D NDEBUG /Zi /EHa")
  set(CMAKE_EXE_LINKER_FLAGS "/DEBUG ${CMAKE_EXE_LINKER_FLAGS}")
endif (MSVC)

if (QT_ANDROID)
  set(CMAKE_BUILD_TYPE Debug)
  add_definitions(-D__WXQT__)
  add_definitions(-D__OCPN__ANDROID__)
  add_definitions(-DANDROID)
  set(CMAKE_CXX_FLAGS "-pthread -fPIC -O2")
  
  add_compile_options("-Wno-inconsistent-missing-override"
        "-Wno-potentially-evaluated-expression"
        "-Wno-overloaded-virtual"
        "-Wno-unused-command-line-argument"
        "-Wno-unknown-pragmas"
        "-O3"
        "-fPIC"
  )
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-soname,libopencpn.so ")
endif (QT_ANDROID)

set(
  LINUX_LIB_PATHS
  /usr/local/lib
  /usr/local/lib64
  /usr/lib/i386-linux-gnu
  /usr/lib/x86_64-linux-gnu
  /usr/lib
  /usr/lib64
)

set(PREFIX_BIN ${CMAKE_INSTALL_BINDIR})
set(PREFIX_INCLUDE ${CMAKE_INSTALL_INCLUDEDIR})
set(PREFIX_DATA ${CMAKE_INSTALL_DATADIR})
set(PREFIX_PKGDATA ${CMAKE_INSTALL_DATADIR}/${PACKAGE_NAME})
set(PREFIX_LIB "${CMAKE_INSTALL_FULL_LIBDIR}")

#
# Version handling
#
include(${CMAKE_SOURCE_DIR}/VERSION.cmake)
include(TargetSetup)

if (OCPN_CI_BUILD)
  include(Utils)
  today(DATE)
  commit_id(COMMIT)
  set(VERSION_TAIL "+${COMMIT}")
  set(VERSION_DATE "${DATE}")
endif (OCPN_CI_BUILD)

set(
  PACKAGE_VERSION
  "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_TAIL}"
)

# Detect the Apple Version of the build machine
if (APPLE)
  exec_program(uname ARGS -v OUTPUT_VARIABLE DARWIN_VERSION)
  string(
    REGEX
      MATCH
      "[0-9]+"
      DARWIN_VERSION
      ${DARWIN_VERSION}
  )
  message(STATUS "*** Building on DARWIN_VERSION=${DARWIN_VERSION}")
  if (DARWIN_VERSION GREATER 11)
    set(APPLE_MODERN 1 INTERNAL)
  endif (DARWIN_VERSION GREATER 11)
endif (APPLE)

message(STATUS "*** Staging to build ${PACKAGE_NAME} ${PACKAGE_VERSION} ***")

#
# Bundled data: docs, tcdata, gshhs
#

if (NOT WIN32 AND NOT APPLE AND NOT QT_ANDROID)
  option(OCPN_FORCE_GTK3 "Force the build to use GTK3" OFF)
endif ()

if (OCPN_BUNDLE_DOCS MATCHES "ON")
  message(STATUS "*** Package will include documentation ***")
else (OCPN_BUNDLE_DOCS MATCHES "ON")
  message(STATUS "*** Package will NOT include documentation ***")
endif (OCPN_BUNDLE_DOCS MATCHES "ON")

if (CMAKE_VERSION VERSION_GREATER 3.11)
  find_package(Python REQUIRED)
else ()
  find_package(Python-314 REQUIRED)
endif ()
message(STATUS "Using python executable: ${Python_EXECUTABLE}")

#
# Linux: set up GTK (wxWidgets prerequisite).
#
if (NOT WIN32 AND NOT APPLE AND NOT QT_ANDROID)
  include(OcpnFindGtk)
endif ()

# set a build flag for arm architecture, to catch any rPI runtime changes
# required
if (ARCH MATCHES "arm*" AND (NOT QT_ANDROID))
  add_definitions(-DocpnARM)
endif (ARCH MATCHES "arm*" AND (NOT QT_ANDROID))

IF(DEFINED _wx_selected_config)
  IF(_wx_selected_config MATCHES "androideabi-qt")
    MESSAGE (STATUS "Qt_Base: " ${Qt_Base})
    MESSAGE (STATUS "wxQt_Base/Build: " ${wxQt_Base} "/" ${wxQt_Build})
    ADD_DEFINITIONS(-DocpnUSE_GLES)
    ADD_DEFINITIONS(-DocpnUSE_GL)
    ADD_DEFINITIONS(-DARMHF)

    SET(OPENGLES_FOUND "YES")
    SET(OPENGL_FOUND "YES")
  
    ADD_DEFINITIONS(-DUSE_GLU_TESS)
    SET(USE_GLES2 ON )
    MESSAGE (STATUS "Using GLESv2 for Android")
    ADD_DEFINITIONS(-DUSE_ANDROID_GLES2)
    ADD_DEFINITIONS(-DUSE_GLSL)
    INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/libs/glshim/include/GLES )

  ENDIF(_wx_selected_config MATCHES "androideabi-qt")
ENDIF(DEFINED _wx_selected_config)

if ((_wx_selected_config MATCHES "qt-armv7"))
  set(wxWidgets_FIND_COMPONENTS base core xml html adv aui)
elseif (OCPN_USE_WEBVIEW)
  set(wxWidgets_FIND_COMPONENTS net xml html adv aui core base webview)
else ()
  set(wxWidgets_FIND_COMPONENTS net xml html adv aui core base)
endif ()

if (OPENGLES_FOUND)
  set(wxWidgets_FIND_COMPONENTS ${wxWidgets_FIND_COMPONENTS} gl)
endif ()

if ((NOT OPENGLES_FOUND) AND (NOT QT_ANDROID))
  if (OCPN_USE_GL)
    find_package(OpenGL)
  else (OCPN_USE_GL)
    message(STATUS "OpenGL disabled by option USE_GL...")
  endif (OCPN_USE_GL)

  if (OPENGL_FOUND)
    set(wxWidgets_FIND_COMPONENTS gl ${wxWidgets_FIND_COMPONENTS})
    include_directories(${OPENGL_INCLUDE_DIR})

    message(STATUS "Found OpenGL....")
    message(STATUS "    GL Lib: " ${OPENGL_LIBRARIES})
    message(STATUS "    GL Include: " ${OPENGL_INCLUDE_DIR})
    add_definitions(-DocpnUSE_GL)

    # We need to remove GLU from the OPENGL_LIBRARIES list
    foreach (_currentLibFile ${OPENGL_LIBRARIES})
      # MESSAGE (STATUS "    Lib File: " ${_currentLibFile})
      set(UCNAME ${_currentLibFile})
      string(TOUPPER ${UCNAME} UCNAME)
      if (NOT ${UCNAME} MATCHES "(.*)GLU(.*)")
        set(
          REVISED_OPENGL_LIBRARIES ${_currentLibFile}
          ${REVISED_OPENGL_LIBRARIES}
        )
      endif ()
    endforeach (_currentLibFile)

    set(OPENGL_LIBRARIES ${REVISED_OPENGL_LIBRARIES})
    message(STATUS "    Revised GL Lib: " ${OPENGL_LIBRARIES})

  else (OPENGL_FOUND)
    message(STATUS "OpenGL not found...")
  endif (OPENGL_FOUND)

endif ()

if (NOT QT_ANDROID)
  # Find wxWidgets here, and the setting get inherited by all plugins. These
  # options can be used to set the linux widgets build type
  set(wxWidgets_USE_DEBUG OFF)
  set(wxWidgets_USE_UNICODE ON)
  set(wxWidgets_USE_UNIVERSAL OFF)
  set(wxWidgets_USE_STATIC OFF)

  if (WXWIDGETS_FORCE_VERSION)
    set(wxWidgets_CONFIG_OPTIONS --version=${WXWIDGETS_FORCE_VERSION})
  endif ()

  if (MSVC)
    # Exclude wxexpat.lib, since we use our own version. Other things are
    # excluded as well, but we don't need them
    set(wxWidgets_EXCLUDE_COMMON_LIBRARIES TRUE)
  endif (MSVC)

#  if (GTK2_FOUND)
  set(wxWidgets_CONFIG_OPTIONS ${wxWidgets_CONFIG_OPTIONS} --toolkit=gtk2)

  if (GTK3_FOUND)
    set(wxWidgets_CONFIG_OPTIONS ${wxWidgets_CONFIG_OPTIONS} --toolkit=gtk3)
  endif ()

  find_package(wxWidgets COMPONENTS ${wxWidgets_FIND_COMPONENTS})
  if (wxWidgets_FOUND AND OCPN_USE_WEBVIEW)
    message(STATUS "Found wxWidgets webview add-on")
    add_definitions(-DwxUSE_WEBVIEW=1 -DHAVE_WEBVIEW)
  else ()
    add_definitions(-DwxUSE_WEBVIEW=0)
    list(REMOVE_ITEM wxWidgets_FIND_COMPONENTS webview)
    message(STATUS "Could not find wxWidgets webview add-on")
    find_package(wxWidgets REQUIRED COMPONENTS ${wxWidgets_FIND_COMPONENTS})
  endif ()
  if (MSYS)
    # Convert msys to windows paths, and handle the missing /usr
    string(
      REGEX
      REPLACE
        "/usr/local"
        ";C:/MinGW/msys/1.0/local"
        wxWidgets_INCLUDE_DIRS
        "${wxWidgets_INCLUDE_DIRS}"
    )
  endif (MSYS)
  include(${wxWidgets_USE_FILE})

  # As of cmake 3.11.2, these libraries are missing in list despite that we
  # looked for them. This is a nasty fix which might fail miserably. Assumption:
  # All builds using GTK uses unicode and wxWidgets 3.0
  if (GTK3_FOUND)
    list(APPEND wxWidgets_LIBRARIES "-lwx_gtk3u_aui-3.0")
    if (OPENGL_FOUND)
      list(APPEND wxWidgets_LIBRARIES "-lwx_gtk3u_gl-3.0")
    endif ()
  endif ()

  message(STATUS "Found wxWidgets...")
  message(STATUS " wxWidgets Include: ${wxWidgets_INCLUDE_DIRS}")
  message(STATUS " wxWidgets Libraries: ${wxWidgets_LIBRARIES}")

  # We need to remove GLU from the wxWidgets_LIBRARIES list It only appears to
  # get on the list for MSW...
  foreach (_currentLibFile ${wxWidgets_LIBRARIES})
    set(UCNAME ${_currentLibFile})
    string(TOUPPER ${UCNAME} UCNAME)
    if (NOT ${UCNAME} MATCHES "(.*)GLU(.*)")
      set(
        REVISED_wxWidgets_LIBRARIES ${REVISED_wxWidgets_LIBRARIES}
        ${_currentLibFile}
      )
    endif ()
  endforeach (_currentLibFile)
  set(wxWidgets_LIBRARIES ${REVISED_wxWidgets_LIBRARIES})

  message(STATUS " Revised wxWidgets Libraries: ${wxWidgets_LIBRARIES}")
endif (NOT QT_ANDROID)

IF(QT_ANDROID)
  SET( wxWidgets_LIBRARIES

        # Link order is critical to avoid circular dependencies
    ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_html-3.1-arm-linux-androideabi.a
    ${wxQt_Base}/${wxQt_Build}/lib/libwx_baseu_xml-3.1-arm-linux-androideabi.a
    ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_qa-3.1-arm-linux-androideabi.a
    ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_adv-3.1-arm-linux-androideabi.a
    ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_core-3.1-arm-linux-androideabi.a
    ${wxQt_Base}/${wxQt_Build}/lib/libwx_baseu-3.1-arm-linux-androideabi.a
    ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_aui-3.1-arm-linux-androideabi.a
    ${wxQt_Base}/${wxQt_Build}/lib/libwxexpat-3.1-arm-linux-androideabi.a
    ${wxQt_Base}/${wxQt_Build}/lib/libwxregexu-3.1-arm-linux-androideabi.a
    ${wxQt_Base}/${wxQt_Build}/lib/libwxjpeg-3.1-arm-linux-androideabi.a
    ${wxQt_Base}/${wxQt_Build}/lib/libwxpng-3.1-arm-linux-androideabi.a
    ${wxQt_Base}/${wxQt_Build}/lib/libwx_qtu_gl-3.1-arm-linux-androideabi.a
    ${wxQt_Base}/${wxQt_Build}/lib/libwx_baseu_net-3.1-arm-linux-androideabi.a
    
    ${Qt_Base}/${Qt_Build}/lib/libQt5Core.so
    ${Qt_Base}/${Qt_Build}/lib/libQt5OpenGL.so
    ${Qt_Base}/${Qt_Build}/lib/libQt5Widgets.so
    ${Qt_Base}/${Qt_Build}/lib/libQt5Gui.so
    ${Qt_Base}/${Qt_Build}/lib/libQt5AndroidExtras.so
 
    )
ENDIF(QT_ANDROID)

message(STATUS "")
message(STATUS "*** Staging to build ${PACKAGE_NAME}  ***")
message(STATUS "*** Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "*** Will install to ${CMAKE_INSTALL_PREFIX}  ***")

IF(QT_ANDROID)
  INCLUDE_DIRECTORIES("${Qt_Base}/${Qt_Build}/include/QtCore")
  INCLUDE_DIRECTORIES("${Qt_Base}/${Qt_Build}/include")
  INCLUDE_DIRECTORIES("${Qt_Base}/${Qt_Build}/include/QtWidgets")
  INCLUDE_DIRECTORIES("${Qt_Base}/${Qt_Build}/include/QtGui")
  INCLUDE_DIRECTORIES("${Qt_Base}/${Qt_Build}/include/QtOpenGL")
  INCLUDE_DIRECTORIES("${Qt_Base}/${Qt_Build}/include/QtTest")

  INCLUDE_DIRECTORIES(
      "${wxQt_Base}/${wxQt_Build}/lib/wx/include/arm-linux-androideabi-qt-unicode-static-3.1")
  INCLUDE_DIRECTORIES("${wxQt_Base}/include")
  include_directories("${CMAKE_SOURCE_DIR}/include")


  add_definitions(-DQT_WIDGETS_LIB)
endif (QT_ANDROID)

set(
  HDRS
  include/about.h
  include/AboutFrame.h
  include/AboutFrameImpl.h
  include/AIS_Bitstring.h
  include/AIS_Decoder.h
  include/ais.h
  include/AISTargetAlertDialog.h
  include/AIS_Target_Data.h
  include/AISTargetListDialog.h
  include/AISTargetQueryDialog.h
  include/bbox.h
  include/canvasMenu.h
  include/catalog_handler.h
  include/catalog_mgr.h
  include/catalog_parser.h
  include/chart1.h
  include/ChartDataInputStream.h
  include/chartdb.h
  include/chartdbs.h
  include/chartimg.h
  include/chcanv.h
  include/ChInfoWin.h
  include/compass.h
  include/concanv.h
  include/ConnectionParams.h
  include/cutil.h
  include/datastream.h
  include/DetailSlider.h
  include/Downloader.h
  include/download_mgr.h
  include/dsPortType.h
  include/emboss_data.h
  include/FlexHash.h
  include/FontDesc.h
  include/FontMgr.h
  include/geodesic.h
  include/georef.h
  include/GoToPositionDialog.h
  include/gshhs.h
  include/Hyperlink.h
  include/IDX_entry.h
  include/iENCToolbar.h
  include/kml.h
  include/Layer.h
  include/LinkPropDlg.h
  include/LLRegion.h
  include/MarkIcon.h
  include/MarkInfo.h
  include/mbtiles.h
  include/multiplexer.h
  include/NavObjectCollection.h
  include/navutil.h
  include/NMEALogWindow.h
  include/ocpCursor.h
  include/OCP_DataStreamInput_Thread.h
  include/OCPN_DataStreamEvent.h
  include/ocpndc.h
  include/OCPNListCtrl.h
  include/ocpn_pixel.h
  include/ocpn_utils.h
  include/OCPNPlatform.h
  include/ocpn_plugin.h
  include/semantic_vers.h
  include/OCPNRegion.h
  include/ocpn_types.h
  include/options.h
  include/piano.h
  include/pluginmanager.h
  include/PluginHandler.h
  include/PluginPaths.h
  include/PositionParser.h
  include/printtable.h
  include/Quilt.h
  include/RolloverWin.h
  include/Route.h
  include/routemanagerdialog.h
  include/routeman.h
  include/RoutePoint.h
  include/routeprintout.h
  include/RoutePropDlg.h
  include/RoutePropDlgImpl.h
  include/S57ClassRegistrar.h
  include/S57Light.h
  include/S57ObjectDesc.h
  include/S57QueryDialog.h
  include/S57Sector.h
  include/Select.h
  include/SelectItem.h
  include/SendToGpsDlg.h
  include/Station_Data.h
  include/styles.h
  include/TCDataFactory.h
  include/TCDataSource.h
  include/TCDS_Ascii_Harmonic.h
  include/TCDS_Binary_Harmonic.h
  include/TC_Error_Code.h
  include/tcmgr.h
  include/TCWin.h
  include/thumbwin.h
  include/tide_time.h
  include/timers.h
  include/toolbar.h
  include/Track.h
  include/trackprintout.h
  include/TrackPropDlg.h
  include/TTYScroll.h
  include/TTYWindow.h
  include/undo.h
  include/vector2D.h
  include/viewport.h
  include/WindowDestroyListener.h
  include/SerialDataStream.h
  include/NetworkDataStream.h
  include/SignalKDataStream.h
  include/SignalKEventHandler.h
  include/GarminProtocolHandler.h
  include/OCPN_SignalKEvent.h
  ${CMAKE_BINARY_DIR}/include/config.h
)

set(
  SRCS
  src/about.cpp
  src/AboutFrame.cpp
  src/AboutFrameImpl.cpp
  src/AIS_Bitstring.cpp
  src/ais.cpp
  src/AIS_Decoder.cpp
  src/AISTargetAlertDialog.cpp
  src/AIS_Target_Data.cpp
  src/AISTargetListDialog.cpp
  src/AISTargetQueryDialog.cpp
  src/bbox.cpp
  src/catalog_handler.cpp
  src/catalog_mgr.cpp
  src/catalog_parser.cpp
  src/CanvasConfig.cpp
  src/canvasMenu.cpp
  src/CanvasOptions.cpp
  src/chart1.cpp
  src/ChartDataInputStream.cpp
  src/chartdb.cpp
  src/chartdbs.cpp
  src/chartimg.cpp
  src/chcanv.cpp
  src/ChInfoWin.cpp
  src/compass.cpp
  src/concanv.cpp
  src/ConfigMgr.cpp
  src/ConnectionParams.cpp
  src/cutil.cpp
  src/datastream.cpp
  src/DetailSlider.cpp
  src/Downloader.cpp
  src/download_mgr.cpp
  src/FlexHash.cpp
  src/FontDesc.cpp
  src/FontMgr.cpp
  src/garmin_wrapper.h
  src/geodesic.cpp
  src/georef.cpp
  src/GoToPositionDialog.cpp
  src/gshhs.cpp
  src/Hyperlink.cpp
  src/IDX_entry.cpp
  src/iENCToolbar.cpp
  src/kml.cpp
  src/Layer.cpp
  src/LinkPropDlg.cpp
  src/LLRegion.cpp
  src/MarkInfo.cpp
  src/mbtiles.cpp
  src/MUIBar.cpp
  src/multiplexer.cpp
  src/NavObjectCollection.cpp
  src/navutil.cpp
  src/NMEALogWindow.cpp
  src/ocpCursor.cpp
  src/OCP_DataStreamInput_Thread.cpp
  src/OCPN_AUIManager.cpp
  src/OCPN_DataStreamEvent.cpp
  src/ocpndc.cpp
  src/OCPNListCtrl.cpp
  src/ocpn_pixel.cpp
  src/semantic_vers.cpp
  src/ocpn_utils.cpp
  src/OCPNPlatform.cpp
  src/OCPNRegion.cpp
  src/options.cpp
  src/piano.cpp
  src/PluginHandler.cpp
  src/PluginPaths.cpp
  src/pluginmanager.cpp
  src/PositionParser.cpp
  src/printtable.cpp
  src/pugixml.cpp
  src/Quilt.cpp
  src/RolloverWin.cpp
  src/Route.cpp
  src/routemanagerdialog.cpp
  src/routeman.cpp
  src/RoutePoint.cpp
  src/routeprintout.cpp
  src/RoutePropDlg.cpp
  src/RoutePropDlgImpl.cpp
  src/S57QueryDialog.cpp
  src/Select.cpp
  src/SelectItem.cpp
  src/SendToGpsDlg.cpp
  src/Station_Data.cpp
  src/styles.cpp
  src/TCDataFactory.cpp
  src/TCDataSource.cpp
  src/TCDS_Ascii_Harmonic.cpp
  src/TCDS_Binary_Harmonic.cpp
  src/tcmgr.cpp
  src/TCWin.cpp
  src/thumbwin.cpp
  src/toolbar.cpp
  src/Track.cpp
  src/trackprintout.cpp
  src/TrackPropDlg.cpp
  src/TTYScroll.cpp
  src/TTYWindow.cpp
  src/undo.cpp
  src/viewport.cpp
  
         src/SerialDataStream.cpp
        src/NetworkDataStream.cpp
        src/SignalKDataStream.cpp
        src/SignalKEventHandler.cpp
        src/GarminProtocolHandler.cpp
        src/OCPN_SignalKEvent.cpp

)

if (OCPN_USE_GARMINHOST)
  message(STATUS "GarminHost is enabled.")
  set(USE_GARMINHOST 1) # -> config.h
  set(SRCS ${SRCS} src/garmin_wrapper.cpp)
endif ()

if (APPLE)
  if (DARWIN_VERSION LESS 16)
    message(
      STATUS "DarkMode not included, requires Mac build host Darwin >= 16"
    )
  else ()
    list(APPEND SRCS src/DarkMode.mm src/DarkMode.h)
    add_definitions(-DOCPN_USE_DARKMODE)
  endif ()
endif (APPLE)

if (NOT WIN32 AND NOT APPLE AND NOT QT_ANDROID AND OCPN_USE_CRASHREPORT)
  list(APPEND HDRS include/crashprint.h)
  list(APPEND SRCS src/crashprint.cpp)
endif (NOT WIN32 AND NOT APPLE AND NOT QT_ANDROID AND OCPN_USE_CRASHREPORT)

IF(QT_ANDROID)
  SET(HDRS ${HDRS} include/androidUTIL.h)
  SET(SRCS ${SRCS} src/androidUTIL.cpp)
ENDIF(QT_ANDROID)

IF(USE_GLES2)
    SET(HDRS ${HDRS} include/shaders.h)
    SET(SRCS ${SRCS} src/shaders.cpp)
ENDIF(USE_GLES2)

SET(SRC_TINYXML
    src/tinyxml.cpp
    src/tinyxmlerror.cpp
    src/tinyxmlparser.cpp
    include/tinyxml.h
)

if (APPLE)
  add_executable(${PACKAGE_NAME} MACOSX_BUNDLE ${HDRS} ${SRCS})
elseif (WIN32)
  add_executable(${PACKAGE_NAME} WIN32 ${HDRS} ${SRCS})
elseif (QT_ANDROID)
  set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
  add_library(${PACKAGE_NAME} SHARED ${HDRS} ${SRCS})
else ()
  add_executable(${PACKAGE_NAME} ${HDRS} ${SRCS})
endif ()

add_library(_opencpn INTERFACE) # plugin link target.
target_link_libraries(_opencpn INTERFACE ${PACKAGE_NAME})
target_include_directories(
  _opencpn
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
add_library(ocpn::opencpn ALIAS _opencpn)

set_target_properties(
  ${PACKAGE_NAME}
  PROPERTIES
    ENABLE_EXPORTS
    1
    OUTPUT_NAME
    ${PACKAGE_NAME}
    ARCHIVE_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(
  ${PACKAGE_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (APPLE)
  target_include_directories(
    ${PACKAGE_NAME}
    PRIVATE /usr/X11/include /usr/X11/include/GL
  )
endif ()

# IF(NOT WIN32) include(OcpnFindGpsd) if (TARGET ocpn::gpsd)
# target_link_libraries(${PACKAGE_NAME} ocpn::gpsd) endif () ENDIF (NOT WIN32)

find_package(Gettext REQUIRED)

add_subdirectory(libs/ssl_sha1)
target_link_libraries(${PACKAGE_NAME} PRIVATE ssl::sha1)

#
# Linux: set up GTK
#
if (NOT WIN32 AND NOT APPLE AND NOT QT_ANDROID)
#  include(OcpnFindGtk)
#  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::gtk)
endif (NOT WIN32 AND NOT APPLE AND NOT QT_ANDROID)

# Search for opengles, short of running a program to test the speed of
# acceleration, I simply use gles on "native linux" arm systems
if (DEFINED _wx_selected_config)
  message(STATUS "selected config ${_wx_selected_config}")
  if (_wx_selected_config MATCHES "androideabi-qt")
    message(STATUS "Building for wxQt-Android")
    message(STATUS "Qt_Base: " ${Qt_Base})
    message(
      STATUS
        "wxQt_Base/Build: "
        ${wxQt_Base}
        "/"
        ${wxQt_Build}
    )
    set(QT_ANDROID "ON")
  endif (_wx_selected_config MATCHES "androideabi-qt")
endif (DEFINED _wx_selected_config)

if ((_wx_selected_config MATCHES "qt-armv7"))
  set(wxWidgets_FIND_COMPONENTS base core xml html adv aui)
else ()
  set(wxWidgets_FIND_COMPONENTS net xml html adv aui core base webview)
endif ()

if (ARCH MATCHES "arm*" AND (NOT QT_ANDROID))
  find_path(OPENGLESv1_INCLUDE_DIR GLES/gl.h)
  if (OPENGLESv1_INCLUDE_DIR)
    message(STATUS "Found OpenGLESv1")
    add_definitions(-DocpnUSE_GL) # ocpnUSE_GLES is in config.h

    set(OPENGLES_FOUND "YES")
    set(OPENGL_FOUND "YES")

    set(OPENGL_LIBRARIES "GL_static" "EGL" "X11" "drm")
  endif ()
endif ()

if (OPENGLES_FOUND)
  list(APPEND wxwidgets_FIND_COMPONENTS gl)
endif ()

if ((NOT OPENGLES_FOUND) AND (NOT QT_ANDROID))
  if (OCPN_USE_GL)
    find_package(OpenGL)
  else (OCPN_USE_GL)
    message(STATUS "OpenGL disabled by option USE_GL...")
  endif (OCPN_USE_GL)

  if (OPENGL_FOUND)
    list(APPEND wxwidgets_FIND_COMPONENTS gl)
    target_include_directories(${PACKAGE_NAME} PRIVATE ${OPENGL_INCLUDE_DIR})

    message(STATUS "Found OpenGL....")
    if (OCPN_VERBOSE)
      message(STATUS "    GL Lib: " ${OPENGL_gl_LIBRARIES})
      message(STATUS "    GL Include: " ${OPENGL_INCLUDE_DIR})
    endif ()

    set(wxWidgets_USE_LIBS gl ${wxWidgets_USE_LIBS})
    add_definitions(-DocpnUSE_GL)
    target_link_libraries(${PACKAGE_NAME} PRIVATE ${OPENGL_gl_LIBRARIES})
    target_include_directories(${PACKAGE_NAME} PRIVATE ${OPENGL_INCLUDE_DIR})
  else (OPENGL_FOUND)
    message(STATUS "OpenGL not found...")
  endif (OPENGL_FOUND)

endif ()

if (OCPN_USE_CRASHREPORT)
  message(STATUS "Crash reporting enabled")
  if (MSVC)
    message(STATUS "Using Windows CrashRpt")
    target_include_directories(
      ${PACKAGE_NAME}
      PRIVATE ${CMAKE_SOURCE_DIR}/buildwin/crashrpt
    )
    target_link_libraries(
      ${PACKAGE_NAME}
      PRIVATE ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/CrashRpt1403.lib
    )
    set(CMAKE_EXE_LINKER_FLAGS "/DEBUG ${CMAKE_EXE_LINKER_FLAGS}")
  else (MSVC)
    if (NOT APPLE AND NOT QT_ANDROID AND NOT MINGW)
      if (CMAKE_BUILD_TYPE MATCHES "Rel*")
        message(STATUS "Using Linux crash reporting")
      endif ()
    endif ()
  endif (MSVC)
endif (OCPN_USE_CRASHREPORT)

add_subdirectory("libs/sqlite")
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::sqlite)

add_subdirectory("libs/SQLiteCpp")
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::sqlite_cpp)

if (UNIX)
  if (NOT QT_ANDROID) # this should be detected by the grib plugin
    find_package(BZip2 REQUIRED)
    target_include_directories(${PACKAGE_NAME} PRIVATE ${BZIP2_INCLUDE_DIR})
    target_link_libraries(${PACKAGE_NAME} PRIVATE ${BZIP2_LIBRARIES})
    find_package(ZLIB REQUIRED)
    target_include_directories(${PACKAGE_NAME} PRIVATE ${ZLIB_INCLUDE_DIR})
    target_link_libraries(${PACKAGE_NAME} PRIVATE ${ZLIB_LIBRARY})
    find_package(X11)
    if (X11_FOUND AND NOT APPLE)
      target_link_libraries(${PACKAGE_NAME} PRIVATE ${X11_LIBRARIES})
      set(OCPN_HAVE_X11 ON)   # => config.h
    endif ()
    if (NOT APPLE)
      # TinyXML from Homebrew is not good enough for our GpxDocument needs We
      # should probably replace it with pugixml completely anyway...
      find_package(TinyXML)
    endif (NOT APPLE)
  endif (NOT QT_ANDROID)
endif (UNIX)

# check for lzma support for reading compressed charts
if (CMAKE_HOST_WIN32)
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/buildwin")
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/buildwin/include")
endif (CMAKE_HOST_WIN32)
if (OCPN_USE_LZMA)
  find_package(LibLZMA)
  if (LIBLZMA_FOUND)
    message(STATUS "Building with lzma support")
    target_include_directories(${PACKAGE_NAME} PUBLIC ${LIBLZMA_INCLUDE_DIRS})
  else (LIBLZMA_FOUND)
    message(STATUS "lzma library not found")
  endif (LIBLZMA_FOUND)
endif (OCPN_USE_LZMA)

if (OCPN_USE_CURL)
  if (CMAKE_HOST_WIN32)
    install(
      FILES
        "${CMAKE_SOURCE_DIR}/buildwin/curl-ca-bundle.crt"
        "${CMAKE_SOURCE_DIR}/buildwin/libeay32.dll"
        "${CMAKE_SOURCE_DIR}/buildwin/ssleay32.dll"
      DESTINATION "."
    )
    if (MSVC)
      set(CURL_LIBRARIES "${CMAKE_SOURCE_DIR}/buildwin/libcurl.lib")
      install(FILES "buildwin/libcurl.dll" DESTINATION ".")
      install(FILES "buildwin/zlib1.dll" DESTINATION ".")
    endif (MSVC)
  else (CMAKE_HOST_WIN32)
    set(OCPN_USE_EXTERN_CURL ON)
  endif (CMAKE_HOST_WIN32)

  if (NOT QT_ANDROID)
    if (OCPN_USE_EXTERN_CURL)
      find_package(CURL REQUIRED)
      target_include_directories(${PACKAGE_NAME} PRIVATE ${CURL_INCLUDE_DIRS})
    else (OCPN_USE_EXTERN_CURL)
      include("Curl")
      message(STATUS "Using bundled curl library...")
    endif (OCPN_USE_EXTERN_CURL)

    pkg_search_module(SYS_WXCURL libwxcurl wxcurl)
    use_bundled_lib(USE_BUNDLED_WXCURL wxcurl)
    if (SYS_WXCURL_FOUND AND NOT USE_BUNDLED_WXCURL)
      message(STATUS "Building with system wxcurl")
      target_include_directories(
        ${PACKAGE_NAME}
        PRIVATE ${CURL_INCLUDE_DIRS} ${SYS_WXCURL_INCLUDE_DIR}
      )
      target_link_libraries(${PACKAGE_NAME} PRIVATE ${SYS_WXCURL_LIBRARIES})
    else ()
      message(STATUS "Building with bundled wxcurl")
      include("Curl")
      add_subdirectory("libs/wxcurl")
      target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::wxcurl)
    endif ()

    target_link_libraries(${PACKAGE_NAME} PRIVATE ${CURL_LIBRARIES})

    if (WIN32 AND OCPN_USE_EXTERN_CURL)
      find_library(
        CURL_LIBRARY_DLL
        NAMES
          curl
          curllib
          libcurl_imp
          curllib_static
          libcurl
        PATH_SUFFIXES dll
      )
      install(FILES ${CURL_LIBRARY_DLL} DESTINATION ".")
    endif (WIN32 AND OCPN_USE_EXTERN_CURL)
  endif (NOT QT_ANDROID)
endif (OCPN_USE_CURL)

if (OCPN_USE_SVG)
  message(STATUS "SVG support enabled...")
  if (NOT QT_ANDROID)
    include(OcpnFindCairo)
    include(OcpnFindExpat)
    if (CMAKE_HOST_WIN32)
      # On Windows, we have our own expat and cairo
      file(GLOB gtkdll_files "${CMAKE_CURRENT_SOURCE_DIR}/buildwin/gtk/*.dll")
      install(FILES ${gtkdll_files} DESTINATION ".")
      file(
        GLOB
          expatdll_files
          "${CMAKE_CURRENT_SOURCE_DIR}/buildwin/expat-2.2.5/*.dll"
      )
      install(FILES ${expatdll_files} DESTINATION ".")
      message(STATUS "GTK and Expat DLLs bundled into the installer package...")
    else (CMAKE_HOST_WIN32)
      # Include(OcpnFindFmpeg)  -- FFMPEG is not enabled currently.
      # target_link_libraries(${PACKAGE_NAME} ocpn::ffmpeg)
      include(FindEXIF)
      target_link_libraries(
        ${PACKAGE_NAME}
        PRIVATE ${EXIF_LIBRARIES} ${GTK_LIBRARIES}
      )
      target_include_directories(${PACKAGE_NAME} PRIVATE ${EXIF_INCLUDE_DIRS})
    endif (CMAKE_HOST_WIN32)
    target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::cairo ocpn::expat)

    add_definitions(-DocpnUSE_SVG) # Goes into config.h, leave as global

    pkg_search_module(WXSVG libwxsvg wxsvg)
    use_bundled_lib(USE_BUNDLED_WXSVG wxsvg)
    if (WXSVG_FOUND AND NOT USE_BUNDLED_WXSVG)
      message(STATUS "Building with system wxsvg includes")
      add_library(WXSVG INTERFACE)
      add_library(ocpn::wxsvg ALIAS WXSVG)
      target_include_directories(WXSVG INTERFACE ${WXSVG_INCLUDE_DIR})
      target_link_libraries(WXSVG INTERFACE ${WXSVG_LIBRARIES})
    else ()
      add_subdirectory("libs/wxsvg")
    endif ()
    target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::wxsvg)

  else(NOT QT_ANDROID)
    ADD_DEFINITIONS(-DocpnUSE_SVG)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libs/wxsvg/include/wxSVG)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libs/wxsvg/include/wxSVGXML)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libs/wxsvg/include)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libs/wxsvg/src)
  endif (NOT QT_ANDROID)
endif (OCPN_USE_SVG)

if (OCPN_USE_NEWSERIAL)
  add_subdirectory("libs/serial")
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::serial)
  message(STATUS "Using new serial library...")
endif (OCPN_USE_NEWSERIAL)

# Search for opengles, short of running a program to test the speed of
# acceleration, I simply use gles on "native linux" arm systems
if (ARCH MATCHES "arm*" AND (NOT QT_ANDROID))
  find_path(OPENGLESv1_INCLUDE_DIR GLES/gl.h)
  if (OPENGLESv1_INCLUDE_DIR)
    message(STATUS "Found OpenGLESv1")
    add_definitions(-DocpnUSE_GLES)
    add_definitions(-DocpnUSE_GL)

    set(OPENGLES_FOUND "YES")
    set(OPENGL_FOUND "YES")

    # SET(wxWidgets_FIND_COMPONENTS ${wxWidgets_FIND_COMPONENTS} gl )
    add_subdirectory(libs/glshim)

    set(OPENGL_LIBRARIES "GL_static" "EGL" "X11" "drm")
  endif ()
endif ()

if (OPENGL_FOUND)
  target_sources(
    ${PACKAGE_NAME}
    PRIVATE
    include/glChartCanvas.h
    include/glTextureDescriptor.h
    include/glTexCache.h
    include/glTextureManager.h
    include/TexFont.h
    src/glTextureDescriptor.cpp
    src/glTexCache.cpp
    src/glChartCanvas.cpp
    src/glTextureManager.cpp
    src/TexFont.cpp
  )
endif ()

# Building for QT_ANDROID involves a cross-building environment, So the include
# directories, flags, etc must be stated explicitly without trying to locate
# them on the host build system.
if (QT_ANDROID)
  message(STATUS "Using GLESv1 for Android")
  add_definitions(-DocpnUSE_GLES)
  add_definitions(-DocpnUSE_GL)
  add_definitions(-DARMHF)

  set(OPENGLES_FOUND "YES")
  set(OPENGL_FOUND "YES")

  # SET(wxWidgets_FIND_COMPONENTS ${wxWidgets_FIND_COMPONENTS} gl )
  add_subdirectory(libs/glshim)
endif (QT_ANDROID)

message(STATUS "    Adding local GLU")
add_subdirectory(libs/glu)
set(OPENGL_LIBRARIES "GLU_static" ${OPENGL_LIBRARIES})
if (OCPN_VERBOSE)
  message(STATUS "    Revised GL Lib (with local): " ${OPENGL_LIBRARIES})
endif ()

find_package(LibArchive REQUIRED)
target_include_directories(${PACKAGE_NAME} PRIVATE ${LibArchive_INCLUDE_DIRS})
target_link_libraries(${PACKAGE_NAME} PRIVATE ${LibArchive_LIBRARIES})

message(STATUS "    Adding local LIBTESS2")
add_subdirectory(libs/libtess2)
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::tess2)

include("SoundConfig")
add_subdirectory(libs/sound)
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::sound)

add_subdirectory(libs/wxJSON)
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::wxjson)

if (TINYXML_FOUND)
  message(STATUS "Building with system tinyxml")
  add_library(TINYXML INTERFACE)
  target_include_directories(TINYXML INTERFACE ${TINYXML_INCLUDE_DIR})
  target_link_libraries(TINYXML INTERFACE ${TINYXML_LIBRARIES})
  add_library(ocpn::tinyxml ALIAS TINYXML)
else (TINYXML_FOUND)
  add_subdirectory(libs/tinyxml)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::tinyxml)
  message(STATUS "Building with bundled tinyxml")
endif (TINYXML_FOUND)
set(TIXML_USE_STL 1) # -> config.h

message(STATUS "S57 ENC support: enabled")
set(
  SRC_S57ENC
  include/cm93.h
  include/mygeom.h
  include/ogr_s57.h
  include/Osenc.h
  include/s52plib.h
  include/s52utils.h
  include/s57chart.h
  include/s57RegistrarMgr.h
  include/SencManager.h
  include/TexFont.h
  src/cm93.cpp
  src/mygeom.cpp
  src/ogrs57datasource.cpp
  src/ogrs57layer.cpp
  src/Osenc.cpp
  src/s52cnsy.cpp
  src/s52plib.cpp
  src/s52utils.cpp
  src/s57chart.cpp
  src/s57classregistrar.cpp
  src/s57featuredefns.cpp
  src/s57obj.cpp
  src/s57reader.cpp
  src/s57RegistrarMgr.cpp
  src/SencManager.cpp
  src/TexFont.cpp
)
add_library(S57ENC STATIC ${SRC_S57ENC})
target_link_libraries(
  S57ENC
  PUBLIC
    ocpn::nmea0183
    ocpn::tess2
    ocpn::sound
    ocpn::wxjson
    ssl::sha1
    ocpn::tinyxml
)

IF(NOT QT_ANDROID)
   target_link_libraries(S57ENC PUBLIC
      ocpn::wxsvg
    ocpn::wxcurl
)
ENDIF(NOT QT_ANDROID)

add_subdirectory(libs/iso8211)
target_link_libraries(S57ENC PUBLIC ocpn::iso8211)

add_subdirectory(libs/gdal)
target_link_libraries(S57ENC PUBLIC ocpn::gdal)

if (OCPN_USE_LZMA AND LIBLZMA_FOUND)
  target_include_directories(S57ENC PRIVATE ${LIBLZMA_INCLUDE_DIR})
  target_link_libraries(S57ENC PUBLIC ${LIBLZMA_LIBRARY})
endif ()

target_include_directories(S57ENC PRIVATE ${CMAKE_SOURCE_DIR}/include)

set_property(TARGET S57ENC PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_link_libraries(${PACKAGE_NAME} PRIVATE S57ENC)

if (OCPN_USE_GARMINHOST)
  add_subdirectory("libs/garmin")
  target_link_libraries(${PACKAGE_NAME} PUBLIC ocpn::garminhost)
  message(STATUS "Garmin Host Mode support: enabled")
else (OCPN_USE_GARMINHOST)
  message(STATUS "Garmin Host Mode support: disabled")
endif (OCPN_USE_GARMINHOST)

# Chart Symbols library
set(SRC_SYMBOLS src/chartsymbols.cpp include/chartsymbols.h)

add_library(SYMBOLS STATIC ${SRC_SYMBOLS})
set_property(TARGET SYMBOLS PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_link_libraries(SYMBOLS PRIVATE ocpn::tinyxml)
target_include_directories(SYMBOLS PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PACKAGE_NAME} PRIVATE SYMBOLS)

# Compile texcompression support library
if (OPENGL_FOUND)
  add_subdirectory("libs/texcmp")
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::texcmp)

  add_subdirectory("libs/mipmap")
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::mipmap)
endif (OPENGL_FOUND)

pkg_search_module(LZ4 liblz4 lz4)
USE_BUNDLED_LIB(USE_BUNDLED_LZ4 lz4)
if (LZ4_FOUND AND NOT USE_BUNDLED_LZ4)
  message(STATUS "Building with system lz4")
  include_directories(${LZ4_INCLUDE_DIR})
  if (COMMAND target_link_directories)
    target_link_directories(TEXCMP PRIVATE ${LZ4_LIBRARY_DIRS})
  else ()
    link_directories(${LZ4_LIBRARY_DIRS})
  endif ()
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${LZ4_LIBRARIES})
else ()
  message(STATUS "Building with bundled lz4")
  add_subdirectory("libs/lz4")
  target_link_libraries(${PACKAGE_NAME} PRIVATE LZ4)
endif ()

if (MINGW)
  target_link_libraries(${PACKAGE_NAME} PRIVATE psapi winmm setupapi)
endif ()

# TODO dnl dnl Use OpenGL tesselator or Internal tesselator dnl
# tess_internal=false tess_glu=true AC_ARG_WITH(tess_internal, [  --with-tess-
# internal    use Internal Polygon Tesselator], [tess_internal=true] )
#
# if [ "$tess_internal" = "false" ] ; then dnl  Look for and qualify an external
# GLU library echo "checking for useable OpenGLU Library" AC_CHECK_LIB(GLU,
# gluNewTess, true, dnl here true is just a nop AC_MSG_ERROR([*** libGLU not
# found.  Run configure using --with-tess-internal.]))
#
# GL_LIBS="-lGL -lGLU" AC_SUBST(GL_LIBS)
#
# GL_CFLAGS="-DUSE_GLU_TESS" AC_SUBST(GL_CFLAGS) fi

if (UNIX AND NOT APPLE)
  find_path(LIBELF_INCLUDE_DIR NAMES libelf.h gelf.h PATH_SUFFIXES libelf)
  find_library(LIBELF_LIBRARY NAMES elf)
  if (LIBELF_INCLUDE_DIR AND LIBELF_LIBRARY)
    message(STATUS "Found LibELF...")
    message(STATUS "    ELF Lib: ${LIBELF_INCLUDE_DIR}")
    message(STATUS "    ELF Include: ${LIBELF_LIBRARY}")
    target_include_directories(${PACKAGE_NAME} PUBLIC "${LIBELF_INCLUDE_DIR}")
    target_link_libraries(${PACKAGE_NAME} PRIVATE "${LIBELF_LIBRARY}")
    set(USE_LIBELF ON)   # => config.h
  else ()
    message(
      WARNING
        "Did not found LibELF, plugin compatibility check will be simplified."
    )
  endif ()
endif (UNIX AND NOT APPLE)

configure_file(config.h.in ${CMAKE_BINARY_DIR}/include/config.h)
include_directories(BEFORE "${CMAKE_BINARY_DIR}/include")

if (NOT APPLE AND NOT QT_ANDROID)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${wxWidgets_LIBRARIES})
endif ()

if (CMAKE_HOST_WIN32)
  target_link_libraries(
    ${PACKAGE_NAME}
    PRIVATE
      setupapi.lib
      # glu32.lib
      "GLU_static" psapi.lib
  )

  # use gdi plus only on MSVC, it is not a free library
  if (MSVC)
    target_link_libraries(${PACKAGE_NAME} PRIVATE gdiplus.lib)
  endif ()

endif (CMAKE_HOST_WIN32)

if (MINGW)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${OPENGL_LIBRARIES} wsock32.lib)
endif (MINGW)

if (UNIX)
  target_link_libraries(
    ${PACKAGE_NAME}
    PRIVATE ${OPENGL_LIBRARIES} ${GETTEXT_LIBRARY} SQLITE_CPP
  )
  if (NOT APPLE AND NOT QT_ANDROID AND NOT WIN32 AND NOT X11_FOUND)
    message(STATUS "Did not find x11 libraries")
    message(
      STATUS "support for transparent toolbar in opengl mode not detected"
    )
  endif ()
endif (UNIX)

# If we build for windows systems, we also include the resource file containing
# the manifest, icon and other resources
if (MSVC)
  target_sources(${PACKAGE_NAME} PRIVATE src/opencpn.rc)
endif ()

set(gshhs_MIN data/gshhs/poly-c-1.dat)

set(
  gshhs_CRUDE
  ${gshhs_MIN}
  data/gshhs/wdb_borders_c.b
  data/gshhs/wdb_rivers_c.b
)

set(
  gshhs_LOW
  ${gshhs_CRUDE}
  data/gshhs/poly-l-1.dat
  data/gshhs/wdb_borders_l.b
  data/gshhs/wdb_rivers_l.b
)

set(
  gshhs_INTERMEDIATE
  ${gshhs_LOW}
  data/gshhs/poly-i-1.dat
  data/gshhs/wdb_borders_i.b
  data/gshhs/wdb_rivers_i.b
)

set(
  gshhs_HIGH
  ${gshhs_INTERMEDIATE}
  data/gshhs/poly-h-1.dat
  data/gshhs/wdb_borders_h.b
  data/gshhs/wdb_rivers_h.b
)

set(
  gshhs_FULL
  ${gshhs_HIGH}
  data/gshhs/poly-f-1.dat
  data/gshhs/wdb_borders_f.b
  data/gshhs/wdb_rivers_f.b
)

# Various data files
if (OCPN_BUNDLE_GSHHS MATCHES "MIN")
  set(gshhs ${gshhs_MIN})
endif (OCPN_BUNDLE_GSHHS MATCHES "MIN")

if (OCPN_BUNDLE_GSHHS MATCHES "CRUDE")
  set(gshhs ${gshhs_CRUDE})
endif (OCPN_BUNDLE_GSHHS MATCHES "CRUDE")

if (OCPN_BUNDLE_GSHHS MATCHES "LOW")
  set(gshhs ${gshhs_LOW})
endif (OCPN_BUNDLE_GSHHS MATCHES "LOW")

if (OCPN_BUNDLE_GSHHS MATCHES "INTERMEDIATE")
  set(gshhs ${gshhs_INTERMEDIATE})
endif (OCPN_BUNDLE_GSHHS MATCHES "INTERMEDIATE")

if (OCPN_BUNDLE_GSHHS MATCHES "HIGH")
  set(gshhs ${gshhs_HIGH})
endif (OCPN_BUNDLE_GSHHS MATCHES "HIGH")

if (OCPN_BUNDLE_GSHHS MATCHES "FULL")
  set(gshhs ${gshhs_FULL})
endif (OCPN_BUNDLE_GSHHS MATCHES "FULL")

set(
  uiData
  src/bitmaps/styles.xml
  src/bitmaps/toolicons_journeyman.png
  src/bitmaps/toolicons_journeyman_flat.png
  src/bitmaps/toolicons_traditional.png
  src/bitmaps/iconRMinus.png
  src/bitmaps/iconRPlus.png
  src/bitmaps/iconMinimum.png
  src/bitmaps/iconStandard.png
  src/bitmaps/iconUserStd.png
  src/bitmaps/iconAll.png
  src/bitmaps/DragHandle.svg
  src/bitmaps/eye.svg
  src/bitmaps/eyex.svg
  src/bitmaps/iENC_All.svg
  src/bitmaps/iENC_Minimum.svg
  src/bitmaps/iENC_Standard.svg
  src/bitmaps/iENC_UserStd.svg
  src/bitmaps/iENC_RPlus.svg
  src/bitmaps/iENC_RMinus.svg
  src/bitmaps/plus.svg
)

set(
  s57data
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

if (OCPN_BUNDLE_TCDATA MATCHES "ON")
  set(
    tcData
    data/tcdata/HARMONIC
    data/tcdata/HARMONIC.IDX
    data/tcdata/README.harmonics
  )
endif (OCPN_BUNDLE_TCDATA MATCHES "ON")

if (APPLE)
  target_sources(${PACKAGE_NAME} PRIVATE include/macutils.h src/macutils.c)
endif ()

# Mac has trouble finding libgobject-2.0.dylib We look for it explicitely
if (xAPPLE)
  # Assume pkg-config is available.
  pkg_search_module(GOBJECT REQUIRED gobject-2.0)
  # MESSAGE (STATUS "    GOBJECT: " ${GOBJECT_LDFLAGS})

  target_compile_definitions(${PACKAGE_NAME} PRIVATE ${GOBJECT_LDFLAGS})
  target_link_libraries(${PACKAGE_NAME} PUBLIC "iconv")
endif (xAPPLE)

if (WIN32)
  target_sources(${PACKAGE_NAME} PRIVATE app.manifest)
endif (WIN32)

add_subdirectory("libs/nmea0183")
target_link_libraries(${PACKAGE_NAME} PUBLIC ocpn::nmea0183)

if (APPLE)
  if (NOT OCPN_BUNDLE_GSHHS MATCHES "NONE")
    set_source_files_properties(
      ${gshhs}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION
      SharedSupport/gshhs
    )
  endif ()

  set_source_files_properties(
    ${uiData}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION
    SharedSupport/uidata
  )
  set_source_files_properties(
    ${s57data}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION
    SharedSupport/s57data
  )
  if (OCPN_BUNDLE_TCDATA MATCHES "ON")
    set_source_files_properties(
      ${tcData}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION
      SharedSupport/tcdata
    )
  endif (OCPN_BUNDLE_TCDATA MATCHES "ON")

  set_source_files_properties(
    data/license.html
    PROPERTIES
    MACOSX_PACKAGE_LOCATION
    SharedSupport
  )

  set_source_files_properties(
    data/authors.html
    PROPERTIES
    MACOSX_PACKAGE_LOCATION
    SharedSupport
  )

  set_source_files_properties(
    data/opencpn.png
    PROPERTIES
    MACOSX_PACKAGE_LOCATION
    SharedSupport
  )

  set_source_files_properties(
    data/CoC-909_2013-InlandECDIS_20170308.pdf
    PROPERTIES
    MACOSX_PACKAGE_LOCATION
    SharedSupport
  )

  set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/data/ocpn-plugins.xml
    PROPERTIES
    MACOSX_PACKAGE_LOCATION
    SharedSupport
  )
  install(
      FILES ${CMAKE_SOURCE_DIR}/data/ocpn-plugins.xml
      DESTINATION ${PREFIX_PKGDATA}
  )
  target_sources(${PACKAGE_NAME}
      PRIVATE ${CMAKE_SOURCE_DIR}/data/ocpn-plugins.xml
  )

  # Known, downloadable plugin icons
  file(GLOB PLUGIN_ICONS ${CMAKE_SOURCE_DIR}/plugins/icons/*)
  foreach (_pluginIcon ${PLUGIN_ICONS})
    set_source_files_properties(
      ${_pluginIcon}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION
      SharedSupport/uidata/plugins
    )
  endforeach ()
  install(
    DIRECTORY plugins/icons/
    DESTINATION ${PREFIX_PKGDATA}/uidata/plugins
  )
  target_sources(${PACKAGE_NAME} PRIVATE ${PLUGIN_ICONS})

  file(GLOB SOUND_FILES ${CMAKE_SOURCE_DIR}/data/sounds/*)
  foreach (_currentSoundFile ${SOUND_FILES})
    set_source_files_properties(
      ${_currentSoundFile}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION
      SharedSupport/sounds
    )
  endforeach (_currentSoundFile)

  file(GLOB CONFIG_FILES ${CMAKE_SOURCE_DIR}/data/configs/*)
  foreach (_currentConfigFile ${CONFIG_FILES})
    set_source_files_properties(
      ${_currentConfigFile}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION
      SharedSupport/configs
    )
  endforeach (_currentConfigFile)

  if (OCPN_USE_SVG)
    # Traditional theme SVG icons
    file(
      GLOB TRADITIONAL_SVG_FILES "${CMAKE_SOURCE_DIR}/data/svg/traditional/*"
    )
    foreach (_currentSvgFile ${TRADITIONAL_SVG_FILES})
      set_source_files_properties(
        ${_currentSvgFile}
        PROPERTIES
        MACOSX_PACKAGE_LOCATION
        SharedSupport/uidata/traditional
      )
    endforeach ()

    # Journeyman theme SVG icons
    file(GLOB JOURNEYMAN_SVG_FILES "${CMAKE_SOURCE_DIR}/data/svg/journeyman/*")
    foreach (_currentSvgFile ${JOURNEYMAN_SVG_FILES})
      set_source_files_properties(
        ${_currentSvgFile}
        PROPERTIES
        MACOSX_PACKAGE_LOCATION
        SharedSupport/uidata/journeyman
      )
    endforeach ()

    # Journeyman_flat theme SVG icons
    file(
      GLOB
        JOURNEYMAN_FLAT_SVG_FILES
        "${CMAKE_SOURCE_DIR}/data/svg/journeyman_flat/*"
    )
    foreach (_currentSvgFile ${JOURNEYMAN_FLAT_SVG_FILES})
      set_source_files_properties(
        ${_currentSvgFile}
        PROPERTIES
        MACOSX_PACKAGE_LOCATION
        SharedSupport/uidata/journeyman_flat
      )
    endforeach ()

    # MUI_flat theme SVG icons
    file(GLOB MUI_FLAT_SVG_FILES "${CMAKE_SOURCE_DIR}/data/svg/MUI_flat/*")
    foreach (_currentSvgFile ${MUI_FLAT_SVG_FILES})
      set_source_files_properties(
        ${_currentSvgFile}
        PROPERTIES
        MACOSX_PACKAGE_LOCATION
        SharedSupport/uidata/MUI_flat
      )
    endforeach (_currentSvgFile)

    # Default SVG Icons
    file(GLOB DEFAULT_ICON_SVG_FILES "${CMAKE_SOURCE_DIR}/data/svg/markicons/*")
    foreach (_currentSvgFile ${DEFAULT_ICON_SVG_FILES})
      set_source_files_properties(
        ${_currentSvgFile}
        PROPERTIES
        MACOSX_PACKAGE_LOCATION
        SharedSupport/uidata/markicons
      )
    endforeach ()

  endif (OCPN_USE_SVG)

  install(
    DIRECTORY data/svg/markicons/
    DESTINATION ${PREFIX_PKGDATA}/uidata/markicons
  )

  install(
    DIRECTORY data/svg/MUI_flat/
    DESTINATION ${PREFIX_PKGDATA}/uidata/MUI_flat
  )

  set_source_files_properties(
    data/doc/help_web.html
    PROPERTIES
    MACOSX_PACKAGE_LOCATION
    SharedSupport/doc
  )

  if (OCPN_BUNDLE_DOCS MATCHES "ON")
    set_source_files_properties(
      data/doc/help_en_US.html
      PROPERTIES
      MACOSX_PACKAGE_LOCATION
      SharedSupport/doc
    )

    file(GLOB_RECURSE WIKI_DOC_FILES "${CMAKE_SOURCE_DIR}/data/doc/*")
    foreach (_currentWikiDocFile ${WIKI_DOC_FILES})

      file(
        RELATIVE_PATH
          _docfileRelPath # Output variable
          "${CMAKE_SOURCE_DIR}/data/doc/" # Base directory
          ${_currentWikiDocFile} # Absolute path to the file
      )

      get_filename_component(_docfileLocation ${_docfileRelPath} DIRECTORY)
      set(_location "SharedSupport/doc/${_docfileLocation}")
      # MESSAGE (STATUS "    Adding Wiki Doc File : " ${_currentWikiDocFile}  "
      # Destination: "  ${_location})
      set_source_files_properties(
        ${_currentWikiDocFile}
        PROPERTIES
        MACOSX_PACKAGE_LOCATION
        ${_location}
      )
    endforeach (_currentWikiDocFile)

  endif (OCPN_BUNDLE_DOCS MATCHES "ON")

  set_source_files_properties(
    buildosx/opencpn.icns
    PROPERTIES
    MACOSX_PACKAGE_LOCATION
    Resources
  )

  set_source_files_properties(
    buildosx/MainMenu.xib
    PROPERTIES
    MACOSX_PACKAGE_LOCATION
    Resources/English.lproj
  )

  # These variables get substituted into the Info.plist template file at build
  # time
  set(MACOSX_BUNDLE_ICON_FILE opencpn.icns)
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PACKAGE_VERSION}")
  set(MACOSX_BUNDLE_BUNDLE_NAME "OpenCPN")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.opencpn")

  if (OCPN_BUNDLE_DOCS MATCHES "ON")
    set(
      DOC_FILES
      data/doc/help_web.html
      data/doc/help_en_US.html
      ${WIKI_DOC_FILES}
    )
  else (OCPN_BUNDLE_DOCS MATCHES "ON")
    set(DOC_FILES data/doc/help_web.html)
  endif (OCPN_BUNDLE_DOCS MATCHES "ON")

  target_sources(
    ${PACKAGE_NAME}
    PRIVATE
    ${gshhs}
    ${uiData}
    ${s57data}
    ${tcData}
    data/license.html
    data/authors.html
    data/opencpn.png
    data/ocpn-plugins.xml
    data/CoC-909_2013-InlandECDIS_20170308s.pdf
    ${SOUND_FILES}
    buildosx/opencpn.icns
    buildosx/MainMenu.xib
    ${TRADITIONAL_SVG_FILES}
    ${JOURNEYMAN_SVG_FILES}
    ${JOURNEYMAN_FLAT_SVG_FILES}
    ${DEFAULT_ICON_SVG_FILES}
    ${MUI_FLAT_SVG_FILES}
    ${DOC_FILES}
    ${CONFIG_FILES}
  )
  install(TARGETS ${PACKAGE_NAME} BUNDLE DESTINATION ${PREFIX_BIN})

  target_link_libraries(${PACKAGE_NAME} PRIVATE ${wxWidgets_LIBRARIES})
  set_target_properties(
    ${PACKAGE_NAME}
    PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/buildosx/Info.plist.in"
  )
endif (APPLE)

if (QT_ANDROID)
  target_link_libraries( ${PACKAGE_NAME} PRIVATE ${wxWidgets_LIBRARIES} )
  target_link_libraries( ${PACKAGE_NAME} PRIVATE GLESv2 EGL z)
  target_link_libraries( ${PACKAGE_NAME} PRIVATE ${OPENGL_LIBRARIES} )
  target_link_libraries( ${PACKAGE_NAME} PRIVATE NMEA0183 )
endif ()

# Sanitizers, part 2/2
if (CMAKE_VERSION VERSION_GREATER 3.4)
  if (NOT "${ENABLE_SANITIZER}" MATCHES "none")
    target_link_libraries(${PACKAGE_NAME} PRIVATE -fsanitize=${ENABLE_SANITIZER})
  endif ()
endif ()

if (LIBLZMA_FOUND)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${LIBLZMA_LIBRARY})
endif (LIBLZMA_FOUND)

# Certain older Cmake FindGTK2 modules ( e.g. cmake-2.8.0-2) do not yield all of
# the required link libraries So, add them manually.  These declarations may be
# redundant in some architectures, but do no harm.
if (UNIX)
  target_link_libraries(${PACKAGE_NAME} PRIVATE dl)
endif ()

#
# Install
#
install(FILES data/license.txt DESTINATION ${PREFIX_DATA}/${PACKAGE_NAME})

if (NOT APPLE)

  if (WIN32)
    install(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION ".")
  endif ()

  if (UNIX AND NOT APPLE AND NOT QT_ANDROID)
    install(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION ${PREFIX_BIN})
    install(FILES LINUX_DEVICES.md DESTINATION ${PREFIX_DATA}/${PACKAGE_NAME})
  endif ()

  if (WIN32)
    set(PREFIX_PKGDATA ".")
  endif ()

  install(
      FILES ${CMAKE_SOURCE_DIR}/data/ocpn-plugins.xml
      DESTINATION ${PREFIX_PKGDATA}
  )

  if (NOT OCPN_BUNDLE_GSHHS MATCHES "NONE")
    install(FILES ${gshhs} DESTINATION ${PREFIX_PKGDATA}/gshhs)
  endif (NOT OCPN_BUNDLE_GSHHS MATCHES "NONE")

  install(FILES ${uiData} DESTINATION ${PREFIX_PKGDATA}/uidata)
  install(
    DIRECTORY data/svg/markicons/
    DESTINATION ${PREFIX_PKGDATA}/uidata/markicons
  )

  if (OCPN_USE_SVG)
    # Traditional theme SVG icons
    install(
      DIRECTORY data/svg/traditional/
      DESTINATION ${PREFIX_PKGDATA}/uidata/traditional
    )
    install(
      DIRECTORY data/svg/journeyman/
      DESTINATION ${PREFIX_PKGDATA}/uidata/journeyman
    )
    install(
      DIRECTORY data/svg/journeyman_flat/
      DESTINATION ${PREFIX_PKGDATA}/uidata/journeyman_flat
    )
    install(
      DIRECTORY data/svg/MUI_flat/
      DESTINATION ${PREFIX_PKGDATA}/uidata/MUI_flat
    )
  endif (OCPN_USE_SVG)

  install(FILES ${s57data} DESTINATION ${PREFIX_PKGDATA}/s57data)

  if (OCPN_BUNDLE_TCDATA MATCHES "ON")
    install(FILES ${tcData} DESTINATION ${PREFIX_PKGDATA}/tcdata)
  endif ()

  file(GLOB LICENSES "${CMAKE_SOURCE_DIR}/COPYING.*")
  install(FILES ${LICENSES} LICENSING DESTINATION "${PREFIX_PKGDATA}")

  install(
    FILES data/CoC-909_2013-InlandECDIS_20170308s.pdf
    DESTINATION ${PREFIX_PKGDATA}
  )
  install(FILES data/copyright DESTINATION ${PREFIX_DATA}/doc/${PACKAGE_NAME})

  install(FILES data/license.html DESTINATION ${PREFIX_PKGDATA})
  install(FILES data/authors.html DESTINATION ${PREFIX_PKGDATA})
  install(FILES data/opencpn.png DESTINATION ${PREFIX_PKGDATA})

  if (PACKAGE_FORMAT EQUAL "DEB")
    install(
      FILES data/changelog.gz
      DESTINATION ${PREFIX_DATA}/doc/${PACKAGE_NAME}
    )
  else (PACKAGE_FORMAT EQUAL "DEB")
    install(FILES data/changelog DESTINATION ${PREFIX_DATA}/doc/${PACKAGE_NAME})
  endif (PACKAGE_FORMAT EQUAL "DEB")

  install(DIRECTORY data/sounds/ DESTINATION ${PREFIX_PKGDATA}/sounds)

  if (OCPN_BUNDLE_DOCS MATCHES "ON")
    install(DIRECTORY data/doc/ DESTINATION ${PREFIX_PKGDATA}/doc)
  else (OCPN_BUNDLE_DOCS MATCHES "ON")
    install(FILES data/doc/help_web.html DESTINATION ${PREFIX_PKGDATA}/doc)
  endif (OCPN_BUNDLE_DOCS MATCHES "ON")

  if (UNIX)
    install(
      FILES data/opencpn.png
      DESTINATION ${PREFIX_DATA}/icons/hicolor/48x48/apps
    )
    install(
      FILES src/bitmaps/opencpn-64.png
      DESTINATION ${PREFIX_DATA}/icons/hicolor/64x64/apps
      RENAME opencpn.png
    )
    install(
      FILES src/bitmaps/other_svg_src/opencpn.svg
      DESTINATION ${PREFIX_DATA}/icons/hicolor/scalable/apps
    )
    install(FILES data/opencpn.desktop DESTINATION ${PREFIX_DATA}/applications)
    install(FILES data/opencpn.appdata.xml DESTINATION ${PREFIX_DATA}/appdata)
    install(FILES opencpn.1 DESTINATION ${PREFIX_DATA}/man/man1)
  endif (UNIX)

endif (NOT APPLE)
# Fix up time_t definition Checks for 32-bit version. And always use 32-bit
# time_t for compatibility

# uninstall-local: rm -fr $(DESTDIR)$(pkgdatadir)/wvsdata/* rm -fr
# $(DESTDIR)$(pkgdatadir)/gshhs/* rm -fr $(DESTDIR) $(pkgdatadir)/s57data/* rm
# -fr $(DESTDIR)$(pkgdatadir)/tcdata/* rmdir $(DESTDIR)$(pkgdatadir)/wvsdata
# rmdir $(DESTDIR)$(pkgdatadir)/gshhs rmdir $(DESTDIR)$(pkgdatadir)/s57data
# rmdir $(DESTDIR)$(pkgdatadir)/tcdata rmdir $(DESTDIR)$(pkgdatadir)
#

#
# Gettext languages setup.
#
find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
if (GETTEXT_XGETTEXT_EXECUTABLE)

  add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot.dummy
    COMMAND
      ${GETTEXT_XGETTEXT_EXECUTABLE}
      --from-code=iso-8859-1
      --force-po
      --package-name=${PACKAGE_NAME}
      --package-version="${PACKAGE_VERSION}"
      --output=${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot
      --keyword=_
      --width=80
      --files-from=${CMAKE_SOURCE_DIR}/po/POTFILES.in
    DEPENDS
      ${CMAKE_SOURCE_DIR}/po/POTFILES.in
      ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "pot-update [${PACKAGE_NAME}]: Generated pot file."
  )
  add_custom_target(
    pot-update
    COMMENT "pot-update: Done."
    DEPENDS ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot.dummy
  )

endif (GETTEXT_XGETTEXT_EXECUTABLE)

macro (GETTEXT_UPDATE_PO _potFile)
  set(_poFiles ${_potFile})
  get_filename_component(_absPotFile ${_potFile} ABSOLUTE)

  foreach (_currentPoFile ${ARGN})
    get_filename_component(_absFile ${_currentPoFile} ABSOLUTE)
    get_filename_component(_poBasename ${_absFile} NAME_WE)

    add_custom_command(
      OUTPUT ${_absFile}.dummy
      COMMAND
        ${GETTEXT_MSGMERGE_EXECUTABLE}
        --width=80
        --strict
        --quiet
        --update
        --backup=none
        --no-location -s ${_absFile} ${_absPotFile}
      DEPENDS ${_absPotFile} ${_absFile}
      COMMENT "po-update [${_poBasename}]: Updated po file."
    )
    set(_poFiles ${_poFiles} ${_absFile}.dummy)
  endforeach (_currentPoFile)

  add_custom_target(po-update COMMENT "po-update: Done." DEPENDS ${_poFiles})
endmacro (GETTEXT_UPDATE_PO)

if (GETTEXT_MSGMERGE_EXECUTABLE)
  file(GLOB PACKAGE_PO_FILES ${CMAKE_SOURCE_DIR}/po/*.po)
  gettext_update_po(
    ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot ${PACKAGE_PO_FILES}
  )
endif ()

set(_gmoFiles)
macro (GETTEXT_BUILD_MO _poFile _lang)

  get_filename_component(_absFile ${_poFile} ABSOLUTE)
  get_filename_component(_poBasename ${_absFile} NAME_WE)
  set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_poBasename}.mo)

  add_custom_command(
    OUTPUT ${_gmoFile}
    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_absFile}
    COMMAND
      ${CMAKE_COMMAND} -E copy ${_gmoFile}
      ${CMAKE_CURRENT_BINARY_DIR}/Resources/${PACKAGE_NAME}_${_lang}.lproj/${PACKAGE_NAME}.mo
    DEPENDS ${_absFile}
    COMMENT "i18n [${_poBasename}]: Created mo file."
  )
  if (APPLE)
    install(
      FILES ${_gmoFile}
      DESTINATION
        ${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/Resources/${_lang}.lproj
      RENAME ${PACKAGE_NAME}.mo
    )
  else ()
    install(
      FILES ${_gmoFile}
      DESTINATION ${PREFIX_DATA}/locale/${_lang}/LC_MESSAGES
      RENAME ${PACKAGE_NAME}.mo
    )
  endif ()

  set(_gmoFiles ${_gmoFiles} ${_gmoFile})
endmacro (GETTEXT_BUILD_MO)

if (GETTEXT_MSGFMT_EXECUTABLE)
  gettext_build_mo(po/opencpn_cs_CZ.po cs)
  gettext_build_mo(po/opencpn_da_DK.po da)
  gettext_build_mo(po/opencpn_de_DE.po de)
  gettext_build_mo(po/opencpn_es_ES.po es)
  gettext_build_mo(po/opencpn_fr_FR.po fr)
  gettext_build_mo(po/opencpn_it_IT.po it)
  gettext_build_mo(po/opencpn_nl_NL.po nl)
  gettext_build_mo(po/opencpn_pl_PL.po pl)
  gettext_build_mo(po/opencpn_ru_RU.po ru)
  gettext_build_mo(po/opencpn_sv_SE.po sv)
  gettext_build_mo(po/opencpn_et_EE.po et)
  gettext_build_mo(po/opencpn_pt_PT.po pt_PT)
  gettext_build_mo(po/opencpn_pt_BR.po pt_BR)
  gettext_build_mo(po/opencpn_nb_NO.po nb_NO)
  gettext_build_mo(po/opencpn_tr_TR.po tr_TR)
  gettext_build_mo(po/opencpn_fi_FI.po fi_FI)
  gettext_build_mo(po/opencpn_el_GR.po el_GR)
  gettext_build_mo(po/opencpn_zh_TW.po zh_TW)
  gettext_build_mo(po/opencpn_zh_CN.po zh_TW)
  gettext_build_mo(po/opencpn_hu_HU.po hu_HU)
  gettext_build_mo(po/opencpn_gl_ES.po gl_ES)
  gettext_build_mo(po/opencpn_ca_ES.po ca_ES)
  gettext_build_mo(po/opencpn_ar_SA.po ar_SA)
  gettext_build_mo(po/opencpn_ja_JP.po ja_JP)
  gettext_build_mo(po/opencpn_vi_VN.po vi_VN)
  add_custom_target(i18n COMMENT "i18n: Done." DEPENDS ${_gmoFiles})

  if (NOT QT_ANDROID)
    add_dependencies(${PACKAGE_NAME} i18n)
  endif ()

endif (GETTEXT_MSGFMT_EXECUTABLE)

#
# Install wxstd.mo in Windows and Mac
#
if (WIN32 OR APPLE)

  file(
    GLOB WXWIDGETS_MO_FILES ${CMAKE_SOURCE_DIR}/buildwin/wxWidgets/locale/*.mo
  )

  foreach (_current_wx_mofile ${WXWIDGETS_MO_FILES})
    get_filename_component(_absFile ${_current_wx_mofile} ABSOLUTE)
    get_filename_component(_mlang ${_absFile} NAME_WE)

    if (APPLE)
      install(
        FILES ${_current_wx_mofile}
        DESTINATION
          ${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/Resources/${_mlang}.lproj
        RENAME wxstd.mo
      )
    else (APPLE)
      install(
        FILES ${_current_wx_mofile}
        DESTINATION ${PREFIX_DATA}/locale/${_mlang}/LC_MESSAGES
        RENAME
          wxstd.mo
          PERMISSIONS
          OWNER_READ
          OWNER_WRITE
          GROUP_READ
          WORLD_READ
      )
    endif (APPLE)

  endforeach (_current_wx_mofile)

endif (WIN32 OR APPLE)

#
# On Windows builds, copy the wxWidgets dlls and helpers to the executable
# directory
#
if (WIN32 AND NOT UNIX)
  if (OCPN_BUNDLE_WXDLLS)
    file(
      GLOB wxdll_files "${CMAKE_CURRENT_SOURCE_DIR}/buildwin/wxWidgets/*.dll"
    )
    install(FILES ${wxdll_files} DESTINATION ".")
    message(STATUS "wxWidgets DLLs bundled into the installer package...")
  endif (OCPN_BUNDLE_WXDLLS)
  if (OCPN_BUNDLE_VCDLLS)
    file(GLOB vcdll_files "${CMAKE_CURRENT_SOURCE_DIR}/buildwin/vc/*.dll")
    install(FILES ${vcdll_files} DESTINATION ".")
    message(
      STATUS "MSVC redistributable DLLs bundled into the installer package..."
    )
  endif (OCPN_BUNDLE_VCDLLS)
  option(
    BUNDLE_LIBARCHIVEDLLS "Bundle the prebuilt LibArchive and LibLZMA DLLs" ON
  )
  if (BUNDLE_LIBARCHIVEDLLS)
    file(
      GLOB
        libarchivedll_files
        "${CMAKE_CURRENT_SOURCE_DIR}/buildwin/archive.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/buildwin/liblzma.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/buildwin/zlib1.dll"
    )
    install(FILES ${libarchivedll_files} DESTINATION ".")
    message(STATUS "LibArchive DLLs bundled into the installer package...")
  endif (BUNDLE_LIBARCHIVEDLLS)
endif (WIN32 AND NOT UNIX)

if (MSVC AND OCPN_USE_CRASHREPORT)
  install(
    FILES ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/CrashRpt1403.dll
    DESTINATION ${PREFIX_PKGDATA}
  )
  install(
    FILES ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/CrashSender1403.exe
    DESTINATION ${PREFIX_PKGDATA}
  )
  install(
    FILES ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/crashrpt_lang.ini
    DESTINATION ${PREFIX_PKGDATA}
  )
  install(
    FILES ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/dbghelp.dll
    DESTINATION ${PREFIX_PKGDATA}
  )
  install(
    FILES ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/PrivacyPolicy.txt
    DESTINATION ${PREFIX_PKGDATA}
  )
  message(STATUS "CrashRpt bundled into the installer package...")
endif (MSVC AND OCPN_USE_CRASHREPORT)

if (MINGW)
  install(SCRIPT ${CMAKE_SOURCE_DIR}/cmake/BundleMingwLibs.cmake)
endif ()

if (NOT SKIP_PLUGINS)
  add_subdirectory(plugins)
endif ()


#
# Prepare CPACK configuration (MINGW)
#
if (MINGW)
  set(
      CMD "sed -e 's|/TRIMLEFT||' NSIS.template.in.in >build/NSIS.template.in"
  )
  file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/fix-NSIS.sh CONTENT "${CMD}")
  add_custom_target(
    NSIS_config ALL
    COMMAND /bin/sh ${CMAKE_BINARY_DIR}/fix-NSIS.sh
    DEPENDS NSIS.template.in.in
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  add_dependencies(${PACKAGE_NAME} NSIS_config)
else ()
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/NSIS.template.in.in
    ${CMAKE_CURRENT_SOURCE_DIR}/NSIS.template.in COPYONLY
  )
endif ()

#
# Build a CPack driven installer package
#
# include (InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "OpenCPN")
set(CPACK_PACKAGE_VENDOR "opencpn.org")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenCPN ${PACKAGE_VERSION}")
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH}${VERSION_TAIL})
set(
  CPACK_INSTALL_CMAKE_PROJECTS
  "${CMAKE_CURRENT_BINARY_DIR};${PACKAGE_NAME};ALL;/"
)
set(CPACK_PACKAGE_EXECUTABLES ${PACKAGE_NAME} "OpenCPN")

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README")
  set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")
endif ()

set(CPACK_SOURCE_GENERATOR "TGZ")

# The following components are regex's to match anywhere (unless anchored) in
# absolute path + filename to find files or directories to be excluded from
# source tarball.
set(
  CPACK_SOURCE_IGNORE_FILES
  "\\\\.git*"
  "^${CMAKE_CURRENT_SOURCE_DIR}/build*"
  "^${CPACK_PACKAGE_INSTALL_DIRECTORY}/*"
)

#
# Windows NSIS package setup.
#
if (WIN32 AND NOT UNIX)
  if (NOT SKIP_VERSION_CONFIG)
    configure_file(
      ${CMAKE_SOURCE_DIR}/src/opencpn.rc.in ${CMAKE_SOURCE_DIR}/src/opencpn.rc
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_GERMAN.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_GERMAN.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_FRENCH.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_FRENCH.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_CZECH.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_CZECH.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_DANISH.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_DANISH.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_SPANISH.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_SPANISH.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_ITALIAN.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_ITALIAN.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_DUTCH.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_DUTCH.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_POLISH.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_POLISH.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_PORTUGUESEBR.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_PORTUGUESEBR.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_PORTUGUESE.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_PORTUGUESE.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_RUSSIAN.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_RUSSIAN.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_SWEDISH.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_SWEDISH.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_FINNISH.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_FINNISH.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_NORWEGIAN.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_NORWEGIAN.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_CHINESETW.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_CHINESETW.nsh"
      @ONLY
    )
    configure_file(
      "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode/Language files/Langstrings_TURKISH.nsh.in"
      "${CMAKE_SOURCE_DIR}//buildwin/NSIS_Unicode/Include/Langstrings_TURKISH.nsh"
      @ONLY
    )
  endif (NOT SKIP_VERSION_CONFIG)

  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.

  set(CPACK_NSIS_INSTALLED_ICON_NAME "opencpn.exe")
  set(CPACK_NSIS_PACKAGE_NAME_LC "opencpn")
  set(CPACK_NSIS_DIR "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode") # Gunther
  set(CPACK_BUILDWIN_DIR "${CMAKE_SOURCE_DIR}/buildwin") # Gunther

  # These lines set the name of the Windows Start Menu shortcut and the icon
  # that goes with it
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME} ${PACKAGE_VERSION}")
  set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/src/bitmaps\\\\opencpn.ico")

  set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${PACKAGE_VERSION}_setup")

  set(
    CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/data/license.txt"
  )

  if (MINGW)
    set(CPACK_STRIP_FILES "bin/opencpn")
  endif (MINGW)

else (WIN32 AND NOT UNIX)
  set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PACKAGE_NAME})
  set(CPACK_STRIP_FILES "bin/opencpn")
  set(CPACK_SOURCE_STRIP_FILES "")
  set(
    CPACK_PACKAGE_FILE_NAME
    "${PACKAGE_NAME}_${PACKAGE_VERSION}-${OCPN_PACKAGE_RELEASE}_${ARCH}"
  )
  set(
    CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/data/license.html"
  )
endif (WIN32 AND NOT UNIX)

if (OCPN_BUNDLE_GSHHS MATCHES "NONE|MIN|CRUDE|LOW|INTERMEDIATE|HIGH|FULL")
  # Possible values: NONE (default), MIN (=Only crude polygons, no
  # borders/rivers), CRUDE, LOW, INTERMEDIATE, HIGH, FULL
  message(
    STATUS
      "*** Package will include GSHHS basechart level: ${OCPN_BUNDLE_GSHHS} ***"
  )
else (OCPN_BUNDLE_GSHHS MATCHES "NONE|MIN|CRUDE|LOW|INTERMEDIATE|HIGH|FULL")
  set(BUNDLE_GSHHS "NONE")
  message(STATUS "*** Package will NOT include GSHHS data ***")
endif (OCPN_BUNDLE_GSHHS MATCHES "NONE|MIN|CRUDE|LOW|INTERMEDIATE|HIGH|FULL")

if (OCPN_BUNDLE_TCDATA MATCHES "ON")
  message(STATUS "*** Package will include tide and current data ***")
else (OCPN_BUNDLE_TCDATA MATCHES "ON")
  message(STATUS "*** Package will NOT include tide and current data ***")
endif (OCPN_BUNDLE_TCDATA MATCHES "ON")

#
# Debian package setup
#
if (UNIX AND NOT APPLE)
  set(CPACK_STRIP_FILES ON)
  set(CPACK_GENERATOR ${PACKAGE_FORMAT})
  set(CPACK_PACKAGE_CONTACT "David S. Register ")

  # autogenerate additional dependency information
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
  set(CPACK_DEBIAN_PACKAGE_RECOMMENDS ${PACKAGE_RECS})
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCH})
  set(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")
  set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
  set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenSource Chart Plotter/Navigator")
  set(
    CPACK_PACKAGE_DESCRIPTION
    "OpenCPN is a concise ChartPlotter/Navigator. The application supports: GPS/GPDS Position Input, BSB Raster Chart Display, S57 Vector ENChart Display, AIS Input Decoding, Waypoint Autopilot Navigation ."
  )
  set(CPACK_PACKAGE_RELOCATABLE OFF)
  set(CPACK_SET_DESTDIR ON)
  set(CPACK_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
endif (UNIX AND NOT APPLE)

#
# Apple application bundle
#
if (APPLE)
  # -- Run the BundleUtilities cmake code
  set(CPACK_BUNDLE_PLIST "${CMAKE_SOURCE_DIR}/buildosx/Info.plist.in")

  set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app")
  set(DIRS "")

  # INSTALL(DIRECTORY DESTINATION "bin/OpenCPN.app/Contents/PlugIns")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/plugins/dashboard_pi/libdashboard_pi.dylib
    DESTINATION "bin/OpenCPN.app/Contents/PlugIns"
  )
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/plugins/grib_pi/libgrib_pi.dylib
    DESTINATION "bin/OpenCPN.app/Contents/PlugIns"
  )
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/plugins/chartdldr_pi/libchartdldr_pi.dylib
    DESTINATION "bin/OpenCPN.app/Contents/PlugIns"
  )
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/plugins/wmm_pi/libwmm_pi.dylib
    DESTINATION "bin/OpenCPN.app/Contents/PlugIns"
  )
  set(
    LIBS
    "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/PlugIns/libdashboard_pi.dylib"
    "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/PlugIns/libgrib_pi.dylib"
    "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/PlugIns/libchartdldr_pi.dylib"
    "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/PlugIns/libwmm_pi.dylib"
  )
  install(CODE "
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\"   \"${LIBS}\"   \"${DIRS}\")
    " COMPONENT Runtime)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_${PACKAGE_VERSION}.dmg
    COMMAND
      ${CMAKE_SOURCE_DIR}/buildosx/create-dmg
      --volname "OpenCPN Installer"
      --background ${CMAKE_SOURCE_DIR}/buildosx/background.png
      --window-pos 200 120
      --window-size 500 300
      --icon-size 80
      --icon OpenCPN.app 120 150
      --hide-extension OpenCPN.app
      --app-drop-link
        390 145
        ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_${PACKAGE_VERSION}.dmg
        ${CMAKE_INSTALL_PREFIX}/bin/
    DEPENDS ${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "create-dmg [${PACKAGE_NAME}]: Generated dmg file."
  )
  add_custom_target(
    create-dmg
    COMMENT "create-dmg: Done."
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_${PACKAGE_VERSION}.dmg
  )
endif (APPLE)

include(CPack)

# Dummy target necessary to make sure the ADDITIONAL_MAKE_CLEAN_FILES directive
# is executed. Apparently, the base CMakeLists.txt file must have "some" target
# to activate all the clean steps.
add_custom_target(dummy COMMENT "dummy: Done." DEPENDS ${PACKAGE_NAME})

set_directory_properties(
  dummy ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_SOURCE_DIR}/include/config.h
)

if (MSVC)
  add_custom_command(
    TARGET ${PACKAGE_NAME} POST_BUILD
    COMMAND
    cmake -E copy "${CMAKE_BINARY_DIR}/$<CONFIG>/opencpn.pdb"
    "${CMAKE_BINARY_DIR}/$<CONFIG>/${PACKAGE_NAME}-${PACKAGE_VERSION}.pdb"
  )
endif (MSVC)
