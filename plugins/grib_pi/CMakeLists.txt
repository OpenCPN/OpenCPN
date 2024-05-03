# ---------------------------------------------------------------------------
# Author:      Dave Register
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
# - 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
# ***************************************************************************

# define minimum cmake version
cmake_minimum_required(VERSION 3.5.0)

if (COMMAND cmake_policy)
  if (POLICY CMP0043)
    cmake_policy(SET CMP0043 NEW)
  endif (POLICY CMP0043)
endif (COMMAND cmake_policy)

if (NOT CMAKE_MODULE_PATH)
  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
endif ()

project(grib_pi)

set(PACKAGE_NAME grib_pi)
set(PLUGIN_SOURCE_DIR .)
message(STATUS "*** Staging to build ${PACKAGE_NAME} ***")

# SET(CMAKE_BUILD_TYPE Debug) SET(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MACOSX_RPATH "ON")

include_directories(${CMAKE_SOURCE_DIR}/include)

# require proper c++ ADD_DEFINITIONS( "-Wall -ansi -pedantic
# -Wno-variadic-macros" ) TODO: Should we use  -fno-stack-protector IF NOT
# DEBUGGING CFLAGS="-O2 -march=native"
if (NOT WIN32 AND NOT APPLE)
  add_definitions("-Wall -fexceptions -fvisibility=hidden")
  if (CMAKE_SYSTEM_NAME MATCHES ".*Linux")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic")
  endif (CMAKE_SYSTEM_NAME MATCHES ".*Linux")
endif (NOT WIN32 AND NOT APPLE)

if (APPLE)
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl -undefined dynamic_lookup")
endif (APPLE)

# Add some definitions to satisfy MS
if (MSVC)
  add_definitions(-D__MSVC__)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
  ADD_DEFINITIONS(-DMAKING_PLUGIN)
endif ()

# QT_ANDROID is a cross-build, so the native FIND_PACKAGE(wxWidgets...) and
# wxWidgets_USE_FILE are not useful.
if (QT_ANDROID)
  set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
  set(BUILD_SHARED_LIBS "ON")
  add_definitions(
    "-Wno-inconsistent-missing-override -Wno-potentially-evaluated-expression"
  )
else (QT_ANDROID)
  if (NOT DEFINED wxWidgets_USE_FILE)
    set(wxWidgets_USE_LIBS base core net xml html adv)
    set(BUILD_SHARED_LIBS TRUE)
    find_package(wxWidgets REQUIRED)
  endif (NOT DEFINED wxWidgets_USE_FILE)

  include(${wxWidgets_USE_FILE})
endif (QT_ANDROID)

find_package(Gettext REQUIRED)

# For convenience we define the sources as a variable. You can add header files
# and cpp/c files and CMake will sort them out

set(SRC_GRIB
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
    src/XyGribPanel.h
    src/XyGribPanel.cpp
    src/XyGribModelDef.h
    src/XyGribModelDef.cpp
    src/icons.h
    src/msg.h
    src/email.h
    src/email.cpp
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

if (OCPN_USE_GL)
set(SRC_GRIB ${SRC_GRIB}
  src/pi_TexFont.h
  src/pi_TexFont.cpp
)

endif ()
if (WIN32)
  set(SRC_GRIB ${SRC_GRIB} src/smapi.h)
  set(SRC_GRIB ${SRC_GRIB} src/smapi.cpp)
endif (WIN32)

#if (QT_ANDROID)
  set(SRC_GRIB ${SRC_GRIB} src/pi_shaders.h)
  set(SRC_GRIB ${SRC_GRIB} src/pi_shaders.cpp)
#endif (QT_ANDROID)

if (NOT UNIX)
  set(SRC_BZIP
      src/bzip2/bzlib.c
      src/bzip2/blocksort.c
      src/bzip2/compress.c
      src/bzip2/crctable.c
      src/bzip2/decompress.c
      src/bzip2/huffman.c
      src/bzip2/randtable.c
  )
  add_library(LIB_BZIP STATIC ${SRC_BZIP})

  include_directories(${CMAKE_SOURCE_DIR}/cache/buildwin/include)
  include_directories(${PLUGIN_SOURCE_DIR}/src/bzip2)
endif (NOT UNIX)

if (QT_ANDROID)
  set(SRC_BZIP_ANDROID
      src/bzip2/bzlib.c
      src/bzip2/blocksort.c
      src/bzip2/compress.c
      src/bzip2/crctable.c
      src/bzip2/decompress.c
      src/bzip2/huffman.c
      src/bzip2/randtable.c
  )
  set(SRC_GRIB ${SRC_GRIB} ${SRC_BZIP_ANDROID})

  include_directories(${CMAKE_SOURCE_DIR}/cache/buildwin/include)
  include_directories(${PLUGIN_SOURCE_DIR}/src/bzip2)

  message(STATUS ${SRC_GRIB})

endif (QT_ANDROID)

add_library(${PACKAGE_NAME} SHARED ${SRC_GRIB})

if (MSVC)
  # Fixme (leamas): Remove when the VS bug is fixed.
  # https://forum.juce.com/t/juceaide-fails-to-build-on-windows-plugin-demo-master-branch/48007
  target_compile_features(${PACKAGE_NAME} PRIVATE cxx_std_17)
endif ()

find_package(Jasper)
use_bundled_lib(USE_BUNDLED_JASPER jasper)
if (JASPER_FOUND AND NOT USE_BUNDLED_JASPER)
  message(STATUS "Using system jasper library...")
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${JASPER_LIBRARIES})
  target_include_directories(
    ${PACKAGE_NAME} BEFORE PRIVATE ${JASPER_INCLUDE_DIR}
  )
else ()
  message(STATUS "Using bundled jasper library...")
  add_subdirectory("libs/jasper")
  target_link_libraries(${PACKAGE_NAME} PRIVATE JASPER)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang")
  # FIXME: Need better handling after 5.0.0
  target_compile_options(${PACKAGE_NAME} PRIVATE -Wno-format)
endif ()

if (WIN32)
  if (MSVC)
    target_link_libraries(
      ${PACKAGE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/cache/buildwin/zlib1.lib"
                              "GLU_static"
    )
  else (MSVC)
    set(CMAKE_SHARED_LINKER_FLAGS "-L../../")
    target_link_libraries(
      ${PACKAGE_NAME}
      PRIVATE ${OPENGL_LIBRARIES}
              # "-L${wxWidgets_ROOT_DIR}/lib -lwxzlib-3.1-i686-w64-mingw32")
              # "-L${wxWidgets_ROOT_DIR}/lib -lwxzlib-2.8"
              "-lz"
    )
  endif (MSVC)

  target_link_libraries(${PACKAGE_NAME} PRIVATE LIB_BZIP)
  add_dependencies(${PACKAGE_NAME} ${PARENT})
endif (WIN32)

if (NOT QT_ANDROID)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${wxWidgets_LIBRARIES})
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${OCPN_Core_LIBRARIES})
  if (OCPN_USE_SVG)
    if (wxWidgets_VERSION_STRING VERSION_LESS "3.1.6")
      target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::wxsvg)
    endif()
  endif()
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::opencpn)
else (NOT QT_ANDROID)
  target_include_directories(
    ${PACKAGE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/android
  )
  target_link_libraries(${PACKAGE_NAME} PRIVATE "-lz")
  target_link_libraries(${PACKAGE_NAME} PRIVATE "-lGLESv1_CM")
  target_link_libraries(${PACKAGE_NAME} PRIVATE "-lGLESv2")
  target_link_libraries(
    ${PACKAGE_NAME} PRIVATE ${Qt_Base}/${Qt_Build}/lib/libQt5Gui.so
  )
  target_link_libraries(
    ${PACKAGE_NAME} PRIVATE ${Qt_Base}/${Qt_Build}/lib/libQt5Core.so
  )
  target_link_libraries(
    ${PACKAGE_NAME} PRIVATE ${Qt_Base}/${Qt_Build}/lib/libQt5Widgets.so
  )
  target_link_libraries(
    ${PACKAGE_NAME} PRIVATE ${Qt_Base}/${Qt_Build}/lib/libQt5AndroidExtras.so
  )
  target_link_libraries(
    ${PACKAGE_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../../libgorp.so
  )
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-soname,lib${PACKAGE_NAME}.so ")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-s")
endif (NOT QT_ANDROID)

target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::wxjson)
target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::opencpn)

if (NOT WIN32)
  target_link_libraries(${PACKAGE_NAME} PRIVATE "-lz")
endif ()

if (APPLE)
  find_package(ZLIB REQUIRED)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${ZLIB_LIBRARIES})
endif (APPLE)

if (NOT WIN32
    AND NOT APPLE
    AND NOT QT_ANDROID
)
  include(OcpnFindGtk)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::gtk)
  find_package(PANGO REQUIRED)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::pango)
endif ()

if (UNIX
    AND NOT APPLE
    AND NOT QT_ANDROID
)
  find_package(BZip2 REQUIRED)
  include_directories(${BZIP2_INCLUDE_DIR})
  find_package(ZLIB REQUIRED)
  include_directories(${ZLIB_INCLUDE_DIR})

  target_link_libraries(
    ${PACKAGE_NAME} PRIVATE ${BZIP2_LIBRARIES} ${ZLIB_LIBRARY}
  )
endif (
  UNIX
  AND NOT APPLE
  AND NOT QT_ANDROID
)

target_link_libraries(${PACKAGE_NAME} PRIVATE ocpn::opencpn)

if (APPLE)
  target_compile_options(
    ${PACKAGE_NAME}
    PRIVATE "-Wall"
            "-Wno-unused"
            "-fexceptions"
            "-Wno-overloaded-virtual"
            "-Wno-deprecated"
            "-Wno-deprecated-declarations"
            "-D_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_"
  )
endif (APPLE)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang|GNU")
  target_compile_options(${PACKAGE_NAME} PRIVATE "-Wno-unknown-pragmas")
endif ()

if (APPLE)
  install(
    TARGETS ${PACKAGE_NAME}
    RUNTIME
    LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/OpenCPN.app/Contents/PlugIns
  )

  if (EXISTS ${PROJECT_SOURCE_DIR}/data)
    install(
      DIRECTORY data
      DESTINATION
        ${CMAKE_BINARY_DIR}/OpenCPN.app/Contents/SharedSupport/plugins/${PACKAGE_NAME}
    )
  endif (EXISTS ${PROJECT_SOURCE_DIR}/data)
endif (APPLE)

if (UNIX AND NOT APPLE)
  if (NOT DEFINED PREFIX_PLUGINS)
    set(PREFIX_PLUGINS /usr/local/lib/opencpn)
  endif (NOT DEFINED PREFIX_PLUGINS)

  install(
    TARGETS ${PACKAGE_NAME}
    RUNTIME
    LIBRARY DESTINATION ${PREFIX_PLUGINS}
  )

  if (EXISTS ${PROJECT_SOURCE_DIR}/data)
    set(PREFIX_PARENTDATA ${PREFIX_DATA}/${PARENT})
    install(DIRECTORY data
            DESTINATION ${PREFIX_PARENTDATA}/plugins/${PACKAGE_NAME}
    )
  endif (EXISTS ${PROJECT_SOURCE_DIR}/data)

endif (UNIX AND NOT APPLE)

if (WIN32)
  install(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION "plugins")
  if (NOT OCPN_CI_BUILD)
    INSTALL(FILES $<TARGET_PDB_FILE:${PACKAGE_NAME}> DESTINATION "plugins")
  endif (NOT OCPN_CI_BUILD)
  set(INSTALL_DIRECTORY "plugins/${PACKAGE_NAME}")

  if (EXISTS ${PROJECT_SOURCE_DIR}/data)
    install(DIRECTORY data DESTINATION "${INSTALL_DIRECTORY}")
  endif (EXISTS ${PROJECT_SOURCE_DIR}/data)

endif (WIN32)

# Copy shared library to execution folder for debugging if not CI build
if (NOT OCPN_CI_BUILD)
  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG /OPT:REF /OPT:ICF")
    add_custom_command(
      TARGET ${PACKAGE_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "Copying dll file"
      COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_FILE:${PACKAGE_NAME}>"
                "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/plugins/${PACKAGE_NAME}.dll"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${PACKAGE_NAME}>"
                "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/plugins/${PACKAGE_NAME}.dll"
    )
    add_custom_command(
      TARGET ${PACKAGE_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "Copying pdb file"
      COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_PDB_FILE:${PACKAGE_NAME}>"
                "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/plugins/${PACKAGE_NAME}.pdb"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_PDB_FILE:${PACKAGE_NAME}>"
                "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/plugins/${PACKAGE_NAME}.pdb"
    )
  endif (MSVC)
endif (NOT OCPN_CI_BUILD)

include("../PluginLocalization.cmake")
plugin_localization()
