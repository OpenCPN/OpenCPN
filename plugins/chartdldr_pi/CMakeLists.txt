##---------------------------------------------------------------------------
## Author:      Pavel Kalian
## Copyright:   2014
## License:     GPLv2 License
##---------------------------------------------------------------------------

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)
IF (COMMAND cmake_policy)
  if (POLICY CMP0043)
    CMAKE_POLICY(SET CMP0043 NEW)
  endif (POLICY CMP0043)
ENDIF (COMMAND cmake_policy)

PROJECT(chartdldr_pi)

SET(PACKAGE_NAME chartdldr_pi)
SET(VERBOSE_NAME ChartDownloader)
SET(TITLE_NAME ChartDownloader)
SET(CPACK_PACKAGE_CONTACT "Pavel Kalian")

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "3")
SET(VERSION_PATCH "0")
SET(VERSION_DATE "2018-01-13")

IF( NOT CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE Release CACHE STRING
       "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
       FORCE )
ENDIF( NOT CMAKE_BUILD_TYPE )

MESSAGE (STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_MACOSX_RPATH "ON")

INCLUDE("cmake/PluginConfigure.cmake")

SET(SRC_CHARTDLDR
            src/chartdldr_pi.h
            src/chartdldr_pi.cpp
            src/icons.h
            src/icons.cpp
            src/chartdldrgui.h
            src/chartdldrgui.cpp
            src/checkedlistctrl.cpp
            src/checkedlistctrl.h
            src/base64.h
            src/chartcatalog.cpp
            src/chartcatalog.h
)
SET(SRC_UNARR
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

IF(UNIX AND NOT APPLE)
 INCLUDE("cmake/FindTinyXML.cmake")
 FIND_PACKAGE(TinyXML QUIET)
ENDIF(UNIX AND NOT APPLE)

IF(APPLE)
  #Homebrew does not link libarchive to /usr/local by default to avoid conflict with the system-wide one (which in turn does not provide includes...)
  #We do not want the systemwide vrsion as it is too old and misses support for many archive formats
  LIST(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/libarchive")
  LIST(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/libarchive/lib")
ENDIF(APPLE)
IF(WIN32)
  LIST(APPEND CMAKE_PREFIX_PATH "../../buildwin/include")
  LIST(APPEND CMAKE_PREFIX_PATH "../../buildwin")
ENDIF(WIN32)

OPTION(CHARTDLDR_USE_UNARR "Use unarr to extract RAR archives instead of libarchive" ON)
IF(CHARTDLDR_USE_UNARR)
    ADD_DEFINITIONS(-DCHARTDLDR_RAR_UNARR)
ENDIF(CHARTDLDR_USE_UNARR)

ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)

FIND_PACKAGE(LibArchive)
IF(LibArchive_FOUND)
  message(STATUS "Chartdldr_pi going to use LibArchive...")
  INCLUDE_DIRECTORIES(${LibArchive_INCLUDE_DIRS})
  #Unset the unrar sources as we will not need them
  ADD_DEFINITIONS(-DDLDR_USE_LIBARCHIVE)
ENDIF(LibArchive_FOUND)

INCLUDE_DIRECTORIES(src/unarr/)

IF(TINYXML_FOUND)
 message (STATUS "Chartdldr_pi building with system tinyxml")
 INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
 ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_UNARR} ${SRC_CHARTDLDR})
 TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${TINYXML_LIBRARIES})
ELSE(TINYXML_FOUND)
 message (STATUS "Chartdldr_pi building with embedded tinyxml")
 INCLUDE_DIRECTORIES(src/tinyxml/)
 SET(SRC_LTINYXML
            src/tinyxml/tinyxml.cpp
            src/tinyxml/tinyxmlparser.cpp
            src/tinyxml/tinyxmlerror.cpp
 )
 ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_UNARR} ${SRC_LTINYXML} ${SRC_CHARTDLDR})
ENDIF(TINYXML_FOUND)
ADD_DEFINITIONS(-DTIXML_USE_STL)

IF(LibArchive_FOUND)
    TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${LibArchive_LIBRARIES})
ELSE(LibArchive_FOUND)
    #We do not have libarchive, try to make unarr more powerful...
    #ADD_DEFINITIONS(-DHAVE_7Z) #TODO if we decide it is needed, quite a bit more work here
    FIND_PACKAGE(ZLIB)
    IF(ZLIB_FOUND)
        ADD_DEFINITIONS(-DHAVE_ZLIB)
        INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
        TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${ZLIB_LIBRARIES})
    ENDIF(ZLIB_FOUND)
    FIND_PACKAGE(BZip2)
    IF(BZIP2_FOUND)
        ADD_DEFINITIONS(-DHAVE_BZIP2)
        INCLUDE_DIRECTORIES(${BZIP2_INCLUDE_DIRS})
        TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${BZIP2_LIBRARIES})
    ENDIF(BZIP2_FOUND)
ENDIF(LibArchive_FOUND)

TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${wxWidgets_LIBRARIES} )

IF (QT_ANDROID )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OCPN_Core_LIBRARIES} )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} "-lgnustl_shared")
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} "-lz" )
ENDIF(QT_ANDROID)


INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")
