# ---------------------------------------------------------------------------
# Author:      Pavel Kalian Copyright:   2014 License:     GPLv2 License
# ---------------------------------------------------------------------------

# define minimum cmake version
cmake_minimum_required(VERSION 3.5.0)
cmake_policy(SET CMP0043 NEW)

project(chartdldr_pi)

set(PACKAGE_NAME chartdldr_pi)
set(VERBOSE_NAME ChartDownloader)
set(TITLE_NAME ChartDownloader)
set(CPACK_PACKAGE_CONTACT "Pavel Kalian")

set(VERSION_MAJOR "1")
set(VERSION_MINOR "4")
set(VERSION_PATCH "0")
set(VERSION_DATE "2019-12-09")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE
        STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE
  )
endif (NOT CMAKE_BUILD_TYPE)

include_directories(${CMAKE_SOURCE_DIR}/include)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# SET(CMAKE_BUILD_TYPE Debug)
set(CMAKE_MACOSX_RPATH "ON")

include("cmake/PluginConfigure.cmake")

set(SRC_CHARTDLDR
    src/chartdldr_pi.h
    src/chartdldr_pi.cpp
    src/icons.h
    src/icons.cpp
    src/chartdldrgui.h
    src/chartdldrgui.cpp
    # src/checkedlistctrl.cpp src/checkedlistctrl.h
    src/base64.h
    src/chartcatalog.cpp
    src/chartcatalog.h
    src/pugiconfig.hpp
    src/pugixml.hpp
    src/pugixml.cpp
)

if (QT_ANDROID)
  set(SRC_CHARTDLDR ${SRC_CHARTDLDR} src/androidSupport.h)
  set(SRC_CHARTDLDR ${SRC_CHARTDLDR} src/androidSupport.cpp)
endif (QT_ANDROID)

set(SRC_UNARR
    src/unarr/common/conv.c
    src/unarr/common/crc32.c
    src/unarr/common/custalloc.c
    src/unarr/common/allocator.h
    src/unarr/common/unarr.c
    src/unarr/common/unarr-imp.h
    src/unarr/common/stream.c
    src/unarr/_7z/_7z.h
    src/unarr/_7z/_7z.c
    src/unarr/lzmasdk/Ppmd7Dec.c
    src/unarr/lzmasdk/Ppmd8.h
    src/unarr/lzmasdk/7zTypes.h
    src/unarr/lzmasdk/Precomp.h
    src/unarr/lzmasdk/CpuArch.h
    src/unarr/lzmasdk/Ppmd7.c
    src/unarr/lzmasdk/LzmaDec.c
    src/unarr/lzmasdk/Ppmd8.c
    src/unarr/lzmasdk/Ppmd.h
    src/unarr/lzmasdk/Ppmd7.h
    src/unarr/lzmasdk/CpuArch.c
    src/unarr/lzmasdk/LzmaDec.h
    src/unarr/lzmasdk/Ppmd8Dec.c
    src/unarr/rar/filter-rar.c
    src/unarr/rar/huffman-rar.c
    src/unarr/rar/parse-rar.c
    src/unarr/rar/rar.h
    src/unarr/rar/rarvm.c
    src/unarr/rar/uncompress-rar.c
    src/unarr/rar/rarvm.h
    src/unarr/rar/lzss.h
    src/unarr/rar/rar.c
    src/unarr/unarr.h
    src/unarr/tar/tar.h
    src/unarr/tar/parse-tar.c
    src/unarr/tar/tar.c
    src/unarr/zip/uncompress-zip.c
    src/unarr/zip/inflate.c
    src/unarr/zip/zip.c
    src/unarr/zip/parse-zip.c
    src/unarr/zip/zip.h
    src/unarr/zip/inflate.h
)

if (APPLE)
  # Homebrew does not link libarchive to /usr/local by default to avoid conflict
  # with the system-wide one (which in turn does not provide includes...) We do
  # not want the systemwide vrsion as it is too old and misses support for many
  # archive formats
  #list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/libarchive")
  #list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/libarchive/lib")
endif ()

if (CMAKE_HOST_WIN32)
  list(APPEND CMAKE_PREFIX_PATH "../../cache/buildwin/include")
  list(APPEND CMAKE_PREFIX_PATH "../../cache/buildwin")
endif ()

option(OCPN_USE_CHART_LIST "Use wxDATAVIEWLISTCTRL for chart downloader" OFF)
if (MSVC)
  set(OCPN_USE_CHART_LIST ON)
endif (MSVC)

if (wxWidgets_VERSION_STRING GREATER 2.9)
  if (OCPN_USE_CHART_LIST)
    add_definitions(-DCHART_LIST)
    message(
      STATUS "chartdldr_pi: Building with wxDATAVIEWLISTCTRL chart listing"
    )
  else (wxWidgets_VERSION_STRING GREATER 2.9)
    message(
      STATUS "chartdldr_pi: Building without wxDATAVIEWLISTCTRL chart listing"
    )
  endif (OCPN_USE_CHART_LIST)
endif (wxWidgets_VERSION_STRING GREATER 2.9)

option(CHARTDLDR_USE_UNARR
       "Use unarr to extract RAR archives instead of libarchive" ON
)
if (CHARTDLDR_USE_UNARR)
  add_definitions(-DCHARTDLDR_RAR_UNARR)
endif ()

add_definitions(-D_FILE_OFFSET_BITS=64)

include_directories(src/unarr/)
pkg_search_module(UNARR libunarr unarr)
use_bundled_lib(USE_BUNDLED_UNARR unarr)
if (UNARR_FOUND AND NOT USE_BUNDLED_UNARR)
  message(STATUS "chartdldr_pi: Building with system unarr")
  include_directories(${UNARR_INCLUDE_DIR})
  set(SRC_UNARR "")
else ()
  message(STATUS "chartdldr_pi: Building with bundled unarr")
  add_library(UNARR ${SRC_UNARR})
  include_directories(src/unarr)
endif ()

add_library(${PACKAGE_NAME} SHARED ${SRC_UNARR} ${SRC_CHARTDLDR})

find_package(OcpnLibarchive)
if (NOT TARGET ocpn::libarchive)
  message(FATAL:_ERROR "No libarchive!!!")
endif ()
target_link_libraries(${PACKAGE_NAME} PUBLIC ocpn::libarchive)
if (APPLE AND OCPN_USE_SYSTEM_LIBARCHIVE)
  # If we use system libarchive, we do not bundle the dylib and need to
  # comment it out from the pkg project file
  set(LIBARCHIVE_COMMENT_START "<!--")
  set(LIBARCHIVE_COMMENT_END "-->")
endif ()

if (LibArchive_FOUND)
  message(STATUS "chartdldr_pi: going to use LibArchive...")
  message(STATUS "chartdldr_pi: " ${LibArchive_INCLUDE_DIRS})
  message(STATUS "chartdldr_pi: " ${LibArchive_LIBRARIES})
  include_directories(${LibArchive_INCLUDE_DIRS})
  # Unset the unrar sources as we will not need them
  add_definitions(-DDLDR_USE_LIBARCHIVE)
endif (LibArchive_FOUND)

if (QT_ANDROID)
  add_definitions(-D__STDC_LIMIT_MACROS)
  add_definitions(-D__STDC_CONSTANT_MACROS)
  add_definitions(
    "-Wno-inconsistent-missing-override -Wno-potentially-evaluated-expression"
  )
endif (QT_ANDROID)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang")
  # FIXME: This is to broad, needs better handling after 5.0.0
  target_compile_options(${PACKAGE_NAME} PRIVATE -Wno-overloaded-virtual)
  target_compile_options(${PACKAGE_NAME} PRIVATE -Wno-unknown-pragmas)
  target_compile_options(${PACKAGE_NAME} PRIVATE -Wno-unused-variable)
endif ()

if (UNARR_LIBRARIES AND NOT USE_BUNDLED_UNARR)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${UNARR_LIBRARIES})
endif ()

if (LibArchive_FOUND)
  target_link_libraries(${PACKAGE_NAME} PRIVATE ${LibArchive_LIBRARIES})
else (LibArchive_FOUND)
  # We do not have libarchive, try to make unarr more powerful...
  # ADD_DEFINITIONS(-DHAVE_7Z) #TODO if we decide it is needed, quite a bit more
  # work here
  find_package(ZLIB)
  if (ZLIB_FOUND)
    add_definitions(-DHAVE_ZLIB)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(${PACKAGE_NAME} PRIVATE ${ZLIB_LIBRARIES})
  endif (ZLIB_FOUND)
  find_package(BZip2)
  if (BZIP2_FOUND)
    add_definitions(-DHAVE_BZIP2)
    include_directories(${BZIP2_INCLUDE_DIRS})
    target_link_libraries(${PACKAGE_NAME} PRIVATE ${BZIP2_LIBRARIES})
  endif (BZIP2_FOUND)
endif (LibArchive_FOUND)

# TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} PRIVATE wxjson::wxjson )

target_link_libraries( ${PACKAGE_NAME} PRIVATE ocpn::opencpn)

if (NOT QT_ANDROID)
  target_link_libraries(
    ${PACKAGE_NAME} PRIVATE ${wxWidgets_LIBRARIES} ocpn::wxjson ocpn::opencpn
  )
else (NOT QT_ANDROID)
  target_link_libraries(
    ${PACKAGE_NAME}
    PRIVATE ocpn::wxjson
            ${Qt_Base}/${Qt_Build}/lib/libQt5Gui.so
            ${Qt_Base}/${Qt_Build}/lib/libQt5Core.so
            ${Qt_Base}/${Qt_Build}/lib/libQt5Widgets.so
            ${Qt_Base}/${Qt_Build}/lib/libQt5AndroidExtras.so
            ${CMAKE_CURRENT_BINARY_DIR}/../../libgorp.so
            "-lz"
  )

  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-soname,lib${PACKAGE_NAME}.so ")
  # SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-s")
endif (NOT QT_ANDROID)

if (APPLE)
  target_compile_options(
    ${PACKAGE_NAME}
    PRIVATE "-Wall"
            "-Wno-unused"
            "-fexceptions"
            "-Wno-overloaded-virtual"
            "-Wno-deprecated"
            "-Wno-deprecated-declarations"
            "-Wno-unknown-pragmas"
            "-D_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_"
  )
endif (APPLE)

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

include("cmake/PluginInstall.cmake")
include("cmake/PluginLocalization.cmake")
include("cmake/PluginPackage.cmake")
