##---------------------------------------------------------------------------
## Author:      Pavel Kalian
## Copyright:   2014
## License:     GPLv2 License
##---------------------------------------------------------------------------

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(grib_pi)

SET(PACKAGE_NAME grib_pi)
SET(VERBOSE_NAME GRIB)
SET(TITLE_NAME GRIB)
SET(CPACK_PACKAGE_CONTACT "dsr")

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "2")

#SET(CMAKE_BUILD_TYPE Debug)


INCLUDE("cmake/PluginConfigure.cmake")

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


ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_GRIB})

TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${wxWidgets_LIBRARIES} )


IF (QT_ANDROID )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OCPN_Core_LIBRARIES} )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} "-lgnustl_shared")
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} "-lz" )
ENDIF(QT_ANDROID)


INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")

