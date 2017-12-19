##---------------------------------------------------------------------------
## Author:      Andras Andras
## Copyright:   
## License:     wxWidgets License ??
##---------------------------------------------------------------------------
 
# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(aa1chartnavhelper_pi)

SET(PACKAGE_NAME aa1chartnavhelper_pi)
SET(VERBOSE_NAME Aa1ChartNavHelper)
SET(TITLE_NAME AA1ChartNavHelper)
SET(CPACK_PACKAGE_CONTACT "Andras Andras")

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "2")

#SET(CMAKE_BUILD_TYPE Debug)

INCLUDE("cmake/PluginConfigure.cmake")

SET(SRC_AA1ChartNavHelper
            src/aa1chartnavhelper_pi.h
            src/aa1chartnavhelper_pi.cpp
            src/icons.h
            src/icons.cpp
	    	src/Aa1ChartNavHelperUIDialog.h
	    	src/Aa1ChartNavHelperUIDialog.cpp
            src/TexFont.cpp
            src/TexFont.h
	)

IF(WIN32)
SET(SRC_JSON
	    src/json_defs.h
	    src/jsonreader.h
	    src/jsonval.h
	    src/jsonwriter.h
	    src/jsonreader.cpp
	    src/jsonval.cpp
	    src/jsonwriter.cpp
        )
ENDIF(WIN32)

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_AA1ChartNavHelper} ${SRC_JSON})

TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${wxWidgets_LIBRARIES} )

IF (QT_ANDROID )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OCPN_Core_LIBRARIES} )
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} "-lgnustl_shared")
ENDIF(QT_ANDROID)

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")
