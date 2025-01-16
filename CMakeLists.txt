#[[
Author:    Dave Register
DEB config from: antonm - Anton Martchukov <anton@martchukov.com>
Update:   sethdart (Jean-Eudes Onfray) with parts from balp (Anders Arnholm)
#]]


#[[
Copyright (C) 2010 by David S. Register
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.
#]]

#[[
TODO:
- Profiling opt
USE_GLU_TESS
USE_GLU_DLL
#]]

cmake_minimum_required(VERSION 3.15.0)
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

if ("${OCPN_TARGET_TUPLE}" MATCHES "Android")
  message(STATUS "Building for wxQt-Android")

  # Specific NDK setting, avoiding "Unwind" linkage problems with
  # managed plugins found in later NDK versions.
  #set( tool_base /home/dsr/Android/Sdk/ndk/22.1.7171670/toolchains/llvm/prebuilt/linux-x86_64)
  #cmake -DOCPN_TARGET_TUPLE:STRING="Android-arm64;16;arm64"   -Dtool_base="$HOME/Android/Sdk/ndk/22.1.7171670/toolchains/llvm/prebuilt/linux-x86_64" ..
  #cmake -DOCPN_TARGET_TUPLE:STRING="Android-armhf;16;armhf"   -Dtool_base="$HOME/Android/Sdk/ndk/22.1.7171670/toolchains/llvm/prebuilt/linux-x86_64" ..

  set(CMAKE_TOOLCHAIN_FILE buildandroid/build_android.cmake)
  set(QT_ANDROID "ON")
  if ("${OCPN_TARGET_TUPLE}" MATCHES "Android-arm64")
    add_definitions("-DANDROID_ARM64")
  else ()
    add_definitions("-DANDROID_ARMHF")
  endif ()
endif ()

if (APPLE)
  set(PACKAGE_NAME OpenCPN)
  string(REPLACE ";" "," CMAKE_OSX_ARCHITECTURES_COMMA_SEP "${CMAKE_OSX_ARCHITECTURES}")
elseif (QT_ANDROID)
  set(PACKAGE_NAME gorp)
else ()
  set(PACKAGE_NAME opencpn)
endif ()

project(OpenCPN)

if (NOT DEFINED OCPN_VERBOSE OR OCPN_VERBOSE)
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif ()

message(STATUS "cmake version: ${CMAKE_VERSION}")

# Locations where cmake looks for cmake modules.
set(
  CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/build ${CMAKE_SOURCE_DIR}/ ${CMAKE_SOURCE_DIR}/cmake
)

include(FindPkgConfig)
include(CMakeDependentOption)
include(CheckSymbolExists)

#
# Options
#
macro (BUNDLE_DOCS_OPT onoff)
  option(OCPN_BUNDLE_DOCS "Include documentation in package" ${onoff})
endmacro ()

macro (BUNDLE_TCDATA_OPT onoff)
  option(OCPN_BUNDLE_TCDATA "Include tide/current harmonics data in package"
         ${onoff}
  )
endmacro ()

macro (BUNDLE_GSHHS_OPT onoff)
  option(OCPN_BUNDLE_GSHHS "Bundle minimal GSHHS data files" ${onoff})
endmacro ()

# If on Windows or Apple, build the monolithic package, to override use a non-
# sense values from the commandline (due to cmake inability to distinguish
# between not set and set to FALSE): cmake -DOCPN_BUNDLE_DOCS=BLAH
# -DOCPN_BUNDLE_TCDATA=BLAH

# Bundle tcdata and gshhs by default also on linux, to keep the defaults close
# to the official Debian package.
bundle_gshhs_opt("ON")
bundle_tcdata_opt("ON")

if (APPLE OR WIN32)
  bundle_docs_opt("ON")
else ()
  bundle_docs_opt("OFF")
endif ()

option(OCPN_VERBOSE "Make verbose builds" ON)
option(OCPN_PEDANTIC "Enable more compiler warnings" OFF)

set(OCPN_PACKAGE_RELEASE
  "1" CACHE STRING "Package release number"
)

option(OCPN_USE_GL "Enable OpenGL support" ON)
option(OCPN_USE_GLES_SHIM "Enable GLESv1 Shim library" OFF)
option(OCPN_USE_UDEV_PORTS "Use udev to enumerate serial ports" ON)
option(OCPN_USE_SYSFS_PORTS "Use sysfs to enumerate serial ports" OFF)
option(OCPN_ENABLE_MDNS_DEBUG "Enable mDNS debug output" OFF)

set(OCPN_TARGET_TUPLE "" CACHE STRING
    "List of target;version;arch like flatpak;20.08;aarch64 [\"\"]"
)

if (MSVC OR MINGW)
  option(OCPN_USE_CRASHREPORT "Enable crash reporting" ON)
else ()
  set(OCPN_USE_CRASHREPORT FALSE)
endif ()

if (WIN32 OR APPLE OR QT_ANDROID)
  set(PA_DEFAULT "OFF")
else ()
  set(PA_DEFAULT "ON")
endif ()
option(OCPN_ENABLE_PORTAUDIO "Use portaudio(3) to play sounds if available"
       ${PA_DEFAULT}
)
cmake_dependent_option(
  OCPN_ENABLE_SNDFILE "Use libsndfile for portaudio if available." ON
  "OCPN_ENABLE_PORTAUDIO" ON
)

option(OCPN_USE_GARMINHOST "Enable Garmin Host Mode support" ON)

if (QT_ANDROID)
  set(OCPN_BUILD_TEST OFF)
else ()
  option(OCPN_BUILD_TEST "Enable test subproject build" ON)
endif ()

execute_process(
  COMMAND "ip" "address" "show" "vcan0"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  OUTPUT_VARIABLE _VCAN_STDOUT
  RESULT_VARIABLE _VCAN_RESULT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if ("${_VCAN_RESULT}" EQUAL 0)
  set(ENABLE_VCAN_TESTS ON)
  message(STATUS "tests using vcan0 are available.")
else ()
  set(ENABLE_VCAN_TESTS OFF)
  message(STATUS "tests using vcan0 are NOT available: \"${_VCAN_STDERR}\"")
endif ()

set(OCPN_WXWIDGETS_FORCE_VERSION
    CACHE STRING "Force usage of a specific wxWidgets version."
)

set(OCPN_WXWIDGETS_OPTIONS CACHE STRING
    "Additional flags to wxWidgets_CONFIG_OPTIONS"
)

if (WIN32 AND NOT UNIX)
  option(OCPN_BUNDLE_WXDLLS "Bundle the prebuilt WX DLLs" ON)
  option(OCPN_BUNDLE_VCDLLS "Bundle the VC redistributable libraries" ON)
  option(BUNDLE_LIBARCHIVEDLLS
         "Bundle the prebuilt LibArchive and LibLZMA DLLs" ON
  )
endif ()

option(OCPN_ENABLE_CLANG_TIDY "Add clang-tidy automatically to builds" OFF)
set(ENABLE_SANITIZER "none" CACHE STRING "Add clang sanitizer to the build")

option(OCPN_ENABLE_CPPCHECK "Add cppcheck automatically to builds" OFF)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose type of build: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE
  )
  # Set the possible values of build type for cmake-gui
  set_property(
    CACHE CMAKE_BUILD_TYPE
    PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
  )
endif ()

if (APPLE)
  option(OCPN_USE_LIBCPP "Use libc++ instead of libstdc++ on macOS" ON)
  option(
    OCPN_USE_DEPS_BUNDLE
    "Use prebuilt dependency bundle instead of searching for libraries on macOS"
    OFF
  )
  if (NOT OCPN_DEPS_BUNDLE_PATH)
    set(OCPN_DEPS_BUNDLE_PATH "/usr/local")
  endif ()
endif ()

option(OCPN_USE_EXTERN_CURL "Use external libcurl" OFF)

if (NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${TENTATIVE_PREFIX})
endif ()

if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(BUNDLED_LIBS_DEFAULT "ON")
else ()
  set(BUNDLED_LIBS_DEFAULT "OFF")
endif ()

set(OCPN_USE_BUNDLED_LIBS ${BUNDLED_LIBS_DEFAULT} CACHE STRING
    "Use bundled libraries instead of system's [ON, OFF or comma-sep list]"
)
set(OCPN_RELEASE "" CACHE STRING
    "Hardcoded value for build number when using OCPN_CI_BUILD"
)
option(OCPN_USE_NEWSERIAL "Use new serial communication implementation" ON)
option(OCPN_USE_CURL "Use Curl libraries" ON)
option(OCPN_USE_SVG "Use SVG graphics via wxSVG or wxBitmapBundle" ON)
option(OCPN_USE_WEBVIEW "Use wxWidget's webview addon if available" ON)
option(OCPN_USE_LZMA "Use LZMA for chart compression" ON)
option(OCPN_CI_BUILD "Use CI build versioning rules" OFF)
option(OCPN_USE_SYSTEM_LIBARCHIVE
       "Use the libarchive version provided by the system on MacOS" ON
)

add_definitions(-D__OCPN_USE_GLEW__)

if (WIN32)
  set(cmd_snd_default OFF)
else ()
  set(cmd_snd_default ON)
endif ()

option(OCPN_ENABLE_SYSTEM_CMD_SOUND
       "Use aplay(1), afplay(1) etc. to play sounds if available"
       ${cmd_snd_default}
)
set(OCPN_SOUND_CMD "" CACHE STRING
    "Hardcoded value for CLI command used in SystemCmdSound backend"
)

if (UNIX AND NOT APPLE AND NOT QT_ANDROID)
  set(LINUX "1")
endif ()

if (QT_ANDROID)
  set(OCPN_NEW_SERIAL OFF)
  set(OCPN_USE_NEWSERIAL OFF)
  set(OCPN_USE_CURL ON)
  set(OCPN_USE_LZMA OFF)
endif ()

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
endmacro ()

#
# Language, compiler and static checkers setup
#
if (NOT DEFINED ENV{ARCH})
  include(GetArch)
  getarch()
else ()
  set(ARCH $ENV{ARCH})
endif ()
message(STATUS "*** Host Build Architecture is ${ARCH}")

set(CMAKE_CXX_STANDARD 17)
message(STATUS "Setting C++17 standard via cmake standard mechanism")
set(CMAKE_CXX_VISIBILITY_PRESET=hidden)
if (NOT MSVC)
  set(OBJ_VISIBILITY "-fvisibility=hidden")
endif ()

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
    set(CMAKE_CXX_CLANG_TIDY
        "${CLANG_TIDY_EXE};-checks=${CLANG_TIDY_CHECKS};-header-filter='${CMAKE_SOURCE_DIR}/*'"
        CACHE STRING "" FORCE
    )
  else ()
    message(AUTHOR_WARNING "clang-tidy not found!")
    set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE) # delete it
  endif ()
endif ()

# Add support for address etc sanitizers, part 1/2 (other half after
# ADD_EXECUTABLE)
set_property(
  CACHE ENABLE_SANITIZER
  PROPERTY STRINGS none address memory thread undefined
)
if (NOT "${ENABLE_SANITIZER}" MATCHES "none")
  add_compile_options(-fsanitize=${ENABLE_SANITIZER})
endif ()

if (OCPN_ENABLE_CPPCHECK)
  find_program(CPPCHECK_EXECUTABLE NAMES "cppcheck")
  set(CMAKE_CXX_CPPCHECK ${CPPCHECK_EXECUTABLE})
endif ()
# ADD_COMPILE_OPTIONS( "-Wall" "-ansi" "-pedantic" "-Wno-variadic-macros" )
# TODO: Should we use  -fno-stack-protector IF NOT DEBUGGING CFLAGS="-O2
# -march=native"
if (NOT WIN32 AND NOT APPLE)
  add_compile_options("-Wall")
  if (NOT OCPN_PEDANTIC)
    add_compile_options(
      "-Wno-unused" "-fexceptions" "-rdynamic" "-fno-strict-aliasing"
      "-Wno-deprecated-declarations"
    )
  endif ()
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")
    #if (CMAKE_BUILD_TYPE MATCHES "Debug")
      string(APPEND CMAKE_CXX_FLAGS_DEBUG " -O0")
      add_compile_options("-O0")
    #endif ()
  endif ()
  add_definitions(" -DPREFIX=\\\"${CMAKE_INSTALL_PREFIX}\\\"")
  # profiling with gprof ADD_COMPILE_OPTIONS( -pg )
  # SET(CMAKE_EXE_LINKER_FLAGS -pg)
  # profiling with gcov ADD_COMPILE_OPTIONS( "-fprofile-arcs -ftest- # coverage" )
endif ()

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
      "-Wno-unknown-warning-option"
      "-D_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_"
    )
  endif ()
endif ()

if (APPLE)
  set(CMAKE_C_FLAGS "-O2 -arch ${ARCH}")
  set(CMAKE_C_FLAGS_DEBUG "-g -O0 -arch ${ARCH}")
  set(CMAKE_C_FLAGS_MINSIZEREL "-O2 -arch ${ARCH}")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -arch ${ARCH}")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -arch ${ARCH}")

  if (OCPN_USE_LIBCPP)
    set(OCPN_LIBCPP "-stdlib=libc++")
  endif ()
  set(CMAKE_CXX_FLAGS "-O2 ${OCPN_LIBCPP} -arch ${ARCH}")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -gdwarf-2 -O0 ${OCPN_LIBCPP} -arch ${ARCH}")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "-O2 ${OCPN_LIBCPP} -arch ${ARCH}")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 ${OCPN_LIBCPP} -arch ${ARCH}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
      "-g -gdwarf-2 -O3 ${OCPN_LIBCPP} -arch ${ARCH}"
  )
  set(CMAKE_EXE_LINKER_FLAGS
      "-O2 ${OCPN_LIBCPP} -arch ${ARCH} ${CMAKE_EXE_LINKER_FLAGS} -framework CoreGraphics"
  )
  set(CMAKE_SHARED_LINKER_FLAGS "-O2 ${OCPN_LIBCPP} -arch ${ARCH}")
  set(CMAKE_MODULE_LINKER_FLAGS "-O2 ${OCPN_LIBCPP} -arch ${ARCH}")
endif ()

if (MSVC)
  add_definitions(-D__MSVC__)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
  add_definitions(-DPSAPI_VERSION=1)
  add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS=1)
  # https://developercommunity.visualstudio.com
  # /t/error-c2872-byte-ambiguous-symbol/93889 https://shorturl.at/hmJSZ
  add_definitions(-D_HAS_STD_BYTE=0)
endif ()

if (MSVC)
  # /D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR works around https://github.com/actions/runner-images/issues/10004
  set(CMAKE_C_FLAGS_DEBUG "/MP /MDd /Ob0 /Od  /D_DEBUG  /Zi /RTC1")
  set(CMAKE_C_FLAGS_MINSIZEREL "/MP /MD  /O1  /Ob1 /D NDEBUG")
  set(CMAKE_C_FLAGS_RELEASE "/MP /MD  /O2  /Ob2 /D NDEBUG /Zi /wd4068")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "/MP /MD  /O2  /Ob1 /D NDEBUG /Zi /wd4068")
  set(CMAKE_CXX_FLAGS_DEBUG "/MP /MDd /Ob0 /Od  /D_DEBUG  /Zi /RTC1 /EHa /D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "/MP /MD  /O1  /Ob1 /D NDEBUG /EHa /D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR")
  set(CMAKE_CXX_FLAGS_RELEASE "/MP /MD  /O2  /Ob2 /D NDEBUG /Zi /EHa /wd4068 /D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
      "/MP /MD  /O2  /Ob1 /D NDEBUG /Zi /EHa /wd4068 /D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR"
  )
  set(CMAKE_EXE_LINKER_FLAGS "/DEBUG /Release ${CMAKE_EXE_LINKER_FLAGS}")
endif ()

if (QT_ANDROID)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
  add_definitions(-D__WXQT__)
  add_definitions(-D__OCPN__ANDROID__)
  add_definitions(-DANDROID)
  add_definitions(-DNOASSERT)
  set(CMAKE_CXX_FLAGS "-pthread -fPIC -g -O2")
  set(CMAKE_C_FLAGS "-pthread -fPIC -g -O2")

  add_compile_options(
    "-Wno-inconsistent-missing-override"
    "-Wno-potentially-evaluated-expression" "-Wno-overloaded-virtual"
    "-Wno-unused-command-line-argument" "-Wno-unknown-pragmas"
  )
  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_options("-O0")
  endif ()
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-soname,libgorp.so -Wl,--build-id")
endif ()

if (ARCH STREQUAL armhf)
  # See: https://forums.raspberrypi.com/viewtopic.php?t=144115
  add_compile_options(-mcpu=cortex-a53  -mfpu=neon-fp-armv8)
endif ()   # Assuming at least RPi 3

message(STATUS "Final compiler options:")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_MINSIZEREL: ${CMAKE_CXX_FLAGS_MINSIZEREL}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(
  STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
)

# Per default, use CMAKE_INSTALL_LIBDIR=lib on Debian (and derivatives) to avoid
# multilib plugin paths. After all, we do not install any "real" libraries.
if (EXISTS /etc/debian_version AND NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR "lib")
endif ()
include(GNUInstallDirs)

set(PREFIX_DATA ${CMAKE_INSTALL_DATADIR})
set(PREFIX_PKGDATA ${CMAKE_INSTALL_DATADIR}/${PACKAGE_NAME})
set(PREFIX_LIB "${CMAKE_INSTALL_FULL_LIBDIR}")

#
# Version handling
#
include(${CMAKE_SOURCE_DIR}/VERSION.cmake)

string(TIMESTAMP VERSION_DATE "%Y-%m-%d")
string(TIMESTAMP UNIX_TIMESTAMP "%s")
if (OCPN_CI_BUILD)
  include(Utils)
  commit_id(COMMIT)
  build_num(BUILD_NUMBER)
  if (NOT "${OCPN_RELEASE}" STREQUAL "")
    set(BUILD_NUMBER ${OCPN_RELEASE})
  endif ()
  set(VERSION_TAIL "+${COMMIT}")
  if (NOT "${BUILD_NUMBER}" STREQUAL "")
    set(VERSION_TAIL "-${BUILD_NUMBER}${VERSION_TAIL}")
  endif ()
endif ()

set(PACKAGE_VERSION
    "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_TAIL}"
)
message(STATUS "Building version ${PACKAGE_VERSION}")

# Detect the Apple Version of the build machine
if (APPLE)
  exec_program(uname ARGS -v OUTPUT_VARIABLE DARWIN_VERSION)
  string(REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
  message(STATUS "*** Building on DARWIN_VERSION=${DARWIN_VERSION}")
  if (DARWIN_VERSION GREATER 11)
    set(APPLE_MODERN 1 INTERNAL)
  endif ()
endif ()

message(STATUS "*** Staging to build ${PACKAGE_NAME} ${PACKAGE_VERSION} ***")

#
# Bundled data: docs, tcdata
#
if (OCPN_BUNDLE_DOCS)
  message(STATUS "*** Package will include documentation ***")
else (OCPN_BUNDLE_DOCS)
  message(STATUS "*** Package will NOT include documentation ***")
endif (OCPN_BUNDLE_DOCS)

#
# Linux: set up GTK (wxWidgets prerequisite).
#
if (LINUX)
  include(OcpnFindGtk)
endif ()

# set a build flag for arm architecture, to catch any rPI runtime changes
# required
if (ARCH MATCHES "arm*" AND (NOT QT_ANDROID))
  add_definitions(-DocpnARM)
  if (ARCH STREQUAL "armhf")
    add_definitions(-DocpnARMHF)
  endif ()
endif ()

if (DEFINED _wx_selected_config)
  if (_wx_selected_config MATCHES "androideabi-qt")
    # MESSAGE (STATUS "Qt_Base: " ${Qt_Base}) MESSAGE (STATUS "wxQt_Base/Build:
    # " ${wxQt_Base} "/" ${wxQt_Build}) ADD_DEFINITIONS(-DocpnUSE_GLES)
    # ADD_DEFINITIONS(-DocpnUSE_GL)

    # SET(OPENGLES_FOUND "YES") SET(OPENGL_FOUND "YES")

    # ADD_DEFINITIONS(-DUSE_GLU_TESS) SET(USE_GLES2 ON ) MESSAGE (STATUS "Using
    # GLESv2 for Android") ADD_DEFINITIONS(-DUSE_ANDROID_GLES2)
    # ADD_DEFINITIONS(-DUSE_GLSL)
    include_directories(${CMAKE_SOURCE_DIR}/libs/glshim/include/GLES)
  endif ()
endif ()

if (APPLE)
  # Bundle all of wxWidgets as a courtesy for plugins, see #2153
  set(wxWidgets_FIND_COMPONENTS
      net xml html adv aui core base media propgrid qa ribbon richtext webview
      stc xrc
  )
elseif (_wx_selected_config MATCHES "qt-armv7")
  set(wxWidgets_FIND_COMPONENTS base core xml html adv aui)
elseif (OCPN_USE_WEBVIEW)
  set(wxWidgets_FIND_COMPONENTS net xml html adv aui core base richtext webview)
else ()
  set(wxWidgets_FIND_COMPONENTS net xml html adv aui core base richtext)
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
    add_definitions(-DocpnUSE_GLSL)

    # We need to remove GLU from the OPENGL_LIBRARIES list
    foreach (_currentLibFile ${OPENGL_LIBRARIES})
      # MESSAGE (STATUS "    Lib File: " ${_currentLibFile})
      set(UCNAME ${_currentLibFile})
      string(TOUPPER ${UCNAME} UCNAME)
      if (NOT ${UCNAME} MATCHES "(.*)GLU(.*)")
        set(REVISED_OPENGL_LIBRARIES ${_currentLibFile}
                                     ${REVISED_OPENGL_LIBRARIES}
        )
      endif ()
    endforeach ()

    set(OPENGL_LIBRARIES ${REVISED_OPENGL_LIBRARIES})
    message(STATUS "    Revised GL Lib: " ${OPENGL_LIBRARIES})
  else ()
    message(STATUS "OpenGL not found...")
  endif ()
endif ()

# configure GLEW
if (OPENGL_FOUND AND CMAKE_HOST_WIN32)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cache/buildwin/include/glew)
  link_libraries(${CMAKE_SOURCE_DIR}/cache/buildwin/glew32.lib)
elseif (OPENGL_FOUND AND NOT APPLE)
  find_package(GLEW REQUIRED)
  include_directories(${GLEW_INCLUDE_DIRS})
  link_libraries(${GLEW_LIBRARIES})
endif ()


if (NOT QT_ANDROID)
  # Find wxWidgets here, and the setting get inherited by all plugins. These
  # options can be used to set the linux widgets build type
  set(wxWidgets_USE_DEBUG OFF)
  set(wxWidgets_USE_UNICODE ON)
  set(wxWidgets_USE_UNIVERSAL OFF)
  set(wxWidgets_USE_STATIC OFF)

  if (OCPN_WXWIDGETS_FORCE_VERSION)
    set(wxWidgets_CONFIG_OPTIONS --version=${OCPN_WXWIDGETS_FORCE_VERSION})
  endif ()

  if (MSVC)
    # Exclude wxexpat.lib, since we use our own version. Other things are
    # excluded as well, but we don't need them
    set(wxWidgets_EXCLUDE_COMMON_LIBRARIES TRUE)
  endif ()

  if (GTK3_FOUND)
    set(wxWidgets_CONFIG_OPTIONS ${wxWidgets_CONFIG_OPTIONS} --toolkit=gtk3)
  endif ()

  # find_package(wxWidgets COMPONENTS ${wxWidgets_FIND_COMPONENTS} REQUIRED )
  if (APPLE AND OCPN_USE_DEPS_BUNDLE)
    set(wxWidgets_CONFIG_EXECUTABLE
        "${OCPN_DEPS_BUNDLE_PATH}/lib/wx/config/osx_cocoa-unicode-3.2"
    )
    set(wxWidgets_CONFIG_OPTIONS "--prefix=${OCPN_DEPS_BUNDLE_PATH}")
  endif ()
  find_package(wxWidgets COMPONENTS ${wxWidgets_FIND_COMPONENTS})
  if (wxWidgets_FOUND AND OCPN_USE_WEBVIEW)
    message(STATUS "Found wxWidgets webview add-on")
    add_definitions(-DwxUSE_WEBVIEW=1 -DHAVE_WEBVIEW)
  else ()
    add_definitions(-DwxUSE_WEBVIEW=0)
    list(REMOVE_ITEM wxWidgets_FIND_COMPONENTS webview)
    message(STATUS "Could not find wxWidgets webview add-on")
    find_package(wxWidgets COMPONENTS ${wxWidgets_FIND_COMPONENTS})
  endif ()
  if (NOT wxWidgets_FOUND)
    message(FATAL_ERROR "Cannot find wxWidgets, giving up.")
  endif ()
  if (MSYS)
    # Convert msys to windows paths, and handle the missing /usr
    string(REGEX REPLACE "/usr/local" ";C:/MinGW/msys/1.0/local"
                         wxWidgets_INCLUDE_DIRS "${wxWidgets_INCLUDE_DIRS}"
    )
  endif ()
  include(${wxWidgets_USE_FILE})

  message(STATUS "Found wxWidgets...")
  message(STATUS " wxWidgets Include: ${wxWidgets_INCLUDE_DIRS}")
  message(STATUS " wxWidgets Libraries: ${wxWidgets_LIBRARIES}")

  # We need to remove GLU from the wxWidgets_LIBRARIES list It only appears to
  # get on the list for MSW...
  foreach (_currentLibFile ${wxWidgets_LIBRARIES})
    set(UCNAME ${_currentLibFile})
    string(TOUPPER ${UCNAME} UCNAME)
    if (NOT ${UCNAME} MATCHES "(.*)GLU(.*)")
      set(REVISED_wxWidgets_LIBRARIES
          ${REVISED_wxWidgets_LIBRARIES} ${_currentLibFile}
      )
    endif ()
  endforeach ()
  set(wxWidgets_LIBRARIES ${REVISED_wxWidgets_LIBRARIES})

  message(STATUS " Revised wxWidgets Libraries: ${wxWidgets_LIBRARIES}")
endif (NOT QT_ANDROID)

# Handle #1500: Force NDEBUG on gtk3 for wxwidgets < 3.2 (all builds)
if (GTK3_FOUND)
  set(_VERSPROG
      [=[
    #include <stdio.h>
    #include <wx/version.h>
    int main(int argc, char**argv) {
      printf("%d\\n", wxMAJOR_VERSION * 10 + wxMINOR_VERSION);
    }
  ]=]
  )
  set(TRY_COMPILE
      "echo '${_VERSPROG}' > wx-32.c; cc $(${wxWidgets_CONFIG_EXECUTABLE} --cflags) wx-32.c; ./a.out"
  )
  execute_process(
    COMMAND "/bin/sh" "-c" "${TRY_COMPILE}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    OUTPUT_VARIABLE _WX_VERSION
    ERROR_VARIABLE _WX_ERRORS
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (NOT "${_WX_ERRORS}" STREQUAL "" OR "${_WX_VERSION}" STREQUAL "")
    message(FATAL_ERROR "Cannot run wxWidgets  version test (!)")
  elseif ("${_WX_VERSION}" LESS 32)
    add_definitions("-DNDEBUG")
    message(STATUS "Forcing -NDEBUG on gtk3 build (#1500).")
  endif ()
endif ()

# Make sure that the variable "wxWidgets_VERSION_STRING" is set, even on older
# CMake versions Seems safe to set to "3.0", since OCPN does not support earlier
# wxWidgets versions
if (wxWidgets_VERSION_STRING)
  message(STATUS "Found wxWidgets_VERSION_STRING: ${wxWidgets_VERSION_STRING}")
else ()
  message(
    STATUS "Forcing wxWidgets_VERSION_STRING: ${wxWidgets_VERSION_STRING}"
  )
  set(wxWidgets_VERSION_STRING "3.0")
endif ()

# Enable gestures on all platforms except MacOS There are no touch capable MacOS
# machines
if ((wxWidgets_VERSION_STRING VERSION_GREATER "3.1.0") AND (NOT APPLE))
  add_definitions("-DHAVE_WX_GESTURE_EVENTS")
endif ()

message(STATUS "")
message(STATUS "*** Staging to build ${PACKAGE_NAME}  ***")
message(STATUS "*** Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "*** Will install to ${CMAKE_INSTALL_PREFIX}  ***")

set(GUI_HDR_DIR ${CMAKE_SOURCE_DIR}/gui/include/gui)
set(GUI_HDRS
    ${GUI_HDR_DIR}/AboutFrame.h
    ${GUI_HDR_DIR}/AboutFrameImpl.h
    ${GUI_HDR_DIR}/about.h
    ${GUI_HDR_DIR}/ais.h
    ${GUI_HDR_DIR}/ais_info_gui.h
    ${GUI_HDR_DIR}/AISTargetAlertDialog.h
    ${GUI_HDR_DIR}/AISTargetListDialog.h
    ${GUI_HDR_DIR}/AISTargetQueryDialog.h
    ${GUI_HDR_DIR}/CanvasConfig.h
    ${GUI_HDR_DIR}/canvasMenu.h
    ${GUI_HDR_DIR}/CanvasOptions.h
    ${GUI_HDR_DIR}/catalog_mgr.h
    ${GUI_HDR_DIR}/cat_settings.h
    ${GUI_HDR_DIR}/chartbase.h
    ${GUI_HDR_DIR}/chart_ctx_factory.h
    ${GUI_HDR_DIR}/chartdb.h
    ${GUI_HDR_DIR}/chartdbs.h
    ${GUI_HDR_DIR}/chartimg.h
    ${GUI_HDR_DIR}/chcanv.h
    ${GUI_HDR_DIR}/ChInfoWin.h
    ${GUI_HDR_DIR}/color_handler.h
    ${GUI_HDR_DIR}/comm_overflow_dlg.h
    ${GUI_HDR_DIR}/compass.h
    ${GUI_HDR_DIR}/concanv.h
    ${GUI_HDR_DIR}/ConfigMgr.h
    ${GUI_HDR_DIR}/connection_edit.h
    ${GUI_HDR_DIR}/connections_dlg.h
    ${GUI_HDR_DIR}/conn_params_panel.h
    ${GUI_HDR_DIR}/DetailSlider.h
    ${GUI_HDR_DIR}/displays.h
    ${GUI_HDR_DIR}/download_mgr.h
    ${GUI_HDR_DIR}/dychart.h
    ${GUI_HDR_DIR}/emboss_data.h
    ${GUI_HDR_DIR}/FlexHash.h
    ${GUI_HDR_DIR}/FontDesc.h
    ${GUI_HDR_DIR}/FontMgr.h
    ${GUI_HDR_DIR}/GoToPositionDialog.h
    ${GUI_HDR_DIR}/gshhs.h
    ${GUI_HDR_DIR}/gui_lib.h
    ${GUI_HDR_DIR}/hotkeys_dlg.h
    ${GUI_HDR_DIR}/IDX_entry.h
    ${GUI_HDR_DIR}/iENCToolbar.h
    ${GUI_HDR_DIR}/kml.h
    ${GUI_HDR_DIR}/Layer.h
    ${GUI_HDR_DIR}/LinkPropDlg.h
    ${GUI_HDR_DIR}/load_errors_dlg.h
    ${GUI_HDR_DIR}/MarkInfo.h
    ${GUI_HDR_DIR}/mbtiles.h
    ${GUI_HDR_DIR}/MUIBar.h
    ${GUI_HDR_DIR}/n0183_ctx_factory.h
    ${GUI_HDR_DIR}/navutil.h
    ${GUI_HDR_DIR}/NMEALogWindow.h
    ${GUI_HDR_DIR}/ocpCursor.h
    ${GUI_HDR_DIR}/ocpndc.h
    ${GUI_HDR_DIR}/ocpn_app.h
    ${GUI_HDR_DIR}/ocpn_frame.h
    ${GUI_HDR_DIR}/OCPNListCtrl.h
    ${GUI_HDR_DIR}/ocpn_pixel.h
    ${GUI_HDR_DIR}/OCPNPlatform.h
    ${GUI_HDR_DIR}/ocpn_print.h
    ${GUI_HDR_DIR}/OCPN_AUIManager.h
    ${GUI_HDR_DIR}/OCPNRegion.h
    ${GUI_HDR_DIR}/options.h
    ${GUI_HDR_DIR}/piano.h
    ${GUI_HDR_DIR}/peer_client_dlg.h
    ${GUI_HDR_DIR}/pluginmanager.h
    ${GUI_HDR_DIR}/printtable.h
    ${GUI_HDR_DIR}/priority_gui.h
    ${GUI_HDR_DIR}/Quilt.h
    ${GUI_HDR_DIR}/rest_server_gui.h
    ${GUI_HDR_DIR}/RolloverWin.h
    ${GUI_HDR_DIR}/route_ctx_factory.h
    ${GUI_HDR_DIR}/route_gui.h
    ${GUI_HDR_DIR}/routemanagerdialog.h
    ${GUI_HDR_DIR}/routeman_gui.h
    ${GUI_HDR_DIR}/route_point_gui.h
    ${GUI_HDR_DIR}/route_validators.h
    ${GUI_HDR_DIR}/routeprintout.h
    ${GUI_HDR_DIR}/RoutePropDlg.h
    ${GUI_HDR_DIR}/RoutePropDlgImpl.h
    ${GUI_HDR_DIR}/s57chart.h
    ${GUI_HDR_DIR}/S57Light.h
    ${GUI_HDR_DIR}/S57ObjectDesc.h
    ${GUI_HDR_DIR}/s57_ocpn_utils.h
    ${GUI_HDR_DIR}/S57QueryDialog.h
    ${GUI_HDR_DIR}/S57Sector.h
    ${GUI_HDR_DIR}/safe_mode_gui.h
    ${GUI_HDR_DIR}/SendToGpsDlg.h
    ${GUI_HDR_DIR}/SendToPeerDlg.h
    ${GUI_HDR_DIR}/Station_Data.h
    ${GUI_HDR_DIR}/styles.h
    ${GUI_HDR_DIR}/svg_utils.h
    ${GUI_HDR_DIR}/TCDataFactory.h
    ${GUI_HDR_DIR}/TCDataSource.h
    ${GUI_HDR_DIR}/TCDS_Ascii_Harmonic.h
    ${GUI_HDR_DIR}/TCDS_Binary_Harmonic.h
    ${GUI_HDR_DIR}/TC_Error_Code.h
    ${GUI_HDR_DIR}/tcmgr.h
    ${GUI_HDR_DIR}/TCWin.h
    ${GUI_HDR_DIR}/thumbwin.h
    ${GUI_HDR_DIR}/tide_time.h
    ${GUI_HDR_DIR}/timers.h
    ${GUI_HDR_DIR}/time_textbox.h
    ${GUI_HDR_DIR}/toolbar.h
    ${GUI_HDR_DIR}/track_gui.h
    ${GUI_HDR_DIR}/trackprintout.h
    ${GUI_HDR_DIR}/TrackPropDlg.h
    ${GUI_HDR_DIR}/TTYScroll.h
    ${GUI_HDR_DIR}/TTYWindow.h
    ${GUI_HDR_DIR}/udev_rule_mgr.h
    ${GUI_HDR_DIR}/undo.h
    ${GUI_HDR_DIR}/update_mgr.h
    ${GUI_HDR_DIR}/usb_devices.h
    ${GUI_HDR_DIR}/viewport.h
    ${GUI_HDR_DIR}/waypointman_gui.h
    ${GUI_HDR_DIR}/WindowDestroyListener.h
    ${GUI_HDR_DIR}/WindowDestroyListener.h
)
set(GUI_SRC_DIR ${CMAKE_SOURCE_DIR}/gui/src)
set(GUI_SRC
    ${GUI_SRC_DIR}/about.cpp
    ${GUI_SRC_DIR}/AboutFrame.cpp
    ${GUI_SRC_DIR}/AboutFrameImpl.cpp
    ${GUI_SRC_DIR}/ais.cpp
    ${GUI_SRC_DIR}/ais_info_gui.cpp
    ${GUI_SRC_DIR}/AISTargetAlertDialog.cpp
    ${GUI_SRC_DIR}/AISTargetListDialog.cpp
    ${GUI_SRC_DIR}/AISTargetQueryDialog.cpp
    ${GUI_SRC_DIR}/CanvasConfig.cpp
    ${GUI_SRC_DIR}/canvasMenu.cpp
    ${GUI_SRC_DIR}/CanvasOptions.cpp
    ${GUI_SRC_DIR}/catalog_mgr.cpp
    ${GUI_SRC_DIR}/cat_settings.cpp
    ${GUI_SRC_DIR}/chartdb.cpp
    ${GUI_SRC_DIR}/chartdbs.cpp
    ${GUI_SRC_DIR}/chartimg.cpp
    ${GUI_SRC_DIR}/chcanv.cpp
    ${GUI_SRC_DIR}/ChInfoWin.cpp
    ${GUI_SRC_DIR}/cm93.cpp
    ${GUI_SRC_DIR}/color_handler.cpp
    ${GUI_SRC_DIR}/comm_overflow_dlg.cpp
    ${GUI_SRC_DIR}/compass.cpp
    ${GUI_SRC_DIR}/concanv.cpp
    ${GUI_SRC_DIR}/ConfigMgr.cpp
    ${GUI_SRC_DIR}/connection_edit.cpp
    ${GUI_SRC_DIR}/connections_dlg.cpp
    ${GUI_SRC_DIR}/conn_params_panel.cpp
    ${GUI_SRC_DIR}/DetailSlider.cpp
    ${GUI_SRC_DIR}/displays.cpp
    ${GUI_SRC_DIR}/download_mgr.cpp
    ${GUI_SRC_DIR}/FlexHash.cpp
    ${GUI_SRC_DIR}/FontDesc.cpp
    ${GUI_SRC_DIR}/FontMgr.cpp
    ${GUI_SRC_DIR}/GoToPositionDialog.cpp
    ${GUI_SRC_DIR}/gshhs.cpp
    ${GUI_SRC_DIR}/gui_lib.cpp
    ${GUI_SRC_DIR}/hotkeys_dlg.cpp
    ${GUI_SRC_DIR}/IDX_entry.cpp
    ${GUI_SRC_DIR}/iENCToolbar.cpp
    ${GUI_SRC_DIR}/kml.cpp
    ${GUI_SRC_DIR}/Layer.cpp
    ${GUI_SRC_DIR}/LinkPropDlg.cpp
    ${GUI_SRC_DIR}/load_errors_dlg.cpp
    ${GUI_SRC_DIR}/MarkInfo.cpp
    ${GUI_SRC_DIR}/mbtiles/mbtiles.cpp
    ${GUI_SRC_DIR}/mbtiles/tile_thread.cpp
    ${GUI_SRC_DIR}/mbtiles/tile_thread.h
    ${GUI_SRC_DIR}/mbtiles/tile_queue.h
    ${GUI_SRC_DIR}/mbtiles/tile_descr.h
    ${GUI_SRC_DIR}/mbtiles/tile_cache.h
    ${GUI_SRC_DIR}/mbtiles/tile_cache.cpp
    ${GUI_SRC_DIR}/MUIBar.cpp
    ${GUI_SRC_DIR}/navutil.cpp
    ${GUI_SRC_DIR}/NMEALogWindow.cpp
    ${GUI_SRC_DIR}/ocpCursor.cpp
    ${GUI_SRC_DIR}/ocpn_app.cpp
    ${GUI_SRC_DIR}/ocpn_plugin_gui.cpp
    ${GUI_SRC_DIR}/OCPN_AUIManager.cpp
    ${GUI_SRC_DIR}/ocpndc.cpp
    ${GUI_SRC_DIR}/ocpn_frame.cpp
    ${GUI_SRC_DIR}/OCPNListCtrl.cpp
    ${GUI_SRC_DIR}/ocpn_pixel.cpp
    ${GUI_SRC_DIR}/OCPNPlatform.cpp
    ${GUI_SRC_DIR}/ocpn_print.cpp
    ${GUI_SRC_DIR}/OCPNRegion.cpp
    ${GUI_SRC_DIR}/options.cpp
    ${GUI_SRC_DIR}/Osenc.cpp
    ${GUI_SRC_DIR}/peer_client_dlg.cpp
    ${GUI_SRC_DIR}/piano.cpp
    ${GUI_SRC_DIR}/pluginmanager.cpp
    ${GUI_SRC_DIR}/printtable.cpp
    ${GUI_SRC_DIR}/priority_gui.cpp
    ${GUI_SRC_DIR}/Quilt.cpp
    ${GUI_SRC_DIR}/rest_server_gui.cpp
    ${GUI_SRC_DIR}/RolloverWin.cpp
    ${GUI_SRC_DIR}/route_gui.cpp
    ${GUI_SRC_DIR}/routemanagerdialog.cpp
    ${GUI_SRC_DIR}/routeman_gui.cpp
    ${GUI_SRC_DIR}/route_point_gui.cpp
    ${GUI_SRC_DIR}/route_validators.cpp
    ${GUI_SRC_DIR}/routeprintout.cpp
    ${GUI_SRC_DIR}/RoutePropDlg.cpp
    ${GUI_SRC_DIR}/RoutePropDlgImpl.cpp
    ${GUI_SRC_DIR}/s57obj.cpp
    ${GUI_SRC_DIR}/s57chart.cpp
    ${GUI_SRC_DIR}/S57QueryDialog.cpp
    ${GUI_SRC_DIR}/safe_mode_gui.cpp
    ${GUI_SRC_DIR}/SencManager.cpp
    ${GUI_SRC_DIR}/SendToGpsDlg.cpp
    ${GUI_SRC_DIR}/SendToPeerDlg.cpp
    ${GUI_SRC_DIR}/shapefile_basemap.cpp
    ${GUI_SRC_DIR}/Station_Data.cpp
    ${GUI_SRC_DIR}/styles.cpp
    ${GUI_SRC_DIR}/svg_utils.cpp
    ${GUI_SRC_DIR}/TCDataFactory.cpp
    ${GUI_SRC_DIR}/TCDataSource.cpp
    ${GUI_SRC_DIR}/TCDS_Ascii_Harmonic.cpp
    ${GUI_SRC_DIR}/TCDS_Binary_Harmonic.cpp
    ${GUI_SRC_DIR}/tcmgr.cpp
    ${GUI_SRC_DIR}/TCWin.cpp
    ${GUI_SRC_DIR}/thumbwin.cpp
    ${GUI_SRC_DIR}/toolbar.cpp
    ${GUI_SRC_DIR}/track_gui.cpp
    ${GUI_SRC_DIR}/trackprintout.cpp
    ${GUI_SRC_DIR}/TrackPropDlg.cpp
    ${GUI_SRC_DIR}/TTYScroll.cpp
    ${GUI_SRC_DIR}/TTYWindow.cpp
    ${GUI_SRC_DIR}/udev_rule_mgr.cpp
    ${GUI_SRC_DIR}/undo.cpp
    ${GUI_SRC_DIR}/update_mgr.cpp
    ${GUI_SRC_DIR}/viewport.cpp
    ${GUI_SRC_DIR}/waypointman_gui.cpp

)

if (NOT QT_ANDROID)
  list(APPEND GUI_HDRS
          ${GUI_HDR_DIR}/wiz_ui_proto.h
          ${GUI_HDR_DIR}/wiz_ui.h
          )
  list(APPEND GUI_SRC
          ${GUI_SRC_DIR}/initwiz/wiz_ui_proto.cpp
          ${GUI_SRC_DIR}/initwiz/wiz_ui.cpp
          )
endif ()

if (OCPN_USE_GL)
  list(APPEND GUI_HDRS${GUI_HDR_DIR}/shaders.h)
  list(APPEND GUI_SRC ${GUI_SRC_DIR}/shaders.cpp)
endif()

if (OCPN_USE_GARMINHOST)
  message(STATUS "GarminHost is enabled.")
  set(USE_GARMINHOST 1) # -> config.h
  list(APPEND MODEL_SRC ${MODEL_SRC_DIR}/garmin_wrapper.cpp)
endif ()

if (LINUX AND OCPN_USE_CRASHREPORT)
  list(APPEND GUI_HDRS ${GUI_HDR_DIR}/crashprint.h)
  list(APPEND GUI_SRC ${GUI_SRC_DIR}/crashprint.cpp)
endif ()

if (QT_ANDROID)
  list(APPEND GUI_HDRS android/androidUTIL.h)
  list(APPEND GUI_SRC ${CMAKE_SOURCE_DIR}/android/androidUTIL.cpp)
endif ()

if (NOT MSVC AND NOT APPLE)
  list(APPEND GUI_HDRS ${GUI_HDR_DIR}/ocpn_fontdlg.h)
  list(APPEND GUI_SRC ${GUI_SRC_DIR}/ocpn_fontdlg.cpp)
endif ()


set(SRCS ${GUI_SRC})
set(HDRS ${GUI_HDRS})

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

if (QT_ANDROID)
  include(libs/AndroidLibs.cmake)
endif ()

add_library(_opencpn INTERFACE) # plugin link target.
target_link_libraries(_opencpn INTERFACE ${PACKAGE_NAME})
target_include_directories(
  _opencpn BEFORE
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/model/include
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/resources
            ${CMAKE_CURRENT_SOURCE_DIR}/gui/src/mbtiles
)
if (MSVC)
  target_include_directories(_opencpn INTERFACE ${CMAKE_SOURCE_DIR}/buildwin)
elseif (APPLE)
  target_include_directories(
    _opencpn INTERFACE ${CMAKE_SOURCE_DIR}/buildosx/include
  )
elseif (QT_ANDROID)
  target_include_directories(_opencpn INTERFACE ${CMAKE_SOURCE_DIR}/android)
endif ()
add_library(ocpn::opencpn ALIAS _opencpn)

set_target_properties(
  ${PACKAGE_NAME}
  PROPERTIES ENABLE_EXPORTS 1
             OUTPUT_NAME ${PACKAGE_NAME}
             ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(
  ${PACKAGE_NAME} BEFORE
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gui/include/gui
          ${CMAKE_CURRENT_SOURCE_DIR}/include
          ${CMAKE_CURRENT_SOURCE_DIR}/resources
          ${CMAKE_CURRENT_SOURCE_DIR}/gui/src/mbtiles
          ${CMAKE_CURRENT_SOURCE_DIR}/gui/src/s57/include
)

if (QT_ANDROID)
  target_include_directories(
    ${PACKAGE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/android
  )
endif ()

if (MSVC AND NOT OCPN_CI_BUILD)
  message(STATUS "PACKAGE_NAME = ${PACKAGE_NAME}")
  set_property(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY VS_STARTUP_PROJECT ${PACKAGE_NAME}
  )
  set_target_properties(
    ${PACKAGE_NAME}
    PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
               "$<TARGET_FILE_DIR:${PACKAGE_NAME}>"
               VS_DEBUGGER_COMMAND_ARGUMENTS "-p"
  )
endif ()

if (APPLE)
  target_include_directories(
    ${PACKAGE_NAME} PRIVATE /usr/X11/include /usr/X11/include/GL
  )
  if (OCPN_USE_DEPS_BUNDLE)
    target_include_directories(
      ${PACKAGE_NAME} PRIVATE "${OCPN_DEPS_BUNDLE_PATH}/include"
    )
    set_target_properties(${PACKAGE_NAME} PROPERTIES
      BUILD_RPATH "${OCPN_DEPS_BUNDLE_PATH}/lib"
      INSTALL_RPATH "${OCPN_DEPS_BUNDLE_PATH}/lib"
  )
  endif ()
endif ()

find_package(Gettext REQUIRED)

add_subdirectory(libs/ssl_sha1)
target_link_libraries(${PACKAGE_NAME} PRIVATE ssl::sha1)

if (OCPN_USE_SYSFS_PORTS)
  set(USE_SYSFS_PORTS 1)
endif ()

if (EXISTS /sys/devices)
  set(HAVE_SYSFS_PORTS 1)
endif ()

if (wxWidgets_VERSION_STRING VERSION_LESS "3.1.6"
        AND NOT QT_ANDROID)
  find_package(PANGO)
  if (PANGO_FOUND)
    target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::pango)
    set(HAVE_PANGO 1)
  endif ()
endif ()

# Some of these "find_path" macros generate "false-positives" under Windows,
# especially if mingw is installed For OpenCPN, these include files and related
# methods are really only valid in complete linux/unix environments So, pending
# further information, we disable these tests for non-UNIX-like builds.
if (UNIX)
  find_path(HAVE_UNISTD_H NAMES unistd.h)
  find_path(HAVE_LIBGEN_H NAMES libgen.h)
  find_path(HAVE_DIRENT_H NAMES dirent.h)
  find_path( HAVE_LINUX_SERIAL_H PATH_SUFFIXES linux NAMES serial.h)
  find_path( HAVE_SYS_STAT_H PATH_SUFFIXES sys NAMES stat.h)
  find_path( HAVE_SYS_IOCTL_H PATH_SUFFIXES sys NAMES ioctl.h)
  find_path(HAVE_FCNTL_H NAMES fcntl.h)
  if (NOT HAVE_FCNTL_H)
    find_path( HAVE_SYS_FCNTL_H PATH_SUFFIXES sys NAMES fcntl.h)
  endif ()
  find_path( HAVE_SYS_TYPES_H PATH_SUFFIXES sys NAMES types.h)
  check_symbol_exists(readlink unistd.h HAVE_READLINK)
endif ()

#
# Linux: set up GTK
#
if (LINUX)
  include(OcpnFindGtk)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::gtk)
endif ()

# Search for opengles, short of running a program to test the speed of
# acceleration, I simply use gles on "native linux" arm systems
if (DEFINED _wx_selected_config)
  message(STATUS "selected config ${_wx_selected_config}")
  if (_wx_selected_config MATCHES "androideabi-qt")
    # message(STATUS "Building for wxQt-Android")
    # message(STATUS "Qt_Base: " # ${Qt_Base})
    # message(STATUS "wxQt_Base/Build:  ${wxQt_Base}/${wxQt_Build}")
    # set(QT_ANDROID "ON")
  endif ()
endif ()

if ((_wx_selected_config MATCHES "qt-armv7"))
  set(wxWidgets_FIND_COMPONENTS base core xml html adv aui)
else ()
  set(wxWidgets_FIND_COMPONENTS net xml html adv aui core base webview)
endif ()

if (ARCH MATCHES "arm*" AND (NOT QT_ANDROID) AND OCPN_USE_GLES_SHIM)
  find_path(OPENGLESv1_INCLUDE_DIR GLES/gl.h)
  if (OPENGLESv1_INCLUDE_DIR)
    message(STATUS "Found OpenGLESv1")
    add_definitions(-DocpnUSE_GL) # ocpnUSE_GLES is in config.h
    # set(OPENGLES_FOUND "YES") set(OPENGL_FOUND "YES")
    set(OPENGL_LIBRARIES "GL_static" "EGL" "X11" "drm")
  endif ()
endif ()

if (OPENGLES_FOUND)
  list(APPEND wxwidgets_FIND_COMPONENTS gl)
endif ()

if ((NOT OPENGLES_FOUND) AND (NOT QT_ANDROID))
  if (OCPN_USE_GL)
    find_package(OpenGL)
  else ()
    message(STATUS "OpenGL disabled by option USE_GL...")
  endif ()

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
  else ()
    message(STATUS "OpenGL not found...")
  endif ()
endif ()

if (MSVC)
  target_include_directories(
    ${PACKAGE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/buildwin
  )
elseif (APPLE)
  target_include_directories(
    ${PACKAGE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/buildosx/include
  )
elseif (QT_ANDROID)
  target_include_directories(
    ${PACKAGE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/android
  )
endif ()

if (OCPN_USE_CRASHREPORT)
  message(STATUS "Crash reporting enabled")
  if (MSVC)
    message(STATUS "Using Windows CrashRpt")
    target_include_directories(
      ${PACKAGE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/buildwin/crashrpt
    )
    target_link_libraries(
      ${PACKAGE_NAME}
      PRIVATE ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/CrashRpt1403.lib
    )
    set(CMAKE_EXE_LINKER_FLAGS "/DEBUG /Release ${CMAKE_EXE_LINKER_FLAGS}")
  elseif (LINUX AND CMAKE_BUILD_TYPE MATCHES "Rel*")
    message(STATUS "Using Linux crash reporting")
  endif ()
endif ()

add_subdirectory("libs/sqlite")
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::sqlite)

add_subdirectory("libs/SQLiteCpp")
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::sqlite_cpp)

find_package(Shapelib)
if (SHAPELIB_FOUND)
  message(STATUS "Using system shapelib")
else ()
  message(STATUS "Using bundled shapelib")
  add_subdirectory("libs/shapelib")
endif ()
target_link_libraries(${PACKAGE_NAME} PRIVATE shapelib::shapelib)

add_subdirectory("libs/ShapefileCpp")
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::ShapefileCpp)
target_include_directories(${PACKAGE_NAME} PRIVATE ${SHAPEFILECPP_INCLUDE_DIR})
target_include_directories(${PACKAGE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/gdal/include/gdal)

if (NOT QT_ANDROID)
  add_subdirectory("libs/libdnet")
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::dnet)
endif ()

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
      target_link_libraries(${PACKAGE_NAME} PRIVATE ${X11_LIBRARIES} ${X11_Xrandr_LIB})
      set(OCPN_HAVE_X11 ON) # => config.h
    endif ()
    if (NOT APPLE)
      # TinyXML from Homebrew is not good enough for our GpxDocument needs We
      # should probably replace it with pugixml completely anyway...
      find_package(TinyXML)
    endif ()
    if (APPLE)
      if (wxWidgets_VERSION_STRING VERSION_LESS "3.1.6")
        # Only needed if we build with wxSVG implementation of SVG support
        message(STATUS "Building with wxSVG support")
        find_package(Freetype REQUIRED)
        target_link_libraries(${PACKAGE_NAME} PRIVATE Freetype::Freetype)
        find_package(Fontconfig REQUIRED)
        target_link_libraries(${PACKAGE_NAME} PRIVATE Fontconfig::Fontconfig)
        find_package(PNG REQUIRED)
        target_link_libraries(${PACKAGE_NAME} PRIVATE PNG::PNG)
        find_package(Pixman REQUIRED)
        target_link_libraries(${PACKAGE_NAME} PRIVATE Pixman::Pixman)
      endif ()
      find_package(ZSTD REQUIRED)
      target_link_libraries(${PACKAGE_NAME} PRIVATE zstd::zstd)
    endif ()
  endif ()
endif ()

# check for lzma support for reading compressed charts
if (CMAKE_HOST_WIN32)
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cache/buildwin")
  list(APPEND CMAKE_PREFIX_PATH
       "${CMAKE_CURRENT_SOURCE_DIR}/cache/buildwin/include"
  )
endif ()

if (QT_ANDROID)
  target_include_directories(
    ${PACKAGE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/buildandroid/libexpat/include
  )
endif ()

if (OCPN_USE_SVG)
  message(STATUS "SVG support enabled...")
  if (wxWidgets_VERSION_STRING VERSION_GREATER_EQUAL "3.1.6")
    message(STATUS "wxWidgets 3.1.6+ detected, using wxBitmapBundle for SVG")
    add_definitions(-DocpnUSE_SVG) # Goes into config.h, leave as global
    add_definitions(-DocpnUSE_wxBitmapBundle)
  else ()
    message(STATUS "wxWidgets older than 3.1.6 detected, using wxSVG")
    if (NOT QT_ANDROID)
      include(OcpnFindCairo)
      include(OcpnFindExpat)
      if (CMAKE_HOST_WIN32)
        # On Windows, we have our own expat and cairo
        file(GLOB gtkdll_files
             "${CMAKE_CURRENT_SOURCE_DIR}/cache/buildwin/gtk/*.dll"
        )
        install(FILES ${gtkdll_files} DESTINATION ".")
        file(GLOB expatdll_files
             "${CMAKE_CURRENT_SOURCE_DIR}/cache/buildwin/expat-2.2.5/*.dll"
        )
        install(FILES ${expatdll_files} DESTINATION ".")
        message(
          STATUS "GTK and Expat DLLs bundled into the installer package..."
        )
      else ()
        # Include(OcpnFindFmpeg)  -- FFMPEG is not enabled currently.
        # target_link_libraries(${PACKAGE_NAME} ocpn::ffmpeg)
        include(FindEXIF)
        target_link_libraries(
          ${PACKAGE_NAME} PRIVATE ${EXIF_LIBRARIES} ${GTK_LIBRARIES}
        )
        target_include_directories(${PACKAGE_NAME} PRIVATE ${EXIF_INCLUDE_DIRS})
      endif ()
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
    else ()
      add_definitions(-DocpnUSE_SVG)
      include_directories(${CMAKE_SOURCE_DIR}/libs/wxsvg/include/wxSVG)
      include_directories(${CMAKE_SOURCE_DIR}/libs/wxsvg/include/wxSVGXML)
      include_directories(${CMAKE_SOURCE_DIR}/libs/wxsvg/include)
      include_directories(${CMAKE_SOURCE_DIR}/libs/wxsvg/src)
    endif ()
  endif ()
endif ()

if (OCPN_USE_NEWSERIAL)
  add_subdirectory("libs/serial")
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::serial)
  message(STATUS "Using new serial library...")
endif ()

# Search for opengles, short of running a program to test the speed of
# acceleration, I simply use gles on "native linux" arm systems
if (ARCH MATCHES "arm*" AND (NOT QT_ANDROID) AND OCPN_USE_GLES_SHIM)
  find_path(OPENGLESv1_INCLUDE_DIR GLES/gl.h)
  if (OPENGLESv1_INCLUDE_DIR)
    message(STATUS "Found OpenGLESv1")
    add_definitions(-DocpnUSE_GLES)
    add_definitions(-DocpnUSE_GL)

    # set(OPENGLES_FOUND "YES") set(OPENGL_FOUND "YES")
    # SET(wxWidgets_FIND_COMPONENTS ${wxWidgets_FIND_COMPONENTS} gl )
    add_subdirectory(libs/glshim)

    set(OPENGL_LIBRARIES "GL_static" "EGL" "X11" "drm")
  endif ()
endif ()

if (OPENGL_FOUND)
  target_sources(
    ${PACKAGE_NAME}
    PRIVATE ${GUI_HDR_DIR}/glChartCanvas.h
            ${GUI_HDR_DIR}/glTextureDescriptor.h
            ${GUI_HDR_DIR}/glTexCache.h
            ${GUI_HDR_DIR}/glTextureManager.h
            ${GUI_SRC_DIR}/glTextureDescriptor.cpp
            ${GUI_SRC_DIR}/glTexCache.cpp
            ${GUI_SRC_DIR}/glChartCanvas.cpp
            ${GUI_SRC_DIR}/glTextureManager.cpp
  )
endif ()

# Building for QT_ANDROID involves a cross-building environment, So the include
# directories, flags, etc must be stated explicitly without trying to locate
# them on the host build system.
if (QT_ANDROID)
  add_definitions(-DocpnUSE_GLES)
  add_definitions(-DocpnUSE_GL)
endif ()

message(STATUS "    Adding local GLU")
add_subdirectory(libs/glu)
set(OPENGL_LIBRARIES "GLU_static" ${OPENGL_LIBRARIES})
if (OCPN_VERBOSE)
  message(STATUS "    Revised GL Lib (with local): " ${OPENGL_LIBRARIES})
endif ()

find_package(OcpnLibarchive)
if (NOT TARGET ocpn::libarchive)
  message(FATAL_ERROR "No libarchive!!!")
endif ()
target_link_libraries(${PACKAGE_NAME} PUBLIC ocpn::libarchive)
if (APPLE AND OCPN_USE_SYSTEM_LIBARCHIVE)
  # If we use system libarchive, we do not bundle the dylib and need to comment
  # it out from the pkg project file
  set(LIBARCHIVE_COMMENT_START "<!--")
  set(LIBARCHIVE_COMMENT_END "-->")
endif ()

# Add android and windows extra, bundled GL headers
target_link_libraries(${PACKAGE_NAME} PUBLIC ocpn::gl-headers)

# Ensure we have ghc::filesystem if std::filesystem is missing.
add_subdirectory(libs/std_filesystem)
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::filesystem)

message(STATUS "    Adding local LIBTESS2")
add_subdirectory(libs/libtess2)
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::tess2)

add_subdirectory(libs/tinyxml)
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::tinyxml)

message(STATUS "    Adding local wxserverdisc")
add_subdirectory(libs/wxservdisc)

add_subdirectory(libs/manual)
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::manual)

add_subdirectory(libs/s57-charts)
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::s57-charts)

if (MSVC)
  install(FILES "cache/buildwin/libssl-3.dll" DESTINATION ".")
  install(FILES "cache/buildwin/libcrypto-3.dll" DESTINATION ".")
endif ()

message(STATUS "S57 ENC support: enabled")

if (OCPN_USE_GARMINHOST)
  add_subdirectory("libs/garmin")
  target_link_libraries(${PACKAGE_NAME} PUBLIC ocpn::garminhost)
  message(STATUS "Garmin Host Mode support: enabled")
else ()
  message(STATUS "Garmin Host Mode support: disabled")
endif ()

# Compile texcompression support library
if (OPENGL_FOUND)
  add_subdirectory("libs/texcmp")
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::texcmp)

  add_subdirectory("libs/mipmap")
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::mipmap)
endif ()

pkg_search_module(LZ4 liblz4 lz4)
use_bundled_lib(USE_BUNDLED_LZ4 lz4)
if (LZ4_FOUND AND NOT USE_BUNDLED_LZ4)
  message(STATUS "Building with system lz4")
  include_directories(${LZ4_INCLUDE_DIR})
  if (OCPN_USE_GL)
    target_link_directories(TEXCMP PRIVATE ${LZ4_LIBRARY_DIRS})
  endif ()
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${LZ4_LIBRARIES})
else ()
  message(STATUS "Building with bundled lz4")
  add_subdirectory("libs/lz4")
  target_link_libraries(${PACKAGE_NAME} PRIVATE LZ4)
endif ()

# Finalize config.h setup and deploy it.
include(TargetSetup)
if ("${wxWidgets_LIBRARIES}" MATCHES "gtk3u")
  if (${PKG_TARGET_VERSION} VERSION_LESS 22.04)
    if (NOT ${PKG_TARGET} MATCHES "-gtk3")
      string(REPLACE ubuntu ubuntu-gtk3 PKG_TARGET ${PKG_TARGET})
    endif ()
  endif ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__WXGTK3__")
endif ()
configure_file(
  ${CMAKE_SOURCE_DIR}/cmake/in-files/config.h.in
  ${CMAKE_BINARY_DIR}/include/config.h
)
include_directories(BEFORE "${CMAKE_BINARY_DIR}/include")

if (NOT APPLE AND NOT QT_ANDROID)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${wxWidgets_LIBRARIES})
endif ()

if (CMAKE_HOST_WIN32)
  target_link_libraries(
    ${PACKAGE_NAME} PRIVATE setupapi.lib "GLU_static" psapi.lib
                            iphlpapi.lib # glu32.lib
  )
  # use gdi plus only on MSVC, it is not a free library
  if (MSVC)
    target_link_libraries(${PACKAGE_NAME} PRIVATE gdiplus.lib)
    target_link_libraries(${PACKAGE_NAME} PRIVATE Shcore.lib)
  endif ()
endif ()

if (UNIX)
  target_link_libraries(
    ${PACKAGE_NAME} PRIVATE ${OPENGL_LIBRARIES} ${GETTEXT_LIBRARY} SQLITE_CPP
  )
  if (NOT APPLE AND NOT QT_ANDROID AND NOT WIN32 AND NOT X11_FOUND)
    message(STATUS "Did not find x11 libraries")
    message(
      STATUS "support for transparent toolbar in opengl mode not detected"
    )
  endif ()
endif ()

# If we build for windows systems, we also include the resource file containing
# the manifest, icon and other resources
if (MSVC)
  target_sources(${PACKAGE_NAME} PRIVATE ${CMAKE_BINARY_DIR}/opencpn.rc)
endif ()

if (OCPN_BUNDLE_GSHHS)
  set(gshhs data/gshhs/poly-c-1.dat data/gshhs/wdb_borders_c.b
            data/gshhs/wdb_rivers_c.b
  )
endif ()

set(basemap_shp
        data/basemap_shp/basemap_crude_10x10.cpg
        data/basemap_shp/basemap_crude_10x10.prj
        data/basemap_shp/basemap_crude_10x10.shp
        data/basemap_shp/basemap_crude_10x10.dbf
        data/basemap_shp/basemap_crude_10x10.qmd
        data/basemap_shp/basemap_crude_10x10.shx
        data/basemap_shp/basemap_low.cpg
        data/basemap_shp/basemap_low.prj
        data/basemap_shp/basemap_low.shx
        data/basemap_shp/basemap_low.dbf
        data/basemap_shp/basemap_low.shp
        )

# Various data files

set(ssfnData
        data/ssfn/FreeSans.sfn
        )

set(uiData
    resources/bitmaps/styles.xml
    resources/bitmaps/toolicons_journeyman.png
    resources/bitmaps/toolicons_journeyman_flat.png
    resources/bitmaps/toolicons_traditional.png
    resources/bitmaps/iconRMinus.png
    resources/bitmaps/iconRPlus.png
    resources/bitmaps/iconMinimum.png
    resources/bitmaps/iconStandard.png
    resources/bitmaps/iconUserStd.png
    resources/bitmaps/iconAll.png
    resources/bitmaps/DragHandle.svg
    resources/bitmaps/eye.svg
    resources/bitmaps/eyex.svg
    resources/bitmaps/eyeGray.svg
    resources/bitmaps/iENC_All.svg
    resources/bitmaps/iENC_Minimum.svg
    resources/bitmaps/iENC_Standard.svg
    resources/bitmaps/iENC_UserStd.svg
    resources/bitmaps/iENC_RPlus.svg
    resources/bitmaps/iENC_RMinus.svg
    resources/bitmaps/package-x-generic.png
    resources/bitmaps/plus.svg
)

set(s57data
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

if (OCPN_BUNDLE_TCDATA)
  set(tcData
      data/tcdata/HARMONICS_NO_US data/tcdata/HARMONICS_NO_US.IDX
      data/tcdata/harmonics-dwf-20210110-free.tcd data/tcdata/README.harmonics
  )
endif ()

# Mac has trouble finding libgobject-2.0.dylib We look for it explicitely
if (xAPPLE)
  # Assume pkg-config is available.
  pkg_search_module(GOBJECT REQUIRED gobject-2.0)
  # MESSAGE (STATUS "    GOBJECT: " ${GOBJECT_LDFLAGS})

  target_compile_definitions(${PACKAGE_NAME} PRIVATE ${GOBJECT_LDFLAGS})
  target_link_libraries(${PACKAGE_NAME} PUBLIC "iconv")
endif ()

if (WIN32)
  target_sources(${PACKAGE_NAME} PRIVATE app.manifest)
endif ()

if (APPLE)
  set_source_files_properties(
    ${gshhs} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/gshhs
  )
  set_source_files_properties(
    ${basemap_shp} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/basemap_shp
  )
  set_source_files_properties(
    ${ssfnData} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/ssfndata
  )
  set_source_files_properties(
    ${uiData} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/uidata
  )
  set_source_files_properties(
    ${s57data} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/s57data
  )
  if (OCPN_BUNDLE_TCDATA)
    set_source_files_properties(
      ${tcData} PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/tcdata
    )
  endif (OCPN_BUNDLE_TCDATA)

  set_source_files_properties(
    data/license.html PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport
  )

  set_source_files_properties(
    data/authors.html PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport
  )

  set_source_files_properties(
    data/opencpn.png PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport
  )

  set_source_files_properties(
    data/CoC-909_2013-InlandECDIS_20170308.pdf
    PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport
  )

  set_source_files_properties(
    data/CoC-1973_2018-InlandECDIS_20220912.pdf
    PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport
  )

  set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/data/ocpn-plugins.xml PROPERTIES MACOSX_PACKAGE_LOCATION
                                                         SharedSupport
  )
  install(FILES ${CMAKE_SOURCE_DIR}/data/ocpn-plugins.xml
          DESTINATION ${PREFIX_PKGDATA}
  )
  target_sources(
    ${PACKAGE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/data/ocpn-plugins.xml
  )

  # Known, downloadable plugin icons
  file(GLOB PLUGIN_ICONS ${CMAKE_SOURCE_DIR}/plugins/icons/*)
  foreach (_pluginIcon ${PLUGIN_ICONS})
    set_source_files_properties(
      ${_pluginIcon} PROPERTIES MACOSX_PACKAGE_LOCATION
                                SharedSupport/uidata/plugins
    )
  endforeach ()
  install(DIRECTORY plugins/icons/ DESTINATION ${PREFIX_PKGDATA}/uidata/plugins)
  target_sources(${PACKAGE_NAME} PRIVATE ${PLUGIN_ICONS})

  file(GLOB SOUND_FILES ${CMAKE_SOURCE_DIR}/data/sounds/*)
  foreach (_currentSoundFile ${SOUND_FILES})
    set_source_files_properties(
      ${_currentSoundFile} PROPERTIES MACOSX_PACKAGE_LOCATION
                                      SharedSupport/sounds
    )
  endforeach ()

  file(GLOB CONFIG_FILES ${CMAKE_SOURCE_DIR}/data/configs/*)
  foreach (_currentConfigFile ${CONFIG_FILES})
    set_source_files_properties(
      ${_currentConfigFile} PROPERTIES MACOSX_PACKAGE_LOCATION
                                       SharedSupport/configs
    )
  endforeach ()

  if (OCPN_USE_SVG)
    # Traditional theme SVG icons
    file(GLOB TRADITIONAL_SVG_FILES
         "${CMAKE_SOURCE_DIR}/data/svg/traditional/*"
    )
    foreach (_currentSvgFile ${TRADITIONAL_SVG_FILES})
      set_source_files_properties(
        ${_currentSvgFile} PROPERTIES MACOSX_PACKAGE_LOCATION
                                      SharedSupport/uidata/traditional
      )
    endforeach ()

    # Journeyman theme SVG icons
    file(GLOB JOURNEYMAN_SVG_FILES "${CMAKE_SOURCE_DIR}/data/svg/journeyman/*")
    foreach (_currentSvgFile ${JOURNEYMAN_SVG_FILES})
      set_source_files_properties(
        ${_currentSvgFile} PROPERTIES MACOSX_PACKAGE_LOCATION
                                      SharedSupport/uidata/journeyman
      )
    endforeach ()

    # Journeyman_flat theme SVG icons
    file(GLOB JOURNEYMAN_FLAT_SVG_FILES
         "${CMAKE_SOURCE_DIR}/data/svg/journeyman_flat/*"
    )
    foreach (_currentSvgFile ${JOURNEYMAN_FLAT_SVG_FILES})
      set_source_files_properties(
        ${_currentSvgFile}
        PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/uidata/journeyman_flat
      )
    endforeach ()

    # MUI_flat theme SVG icons
    file(GLOB MUI_FLAT_SVG_FILES "${CMAKE_SOURCE_DIR}/data/svg/MUI_flat/*")
    foreach (_currentSvgFile ${MUI_FLAT_SVG_FILES})
      set_source_files_properties(
        ${_currentSvgFile}
        PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/uidata/MUI_flat
      )
    endforeach ()

    # Default SVG Icons
    file(GLOB DEFAULT_ICON_SVG_FILES "${CMAKE_SOURCE_DIR}/data/svg/markicons/*")
    foreach (_currentSvgFile ${DEFAULT_ICON_SVG_FILES})
      set_source_files_properties(
        ${_currentSvgFile}
        PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/uidata/markicons
      )
    endforeach ()

  endif (OCPN_USE_SVG)

  install(DIRECTORY data/svg/markicons/
          DESTINATION ${PREFIX_PKGDATA}/uidata/markicons
  )

  install(DIRECTORY data/svg/MUI_flat/
          DESTINATION ${PREFIX_PKGDATA}/uidata/MUI_flat
  )

  set_source_files_properties(
    data/doc/help_web.html PROPERTIES MACOSX_PACKAGE_LOCATION SharedSupport/doc
  )

  if (OCPN_BUNDLE_DOCS)
    set_source_files_properties(
      data/doc/local/toc_flat.html PROPERTIES MACOSX_PACKAGE_LOCATION
                                          SharedSupport/doc/local
    )
    file(GLOB_RECURSE WIKI_DOC_FILES "${CMAKE_SOURCE_DIR}/data/doc/local/*")
    foreach (_currentWikiDocFile ${WIKI_DOC_FILES})
      file(RELATIVE_PATH _docfileRelPath # Output variable
           "${CMAKE_SOURCE_DIR}/data/doc/local" # Base directory
           ${_currentWikiDocFile} # Absolute path to the file
      )
      get_filename_component(_docfileLocation ${_docfileRelPath} DIRECTORY)
      set(_location "SharedSupport/doc/local/${_docfileLocation}")
      set_source_files_properties(
        ${_currentWikiDocFile} PROPERTIES MACOSX_PACKAGE_LOCATION ${_location}
      )
    endforeach ()
  endif ()

  set_source_files_properties(
    buildosx/opencpn.icns
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources
  )

  set_source_files_properties(
    buildosx/MainMenu.xib
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources/English.lproj
  )

  # These variables get substituted into the Info.plist template file at build
  # time
  set(MACOSX_BUNDLE_ICON_FILE opencpn.icns)
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PACKAGE_VERSION}")
  set(MACOSX_BUNDLE_BUNDLE_NAME "OpenCPN")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.opencpn")

  if (OCPN_BUNDLE_DOCS)
    set(DOC_FILES data/doc/help_web.html
                  ${WIKI_DOC_FILES}
    )
  else ()
    set(DOC_FILES data/doc/help_web.html)
  endif ()

  target_sources(
    ${PACKAGE_NAME}
    PRIVATE ${gshhs}
            ${basemap_shp}
            ${ssfnData}
            ${uiData}
            ${s57data}
            ${tcData}
            data/license.html
            data/authors.html
            data/opencpn.png
            data/ocpn-plugins.xml
            data/CoC-909_2013-InlandECDIS_20170308s.pdf
            data/CoC-1973_2018-InlandECDIS_20220912.pdf
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
  install(TARGETS ${PACKAGE_NAME} BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR})

  target_link_libraries(${PACKAGE_NAME} PRIVATE ${wxWidgets_LIBRARIES})
  set_target_properties(
    ${PACKAGE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST
                               "${CMAKE_SOURCE_DIR}/buildosx/Info.plist.in"
                               XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
  )
endif ()

if (QT_ANDROID)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${wxWidgets_LIBRARIES})
  target_link_libraries(${PACKAGE_NAME} PRIVATE GLESv2 EGL z)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${OPENGL_LIBRARIES})
  target_link_libraries(${PACKAGE_NAME} PRIVATE NMEA0183)
endif ()

# Sanitizers, part 2/2
if (NOT "${ENABLE_SANITIZER}" MATCHES "none")
  target_link_libraries(${PACKAGE_NAME} PRIVATE -fsanitize=${ENABLE_SANITIZER})
endif ()

if (LIBLZMA_FOUND)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${LIBLZMA_LIBRARY})
endif ()

# Add targets used elsewhere, notably by model
if (QT_ANDROID)
  add_subdirectory(libs/android_jvm)
endif ()

add_subdirectory(libs/IXWebSocket)
add_subdirectory(libs/gdal)
add_subdirectory(libs/gl_headers)
add_subdirectory(libs/geoprim)
add_subdirectory(libs/iso8211)
add_subdirectory(libs/mdns)

set(MSVC_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cache/buildwin/include)
add_subdirectory(libs/mongoose)

add_subdirectory(libs/N2KParser)
add_subdirectory(libs/nmea0183)
add_subdirectory(libs/observable)
add_subdirectory(libs/picosha2)
add_subdirectory(libs/pugixml)
add_subdirectory(libs/rapidjson)
add_subdirectory(libs/s52plib)

if (QT_ANDROID)
  set(SOUND_WX_INCLUDE_DIR "${ANDROID_WX_INCLUDES}" CACHE STRING "")
  set(SOUND_WX_LIBS "${ANDROID_WX_LIBS}" CACHE STRING "")
else ()
  set(SOUND_WX_INCLUDE_DIR "${wxWidgets_INCLUDE_DIRS}" CACHE STRING "")
  set(SOUND_WX_LIBS "${wxWidgets_LIBRARIES}" CACHE STRING "")
endif ()
add_subdirectory(libs/sound)

add_subdirectory(libs/wxJSON)

#
# Install
#
install(FILES data/license.txt DESTINATION ${PREFIX_DATA}/${PACKAGE_NAME})

if (NOT APPLE)
  if (WIN32)
    set(PREFIX_PKGDATA ".")
  endif ()
  if (WIN32)
    install(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION ".")
  endif ()
  if (LINUX)
    install(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES LINUX_DEVICES.md DESTINATION ${PREFIX_DATA}/${PACKAGE_NAME})
  endif ()

  install(
    FILES ${CMAKE_SOURCE_DIR}/data/ocpn-plugins.xml
    DESTINATION ${PREFIX_PKGDATA}
  )
  install(FILES ${uiData} DESTINATION ${PREFIX_PKGDATA}/uidata)
  install(DIRECTORY data/svg/markicons/
          DESTINATION ${PREFIX_PKGDATA}/uidata/markicons
  )
  if (OCPN_USE_SVG)
    # Traditional theme SVG icons
    install(DIRECTORY data/svg/traditional/
            DESTINATION ${PREFIX_PKGDATA}/uidata/traditional
    )
    install(DIRECTORY data/svg/journeyman/
            DESTINATION ${PREFIX_PKGDATA}/uidata/journeyman
    )
    install(DIRECTORY data/svg/journeyman_flat/
            DESTINATION ${PREFIX_PKGDATA}/uidata/journeyman_flat
    )
    install(DIRECTORY data/svg/MUI_flat/
            DESTINATION ${PREFIX_PKGDATA}/uidata/MUI_flat
    )
  endif ()

  install(FILES ${s57data} DESTINATION ${PREFIX_PKGDATA}/s57data)

  if (OCPN_BUNDLE_TCDATA)
    install(FILES ${tcData} DESTINATION ${PREFIX_PKGDATA}/tcdata)
  endif ()

  if (OCPN_BUNDLE_GSHHS)
    install(FILES ${gshhs} DESTINATION ${PREFIX_PKGDATA}/gshhs)
  endif ()

  install(FILES ${basemap_shp} DESTINATION ${PREFIX_PKGDATA}/basemap_shp)

  install(FILES ${ssfnData} DESTINATION ${PREFIX_PKGDATA}/ssfndata)

  file(GLOB LICENSES "${CMAKE_SOURCE_DIR}/COPYING.*")
  install(FILES ${LICENSES} LICENSING DESTINATION "${PREFIX_PKGDATA}")

  install(FILES data/CoC-909_2013-InlandECDIS_20170308s.pdf
                data/CoC-1973_2018-InlandECDIS_20220912.pdf
          DESTINATION ${PREFIX_PKGDATA}
  )
  install(FILES data/copyright DESTINATION ${PREFIX_DATA}/doc/${PACKAGE_NAME})
  install(FILES data/license.html DESTINATION ${PREFIX_PKGDATA})
  install(FILES data/authors.html DESTINATION ${PREFIX_PKGDATA})
  install(FILES data/opencpn.png DESTINATION ${PREFIX_PKGDATA})

  if (PACKAGE_FORMAT EQUAL "DEB")
    install(FILES data/changelog.gz
            DESTINATION ${PREFIX_DATA}/doc/${PACKAGE_NAME}
    )
  else ()
    install(FILES data/changelog DESTINATION ${PREFIX_DATA}/doc/${PACKAGE_NAME})
  endif ()

  install(DIRECTORY data/sounds/ DESTINATION ${PREFIX_PKGDATA}/sounds)

  if (OCPN_BUNDLE_DOCS)
    install(DIRECTORY data/doc DESTINATION ${PREFIX_PKGDATA})
  else ()
    install(FILES data/doc/help_web.html DESTINATION ${PREFIX_PKGDATA}/doc)
  endif ()

  if (UNIX)
    install(FILES data/opencpn.png
            DESTINATION ${PREFIX_DATA}/icons/hicolor/48x48/apps
    )
    install(
      FILES resources/bitmaps/opencpn-64.png
      DESTINATION ${PREFIX_DATA}/icons/hicolor/64x64/apps
      RENAME opencpn.png
    )
    install(FILES resources/bitmaps/other_svg_src/opencpn.svg
            DESTINATION ${PREFIX_DATA}/icons/hicolor/scalable/apps
    )
    install(FILES resources/opencpn.desktop
            DESTINATION ${PREFIX_DATA}/applications
    )
    install(
      FILES ${CMAKE_BINARY_DIR}/opencpn.appdata.xml
      DESTINATION ${PREFIX_DATA}/metainfo
      RENAME opencpn.appdata.xml
    )
    install(FILES opencpn.1 opencpn-cmd.1 DESTINATION ${PREFIX_DATA}/man/man1)
  endif ()

  install(CODE "
    if (EXISTS /lib/udev/rules.d)
      execute_process(
        COMMAND sh -c \"test -w /lib/udev/rules.d 2>/dev/null \"
        RESULT_VARIABLE _rv
      )
      if (_rv EQUAL 0)
        execute_process(COMMAND
          sh -c \"cp ${PROJECT_SOURCE_DIR}/buildlinux/98-sglock-ocpn.rules \
                     /usr/lib/udev/rules.d/98-sglock-ocpn.rules\"
        )
        execute_process(COMMAND
          sh -c \"chmod 644 \
              ${PROJECT_SOURCE_DIR}/buildlinux/98-sglock-ocpn.rules\"
        )
        message(STATUS \"Installed udev dongle rule\")
      else ()
        message(STATUS \"Skipped installing udev dongle rule\")
  endif ()
endif ()
  ")

endif ()

#
# Gettext languages setup.
#
find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
if (GETTEXT_XGETTEXT_EXECUTABLE)
  add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot.dummy
    COMMAND
      ${GETTEXT_XGETTEXT_EXECUTABLE} --from-code=iso-8859-1 --force-po
      --package-name=${PACKAGE_NAME} --package-version="${PACKAGE_VERSION}"
      --output=${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot --keyword=_ --width=80
      --files-from=${CMAKE_SOURCE_DIR}/po/POTFILES.in
    DEPENDS ${CMAKE_SOURCE_DIR}/po/POTFILES.in
            ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "pot-update [${PACKAGE_NAME}]: Generated pot file."
  )
  add_custom_target(
    pot-update
    COMMENT "pot-update: Done."
    DEPENDS ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot.dummy
  )
endif ()

macro (GETTEXT_UPDATE_PO _potFile)
  set(_poFiles ${_potFile})
  get_filename_component(_absPotFile ${_potFile} ABSOLUTE)

  foreach (_currentPoFile ${ARGN})
    get_filename_component(_absFile ${_currentPoFile} ABSOLUTE)
    get_filename_component(_poBasename ${_absFile} NAME_WE)

    add_custom_command(
      OUTPUT ${_absFile}.dummy
      COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --width=80 --strict --quiet
              --update --backup=none --no-location -s ${_absFile} ${_absPotFile}
      DEPENDS ${_absPotFile} ${_absFile}
      COMMENT "po-update [${_poBasename}]: Updated po file."
    )
    set(_poFiles ${_poFiles} ${_absFile}.dummy)
  endforeach (_currentPoFile)

  add_custom_target(
    po-update
    COMMENT "po-update: Done."
    DEPENDS ${_poFiles}
  )
endmacro ()

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
      ${CMAKE_CURRENT_BINARY_DIR}/Resources/opencpn_${_lang}.lproj/opencpn.mo
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
endmacro ()

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
  gettext_build_mo(po/opencpn_zh_CN.po zh_CN)
  gettext_build_mo(po/opencpn_hu_HU.po hu_HU)
  gettext_build_mo(po/opencpn_gl_ES.po gl_ES)
  gettext_build_mo(po/opencpn_ca_ES.po ca_ES)
  gettext_build_mo(po/opencpn_ar_SA.po ar_SA)
  gettext_build_mo(po/opencpn_ja_JP.po ja_JP)
  gettext_build_mo(po/opencpn_vi_VN.po vi_VN)
  gettext_build_mo(po/opencpn_he_IL.po he_IL)
  gettext_build_mo(po/opencpn_en_GB.po en_GB)

  add_custom_target( i18n COMMENT "i18n: Done." DEPENDS ${_gmoFiles})
  if (NOT QT_ANDROID)
    add_dependencies(${PACKAGE_NAME} i18n)
  endif ()
endif ()

#
# Install wxstd.mo in Windows
#
if (WIN32)
  file(GLOB WXWIDGETS_MO_FILES
       ${CMAKE_SOURCE_DIR}/cache/buildwin/wxWidgets/locale/*.mo
  )
  foreach (_current_wx_mofile ${WXWIDGETS_MO_FILES})
    get_filename_component(_absFile ${_current_wx_mofile} ABSOLUTE)
    get_filename_component(_mlang ${_absFile} NAME_WE)
    install(
      FILES ${_current_wx_mofile}
      DESTINATION ${PREFIX_DATA}/locale/${_mlang}/LC_MESSAGES
      RENAME wxstd.mo
      PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    )
  endforeach ()
endif ()

#
# Install wxstd.mo in Mac
#
if (APPLE)
  file(GLOB WXWIDGETS_MO_FILES
       ${CMAKE_SOURCE_DIR}/buildosx/wxWidgets/locale/*.mo
  )
  foreach (_current_wx_mofile ${WXWIDGETS_MO_FILES})
    get_filename_component(_absFile ${_current_wx_mofile} ABSOLUTE)
    get_filename_component(_mlang ${_absFile} NAME_WE)

    install(
      FILES ${_current_wx_mofile}
      DESTINATION
        ${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/Resources/${_mlang}.lproj
      RENAME wxstd.mo
    )
  endforeach ()
endif ()

#
# On Windows builds, copy the wxWidgets dlls and helpers to the executable
# directory
#
if (WIN32 AND NOT UNIX)
  if (OCPN_BUNDLE_WXDLLS)
    file(GLOB wxdll_files "${wxWidgets_LIB_DIR}/wxmsw32*u_*.dll"
         "${wxWidgets_LIB_DIR}/wxbase32*u_*.dll"
    )
    file(GLOB wxdll_debug_files "${wxWidgets_LIB_DIR}/wxmsw32*ud_*.dll"
         "${wxWidgets_LIB_DIR}/wxbase32*ud_*.dll"
    )
    list(LENGTH wxdll_files wxdll_count)
    if (wxdll_count LESS 5)
      message(FATAL_ERROR "Could not find all the WX DLLs.")
    endif ()
    list(LENGTH wxdll_debug_files wxdllud_count)
    if (wxdllud_count GREATER 0)
      install(
        FILES ${wxdll_debug_files}
        DESTINATION "."
        CONFIGURATIONS Debug
      )
      install(
        FILES ${wxdll_files}
        DESTINATION "."
        CONFIGURATIONS Release|RelWithDebInfo|MinSizeRel
      )
    else ()
      install(FILES ${wxdll_files} DESTINATION ".")
    endif ()
    message(
      STATUS
        "${wxdll_count} wxWidgets DLLs available to be bundled into the installer package..."
    )
    message(
      STATUS
        "${wxdllud_count} wxWidgets debug DLLs available to be bundled into the installer package..."
    )
    if (OCPN_BUNDLE_WXDLLS AND NOT OCPN_CI_BUILD)
      file(GLOB wxpdb_files "${wxWidgets_LIB_DIR}/wxmsw32*u_*.pdb"
           "${wxWidgets_LIB_DIR}/wxbase32*u_*.pdb"
      )
      file(GLOB wxpdb_debug_files "${wxWidgets_LIB_DIR}/wxmsw32*ud_*.pdb"
           "${wxWidgets_LIB_DIR}/wxbase32*ud_*.pdb"
      )
      list(LENGTH wxpdb_files wxpdb_count)
      list(LENGTH wxpdb_debug_files wxpdbud_count)
      install(
        FILES ${wxpdb_debug_files}
        DESTINATION "."
        CONFIGURATIONS Debug
      )
      install(
        FILES ${wxpdb_files}
        DESTINATION "."
        CONFIGURATIONS Release|RelWithDebInfo|MinSizeRel
      )
      message(
        STATUS
          "${wxpdb_count} wxWidgets PDBs available to be bundled into the installer package..."
      )
      message(
        STATUS
          "${wxpdbud_count} wxWidgets debug PDBs available to be bundled into the installer package..."
      )
    endif ()
  endif ()
  if (OCPN_BUNDLE_VCDLLS)
    file(GLOB vcdll_files "${CMAKE_CURRENT_SOURCE_DIR}/cache/buildwin/vc/*.dll")
    install(FILES ${vcdll_files} DESTINATION ".")
    message(
      STATUS "MSVC redistributable DLLs bundled into the installer package..."
    )
  endif ()
  if (BUNDLE_LIBARCHIVEDLLS)
    file(
      GLOB
      libarchivedll_files
      "${CMAKE_CURRENT_SOURCE_DIR}/cache/buildwin/archive.dll"
      "${CMAKE_CURRENT_SOURCE_DIR}/cache/buildwin/liblzma.dll"
      "${CMAKE_CURRENT_SOURCE_DIR}/cache/buildwin/zlib1.dll"
    )
    install(FILES ${libarchivedll_files} DESTINATION ".")
    message(STATUS "LibArchive DLLs bundled into the installer package...")
  endif ()
endif ()

if (APPLE)
  if (OCPN_USE_DEPS_BUNDLE)
    message(status "PATH: ${OCPN_DEPS_BUNDLE_PATH}")
    find_library(
      MPG123
      NAMES libmpg3 mpg123
      HINTS "${OCPN_DEPS_BUNDLE_PATH}/lib"
      NO_DEFAULT_PATH REQUIRED
    )
  else ()
    find_library(MPG123 NAMES libmpg3 mpg123 REQUIRED)
  endif ()
  target_link_libraries(${PACKAGE_NAME} PUBLIC ${MPG123})
  message(STATUS "MPG123: ${MPG123}")
  if (OCPN_USE_DEPS_BUNDLE)
    find_library(
      MP3LAME
      NAMES mp3lame lame
      HINTS "${OCPN_DEPS_BUNDLE_PATH}/lib"
      NO_DEFAULT_PATH REQUIRED
    )
  else ()
    find_library(MP3LAME NAMES mp3lame lame REQUIRED)
  endif ()
  target_link_libraries(${PACKAGE_NAME} PUBLIC ${MP3LAME})
  message(STATUS "MP3LAME: ${MP3LAME}")
endif ()

if (MSVC AND OCPN_USE_CRASHREPORT)
  install(FILES ${CMAKE_SOURCE_DIR}/cache/buildwin/crashrpt/CrashRpt1403.dll
          DESTINATION ${PREFIX_PKGDATA}
  )
  install(FILES ${CMAKE_SOURCE_DIR}/cache/buildwin/crashrpt/CrashSender1403.exe
          DESTINATION ${PREFIX_PKGDATA}
  )
  install(FILES ${CMAKE_SOURCE_DIR}/cache/buildwin/crashrpt/crashrpt_lang.ini
          DESTINATION ${PREFIX_PKGDATA}
  )
  install(FILES ${CMAKE_SOURCE_DIR}/cache/buildwin/crashrpt/dbghelp.dll
          DESTINATION ${PREFIX_PKGDATA}
  )
  install(FILES ${CMAKE_SOURCE_DIR}/buildwin/crashrpt/PrivacyPolicy.txt
          DESTINATION ${PREFIX_PKGDATA}
  )
  message(STATUS "CrashRpt bundled into the installer package...")
endif ()

# Install glew dll
if (MSVC AND OCPN_USE_GL)
  install(FILES ${CMAKE_SOURCE_DIR}/cache/buildwin/glew32.dll
          DESTINATION ${PREFIX_PKGDATA}
  )
endif ()

if (NOT SKIP_PLUGINS)
  add_subdirectory(plugins)
endif ()

#
# Prepare CPACK configuration
#
if (WIN32)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/NSIS.template.in.in
    ${CMAKE_BINARY_DIR}/NSIS.template.in COPYONLY
  )
endif ()

#
# Build a CPack driven installer package
#
# include (InstallRequiredSystemLibraries)
set(CPACK_BINDIR "${CMAKE_BINARY_DIR}")
set(CPACK_PACKAGE_NAME "OpenCPN")
set(CPACK_PACKAGE_VENDOR "opencpn.org")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenCPN ${PACKAGE_VERSION}")
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH}${VERSION_TAIL})
set(CPACK_INSTALL_CMAKE_PROJECTS
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
# source tarball created by 'make package_source'.
set(CPACK_SOURCE_IGNORE_FILES
    "\\\\.flatpak-builder*" "\\\\.git*" "${CMAKE_BINARY_DIR}*"
)

#
# Windows NSIS package setup.
#
if (WIN32)
  if (NOT SKIP_VERSION_CONFIG)
    configure_file(
      ${CMAKE_SOURCE_DIR}/resources/opencpn.rc.in
      ${CMAKE_BINARY_DIR}/opencpn.rc
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
  endif ()

  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.

  set(CPACK_NSIS_INSTALLED_ICON_NAME "opencpn.exe")
  set(CPACK_NSIS_PACKAGE_NAME_LC "opencpn")
  set(CPACK_NSIS_DIR "${CMAKE_SOURCE_DIR}/buildwin/NSIS_Unicode") # Gunther
  set(CPACK_BUILDWIN_DIR "${CMAKE_SOURCE_DIR}/buildwin") # Gunther

  # These lines set the name of the Windows Start Menu shortcut and the icon
  # that goes with it
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME} ${PACKAGE_VERSION}")
  set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources/bitmaps\\\\opencpn.ico")
  set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${PACKAGE_VERSION}_setup")
  set(CPACK_RESOURCE_FILE_LICENSE
      "${CMAKE_CURRENT_SOURCE_DIR}/data/license.txt"
  )
else ()
  set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PACKAGE_NAME})
  set(CPACK_STRIP_FILES "bin/opencpn")
  set(CPACK_SOURCE_STRIP_FILES "")
  set(CPACK_PACKAGE_FILE_NAME
      "${PACKAGE_NAME}_${PACKAGE_VERSION}-${OCPN_PACKAGE_RELEASE}_${ARCH}"
  )
  set(CPACK_RESOURCE_FILE_LICENSE
      "${CMAKE_CURRENT_SOURCE_DIR}/data/license.html"
  )
endif ()

if (OCPN_BUNDLE_TCDATA)
  message(STATUS "*** Package will include tide and current data ***")
else ()
  message(STATUS "*** Package will NOT include tide and current data ***")
endif ()

#
# Appstream data (Linux in general, in particular Flatpak).
#
configure_file(
  "${CMAKE_SOURCE_DIR}/resources/opencpn.appdata.xml.in"
  "${CMAKE_BINARY_DIR}/opencpn.appdata.xml" @ONLY
)

# Command line tools - must be added before the macOS bundle is defined,
# otherwise they do not get installed before fixup_bundle is performed
if (NOT QT_ANDROID)
  add_subdirectory(cli)
endif ()

if (NOT QT_ANDROID AND OCPN_USE_GL)
  add_subdirectory(glutil)
endif ()

#
# Apple application bundle
#
if (APPLE)
  if ($ENV{pkg_mod})
    set(APKG_MOD $ENV{pkg_mod})
  else ()
    set(APKG_MOD 11)
  endif ()
  message(STATUS "MacOS PKG_MOD: ${APKG_MOD}")

  # -- Run the BundleUtilities cmake code
  set(CPACK_BUNDLE_PLIST "${CMAKE_SOURCE_DIR}/buildosx/Info.plist.in")

  set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app")
  set(DIRS "/usr/local/lib")

  # INSTALL(DIRECTORY DESTINATION "bin/OpenCPN.app/Contents/Plugins")
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/plugins/dashboard_pi/libdashboard_pi.dylib
    DESTINATION "bin/OpenCPN.app/Contents/Plugins"
  )
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugins/grib_pi/libgrib_pi.dylib
          DESTINATION "bin/OpenCPN.app/Contents/Plugins"
  )
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/plugins/chartdldr_pi/libchartdldr_pi.dylib
    DESTINATION "bin/OpenCPN.app/Contents/Plugins"
  )
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugins/wmm_pi/libwmm_pi.dylib
          DESTINATION "bin/OpenCPN.app/Contents/Plugins"
  )
  set(LIBS
      "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/Plugins/libdashboard_pi.dylib"
      "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/Plugins/libgrib_pi.dylib"
      "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/Plugins/libchartdldr_pi.dylib"
      "\${CMAKE_INSTALL_PREFIX}/bin/OpenCPN.app/Contents/Plugins/libwmm_pi.dylib"
  )
  install(
    CODE "
    include(BundleUtilities)
    set(BU_CHMOD_BUNDLE_ITEMS TRUE)
    fixup_bundle(\"${APPS}\"  \"${LIBS}\"  \"${DIRS}\")
    "
    COMPONENT Runtime
  )

  configure_file(
    ${PROJECT_SOURCE_DIR}/COPYING.gplv2 ${CMAKE_CURRENT_BINARY_DIR}/GPLv2.txt
    COPYONLY
  )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/buildosx/distro.plist.in
    ${CMAKE_CURRENT_BINARY_DIR}/distro.plist
  )

  # Build standard MacOS .dmg
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_${PACKAGE_VERSION}.dmg
    COMMAND
      create-dmg --volname "OpenCPN Installer"
      --background ${CMAKE_SOURCE_DIR}/buildosx/background.png --window-pos 200
      120 --window-size 500 300 --icon-size 80 --icon OpenCPN.app 120 150
      --hide-extension OpenCPN.app --app-drop-link 390 145 --no-internet-enable
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
  execute_process(
    COMMAND sw_vers -productVersion
    OUTPUT_VARIABLE _product_version
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  # Only use main version nr
  string(REGEX REPLACE "\\..*" "" OS_VERS ${_product_version})
  message(STATUS "OS_VERS: ${OS_VERS}, _product_version: ${_product_version}")

  # Build standard installer using pkgbuild/productbuild
  string(CONCAT MACOS_PKG "${CMAKE_CURRENT_BINARY_DIR}"
                "/${PACKAGE_NAME}_${PACKAGE_VERSION}-${OS_VERS}.pkg"
  )
  add_custom_target(
    show-bundled-libs COMMAND ls -l
                              /tmp/opencpn/bin/OpenCPN.app/Contents/Frameworks
  )

  add_custom_target(pkg-tmp COMMAND cmake -E make_directory pkg_tmp)

  add_custom_target(
    component-pkg
    COMMAND
      pkgbuild --root /tmp/opencpn/bin/OpenCPN.app --identifier org.opencpn
      --install-location /Applications/OpenCPN.app
      --scripts "${CMAKE_SOURCE_DIR}/buildosx/scripts" OpenCPN.app.pkg
  )
  add_dependencies(component-pkg show-bundled-libs pkg-tmp)

  add_custom_target(
    build-macos-pkg
    COMMAND
      productbuild --distribution distro.plist --version
      ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH} --identifier
      org.opencpn ${MACOS_PKG}
  )
  add_dependencies(build-macos-pkg component-pkg)

  add_custom_target(remove-tmp-pkg COMMAND cmake -E rm -f OpenCPN.app.pkg)
  add_dependencies(remove-tmp-pkg build-macos-pkg)

  add_custom_target(macos-pkg)
  add_dependencies(macos-pkg remove-tmp-pkg)

  # Select what to do for the "create-pkg" target
  if ("${APKG_MOD}" EQUAL 11)
    add_custom_target(create-pkg)
    add_dependencies(create-pkg macos-pkg)
  else ()
    message(FATAL_ERROR "Illegal value for pkg_mod environ var: ${APKG_MOD}")
  endif ()
endif ()

include(CPack)

if (MSVC)
  add_custom_command(
    TARGET ${PACKAGE_NAME}
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_PDB_FILE:${PACKAGE_NAME}>"
      "$<TARGET_PDB_FILE_DIR:${PACKAGE_NAME}>/${PACKAGE_NAME}-${PACKAGE_VERSION}.pdb"
  )
endif ()

# uninstall target
if (NOT TARGET uninstall)
  configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/uninstall.cmake.in"
    "${CMAKE_BINARY_DIR}/uninstall.cmake" IMMEDIATE @ONLY
  )
  add_custom_target(
    uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/uninstall.cmake
  )
endif ()

add_subdirectory(model)
target_link_libraries(${PACKAGE_NAME} PUBLIC ocpn::model-src ocpn::model)

if (OCPN_BUILD_TEST)
  add_subdirectory(libs/gtest)
  add_subdirectory(test)
  add_custom_target(
    run-tests
    COMMAND ctest -C $<CONFIG> -E tests --timeout 10
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test
  )
  add_dependencies(run-tests tests)
  if (UNIX AND NOT DEFINED ENV{FLATPAK_ID})
    add_dependencies(run-tests ipc-srv-tests)
  endif ()
  if (LINUX)
    add_dependencies(run-tests dbus_tests)
  endif ()
else ()
  add_custom_target(run-tests COMMAND cmake -E echo "No tests available")
endif ()

if (QT_ANDROID)
  target_link_libraries(${PACKAGE_NAME} PUBLIC ocpn::java-jvm)
  target_include_directories(
    ${PACKAGE_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/buildandroid/libopenssl/${ARCH}/openssl/include
  )
  target_link_libraries(
    ${PACKAGE_NAME}
    PRIVATE
      ${CMAKE_SOURCE_DIR}/buildandroid/libopenssl/${ARCH}/openssl/lib/libssl.a
      ${CMAKE_SOURCE_DIR}/buildandroid/libopenssl/${ARCH}/openssl/lib/libcrypto.a
  )
endif ()
