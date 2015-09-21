##---------------------------------------------------------------------------
## Author:      Pavel Kalian
## Copyright:   2014
## License:     GPLv2 License
##---------------------------------------------------------------------------

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(chartdldr_pi)

SET(PACKAGE_NAME chartdldr_pi)
SET(VERBOSE_NAME ChartDownloader)
SET(TITLE_NAME ChartDownloader)
SET(CPACK_PACKAGE_CONTACT "Pavel Kalian")

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "2")

#SET(CMAKE_BUILD_TYPE Debug)

# Definitions
add_definitions (
-D_FILE_OFFSET_BITS=64
-D_LARGEFILE_SOURCE
-DLIBUNRAR_BUILD
-DRARDLL
-DUNRAR
-DSILENT
)

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

            src/unrar/rar.cpp
            src/unrar/strlist.cpp
            src/unrar/strfn.cpp
            src/unrar/pathfn.cpp
            src/unrar/smallfn.cpp
            src/unrar/global.cpp
            src/unrar/file.cpp
            src/unrar/filefn.cpp
            src/unrar/filcreat.cpp
            src/unrar/archive.cpp
            src/unrar/arcread.cpp
            src/unrar/unicode.cpp
            src/unrar/system.cpp
            src/unrar/isnt.cpp
            src/unrar/crypt.cpp
            src/unrar/crc.cpp
            src/unrar/rawread.cpp
            src/unrar/encname.cpp
            src/unrar/resource.cpp
            src/unrar/match.cpp
            src/unrar/timefn.cpp
            src/unrar/rdwrfn.cpp
            src/unrar/consio.cpp
            src/unrar/options.cpp
            src/unrar/errhnd.cpp
            src/unrar/rarvm.cpp
            src/unrar/secpassword.cpp
            src/unrar/rijndael.cpp
            src/unrar/getbits.cpp
            src/unrar/sha1.cpp
            src/unrar/sha256.cpp
            src/unrar/blake2s.cpp
            src/unrar/hash.cpp
            src/unrar/extinfo.cpp
            src/unrar/extract.cpp
            src/unrar/volume.cpp
            src/unrar/list.cpp
            src/unrar/find.cpp
            src/unrar/unpack.cpp
            src/unrar/headers.cpp
            src/unrar/threadpool.cpp
            src/unrar/rs16.cpp
            src/unrar/cmddata.cpp
            src/unrar/ui.cpp
            src/unrar/filestr.cpp
            src/unrar/scantree.cpp
            #src/unrar/dll.cpp
            src/unrar/qopen.cpp
        )




IF(UNIX)
 INCLUDE("cmake/FindTinyXML.cmake")
 FIND_PACKAGE(TinyXML QUIET)
ENDIF(UNIX)

INCLUDE_DIRECTORIES(src/unrar/)

IF(TINYXML_FOUND)
 message (STATUS "Chartdldr_pi building with system tinyxml")
 INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
 ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_CHARTDLDR})
 TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${TINYXML_LIBRARIES})
ELSE(TINYXML_FOUND)
 message (STATUS "Chartdldr_pi building with embedded tinyxml")
 INCLUDE_DIRECTORIES(src/tinyxml/)
 SET(SRC_LTINYXML
            src/tinyxml/tinyxml.cpp
            src/tinyxml/tinyxmlparser.cpp
            src/tinyxml/tinyxmlerror.cpp
 )
 ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_CHARTDLDR} ${SRC_LTINYXML})
ENDIF(TINYXML_FOUND)
ADD_DEFINITIONS(-DTIXML_USE_STL)

TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${wxWidgets_LIBRARIES} )

IF (QT_ANDROID )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OCPN_Core_LIBRARIES} )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} "-lgnustl_shared")
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} "-lz" )
ENDIF(QT_ANDROID)


INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")
